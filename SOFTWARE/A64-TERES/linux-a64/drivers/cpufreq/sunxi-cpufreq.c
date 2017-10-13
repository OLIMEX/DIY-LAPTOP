/*
 * drivers/cpufreq/sunxi-cpufreq.c
 *
 * Copyright (c) 2014 softwinner.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/cpufreq.h>
#include <linux/cpu.h>
#include <linux/of.h>
#include <linux/clk.h>
#include <linux/debugfs.h>
#include <linux/err.h>
#include <linux/cpu.h>
#ifdef CONFIG_SUNXI_ARISC
#include <linux/arisc/arisc.h>
#endif
#include <linux/clk/sunxi.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/sys_config.h>

enum {
	DEBUG_NONE = 0,
	DEBUG_FREQ = 1,
};
static int debug_mask = DEBUG_NONE;
module_param(debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);
#define CPUFREQ_DBG(mask,format,args...) \
	do { if (mask & debug_mask) printk("[cpufreq] "format,##args); } while (0)
#define CPUFREQ_ERR(format,args...) \
	printk(KERN_ERR "[cpufreq] error:"format,##args)

#define SUNXI_CPUFREQ_MAX      (1536000000)
#define SUNXI_CPUFREQ_MIN       (480000000)
#define PLL_CPU_CLK               "pll_cpu"
#define CPU_CLK                       "cpu"
#define CPU_VDD                  "vdd-cpua"

/* sunxi CPUFreq driver data structure */
static struct {
	struct clk *clk_pll;
	struct clk *clk_cpu;
	struct regulator *vdd_cpu;

	struct cpufreq_frequency_table *freq_table;

#ifdef CONFIG_DEBUG_FS
	s64 cpufreq_set_us;
	s64 cpufreq_get_us;
#endif

	u32 transition_latency;
	u32 max_freq;
	u32 min_freq;
	u32 ext_freq;
	u32 boot_freq;
	u32 last_freq;
#ifdef CONFIG_CPU_VOLTAGE_SCALING
	u32 last_vdd;
#endif

	struct mutex lock;
} sunxi_cpufreq;

unsigned int sunxi_boot_lock = 0;

#ifdef CONFIG_CPU_VOLTAGE_SCALING
#define TABLE_LENGTH (8)
struct cpufreq_dvfs {
	unsigned int freq;   /* cpu frequency */
	unsigned int volt;   /* voltage for the frequency */
};
static struct cpufreq_dvfs dvfs_table_syscfg[TABLE_LENGTH];
static unsigned int table_length_syscfg = 0;

static void __vftable_show(void)
{
	int i;

	pr_debug("-------------------V-F Table-------------------\n");
	for (i = 0; i < table_length_syscfg; i++){
		pr_debug("\tvoltage = %4dmv \tfrequency = %4dKHz\n",
			dvfs_table_syscfg[i].volt, dvfs_table_syscfg[i].freq / 1000);
	}
	pr_debug("-----------------------------------------------\n");
}

static unsigned int __get_vdd_value(unsigned int freq)
{
	struct cpufreq_dvfs *dvfs_inf = NULL;

	dvfs_inf = &dvfs_table_syscfg[0];
	while ((dvfs_inf+1)->freq >= freq) {
		dvfs_inf++;
	}

	return dvfs_inf->volt;
}
#endif

/*
 * check if the cpu frequency policy is valid;
 */
static int sunxi_cpufreq_verify(struct cpufreq_policy *policy)
{
	return cpufreq_frequency_table_verify(policy, sunxi_cpufreq.freq_table);
}

/*
 * get the current cpu vdd;
 * return: cpu vdd, based on mv;
 */
int sunxi_cpufreq_getvolt(void)
{
	return regulator_get_voltage(sunxi_cpufreq.vdd_cpu) / 1000;
}

/*
 * get the frequency that cpu currently is running;
 * cpu:    cpu number, all cpus use the same clock;
 * return: cpu frequency, based on khz;
 */
static unsigned int sunxi_cpufreq_get(unsigned int cpu)
{
	unsigned int current_freq = 0;
#ifdef CONFIG_DEBUG_FS
	ktime_t calltime = ktime_get();
#endif

	clk_get_rate(sunxi_cpufreq.clk_pll);
	current_freq = clk_get_rate(sunxi_cpufreq.clk_cpu) / 1000;

#ifdef CONFIG_DEBUG_FS
	sunxi_cpufreq.cpufreq_get_us = ktime_to_us(ktime_sub(ktime_get(), calltime));
#endif

	return current_freq;
}

#ifndef CONFIG_SUNXI_ARISC
static int __set_cpufreq_by_ccu(unsigned int freq)
{
	if (clk_prepare_enable(sunxi_cpufreq.clk_pll)) {
		CPUFREQ_ERR("try to enable clk_pll failed!\n");
		goto err;
	}

	if (clk_set_rate(sunxi_cpufreq.clk_pll, freq * 1000)) {
		CPUFREQ_ERR("try to set clk_pll rate to %u failed!\n", freq);
		goto err;
	}

	return 0;

err:
	return -EINVAL;
}
#endif

/*
 * adjust the frequency that cpu is currently running;
 * policy:   cpu frequency policy;
 * freq:     target frequency to be set, based on khz;
 * relation: method for selecting the target requency;
 * return:   return 0 if set target frequency successed, else, return -EINVAL;
 * notes:    this function is called by the cpufreq core;
 */
static int sunxi_cpufreq_target(struct cpufreq_policy *policy,
					__u32 freq, __u32 relation)
{
	int ret = 0;
	unsigned int            index;
	struct cpufreq_freqs    freqs;
#ifdef CONFIG_DEBUG_FS
	ktime_t calltime;
#endif
#ifdef CONFIG_SMP
	int i;
#endif
#ifdef CONFIG_CPU_VOLTAGE_SCALING
unsigned int new_vdd;
#endif

	mutex_lock(&sunxi_cpufreq.lock);

	/* avoid repeated calls which cause a needless amout of duplicated
	 * logging output (and CPU time as the calculation process is
	 * done) */
	if (freq == sunxi_cpufreq.last_freq)
		goto out;

	CPUFREQ_DBG(DEBUG_FREQ, "request frequency is %uKHz\n", freq);

	if (unlikely(sunxi_boot_lock))
		freq = freq > sunxi_cpufreq.boot_freq ? sunxi_cpufreq.boot_freq : freq;

	/* try to look for a valid frequency value from cpu frequency table */
	if (cpufreq_frequency_table_target(policy, sunxi_cpufreq.freq_table,
					freq, relation, &index)) {
		CPUFREQ_ERR("try to look for %uKHz failed!\n", freq);
		ret = -EINVAL;
		goto out;
	}

	/* frequency is same as the value last set, need not adjust */
	if (sunxi_cpufreq.freq_table[index].frequency == sunxi_cpufreq.last_freq)
		goto out;

	freq = sunxi_cpufreq.freq_table[index].frequency;

	CPUFREQ_DBG(DEBUG_FREQ, "target is find: %uKHz, entry %u\n", freq, index);

	/* notify that cpu clock will be adjust if needed */
	if (policy) {
		freqs.cpu = policy->cpu;
		freqs.old = sunxi_cpufreq.last_freq;
		freqs.new = freq;

#ifdef CONFIG_SMP
		/* notifiers */
		for_each_cpu(i, policy->cpus) {
			freqs.cpu = i;
			cpufreq_notify_transition(policy, &freqs, CPUFREQ_PRECHANGE);
		}
#else
		cpufreq_notify_transition(policy, &freqs, CPUFREQ_PRECHANGE);
#endif
	}

#ifdef CONFIG_CPU_VOLTAGE_SCALING
	/* get vdd value for new frequency */
	new_vdd = __get_vdd_value(freq * 1000);
	CPUFREQ_DBG(DEBUG_FREQ, "set cpu vdd to %dmv\n", new_vdd);
	if (sunxi_cpufreq.vdd_cpu && (new_vdd > sunxi_cpufreq.last_vdd)) {
		CPUFREQ_DBG(DEBUG_FREQ, "set cpu vdd to %dmv\n", new_vdd);
		if (regulator_set_voltage(sunxi_cpufreq.vdd_cpu, new_vdd*1000, new_vdd*1000)) {
			CPUFREQ_ERR("try to set cpu vdd failed!\n");

			/* notify everyone that clock transition finish */
			if (policy) {
				freqs.cpu = policy->cpu;;
				freqs.old = freqs.new;
				freqs.new = sunxi_cpufreq.last_freq;
#ifdef CONFIG_SMP
				/* notifiers */
				for_each_cpu(i, policy->cpus) {
					freqs.cpu = i;
					cpufreq_notify_transition(policy, &freqs, CPUFREQ_POSTCHANGE);
				}
#else
				cpufreq_notify_transition(policy, &freqs, CPUFREQ_POSTCHANGE);
#endif
			}
			return -EINVAL;
		}
	}
#endif

#ifdef CONFIG_DEBUG_FS
	calltime = ktime_get();
#endif

	/* try to set cpu frequency */
#ifndef CONFIG_SUNXI_ARISC
	if (__set_cpufreq_by_ccu(freq))
#else
	if (arisc_dvfs_set_cpufreq(freq, ARISC_DVFS_PLL1, ARISC_DVFS_SYN, NULL, NULL))
#endif
	{
		CPUFREQ_ERR("set cpu frequency to %uKHz failed!\n", freq);

#ifdef CONFIG_CPU_VOLTAGE_SCALING
		if (sunxi_cpufreq.vdd_cpu && (new_vdd > sunxi_cpufreq.last_vdd)) {
			if (regulator_set_voltage(sunxi_cpufreq.vdd_cpu,
					sunxi_cpufreq.last_vdd*1000, sunxi_cpufreq.last_vdd*1000)) {
				CPUFREQ_ERR("try to set voltage failed!\n");
				sunxi_cpufreq.last_vdd = new_vdd;
			}
		}
#endif

		/* set cpu frequency failed */
		if (policy) {
			freqs.cpu = policy->cpu;
			freqs.old = freqs.new;
			freqs.new = sunxi_cpufreq.last_freq;

#ifdef CONFIG_SMP
			/* notifiers */
			for_each_cpu(i, policy->cpus) {
				freqs.cpu = i;
				cpufreq_notify_transition(policy, &freqs, CPUFREQ_POSTCHANGE);
			}
#else
			cpufreq_notify_transition(policy, &freqs, CPUFREQ_POSTCHANGE);
#endif
		}

		ret = -EINVAL;
		goto out;
	}

#ifdef CONFIG_DEBUG_FS
	sunxi_cpufreq.cpufreq_set_us = ktime_to_us(ktime_sub(ktime_get(), calltime));
#endif

#ifdef CONFIG_CPU_VOLTAGE_SCALING
	if(sunxi_cpufreq.vdd_cpu && (new_vdd < sunxi_cpufreq.last_vdd)) {
		CPUFREQ_DBG(DEBUG_FREQ, "set cpu vdd to %dmv\n", new_vdd);
		if(regulator_set_voltage(sunxi_cpufreq.vdd_cpu, new_vdd*1000, new_vdd*1000)) {
			CPUFREQ_ERR("try to set voltage failed!\n");
			new_vdd = sunxi_cpufreq.last_vdd;
		}
	}

	sunxi_cpufreq.last_vdd = new_vdd;
#endif

	/* notify that cpu clock will be adjust if needed */
	if (policy) {
#ifdef CONFIG_SMP
		for_each_cpu(i, policy->cpus) {
			freqs.cpu = i;
			cpufreq_notify_transition(policy, &freqs, CPUFREQ_POSTCHANGE);
		}
#else
		cpufreq_notify_transition(policy, &freqs, CPUFREQ_POSTCHANGE);
#endif
	}

	sunxi_cpufreq.last_freq = freq;

	CPUFREQ_DBG(DEBUG_FREQ, "DVFS done! Freq[%uMHz] Volt[%umv] ok\n", \
			sunxi_cpufreq_get(0) / 1000, sunxi_cpufreq_getvolt());

out:
	mutex_unlock(&sunxi_cpufreq.lock);

	return ret;
}

/*
 * cpu frequency initialise a policy;
 * policy:  cpu frequency policy;
 * result:  return 0 if init ok, else, return -EINVAL;
 */
static int sunxi_cpufreq_init(struct cpufreq_policy *policy)
{
	int ret;

	ret = cpufreq_frequency_table_cpuinfo(policy, sunxi_cpufreq.freq_table);
	if (ret) {
		CPUFREQ_ERR("init cpuinfo failed\n");
		return ret;
	}

	cpufreq_frequency_table_get_attr(sunxi_cpufreq.freq_table, policy->cpu);
	policy->cpuinfo.transition_latency = sunxi_cpufreq.transition_latency;
	policy->cpuinfo.boot_freq = sunxi_cpufreq.boot_freq;
	policy->cur = sunxi_cpufreq_get(0);
	policy->governor = CPUFREQ_DEFAULT_GOVERNOR;

	if (policy->min < sunxi_cpufreq.min_freq)
		policy->min = sunxi_cpufreq.min_freq;

	if (policy->max > sunxi_cpufreq.max_freq)
		policy->max = sunxi_cpufreq.max_freq;

#ifdef CONFIG_SMP
	policy->shared_type = CPUFREQ_SHARED_TYPE_ANY;
	cpumask_copy(policy->cpus, cpu_possible_mask);
#endif

	return 0;
}

static struct freq_attr *sunxi_cpufreq_attr[] = {
	 &cpufreq_freq_attr_scaling_available_freqs,
	 NULL,
};

static struct cpufreq_driver sunxi_cpufreq_driver = {
	.name   = "cpufreq-sunxi",
	.flags  = CPUFREQ_STICKY,
	.init   = sunxi_cpufreq_init,
	.verify = sunxi_cpufreq_verify,
	.target = sunxi_cpufreq_target,
	.get    = sunxi_cpufreq_get,
	.attr   = sunxi_cpufreq_attr,
};

/*
 * get a valid frequency from cpu frequency table;
 * target_freq: target frequency to be judge, based on KHz;
 * return: cpu frequency, based on khz;
 */
static unsigned int __get_valid_freq(unsigned int target_freq)
{
	struct cpufreq_frequency_table *tmp = &sunxi_cpufreq.freq_table[0];

	while(tmp->frequency != CPUFREQ_TABLE_END){
		if((tmp+1)->frequency <= target_freq)
			tmp++;
		else
			break;
	}

	return tmp->frequency;
}

/*
 * init cpu max/min frequency from dt;
 * return: 0 - init cpu max/min successed, !0 - init cpu max/min failed;
 */
static int __init_freq_dt(void)
{
	struct device_node *np;
	int ret = -1;
#ifdef CONFIG_CPU_VOLTAGE_SCALING
	char name[16] = {0};
	int i;
#endif

	np = of_find_node_by_path("/dvfs_table");
	if (!np) {
		CPUFREQ_ERR("No dvfs table node found\n");
		return -ENODEV;
	}

	if (of_property_read_u32(np, "max_freq", &sunxi_cpufreq.max_freq)) {
		CPUFREQ_ERR("get cpu max freq from dt failed\n");
		goto fail;
	}

	if (of_property_read_u32(np, "min_freq", &sunxi_cpufreq.min_freq)) {
		CPUFREQ_ERR("get cpu min freq from df failed\n");
		goto fail;
	}

	if (of_property_read_u32(np, "extremity_freq", &sunxi_cpufreq.ext_freq))
		sunxi_cpufreq.ext_freq = sunxi_cpufreq.max_freq;

	if (of_property_read_u32(np, "boot_freq", &sunxi_cpufreq.boot_freq))
		sunxi_cpufreq.boot_freq = sunxi_cpufreq.max_freq;
	else
		sunxi_boot_lock = 1;

	if (sunxi_cpufreq.max_freq > SUNXI_CPUFREQ_MAX
				|| sunxi_cpufreq.max_freq < SUNXI_CPUFREQ_MIN) {
		CPUFREQ_ERR("cpu max freq from sysconfig is more than range\n");
		goto fail;
	}

	if (sunxi_cpufreq.min_freq < SUNXI_CPUFREQ_MIN
				|| sunxi_cpufreq.min_freq > SUNXI_CPUFREQ_MAX) {
		CPUFREQ_ERR("cpu min freq from sysconfig is more than range\n");
		goto fail;
	}

	if (sunxi_cpufreq.min_freq > sunxi_cpufreq.max_freq) {
		CPUFREQ_ERR("cpu min freq can not be more than cpu max freq\n");
		goto fail;
	}

	if (sunxi_cpufreq.ext_freq < sunxi_cpufreq.max_freq) {
		CPUFREQ_ERR("cpu ext freq can not be less than cpu max freq\n");
		goto fail;
	}

	if (sunxi_cpufreq.boot_freq > sunxi_cpufreq.max_freq) {
		CPUFREQ_ERR("cpu boot freq can not be more than cpu max freq\n");
		goto fail;
	}

	if (sunxi_cpufreq.boot_freq < sunxi_cpufreq.min_freq) {
		CPUFREQ_ERR("cpu boot freq can not be less than cpu min freq\n");
		goto fail;
	}

	/* get valid max/min frequency from cpu frequency table */
	sunxi_cpufreq.max_freq  = __get_valid_freq(sunxi_cpufreq.max_freq  / 1000);
	sunxi_cpufreq.min_freq  = __get_valid_freq(sunxi_cpufreq.min_freq  / 1000);
	sunxi_cpufreq.ext_freq  = __get_valid_freq(sunxi_cpufreq.ext_freq  / 1000);
	sunxi_cpufreq.boot_freq = __get_valid_freq(sunxi_cpufreq.boot_freq / 1000);

#ifdef CONFIG_CPU_VOLTAGE_SCALING
	if (of_property_read_u32(np, "lv_count", &table_length_syscfg)) {
		CPUFREQ_ERR("get lv_count failed\n");
		ret = -EINVAL;
		goto fail;
	}

	if(table_length_syscfg != TABLE_LENGTH){
		CPUFREQ_ERR("lv_count is invalid\n");
		ret = -EINVAL;
		goto fail;
	}

	for (i = 1; i <= table_length_syscfg; i++) {
		sprintf(name, "lv%d_freq", i);
		if (of_property_read_u32(np, name, &dvfs_table_syscfg[i-1].freq)) {
			CPUFREQ_ERR("get lv%d_freq failed\n", i);
			ret = -EINVAL;
			goto fail;
		}

		sprintf(name, "lv%d_volt", i);
		if (of_property_read_u32(np, name, &dvfs_table_syscfg[i-1].volt)) {
			CPUFREQ_ERR("get lv%d_volt failed\n", i);
			ret = -EINVAL;
			goto fail;
		}
	}
#endif

	return 0;

fail:
	/* use default cpu max/min frequency */
	sunxi_cpufreq.max_freq  = SUNXI_CPUFREQ_MAX / 1000;
	sunxi_cpufreq.min_freq  = SUNXI_CPUFREQ_MIN / 1000;
	sunxi_cpufreq.ext_freq  = SUNXI_CPUFREQ_MAX / 1000;
	sunxi_cpufreq.boot_freq = SUNXI_CPUFREQ_MAX / 1000;

	return ret;
}

/*
 * cpu frequency driver init
 */
static int __init sunxi_cpufreq_initcall(void)
{
	struct device_node *np;
	const struct property *prop;
	struct cpufreq_frequency_table *freq_tbl;
	const __be32 *val;
	int ret, cnt, i;

	np = of_find_node_by_path("/cpus/cpu@0");
	if (!np) {
		CPUFREQ_ERR("No cpu node found\n");
		return -ENODEV;
	}

	if (of_property_read_u32(np, "clock-latency",
					&sunxi_cpufreq.transition_latency))
		sunxi_cpufreq.transition_latency = CPUFREQ_ETERNAL;

	prop = of_find_property(np, "cpufreq_tbl", NULL);
	if (!prop || !prop->value) {
		CPUFREQ_ERR("Invalid cpufreq_tbl\n");
		ret = -ENODEV;
		goto out_put_node;
	}

	cnt = prop->length / sizeof(u32);
	val = prop->value;

	freq_tbl = kmalloc(sizeof(*freq_tbl) * (cnt + 1), GFP_KERNEL);
	if (!freq_tbl) {
		ret = -ENOMEM;
		goto out_put_node;
	}

	for (i = 0; i < cnt; i++) {
		freq_tbl[i].index = i;
		freq_tbl[i].frequency = be32_to_cpup(val++);
	}

	freq_tbl[i].index = i;
	freq_tbl[i].frequency = CPUFREQ_TABLE_END;
	sunxi_cpufreq.freq_table = freq_tbl;

#ifdef CONFIG_DEBUG_FS
	sunxi_cpufreq.cpufreq_set_us = 0;
	sunxi_cpufreq.cpufreq_get_us = 0;
#endif

	sunxi_cpufreq.last_freq = ~0;

	sunxi_cpufreq.clk_pll = clk_get(NULL, PLL_CPU_CLK);
	if (IS_ERR(sunxi_cpufreq.clk_pll)) {
		CPUFREQ_ERR("Unable to get PLL CPU clock\n");
		ret = PTR_ERR(sunxi_cpufreq.clk_pll);
		goto out_err_clk_pll;
	}

	sunxi_cpufreq.clk_cpu = clk_get(NULL, CPU_CLK);
	if (IS_ERR(sunxi_cpufreq.clk_cpu)) {
		CPUFREQ_ERR("Unable to get CPU clock\n");
		ret = PTR_ERR(sunxi_cpufreq.clk_cpu);
		goto out_err_clk_cpu;
	}

	sunxi_cpufreq.vdd_cpu = regulator_get(NULL, CPU_VDD);
	if (IS_ERR(sunxi_cpufreq.vdd_cpu)) {
		CPUFREQ_ERR("Unable to get CPU regulator\n");
		ret = PTR_ERR(sunxi_cpufreq.vdd_cpu);
		goto out_err_vdd_cpu;
	}

	/* init cpu frequency from dt */
	ret = __init_freq_dt();
	if (ret == -ENODEV
#ifdef CONFIG_CPU_VOLTAGE_SCALING
		|| ret == -EINVAL
#endif
	)
		goto out_err_dt;

	pr_debug("[cpufreq] max: %uMHz, min: %uMHz, ext: %uMHz, boot: %uMHz\n",
				sunxi_cpufreq.max_freq / 1000, sunxi_cpufreq.min_freq / 1000,
				sunxi_cpufreq.ext_freq / 1000, sunxi_cpufreq.boot_freq / 1000);

#ifdef CONFIG_CPU_VOLTAGE_SCALING
	__vftable_show();
	sunxi_cpufreq.last_vdd = sunxi_cpufreq_getvolt();
#endif

	mutex_init(&sunxi_cpufreq.lock);

	ret = cpufreq_register_driver(&sunxi_cpufreq_driver);
	if (ret) {
		CPUFREQ_ERR("failed register driver\n");
		goto out_err_register;
	} else {
		goto out_put_node;
	}

out_err_register:
	mutex_destroy(&sunxi_cpufreq.lock);
out_err_dt:
	regulator_put(sunxi_cpufreq.vdd_cpu);
out_err_vdd_cpu:
	clk_put(sunxi_cpufreq.clk_cpu);
out_err_clk_cpu:
	clk_put(sunxi_cpufreq.clk_pll);
out_err_clk_pll:
	kfree(freq_tbl);
out_put_node:
	of_node_put(np);

	return ret;
}
module_init(sunxi_cpufreq_initcall);

/*
 * cpu frequency driver exit
 */
static void __exit sunxi_cpufreq_exitcall(void)
{
	mutex_destroy(&sunxi_cpufreq.lock);
	regulator_put(sunxi_cpufreq.vdd_cpu);
	clk_put(sunxi_cpufreq.clk_pll);
	clk_put(sunxi_cpufreq.clk_cpu);
	cpufreq_unregister_driver(&sunxi_cpufreq_driver);
}
module_exit(sunxi_cpufreq_exitcall);

#ifdef CONFIG_DEBUG_FS
static struct dentry *debugfs_cpufreq_root;

static int cpufreq_debugfs_gettime_show(struct seq_file *s, void *data)
{
	seq_printf(s, "%lld\n", sunxi_cpufreq.cpufreq_get_us);
	return 0;
}

static int cpufreq_debugfs_gettime_open(struct inode *inode, struct file *file)
{
	return single_open(file, cpufreq_debugfs_gettime_show, inode->i_private);
}

static const struct file_operations cpufreq_debugfs_gettime_fops = {
	.open = cpufreq_debugfs_gettime_open,
	.read = seq_read,
};

static int cpufreq_debugfs_settime_show(struct seq_file *s, void *data)
{
	seq_printf(s, "%lld\n", sunxi_cpufreq.cpufreq_set_us);
	return 0;
}

static int cpufreq_debugfs_settime_open(struct inode *inode, struct file *file)
{
	return single_open(file, cpufreq_debugfs_settime_show, inode->i_private);
}

static const struct file_operations cpufreq_debugfs_settime_fops = {
	.open = cpufreq_debugfs_settime_open,
	.read = seq_read,
};

static int __init cpufreq_debugfs_init(void)
{
	int err = 0;

	debugfs_cpufreq_root = debugfs_create_dir("cpufreq", 0);
	if (!debugfs_cpufreq_root)
		return -ENOMEM;

	if (!debugfs_create_file("get_time", 0444, debugfs_cpufreq_root, NULL,
				&cpufreq_debugfs_gettime_fops)) {
		err = -ENOMEM;
		goto out;
	}

	if (!debugfs_create_file("set_time", 0444, debugfs_cpufreq_root, NULL,
				&cpufreq_debugfs_settime_fops)) {
		err = -ENOMEM;
		goto out;
	}

	return 0;

out:
	debugfs_remove_recursive(debugfs_cpufreq_root);
	return err;
}

static void __exit cpufreq_debugfs_exit(void)
{
	debugfs_remove_recursive(debugfs_cpufreq_root);
}

late_initcall(cpufreq_debugfs_init);
module_exit(cpufreq_debugfs_exit);
#endif /* CONFIG_DEBUG_FS */

MODULE_DESCRIPTION("cpufreq driver for sunxi SOCs");
MODULE_LICENSE("GPL");
