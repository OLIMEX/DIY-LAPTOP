/*
 * drivers/devfreq/ddrfreq/sunxi_dramfreq.c
 *
 * Copyright(c) 2013-2015 Allwinnertech Co., Ltd.
 *
 * Author: Pan Nan <pannan@allwinnertech.com>
 *
 * SUNXI dram frequency dynamic scaling driver
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <asm/tlbflush.h>
#include <linux/clk.h>
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/devfreq.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/reboot.h>
#include <linux/suspend.h>
#include <linux/sunxi_dramfreq.h>

enum {
	DEBUG_NONE = 0,
	DEBUG_FREQ = 1,
};
static int debug_mask = DEBUG_FREQ;
module_param(debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);
#define DRAMFREQ_DBG(mask,format,args...) \
	do { if (mask & debug_mask) printk("[dramfreq] "format,##args); } while (0)
#define DRAMFREQ_ERR(format,args...) \
	printk(KERN_ERR "[dramfreq] ERR:"format,##args)

#define SUNXI_DRAMFREQ_MDFS_RETRIES (3)

static unsigned int sunxi_dramfreq_table[LV_END] = {
	672000, //LV_0
	480000, //LV_1
	336000, //LV_2
	240000, //LV_3
	168000, //LV_4
};

struct sunxi_dramfreq *dramfreq = NULL;
struct task_struct *sunxi_dramfreq_task = NULL;

#ifdef CONFIG_DEVFREQ_DRAM_FREQ_IN_VSYNC
#define MDFS_LEAST_TIME_RETRIES    (5)
#define MDFS_LEAST_TIME_US         (2000)

struct dramfreq_vb_time_ops {
	int (*get_vb_time) (void);
	int (*get_next_vb_time) (void);
	int (*is_in_vb) (void);
} dramfreq_vbtime_ops;

int dramfreq_set_vb_time_ops(struct dramfreq_vb_time_ops *ops)
{
	dramfreq_vbtime_ops.get_vb_time = ops->get_vb_time;
	dramfreq_vbtime_ops.get_next_vb_time = ops->get_next_vb_time;
	dramfreq_vbtime_ops.is_in_vb = ops->is_in_vb;

	return 0;
}
EXPORT_SYMBOL(dramfreq_set_vb_time_ops);
#endif

#ifdef CONFIG_DEVFREQ_DRAM_FREQ_BUSFREQ
static DEFINE_MUTEX(busfreq_lock);

struct busfreq_table {
	char *name;
	struct clk *bus_clk;
	unsigned long normal_freq;
	unsigned long idle_freq;
};

static struct busfreq_table busfreq_tbl[] = {
	{ .name = "ahb1",   .normal_freq = 200000000, .idle_freq =  50000000 },
};

static struct clk *clk_ahb1;

static int sunxi_dramfreq_busfreq_target(const char *name, struct clk *clk,
					unsigned long target_freq)
{
	unsigned long cur_freq;

	mutex_lock(&busfreq_lock);

	if (clk_prepare_enable(clk)) {
		DRAMFREQ_ERR("try to enable %s output failed!\n", name);
		goto err;
	}

	cur_freq = clk_get_rate(clk);
	if (cur_freq == target_freq) {
		mutex_unlock(&busfreq_lock);
		return 0;
	}

	if (clk_set_rate(clk, target_freq)) {
		DRAMFREQ_ERR("try to set %s rate to %lu failed!\n", name, target_freq);
		goto err;
	}

	cur_freq = clk_get_rate(clk);
	if (cur_freq != target_freq) {
		DRAMFREQ_ERR("%s: %lu != %lu\n", name, cur_freq, target_freq);
		goto err;
	}

	mutex_unlock(&busfreq_lock);
	return 0;

err:
	mutex_unlock(&busfreq_lock);
	return -1;
}
#endif /* CONFIG_DEVFREQ_DRAM_FREQ_BUSFREQ */

#ifdef CONFIG_SMP
struct cpumask dramfreq_ipi_mask;
static volatile bool cpu_pause[NR_CPUS];
static volatile bool pause_flag;
static bool mdfs_is_paused(void)
{
	smp_rmb();
	return pause_flag;
}
static void mdfs_set_paused(bool pause)
{
	pause_flag = pause;
	smp_wmb();
}
static bool mdfs_cpu_is_paused(unsigned int cpu)
{
	smp_rmb();
	return cpu_pause[cpu];
}
static void mdfs_cpu_set_paused(unsigned int cpu, bool pause)
{
	cpu_pause[cpu] = pause;
	smp_wmb();
}
static void mdfs_cpu_pause(void *info)
{
	unsigned int cpu = raw_smp_processor_id();
	dsb(sy);
	isb();
	mdfs_cpu_set_paused(cpu, true);
	while (mdfs_is_paused());
	mdfs_cpu_set_paused(cpu, false);
}
static void mdfs_cpu_wait(void *info) {}
#endif /* CONFIG_SMP */

#ifdef CONFIG_DEVFREQ_DRAM_FREQ_WITH_SOFT_NOTIFY
int dramfreq_master_access(enum DRAM_KEY_MASTER master, bool access)
{
	if (dramfreq == NULL)
		return -EINVAL;

	dramfreq->key_masters[master] = access ? 1 : 0;

	if (!dramfreq->pause)
		wake_up_process(sunxi_dramfreq_task);

	return 0;
}
EXPORT_SYMBOL(dramfreq_master_access);
#endif

static int mdfs_cfs(unsigned int freq_jump, struct sunxi_dramfreq *dramfreq,
				unsigned int freq)
{
	volatile unsigned int reg_val;
	unsigned int i, div, n = 4;
	unsigned int rank_num, trefi, trfc, ctrl_freq;
	unsigned int timeout = 1000000000;
	struct dram_para_t *para = &dramfreq->dram_para;

	/* wait for process finished */
	do {
		reg_val = readl(dramfreq->dramcom_base + MC_MDFSCR) & 0x1;
	} while (reg_val && timeout--);

	if (timeout == 0)
		return -EBUSY;

	/*for CFS only support LPDDR */
	ctrl_freq = freq >> 1;
	if ((para->dram_type == 3) || (para->dram_type == 2)) {
		trefi = ((7800*ctrl_freq)/1000 + ((((7800*ctrl_freq)%1000) != 0) ? 1 :0))/32;
		trfc = (350*ctrl_freq)/1000 + ((((350*ctrl_freq)%1000) != 0) ? 1 :0);
	} else {
		trefi = ((3900*ctrl_freq)/1000 + ((((3900*ctrl_freq)%1000) != 0) ? 1 :0))/32;
		trfc = (210*ctrl_freq)/1000 + ((((210*ctrl_freq)%1000) != 0) ? 1 :0);
	}

	/* set dual buffer for timing change and power save */
	reg_val = readl(dramfreq->dramcom_base + MC_MDFSCR);
	reg_val |=(0x1U<<15);
	writel(reg_val, dramfreq->dramcom_base + MC_MDFSCR);

	/* change refresh timing */
	reg_val = readl(dramfreq->dramctl_base + RFSHTMG);
	reg_val &= ~((0xfff<<0)|(0xfff<<16));
	reg_val |= ((trfc<<0)|(trefi<<16));
	writel(reg_val, dramfreq->dramctl_base + RFSHTMG);

	/* make sure clk always on */
	// reg_val = readl(dramfreq->dramctl_base + PGCR0);
	// reg_val &= ~(0xf<<12);
	// reg_val |= (0x5<<12);
	// writel(reg_val, dramfreq->dramctl_base + PGCR0);

	/* change ODT status for power save  */
	if (!((para->dram_tpr13>>12) & 0x1)) {
		if (freq > 400) {
			if ((para->dram_odt_en & 0x1)) {
				for (i = 0; i < n; i++) {
					//byte 0/byte 1
					reg_val = readl(dramfreq->dramctl_base + DXnGCR0(i));
					reg_val &= ~(0x3U<<4);
					reg_val |= (0x0<<4);//ODT dynamic
					writel(reg_val, dramfreq->dramctl_base + DXnGCR0(i));
				}

				rank_num = readl(dramfreq->dramcom_base + MC_WORK_MODE) & 0x1;
				if (rank_num)
					writel(0x00000303, dramfreq->dramctl_base + ODTMAP);
				else
					writel(0x00000201, dramfreq->dramctl_base + ODTMAP);
			}
		} else {
			if ((para->dram_odt_en & 0x1)) {
				for (i = 0; i < n; i++) {
					//byte 0/byte 1
					reg_val = readl(dramfreq->dramctl_base + DXnGCR0(i));
					reg_val &= ~(0x3U<<4);
					reg_val |= (0x2<<4);
					writel(reg_val, dramfreq->dramctl_base + DXnGCR0(i));
				}
				writel(0x0, dramfreq->dramctl_base + ODTMAP);
			}
		}
	}

	/* change pll-ddr N value */
	div = freq / 12;
	reg_val = readl(dramfreq->ccu_base + CCM_PLL_DDR1_REG);
	reg_val &= ~(0x7f<<8);
	reg_val |=(((div-1)<<8));
	writel(reg_val, dramfreq->ccu_base + CCM_PLL_DDR1_REG);

	/* setting MDFS configuration */
	reg_val = readl(dramfreq->dramcom_base + MC_MDFSCR);
	reg_val &= ~(0x1<<2);
	reg_val |= ((freq_jump & 0x1) << 2);    //1: increase  0:decrease
	reg_val |= (0x1<<1);  //CFS mode
	reg_val |= (0x1<<11); //vtf enable
	reg_val |= (0x1<<4);  //bypass
	writel(reg_val, dramfreq->dramcom_base + MC_MDFSCR);

	return 0;
}

static int mdfs_dfs(unsigned int freq_jump, struct sunxi_dramfreq *dramfreq,
				unsigned int freq)
{
	volatile unsigned int reg_val;
	unsigned int rank_num, trefi, trfc, ctrl_freq;
	unsigned int i, n = 4;
	unsigned int div, source;
	unsigned int vtf_status;
	// unsigned int hdr_clk_status;
	unsigned int timeout = 1000000;
	struct dram_para_t *para = &dramfreq->dram_para;

	/* calculate source and divider */
	if (para->dram_tpr9 != 0) {
		if (((para->dram_clk % freq) == 0 ) && ((para->dram_tpr9 % freq) == 0)) {
			if ((para->dram_clk / freq) > (para->dram_tpr9 / freq)) {
				source = 0;
				div = para->dram_tpr9 / freq ;
			} else {
				source = 1;
				div = para->dram_clk / freq ;
			}
		} else if((para->dram_clk % freq) == 0) {
			source = 1;
			div = para->dram_clk / freq ;
		} else if((para->dram_tpr9 % freq) == 0) {
			source = 0;
			div = para->dram_tpr9 / freq ;
		} else {
			DRAMFREQ_ERR("unsupported freq!\n");
			return 1;
		}
	} else {
		source = 1;
		div = para->dram_clk / freq ;
	}

	ctrl_freq = freq >> 1;
	if ((para->dram_type == 3) || (para->dram_type == 2)) {
		trefi = ((7800*ctrl_freq)/1000 + ((((7800*ctrl_freq)%1000) != 0) ? 1 :0))/32;
		trfc = (350*ctrl_freq)/1000 + ((((350*ctrl_freq)%1000) != 0) ? 1 :0);
	}else{
		trefi = ((3900*ctrl_freq)/1000 + ((((3900*ctrl_freq)%1000) != 0) ? 1 :0))/32;
		trfc = (210*ctrl_freq)/1000 + ((((210*ctrl_freq)%1000) != 0) ? 1 :0);
	}

	/* make sure clk always on */
	// hdr_clk_status = (readl(dramfreq->dramctl_base + PGCR0)>>12) & 0xf;
	// reg_val = readl(dramfreq->dramctl_base + PGCR0);
	// reg_val &= ~(0xf<<12);
	// reg_val |= (0x5<<12);
	// writel(reg_val, dramfreq->dramctl_base + PGCR0);

	/* save vtf status,global vtf off when DFS */
	reg_val = readl(dramfreq->dramctl_base + VTFCR);
	vtf_status = (reg_val & (0x1<<8));
	if (vtf_status) {
		reg_val &= ~(0x1<<8);
		writel(reg_val, dramfreq->dramctl_base + VTFCR);
	}

	/* set dual buffer for timing change and power save */
	reg_val = readl(dramfreq->dramcom_base + MC_MDFSCR);
	/* VTC dual buffer can not be used */
	reg_val |=(0x1U << 15);
	writel(reg_val, dramfreq->dramcom_base + MC_MDFSCR);

	/* change refresh timing */
	reg_val = readl(dramfreq->dramctl_base + RFSHTMG);
	reg_val &= ~((0xfff<<0)|(0xfff<<16));
	reg_val |= ((trfc<<0)|(trefi<<16));
	writel(reg_val, dramfreq->dramctl_base + RFSHTMG);

	/* change ODT status for power save */
	if (!((para->dram_tpr13 >> 12) & 0x1)) {
		if (freq > 400) {
			if ((para->dram_odt_en & 0x1)) {
				for (i = 0; i < n; i++) {
					/* byte 0/byte 1 */
					reg_val = readl(dramfreq->dramctl_base + DXnGCR0(i));
					reg_val &= ~(0x3U<<4);
					reg_val |= (0x0<<4);//ODT dynamic
					writel(reg_val, dramfreq->dramctl_base + DXnGCR0(i));
				}

				rank_num = readl(dramfreq->dramcom_base + MC_WORK_MODE) & 0x1;
				if (rank_num)
					writel(0x00000303, dramfreq->dramctl_base + ODTMAP);
				else
					writel(0x00000201, dramfreq->dramctl_base + ODTMAP);
			}
		} else {
			if ((para->dram_odt_en & 0x1)) {
				for (i = 0; i < n; i++) {
					/* byte 0/byte 1*/
					reg_val = readl(dramfreq->dramctl_base + DXnGCR0(i));
					reg_val &= ~(0x3U<<4);
					reg_val |= (0x2<<4);
					writel(reg_val, dramfreq->dramctl_base + DXnGCR0(i));
				}
				writel(0x0, dramfreq->dramctl_base + ODTMAP);
			}
		}
	}

	/* set the DRAM_CFG_REG divider in CCMU */
	reg_val = readl(dramfreq->ccu_base + CCM_DRAM_CFG_REG);
	reg_val &= ~(0xf<<0);
	reg_val |= ((div-1)<<0);
	writel(reg_val, dramfreq->ccu_base + CCM_DRAM_CFG_REG);

	/* set clock source in CCMU */
	reg_val &= ~(0x3<<20);
	reg_val |= (source<<20);
	writel(reg_val, dramfreq->ccu_base + CCM_DRAM_CFG_REG);

	/* set MDFS register */
	reg_val = readl(dramfreq->dramcom_base + MC_MDFSCR);
	reg_val |= (0x1<<4);   //bypass
	reg_val |= (0x1<<13);  //pad hold
	reg_val &= ~(0x1U<<1); //DFS mode
	writel(reg_val, dramfreq->dramcom_base + MC_MDFSCR);

	reg_val = readl(dramfreq->dramcom_base + MC_MDFSCR);
	reg_val |= (0x1U<<0); //start mdfs
	writel(reg_val, dramfreq->dramcom_base + MC_MDFSCR);

	/* wait for process finished */
	while (timeout--) {
		reg_val = readl(dramfreq->dramcom_base + MC_MDFSCR) & 0x1;
		if (!reg_val)
			break;
	}

	if (timeout == 0)
		return -EBUSY;

	/* recovery vtf status */
	if (vtf_status) {
		reg_val = readl(dramfreq->dramctl_base + VTFCR);
		vtf_status |= (0x1<<8);
		writel(reg_val, dramfreq->dramctl_base + VTFCR);
	}

	/* turn off dual buffer */
	reg_val = readl(dramfreq->dramcom_base + MC_MDFSCR);
	reg_val &= ~(0x1U<<15);
	writel(reg_val, dramfreq->dramcom_base + MC_MDFSCR);

	/* revovery hdr clk status */
	// reg_val = readl(dramfreq->dramctl_base + PGCR0);
	// reg_val &= ~(0xf<<12);
	// reg_val |= (hdr_clk_status<<12);
	// writel(reg_val, dramfreq->dramctl_base + PGCR0);

	return 0;
}

static int sunxi_dramfreq_get_cur_freq(struct device *dev, unsigned long *freq)
{
	unsigned long pll_ddr_rate;
	unsigned int dram_div_m;
#ifdef CONFIG_DEBUG_FS
	ktime_t calltime = ktime_get();
#endif

	if ((readl(dramfreq->ccu_base + 0xF4) >> 20) & 0x1)
		pll_ddr_rate = clk_get_rate(dramfreq->clk_pll_ddr1) / 1000;
	else
		pll_ddr_rate = clk_get_rate(dramfreq->clk_pll_ddr0) / 1000;

	dram_div_m = (readl(dramfreq->ccu_base + 0xF4) & 0x3) + 1;
	*freq = pll_ddr_rate / 2 / dram_div_m;

#ifdef CONFIG_DEBUG_FS
	dramfreq->dramfreq_get_us = ktime_to_us(ktime_sub(ktime_get(), calltime));
#endif

	return 0;
}

static int sunxi_dramfreq_get_max_freq(void)
{
	return sunxi_dramfreq_table[0];
}

static int sunxi_dramfreq_get_min_freq(void)
{
	unsigned int i, min = UINT_MAX;

	for (i = 0; i < LV_END; i++) {
		if (sunxi_dramfreq_table[i] == 0)
			continue;

		if (sunxi_dramfreq_table[i] < min)
			min = sunxi_dramfreq_table[i];
	}

	return min;
}

static int sunxi_dramfreq_get_valid_freq(unsigned long freq)
{
	unsigned int *valid_freq = &sunxi_dramfreq_table[0];

	while (*(valid_freq+1) >= freq)
		valid_freq++;

	return *valid_freq;
}

static int sunxi_dramfreq_set_rate(unsigned int jump, unsigned int freq_target,
				struct sunxi_dramfreq *dramfreq, unsigned int *masters_access)
{
	int i, ret = 0;
	s64 mdfs_time_us = 0;
	ktime_t calltime;
#ifdef CONFIG_SMP
	unsigned int cpu, cur_cpu, timeout = 0;
#endif

#ifdef CONFIG_SMP
	cpumask_clear(&dramfreq_ipi_mask);
	cpumask_copy(&dramfreq_ipi_mask, cpu_online_mask);
	cur_cpu = raw_smp_processor_id();
	cpumask_clear_cpu(cur_cpu, &dramfreq_ipi_mask);

	local_bh_disable();
	mdfs_set_paused(true);

	preempt_disable();
	smp_call_function_many(&dramfreq_ipi_mask, mdfs_cpu_pause, NULL, 0);
	preempt_enable();

	dsb(sy);
	isb();

	for_each_online_cpu(cpu) {
		if (cpu == cur_cpu)
			continue;
		while (!mdfs_cpu_is_paused(cpu) && timeout < 100) {
			udelay(100);
			timeout++;
		}

		if (timeout >= 100) {
			DRAMFREQ_ERR("Pause cpu%d time out!\n", cpu);
			ret = -EAGAIN;
			goto out;
		}
	}
#endif

	if (jump == FREQ_DOWN) {
		for (i = 0; i < MASTER_MAX; i++) {
			if (masters_access[i] < dramfreq->key_masters[i]) {
				ret = -EINVAL;
				goto out;
			}
		}
	}

	local_irq_disable();

	if (dramfreq->mode == DFS_MODE) {
#ifdef CONFIG_DEVFREQ_DRAM_FREQ_IN_VSYNC
		while(!dramfreq_vbtime_ops.is_in_vb());
#endif
	}

	calltime = ktime_get();
	flush_tlb_all();
	isb();

	if (dramfreq->mode == CFS_MODE)
		ret = mdfs_cfs(jump, dramfreq, freq_target / 1000);
	else if (dramfreq->mode == DFS_MODE)
		ret = mdfs_dfs(jump, dramfreq, freq_target / 1000);

	mdfs_time_us = ktime_to_us(ktime_sub(ktime_get(), calltime));
	local_irq_enable();

	DRAMFREQ_DBG(DEBUG_FREQ, "[cpu%d] elapsed:%lldus\n", cur_cpu, mdfs_time_us);

#ifdef CONFIG_DEBUG_FS
	dramfreq->dramfreq_set_us = mdfs_time_us;
#endif

out:
#ifdef CONFIG_SMP
	mdfs_set_paused(false);
	local_bh_enable();
	preempt_disable();
	smp_call_function_many(&dramfreq_ipi_mask, mdfs_cpu_wait, NULL, true);
	preempt_enable();
#endif

	return ret;
}

static int sunxi_dramfreq_target(struct device *dev,
								unsigned long *freq, u32 flags)
{
	struct platform_device *pdev = container_of(dev, struct platform_device, dev);
	struct sunxi_dramfreq *dramfreq = platform_get_drvdata(pdev);
	int i, ret = 0, retries = 0, jump = 0;
	unsigned int masters_access[MASTER_MAX];
#ifdef CONFIG_DEVFREQ_DRAM_FREQ_IN_VSYNC
	int next_vbtime_us = 0, next_vb_retries = 0;
#endif
	unsigned long cur_freq, valid_freq;
#ifdef CONFIG_CPU_FREQ
	struct cpufreq_policy policy;
#endif

	for (i = 0; i < MASTER_MAX; i++)
		masters_access[i] = dramfreq->key_masters[i];

	if ((dramfreq == NULL) || (dramfreq->devfreq == NULL))
		return -EINVAL;

	mutex_lock(&dramfreq->lock);
	get_online_cpus();

	valid_freq = sunxi_dramfreq_get_valid_freq(*freq);
	if (valid_freq == dramfreq->devfreq->previous_freq) {
		if (*freq != valid_freq)
			*freq = valid_freq;
		goto unlock;
	}

	jump = (valid_freq > dramfreq->devfreq->previous_freq) ? FREQ_UP : FREQ_DOWN;

	DRAMFREQ_DBG(DEBUG_FREQ, "%luKHz->%luKHz start\n",
						dramfreq->devfreq->previous_freq, valid_freq);

#ifdef CONFIG_DEVFREQ_DRAM_FREQ_BUSFREQ
	if ((dramfreq->devfreq->previous_freq == dramfreq->devfreq->min_freq) &&
			(jump == FREQ_UP)) {
		for (i = ARRAY_SIZE(busfreq_tbl) - 1; i >= 0; i--) {
			sunxi_dramfreq_busfreq_target(busfreq_tbl[i].name,
					busfreq_tbl[i].bus_clk, busfreq_tbl[i].normal_freq);
		}
		DRAMFREQ_DBG(DEBUG_FREQ, "AHB1:%lu\n", clk_get_rate(clk_ahb1) / 1000000);
	}
#endif

	if (dramfreq->mode == DFS_MODE) {
#ifdef CONFIG_DEVFREQ_DRAM_FREQ_IN_VSYNC
		if (!(dramfreq_vbtime_ops.get_next_vb_time
				&& dramfreq_vbtime_ops.is_in_vb)) {
			DRAMFREQ_ERR("dramfreq_vbtime_ops is not initialized!\n");
			ret = -EINVAL;
			goto unlock;
		}

		do {
			next_vbtime_us = dramfreq_vbtime_ops.get_next_vb_time();
			if (next_vbtime_us < MDFS_LEAST_TIME_US) {
				next_vb_retries++;
				msleep(1);
			} else {
				break;
			}
		} while (next_vb_retries < MDFS_LEAST_TIME_RETRIES);

		if (next_vb_retries >= MDFS_LEAST_TIME_RETRIES) {
			DRAMFREQ_ERR("Retrying next vb time failed, next time!\n");
			ret = -EINVAL;
			goto unlock;
		} else {
			usleep_range(next_vbtime_us - MDFS_LEAST_TIME_US,
							next_vbtime_us - MDFS_LEAST_TIME_US);
		}
#endif
	}

#ifdef CONFIG_CPU_FREQ
	if ((!cpufreq_get_policy(&policy, 0)) && (policy.cur < policy.max))
		__cpufreq_driver_target(&policy, policy.max, CPUFREQ_RELATION_H);
#endif

	do {
		ret = sunxi_dramfreq_set_rate(jump, valid_freq, dramfreq, masters_access);
		if (ret == -EAGAIN)
			retries++;
		else
			break;
	} while (retries < SUNXI_DRAMFREQ_MDFS_RETRIES);

	if (retries >= SUNXI_DRAMFREQ_MDFS_RETRIES) {
		DRAMFREQ_ERR("Retrying mdfs failed, next time!\n");
		ret = -EINVAL;
		goto unlock;
	}

	if (ret == -EBUSY) {
		DRAMFREQ_ERR("mdfs timeout\n");
		goto unlock;
	} else if (ret == -EINVAL) {
		DRAMFREQ_DBG(DEBUG_FREQ, "no need for freq down\n");
		goto unlock;
	}

	sunxi_dramfreq_get_cur_freq(dev, &cur_freq);
	if (cur_freq != valid_freq) {
		DRAMFREQ_ERR("current freq is %lu != %lu\n", cur_freq, valid_freq);
		goto unlock;
	}

	if (*freq != cur_freq)
		*freq = cur_freq;

	DRAMFREQ_DBG(DEBUG_FREQ, "%luKHz->%luKHz ok\n",
					dramfreq->devfreq->previous_freq, cur_freq);

#ifdef CONFIG_DEVFREQ_DRAM_FREQ_BUSFREQ
	if ((cur_freq == dramfreq->devfreq->min_freq) && (jump == FREQ_DOWN)) {
		for (i = 0; i < ARRAY_SIZE(busfreq_tbl); i++) {
			sunxi_dramfreq_busfreq_target(busfreq_tbl[i].name,
					busfreq_tbl[i].bus_clk, busfreq_tbl[i].idle_freq);
		}
		DRAMFREQ_DBG(DEBUG_FREQ, "AHB1:%lu\n", clk_get_rate(clk_ahb1) / 1000000);
	}
#endif

unlock:
	put_online_cpus();
	mutex_unlock(&dramfreq->lock);

	return ret;
}

static struct devfreq_dev_profile sunxi_dramfreq_profile = {
	.get_cur_freq = sunxi_dramfreq_get_cur_freq,
	.target       = sunxi_dramfreq_target,
	.freq_table   = sunxi_dramfreq_table,
	.max_state    = LV_END,
};

extern int update_devfreq(struct devfreq *devfreq);
static int sunxi_dramfreq_task_func(void *data)
{
	struct devfreq *df = (struct devfreq *)data;

	while (1) {
#if 0
		mutex_lock(&df->lock);
		update_devfreq(df);
		mutex_unlock(&df->lock);
#endif
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
		if (kthread_should_stop())
			break;
		set_current_state(TASK_RUNNING);
	}
	return 0;
}

static void sunxi_dramfreq_masters_state_init(struct sunxi_dramfreq *dramfreq)
{
	int i;

	for (i = 0; i < MASTER_MAX; i++)
		dramfreq->key_masters[i] = (i == MASTER_CSI) ? 0 : 1;
}

static int sunxi_dramfreq_governor_state_update(enum GOVERNOR_STATE type)
{
	switch (type) {
	case STATE_INIT:
		dramfreq->pause = 1;
		sunxi_dramfreq_task = kthread_create(sunxi_dramfreq_task_func,
											dramfreq->devfreq, "dramfreq_task");
		if (IS_ERR(sunxi_dramfreq_task))
			return PTR_ERR(sunxi_dramfreq_task);

		get_task_struct(sunxi_dramfreq_task);
		wake_up_process(sunxi_dramfreq_task);
		break;
	case STATE_RUNNING:
		dramfreq->pause = 0;
		wake_up_process(sunxi_dramfreq_task);
		break;
	case STATE_PAUSE:
		dramfreq->pause = 1;
		wake_up_process(sunxi_dramfreq_task);
		break;
	case STATE_EXIT:
		dramfreq->pause = 1;
		mutex_lock(&dramfreq->devfreq->lock);
		update_devfreq(dramfreq->devfreq);
		mutex_unlock(&dramfreq->devfreq->lock);

		kthread_stop(sunxi_dramfreq_task);
		put_task_struct(sunxi_dramfreq_task);
		break;
	}

	return 0;
}

static int sunxi_dramfreq_paras_init(struct sunxi_dramfreq *dramfreq)
{
	struct device_node *dram_np;
	int ret = 0;

	dram_np = of_find_node_by_path("/dram");
	if (!dram_np) {
		DRAMFREQ_ERR("Get dram node failed!\n");
		return -ENODEV;
	}

	if (of_property_read_u32(dram_np, "dram_clk",
								&dramfreq->dram_para.dram_clk)) {
		DRAMFREQ_ERR("Get dram_clk failed!\n");
		ret = -ENODEV;
		goto out_put_node;
	}

	if (of_property_read_u32(dram_np, "dram_type",
								&dramfreq->dram_para.dram_type)) {
		DRAMFREQ_ERR("Get dram_type failed!\n");
		ret = -ENODEV;
		goto out_put_node;
	}

	if (of_property_read_u32(dram_np, "dram_odt_en",
								&dramfreq->dram_para.dram_odt_en)) {
		DRAMFREQ_ERR("Get dram_odt_en failed!\n");
		ret = -ENODEV;
		goto out_put_node;
	}

	if (of_property_read_u32(dram_np, "dram_tpr9",
								&dramfreq->dram_para.dram_tpr9)) {
		DRAMFREQ_ERR("Get dram_tpr9 failed!\n");
		ret = -ENODEV;
		goto out_put_node;
	}

	if (of_property_read_u32(dram_np, "dram_tpr13",
								&dramfreq->dram_para.dram_tpr13)) {
		DRAMFREQ_ERR("Get dram_tpr13 failed!\n");
		ret = -ENODEV;
		goto out_put_node;
	}

	if (!((dramfreq->dram_para.dram_tpr13 >> 11) & 0x1)) {
		ret = -EINVAL;
		goto out_put_node;
	}

	dramfreq->mode = (dramfreq->dram_para.dram_tpr13 >> 10) & 0x1;

out_put_node:
	of_node_put(dram_np);
	return ret;
}

static int sunxi_dramfreq_resource_init(struct platform_device *pdev,
					struct sunxi_dramfreq *dramfreq)
{
	int ret = 0;

	dramfreq->dramcom_base = of_iomap(pdev->dev.of_node, 0);
	if (!dramfreq->dramcom_base) {
		DRAMFREQ_ERR("Map dramcom_base failed!\n");
		ret = -EBUSY;
		goto out;
	}

	dramfreq->dramctl_base = of_iomap(pdev->dev.of_node, 1);
	if (!dramfreq->dramctl_base) {
		DRAMFREQ_ERR("Map dramctl_base failed!\n");
		ret = -EBUSY;
		goto out;
	}

	dramfreq->ccu_base = of_iomap(pdev->dev.of_node, 2);
	if (!dramfreq->ccu_base) {
		DRAMFREQ_ERR("Map ccu_base failed!\n");
		ret = -EBUSY;
		goto out;
	}

	dramfreq->clk_pll_ddr0 = of_clk_get(pdev->dev.of_node, 0);
	if (IS_ERR(dramfreq->clk_pll_ddr0)) {
		DRAMFREQ_ERR("Get clk_pll_ddr0 failed!\n");
		ret = -EINVAL;
		goto out;
	}

	dramfreq->clk_pll_ddr1 = of_clk_get(pdev->dev.of_node, 1);
	if (IS_ERR(dramfreq->clk_pll_ddr1)) {
		DRAMFREQ_ERR("Get clk_pll_ddr1 failed!\n");
		ret = -EINVAL;
		goto out;
	}

#ifdef CONFIG_DEVFREQ_DRAM_FREQ_BUSFREQ
	clk_ahb1 = of_clk_get(pdev->dev.of_node, 2);
	if (IS_ERR(clk_ahb1)) {
		DRAMFREQ_ERR("Get clk_ahb1 failed!\n");
		ret = -EINVAL;
		goto out;
	}

	busfreq_tbl[0].bus_clk = clk_ahb1;
#endif

out:
	return ret;
}

static void __array_insert_sort(unsigned int *array, int count)
{
	unsigned int i, j, temp;

	for (i = 1; i < count; i++) {
		temp = array[i];
		j = i-1;

		while (array[j] < temp && j >= 0) {
			array[j+1] = array[j];
			j--;
		}

		if (j != (i-1))
			array[j+1] = temp;
	}
}

static void __array_delete_repeat(unsigned int *array, int count)
{
	int i, j;

	for (i = 0, j = 0; i < count; i++) {
		while (array[i] == array[i+1]) {
			i++;
		}
		array[j++] = array[i];
	}

	for (i = j; i < count; i++)
		array[i] = 0;
}

static int sunxi_dramfreq_opp_init(struct platform_device *pdev,
					struct sunxi_dramfreq *dramfreq)
{
	int i, j, ret = 0;
	unsigned int tmp_table[LV_END], tmp_max_state = 0;

	for (i = 0; i < 3; i++)
		tmp_table[i] = dramfreq->dram_para.dram_clk * 1000 / (i + 1);

	if ((dramfreq->dram_para.dram_clk * 1000 / (i + 1)) >= SUNXI_DRAMFREQ_IDLE)
		tmp_table[i-1] = dramfreq->dram_para.dram_clk * 1000 / (i + 1);

	for (j = 0; i < LV_END; i++, j++) {
		if ((dramfreq->dram_para.dram_tpr9 / (j + 1))
				> dramfreq->dram_para.dram_clk) {
			tmp_table[i] = 0;
		} else {
			tmp_table[i] = dramfreq->dram_para.dram_tpr9 * 1000 / (j + 1);
		}
	}

	if (dramfreq->mode == CFS_MODE) {
		for (i = 0; i < LV_END; i++) {
			if ((tmp_table[i] % 12) != 0) {
				tmp_table[i] = ((tmp_table[i] / 1000) / 12) * 12 * 1000;
			}
		}
	}

	for (i = 0; i < LV_END; i++) {
		if (tmp_table[i] < SUNXI_DRAMFREQ_IDLE)
			tmp_table[i] = 0;
	}

	__array_insert_sort(tmp_table, LV_END);
	__array_delete_repeat(tmp_table, LV_END);
	memcpy(sunxi_dramfreq_table, tmp_table, sizeof(tmp_table));

	for (i = 0; i < LV_END; i++) {
		if (sunxi_dramfreq_table[i] == 0)
			continue;

		ret = opp_add(&pdev->dev, sunxi_dramfreq_table[i] * 1000, 0);
		if (ret) {
			DRAMFREQ_ERR("Failed to add OPP[%d]\n", i);
			goto out;
		}

		tmp_max_state++;
	}

	if (tmp_max_state < sunxi_dramfreq_profile.max_state)
		sunxi_dramfreq_profile.max_state = tmp_max_state;

out:
	return ret;
}

static int sunxi_dramfreq_reboot(struct notifier_block *this,
						unsigned long code, void *_cmd)
{
	dramfreq->pause = 1;

	printk("%s:%s: stop dramfreq done\n", __FILE__, __func__);
	return NOTIFY_OK;
}

static struct notifier_block reboot_notifier = {
	.notifier_call = sunxi_dramfreq_reboot,
};

static void sunxi_dramfreq_hw_init(struct sunxi_dramfreq *dramfreq)
{
	volatile unsigned int reg_val;

	if (dramfreq->mode == DFS_MODE) {
		writel(0xFFFFFFFF, dramfreq->dramcom_base + MC_MDFSMRMR);

		/* set DFS time */
		reg_val = readl(dramfreq->dramctl_base + PTR2);
		reg_val &= ~0x7fff;
		reg_val |= (0x7<<10 | 0x7<<5 | 0x7<<0);
		writel(reg_val, dramfreq->dramctl_base + PTR2);
	}
}

static int sunxi_dramfreq_probe(struct platform_device *pdev)
{
	int ret = 0;

	dramfreq = kzalloc(sizeof(struct sunxi_dramfreq), GFP_KERNEL);
	if (dramfreq == NULL) {
		DRAMFREQ_ERR("Allocate memory failed!\n");
		return -ENOMEM;
	}

	ret = sunxi_dramfreq_paras_init(dramfreq);
	if (ret == -ENODEV) {
		DRAMFREQ_ERR("Init dram para failed!\n");
		goto err;
	} else if (ret == -EINVAL) {
		printk("[ddrfreq] disabled!\n");
		goto err;
	}

	ret = sunxi_dramfreq_resource_init(pdev, dramfreq);
	if (ret) {
		DRAMFREQ_ERR("Init resource failed!\n");
		goto err;
	}

	ret = sunxi_dramfreq_opp_init(pdev, dramfreq);
	if (ret) {
		DRAMFREQ_ERR("Init opp failed!\n");
		goto err;
	}

	dev_set_name(&pdev->dev, "dramfreq");
	platform_set_drvdata(pdev, dramfreq);
	sunxi_dramfreq_get_cur_freq(&pdev->dev, &sunxi_dramfreq_profile.initial_freq);
	dramfreq->devfreq = devfreq_add_device(&pdev->dev, &sunxi_dramfreq_profile,
					   "adaptive", NULL);
	if (IS_ERR(dramfreq->devfreq)) {
		DRAMFREQ_ERR("Add devfreq device failed!\n");
		ret = PTR_ERR(dramfreq->devfreq);
		goto err;
	}

	dramfreq->max = sunxi_dramfreq_get_max_freq();
	dramfreq->min = sunxi_dramfreq_get_min_freq();
	dramfreq->devfreq->max_freq = dramfreq->max;
	dramfreq->devfreq->min_freq = dramfreq->min;

	dramfreq->pause = 1;
	dramfreq->governor_state_update = sunxi_dramfreq_governor_state_update;

	mutex_init(&dramfreq->lock);
	spin_lock_init(&dramfreq->master_lock);

	register_reboot_notifier(&reboot_notifier);

	/* init some hardware paras*/
	sunxi_dramfreq_hw_init(dramfreq);

	/* set master access init state */
	sunxi_dramfreq_masters_state_init(dramfreq);

	return 0;

err:
	kfree(dramfreq);
	dramfreq = NULL;
	return ret;
}

static int sunxi_dramfreq_remove(struct platform_device *pdev)
{
	struct sunxi_dramfreq *dramfreq = platform_get_drvdata(pdev);

	if (!dramfreq)
		return -EINVAL;

	mutex_destroy(&dramfreq->lock);

	if (dramfreq->devfreq)
		devfreq_remove_device(dramfreq->devfreq);

	if (dramfreq->clk_pll_ddr1) {
		clk_put(dramfreq->clk_pll_ddr1);
		dramfreq->clk_pll_ddr1 = NULL;
	}

	if (dramfreq->clk_pll_ddr0) {
		clk_put(dramfreq->clk_pll_ddr0);
		dramfreq->clk_pll_ddr0 = NULL;
	}

#ifdef CONFIG_DEVFREQ_DRAM_FREQ_BUSFREQ
	if (clk_ahb1) {
		clk_put(clk_ahb1);
		clk_ahb1 = NULL;
	}
#endif

	kfree(dramfreq);

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id sunxi_dramfreq_match[] = {
	{ .compatible = "allwinner,sunxi-dramfreq", },
	{},
};
#else
struct platform_device sunxi_dramfreq_device = {
	.name = "sunxi-dramfreq",
	.id   = -1,
};
#endif

#ifdef CONFIG_PM
static bool sunxi_dramfreq_cur_pause = false;
static int sunxi_dramfreq_suspend(struct platform_device *pdev,
								pm_message_t state)
{
	struct sunxi_dramfreq *dramfreq = platform_get_drvdata(pdev);
	unsigned long cur_freq, target = dramfreq->max;
	int err = -1;

	sunxi_dramfreq_cur_pause = dramfreq->pause;
	if (!sunxi_dramfreq_cur_pause) {
		dramfreq->pause = 1;
		sunxi_dramfreq_get_cur_freq(&pdev->dev, &cur_freq);
		if (cur_freq != target) {
			err = sunxi_dramfreq_target(&pdev->dev, &target, 0);
			if (!err)
				dramfreq->devfreq->previous_freq = target;
		}
	}

	printk("%s:%d\n", __func__, __LINE__);
	return 0;
}

static int sunxi_dramfreq_resume(struct platform_device *pdev)
{
	struct sunxi_dramfreq *dramfreq = platform_get_drvdata(pdev);
	unsigned long cur_freq;

	sunxi_dramfreq_get_cur_freq(&pdev->dev, &cur_freq);
	if (dramfreq->devfreq->previous_freq != cur_freq)
		dramfreq->devfreq->previous_freq = cur_freq;

	if (!sunxi_dramfreq_cur_pause) {
		dramfreq->pause = 0;
		sunxi_dramfreq_hw_init(dramfreq);
	}

	printk("%s:%d\n", __func__, __LINE__);
	return 0;
}
#endif

static struct platform_driver sunxi_dramfreq_driver = {
	.probe  = sunxi_dramfreq_probe,
	.remove = sunxi_dramfreq_remove,
#ifdef CONFIG_PM
	.suspend = sunxi_dramfreq_suspend,
	.resume  = sunxi_dramfreq_resume,
#endif
	.driver = {
		.name  = "sunxi-dramfreq",
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = sunxi_dramfreq_match,
#endif
	},
};

static int __init sunxi_dramfreq_initcall(void)
{
	int ret = 0;

#ifndef CONFIG_OF
	ret = platform_device_register(&sunxi_dramfreq_device);
	if (ret) {
		DRAMFREQ_ERR("Register dramfreq device failed!\n");
		goto out;
	}
#endif

	ret = platform_driver_register(&sunxi_dramfreq_driver);
	if (ret) {
		DRAMFREQ_ERR("Register dramfreq driver failed!\n");
		goto out;
	}

out:
	return ret;
}
fs_initcall(sunxi_dramfreq_initcall);

#ifdef CONFIG_DEBUG_FS
static struct dentry *debugfs_dramfreq_root;

static int dramfreq_debugfs_gettime_show(struct seq_file *s, void *data)
{
	if (!dramfreq)
		seq_printf(s, "Invalid paras\n");

	seq_printf(s, "%lld\n", dramfreq->dramfreq_get_us);
	return 0;
}

static int dramfreq_debugfs_gettime_open(struct inode *inode, struct file *file)
{
	return single_open(file, dramfreq_debugfs_gettime_show, inode->i_private);
}

static const struct file_operations dramfreq_debugfs_gettime_fops = {
	.open = dramfreq_debugfs_gettime_open,
	.read = seq_read,
};

static int dramfreq_debugfs_settime_show(struct seq_file *s, void *data)
{
	if (!dramfreq)
		seq_printf(s, "Invalid paras\n");

	seq_printf(s, "%lld\n", dramfreq->dramfreq_set_us);
	return 0;
}

static int dramfreq_debugfs_settime_open(struct inode *inode, struct file *file)
{
	return single_open(file, dramfreq_debugfs_settime_show, inode->i_private);
}

static const struct file_operations dramfreq_debugfs_settime_fops = {
	.open = dramfreq_debugfs_settime_open,
	.read = seq_read,
};

static int __init dramfreq_debugfs_init(void)
{
	int err = 0;

	debugfs_dramfreq_root = debugfs_create_dir("dramfreq", 0);
	if (!debugfs_dramfreq_root)
		return -ENOMEM;

	if (!debugfs_create_file("get_time", 0444, debugfs_dramfreq_root, NULL,
				&dramfreq_debugfs_gettime_fops)) {
		err = -ENOMEM;
		goto out;
	}

	if (!debugfs_create_file("set_time", 0444, debugfs_dramfreq_root, NULL,
				&dramfreq_debugfs_settime_fops)) {
		err = -ENOMEM;
		goto out;
	}

	return 0;

out:
	debugfs_remove_recursive(debugfs_dramfreq_root);
	return err;
}

static void __exit dramfreq_debugfs_exit(void)
{
	debugfs_remove_recursive(debugfs_dramfreq_root);
}

late_initcall(dramfreq_debugfs_init);
module_exit(dramfreq_debugfs_exit);
#endif /* CONFIG_DEBUG_FS */

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SUNXI dramfreq driver with devfreq framework");
MODULE_AUTHOR("Pan Nan <pannan@allwinnertech.com>");
