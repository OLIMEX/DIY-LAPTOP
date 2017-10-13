#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cpufreq.h>
#include <linux/thermal.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/cpufreq.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/cpu.h>
#include <linux/cpumask.h>
#include "sunxi-budget-cooling.h"

static int boot_cpu;
static u32 old_cooling_state;
#ifdef CONFIG_CPU_AUTOHOTPLUG_ROOMAGE
extern int autohotplug_roomage_limit(unsigned int cluster_id, unsigned int min, unsigned int max);
#endif

static int budget_get_any_online_cpu(const cpumask_t *mask)
{
	int cpu,lastcpu=0xffff;

	for_each_cpu(cpu, mask) {
		if ((cpu != boot_cpu) && cpu_online(cpu)){
			if(lastcpu == 0xffff)
				lastcpu = cpu;
			else if(cpu >lastcpu)
				lastcpu = cpu;
		}
	}
	return lastcpu;
}

static int budget_get_any_offline_cpu(const cpumask_t *mask)
{
	int cpu, lastcpu = 0xffff;

	for_each_cpu(cpu, mask) {
		if (!cpu_online(cpu)) {
			if (lastcpu == 0xffff)
				lastcpu = cpu;
			else if (cpu > lastcpu)
				lastcpu = cpu;
		}
	}
	return lastcpu;
}

static int budget_get_online_cpus(const cpumask_t *mask)
{
	int cpu,num =0;

	for_each_cpu(cpu, mask) {
		if (cpu_online(cpu))
			num++;
	}
	return num;
}

int sunxi_hotplug_update_state(struct sunxi_budget_cooling_device *cooling_device, u32 cluster)
{
	s32 ret = 0;
	u32 online = 0, i = 0, cpuid;
	u32 max, min;
	u32 takedown, bringup = 0;
	u32 cooling_state = cooling_device->cooling_state;
	unsigned long cpuid_l, flags;
	struct sunxi_budget_hotplug *hotplug = cooling_device->hotplug;
	if(NULL == hotplug)
		return 0;

	spin_lock_irqsave(&hotplug->lock, flags);

	hotplug->cluster_num_limit[cluster] = hotplug->tbl[cooling_state].cluster_num[cluster];

	spin_unlock_irqrestore(&hotplug->lock, flags);

	max = (hotplug->cluster_num_roof[cluster] >=hotplug->cluster_num_limit[cluster])?
			hotplug->cluster_num_limit[cluster]:hotplug->cluster_num_roof[cluster];
	min = (hotplug->cluster_num_floor[cluster] >=max)?
			max:hotplug->cluster_num_floor[cluster];
#ifdef CONFIG_CPU_AUTOHOTPLUG_ROOMAGE
	pr_info("CPU Budget hotplug: cluster%d min:%d max:%d\n",cluster, min, max);
	autohotplug_roomage_limit(cluster, min, max);
#endif

	for_each_online_cpu(i){
		if (cpumask_test_cpu(i, &cooling_device->cluster_cpus[cluster]))
				online++;
	}

	takedown = (online > max)?(online - max):0;

	if ((cooling_state < old_cooling_state) && (takedown == 0))
	{
		/* pr_info("CPU Budget:plugging cores, old state %d, new state %d\n",old_cooling_state,cooling_state); */
		switch (cooling_state)
		{
		case 2:
		case 1:
		case 0:
			bringup = (online < max)?(max - online):0;
			break;
		}
	}
	old_cooling_state = cooling_state;

	while(takedown){
		cpuid = budget_get_any_online_cpu(&cooling_device->cluster_cpus[cluster]);
		if (cpuid < nr_cpu_ids){
			pr_info("CPU Budget:Try to down cpu %d, cluster%d online %d, max %d\n",cpuid,cluster,online,max);
			cpuid_l = cpuid;
			ret = work_on_cpu(boot_cpu, (long(*)(void *))cpu_down, (void *)cpuid_l);
		}
		takedown--;
	}

	while (bringup) {
		cpuid = budget_get_any_offline_cpu(&cooling_device->cluster_cpus[cluster]);
		if (cpuid < nr_cpu_ids) {
			pr_info("CPU Budget:Try to up cpu %d, cluster%d online %d, max %d\n", cpuid, cluster, online, max);
			cpuid_l = cpuid;
			ret = work_on_cpu(boot_cpu, (long(*)(void *))cpu_up, (void *)cpuid_l);
			if (unlikely(ret))
				pr_err("CPU Budget:Failed to bring up cpu %d\n", cpuid);
		}
		bringup--;
	}

	return ret;
}
EXPORT_SYMBOL(sunxi_hotplug_update_state);

int sunxi_hotplug_get_roomage(struct sunxi_budget_cooling_device *cooling_device,
				u32 *num_floor, u32 *num_roof, u32 cluster)
{
	struct sunxi_budget_hotplug *hotplug = cooling_device->hotplug;
	if(NULL == hotplug)
		return 0;
	*num_floor = hotplug->cluster_num_floor[cluster];
	*num_roof = hotplug->cluster_num_roof[cluster];
	return 0;
}
EXPORT_SYMBOL(sunxi_hotplug_get_roomage);

int sunxi_hotplug_set_roomage(struct sunxi_budget_cooling_device *cooling_device,
				u32 num_floor, u32 num_roof, u32 cluster)
{
	unsigned long flags;
	struct sunxi_budget_hotplug *hotplug = cooling_device->hotplug;
	if(NULL == hotplug)
		return 0;
	spin_lock_irqsave(&hotplug->lock, flags);

	hotplug->cluster_num_floor[cluster] = num_floor;
	hotplug->cluster_num_roof[cluster] = num_roof;

	spin_unlock_irqrestore(&hotplug->lock, flags);
	sunxi_hotplug_update_state(cooling_device, cluster);
	return 0;
}
EXPORT_SYMBOL(sunxi_hotplug_set_roomage);

static int hotplug_thermal_notifier(struct notifier_block *nfb,
					unsigned long action, void *hcpu)
{
	struct sunxi_budget_hotplug *hotplug = container_of(nfb ,struct sunxi_budget_hotplug, notifer);
	struct sunxi_budget_cooling_device *bcd = hotplug->bcd;
	u32 i;
	u32 online=0;
	u32 max=0;
	u32 cpu = (unsigned long)hcpu;

	switch (action) {
	case CPU_UP_PREPARE:
	for(i = 0; i < CLUSTER_MAX; i++){
		if (!cpumask_test_cpu(cpu, &bcd->cluster_cpus[i]))
			continue;
		online = budget_get_online_cpus(&bcd->cluster_cpus[i]);
		if(hotplug->cluster_num_roof[i] >= hotplug->cluster_num_limit[i])
			max = hotplug->cluster_num_limit[i];
		else
			max = hotplug->cluster_num_roof[i];
		if(online >= max){
			pr_info("CPU Budget:reject cpu %d to up, cluster%d online %d, limit %d\n",cpu,i,online,max);
			return NOTIFY_BAD;
		}
	}
	return NOTIFY_DONE;
	default:
		break;
	}

	return NOTIFY_DONE;
}

/**
 * sunxi_hotplug_cooling_parse - parse the hotplug limit value and fill in struct sunxi_hotplug_cooling_table
 */
static struct sunxi_hotplug_cooling_table *
sunxi_hotplug_cooling_parse(struct device_node *np, u32 tbl_num, u32 cluster_num)
{
	struct sunxi_hotplug_cooling_table *tbl;
	u32 i, j, ret = 0;
	char name[32];

	tbl = kzalloc(tbl_num * sizeof(*tbl), GFP_KERNEL);
	if (IS_ERR_OR_NULL(tbl)) {
		pr_err("cooling_dev: not enough memory for hotplug cooling table\n");
		return NULL;
	}
	for(i = 0; i < tbl_num; i++){
		sprintf(name, "state%d", i);
		for(j = 0; j < cluster_num; j++){
			if (of_property_read_u32_index(np, (const char *)&name,
					(j * 2 + 1), &(tbl[i].cluster_num[j]))) {
				pr_err("node %s get failed!\n", name);
				ret = -EBUSY;
			}
		}
	}
	if(ret){
		kfree(tbl);
		tbl = NULL;
	}
	return tbl;
}

struct sunxi_budget_hotplug *
sunxi_hotplug_cooling_register(struct sunxi_budget_cooling_device *bcd)
{
	struct sunxi_budget_hotplug *hotplug;
	struct sunxi_hotplug_cooling_table *tbl;
	struct device_node *np = bcd->dev->of_node;
	u32 cluster;

	hotplug = kzalloc(sizeof(*hotplug), GFP_KERNEL);
	if (IS_ERR_OR_NULL(hotplug)) {
		pr_err("cooling_dev: not enough memory for hotplug cooling data\n");
		goto fail;
	}

	tbl = sunxi_hotplug_cooling_parse(np, bcd->state_num, bcd->cluster_num);
	if(!tbl){
		kfree(hotplug);
		goto fail;
	}
	hotplug->tbl_num = bcd->state_num;
	hotplug->tbl = tbl;
	spin_lock_init(&hotplug->lock);
	for(cluster = 0; cluster < bcd->cluster_num; cluster ++){
			hotplug->cluster_num_limit[cluster] = hotplug->tbl[0].cluster_num[cluster];
			hotplug->cluster_num_roof[cluster] = hotplug->cluster_num_limit[cluster];
	}

	hotplug->notifer.notifier_call=hotplug_thermal_notifier;
	register_cpu_notifier(&(hotplug->notifer));

	hotplug->bcd = bcd;
	boot_cpu = cpumask_first(cpu_online_mask);
	pr_info("CPU hotplug cooling register Success\n");
	return hotplug;
fail:
	return NULL;
}
EXPORT_SYMBOL(sunxi_hotplug_cooling_register);

void sunxi_hotplug_cooling_unregister(struct sunxi_budget_cooling_device *bcd)
{
	if(!bcd->hotplug)
		return;
	unregister_cpu_notifier(&(bcd->hotplug->notifer));
	kfree(bcd->hotplug->tbl);
	kfree(bcd->hotplug);
	bcd->hotplug = NULL;
	return;
}
EXPORT_SYMBOL(sunxi_hotplug_cooling_unregister);

