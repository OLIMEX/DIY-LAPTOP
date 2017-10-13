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
#include "thermal_core.h"
#include "sunxi-cpufreq-cooling.h"

#define BOOT_CPU    0
#define SUNXI_CPUFREQ_COOLING_NAME "sunxi_cpufreq_cool"

static struct sunxi_cpufreq_cooling_device *cpufreq_cdev;

static int is_cpufreq_valid(unsigned int cpu)
{
	struct cpufreq_policy policy;
	return !cpufreq_get_policy(&policy, cpu);
}

int sunxi_cpufreq_update_state(struct sunxi_cpufreq_cooling_device *cooling_device)
{
	int ret = 0,cluster;
	unsigned int cpuid;

	struct cpufreq_policy policy;
	
	for(cluster = 0; cluster < cpufreq_cdev->cluster_num; cluster++){
		for_each_cpu(cpuid, &cooling_device->cluster_cpus[cluster]) {
			if (is_cpufreq_valid(cpuid))
		        {
				if((cpufreq_get_policy(&policy, cpuid) == 0) && 
							policy.user_policy.governor){
					cpufreq_update_policy(cpuid);
					break;
				}
		        }
		}
	}

	return ret;
}
EXPORT_SYMBOL(sunxi_cpufreq_update_state);

static int cpu_budget_apply_cooling(struct sunxi_cpufreq_cooling_device *cooling_device,
				unsigned long cooling_state)
{
	unsigned long flags;
	int cluster;
    
    	/* struct thermal_instance *instance; */
    	/* int temperature = 0; */

	/* Check if the old cooling action is same as new cooling action */
	if (cooling_device->cooling_state == cooling_state)
		return 0;
	cooling_device->cooling_state = cooling_state;

    	if(cooling_state >= cooling_device->tbl_num)
		return -EINVAL;

	spin_lock_irqsave(&cooling_device->lock, flags);
	for(cluster = 0; cluster < cooling_device->cluster_num; cluster++)
		cooling_device->cluster_freq_limit[cluster] = cooling_device->tbl[cooling_state].cluster_freq[cluster];
	spin_unlock_irqrestore(&cooling_device->lock, flags);
/*
    	trace_cpu_budget_throttle(cooling_state,
		cooling_device->cluster0_freq_limit,
		cooling_device->cluster0_num_limit ,
 		cooling_device->cluster1_freq_limit,
		cooling_device->cluster1_num_limit,
		cooling_device->gpu_throttle);
	list_for_each_entry(instance, &(cooling_device->cool_dev->thermal_instances), cdev_node) {
		if(instance->tz->temperature > temperature)
		temperature = instance->tz->temperature;
    	}
	pr_info("CPU Budget: Temperature: %u Limit state:%lu item[%d,%d,%d,%d %d]\n",temperature,cooling_state,
	cooling_device->cluster0_freq_limit,
	cooling_device->cluster0_num_limit ,
	cooling_device->cluster1_freq_limit ,
	cooling_device->cluster1_num_limit,
	cooling_device->gpu_throttle);
*/
	return sunxi_cpufreq_update_state(cooling_device);
}

static int cpufreq_thermal_notifier(struct notifier_block *nb,
					unsigned long event, void *data)
{
	struct cpufreq_policy *policy = data;
	int cluster = 0;
	unsigned long limit_freq=0,base_freq=0,head_freq=0;
	unsigned long max_freq=0,min_freq=0;

	if (event != CPUFREQ_ADJUST || cpufreq_cdev == NOTIFY_INVALID)
		return 0;

	while(cluster < cpufreq_cdev->cluster_num){
		if (cpumask_test_cpu(policy->cpu, &cpufreq_cdev->cluster_cpus[cluster])){
                	limit_freq = cpufreq_cdev->cluster_freq_limit[cluster];
                	base_freq = cpufreq_cdev->cluster_freq_floor[cluster];
                	head_freq = cpufreq_cdev->cluster_freq_roof[cluster];
			break;
		}else
			cluster ++;
	}
	if(cluster == cpufreq_cdev->cluster_num)
		return 0;

	if(limit_freq && limit_freq != INVALID_FREQ)
	{
		max_freq =(head_freq >= limit_freq)?limit_freq:head_freq;
		min_freq = base_freq;
		/* Never exceed policy.max*/
		if (max_freq > policy->max)
			max_freq = policy->max;
		if (min_freq < policy->min)
		{
			min_freq = policy->min;
		}
		min_freq = (min_freq < max_freq)?min_freq:max_freq;
		if (policy->max != max_freq || policy->min != min_freq )
		{
			cpufreq_verify_within_limits(policy, min_freq, max_freq);
				policy->user_policy.max = policy->max;
			pr_info("CPU Budget:update CPU %d cpufreq max to %lu min to %lu\n",policy->cpu,max_freq, min_freq);
		}
	}
	return 0;
}

/*
 * cpufreq cooling device callback functions are defined below
 */

/**
 * cpu_budget_get_max_state - callback function to get the max cooling state.
 * @cdev: thermal cooling device pointer.
 * @state: fill this variable with the max cooling state.
 */
static int cpu_freq_get_max_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{
	struct sunxi_cpufreq_cooling_device *cooling_device = cdev->devdata;
	*state = cooling_device->tbl_num-1;
	return 0;
}

/**
 * cpu_budget_get_cur_state - callback function to get the current cooling state.
 * @cdev: thermal cooling device pointer.
 * @state: fill this variable with the current cooling state.
 */
static int cpu_freq_get_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{
	struct sunxi_cpufreq_cooling_device *cooling_device = cdev->devdata;
	*state = cooling_device->cooling_state;
	return 0;
}

/**
 * cpu_budget_set_cur_state - callback function to set the current cooling state.
 * @cdev: thermal cooling device pointer.
 * @state: set this variable to the current cooling state.
 */
static int cpu_freq_set_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long state)
{
	struct sunxi_cpufreq_cooling_device *cooling_device = cdev->devdata;
	return cpu_budget_apply_cooling(cooling_device, state);
}

/* Bind cpufreq callbacks to thermal cooling device ops */
static struct thermal_cooling_device_ops const sunxi_cpu_cooling_ops = {
	.get_max_state = cpu_freq_get_max_state,
	.get_cur_state = cpu_freq_get_cur_state,
	.set_cur_state = cpu_freq_set_cur_state,
};

static int sunxi_cpufreq_cooling_get_cpu(void)
{
	int cluster,i;
	unsigned int min = 0, max = 0;
	struct cpufreq_policy policy;
#if defined(CONFIG_SCHED_HMP)
	arch_get_fast_and_slow_cpus(&cpufreq_cdev->cluster_cpus[1],&cpufreq_cdev->cluster_cpus[0]);
#elif defined(CONFIG_SCHED_SMP_DCMP)
	if (strlen(CONFIG_CLUSTER0_CPU_MASK) && strlen(CONFIG_CLUSTER1_CPU_MASK)) {
		if (cpulist_parse(CONFIG_CLUSTER0_CPU_MASK, &cpufreq_cdev->cluster_cpus[0])) {
			pr_err("Failed to parse cluster0 cpu mask!\n");
			return -EBUSY;
		}
		if (cpulist_parse(CONFIG_CLUSTER1_CPU_MASK, &cpufreq_cdev->cluster_cpus[1])) {
			pr_err("Failed to parse cluster1 cpu mask!\n");
			return -EBUSY;
		}
	}
#else
        cpumask_copy(&cpufreq_cdev->cluster_cpus[0], cpu_possible_mask);
	cpufreq_cdev->cluster_num = 1;
#endif
	for(cluster = 0; cluster < cpufreq_cdev->cluster_num; min = 0, max = 0, cluster ++){
		for_each_cpu(i, &cpufreq_cdev->cluster_cpus[cluster]) {
			/*continue if cpufreq policy not found and not return error*/
			if (!cpufreq_get_policy(&policy, i))
				continue;
			if (min == 0 && max == 0) {
				min = policy.cpuinfo.min_freq;
				max = policy.cpuinfo.max_freq;
			} else {
				if (min != policy.cpuinfo.min_freq ||
					max != policy.cpuinfo.max_freq){
						pr_err("different freq, return.\n");
						return -EBUSY;
					}
			}
		}
	}
	return 0;
}

static int sunxi_cpufreq_cooling_parse(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	struct sunxi_cpufreq_cooling_table *ctbl = NULL;
	int i,j,cluster;
	int ret = 0;
	char name[32];

	np = pdev->dev.of_node;

	if (!of_device_is_available(np)) {
		pr_err("%s: cpufreq cooling is disable", __func__);
		return -EPERM;
	}

	cpufreq_cdev = kzalloc(sizeof(*cpufreq_cdev), GFP_KERNEL);
	if (IS_ERR_OR_NULL(cpufreq_cdev)) {
		pr_err("cooling_dev: not enough memory for cpufreq cooling data\n");
		return -ENOMEM;
	}

	if (of_property_read_u32(np, "cluster_num", &cpufreq_cdev->cluster_num)) {
		pr_err("%s: get cluster_num failed", __func__);
		ret =  -EBUSY;
		goto parse_fail_1;
	}
	if (of_property_read_u32(np, "state_cnt", &cpufreq_cdev->tbl_num)) {
		pr_err("%s: get state_cnt failed", __func__);
		ret =  -EBUSY;
		goto parse_fail_1;
	}
	
	ret = sunxi_cpufreq_cooling_get_cpu();
	if(ret)
		goto parse_fail_1;

	ctbl = kzalloc(cpufreq_cdev->tbl_num * sizeof(*ctbl), GFP_KERNEL);
	if (!ctbl) {
		ret = -ENOMEM;
		goto parse_fail_1;
	}

	for(i = 0; i < cpufreq_cdev->tbl_num; i++){
		sprintf(name, "state%d", i);
		for(j = 0; j < cpufreq_cdev->cluster_num; j++){
			if (of_property_read_u32_index(np, (const char *)&name,
					j, &(ctbl[i].cluster_freq[j]))) {
			pr_err("node %s get failed!\n", name);
			ret = -EBUSY;
			goto parse_fail_0;
			}
		}
	}
	cpufreq_cdev->tbl = ctbl;

	for(cluster = 0; cluster < cpufreq_cdev->cluster_num; cluster ++){
			cpufreq_cdev->cluster_freq_limit[cluster] = cpufreq_cdev->tbl[0].cluster_freq[cluster];
			cpufreq_cdev->cluster_freq_roof[cluster] = cpufreq_cdev->cluster_freq_limit[cluster];
	}

	return 0;
parse_fail_0:
	kfree(ctbl);
parse_fail_1:
	kfree(cpufreq_cdev);
	return ret;
	
}

static int sunxi_cpufreq_cooling_probe(struct platform_device *pdev)
{
	s32 err = 0;
	struct thermal_cooling_device *cool_dev;

	pr_info("sunxi cpufreq cooling probe start !\n");

	if (pdev->dev.of_node) {
		/* get dt and sysconfig */
		err = sunxi_cpufreq_cooling_parse(pdev);
	}else{
		pr_err("sunxi cpufreq cooling device tree err!\n");
		return -EBUSY;
	}
	if(err){
		pr_err("sunxi cpufreq cooling device tree parse err!\n");
		return -EBUSY;
	}

	spin_lock_init(&cpufreq_cdev->lock);
	
	cool_dev = thermal_of_cooling_device_register(pdev->dev.of_node, SUNXI_CPUFREQ_COOLING_NAME, 
						cpufreq_cdev, &sunxi_cpu_cooling_ops);
	if (!cool_dev)
		goto fail;
	cpufreq_cdev->cool_dev = cool_dev;
	cpufreq_cdev->cooling_state = 0;
	cpufreq_cdev->cpufreq_notifer.notifier_call=cpufreq_thermal_notifier;
	cpufreq_register_notifier(&(cpufreq_cdev->cpufreq_notifer),
						CPUFREQ_POLICY_NOTIFIER);
	pr_info("CPU freq cooling register Success\n");
	return 0;	
fail:	
	kfree(cpufreq_cdev->tbl);
	kfree(cpufreq_cdev);
	return -EBUSY;
}

static int sunxi_cpufreq_cooling_remove(struct platform_device *pdev)
{
	cpufreq_unregister_notifier(&(cpufreq_cdev->cpufreq_notifer),
						CPUFREQ_POLICY_NOTIFIER);
	pr_info("CPU freq notifer unregister Success\n");
	thermal_cooling_device_unregister(cpufreq_cdev->cool_dev);
	kfree(cpufreq_cdev->tbl);
	kfree(cpufreq_cdev);
	pr_info("CPU freq cooling unregister Success\n");
	return 0;
}

#ifdef CONFIG_OF
/* Translate OpenFirmware node properties into platform_data */
static struct of_device_id sunxi_cpufreq_cooling_of_match[] = {
	{ .compatible = "allwinner,cpufreq_cooling", },
	{ },
};
MODULE_DEVICE_TABLE(of, sunxi_cpufreq_cooling_of_match);
#else /* !CONFIG_OF */
#endif


static struct platform_driver sunxi_cpufreq_cooling_driver = {
	.probe  = sunxi_cpufreq_cooling_probe,
	.remove = sunxi_cpufreq_cooling_remove,
	.driver = {
		.name   = SUNXI_CPUFREQ_COOLING_NAME,
		.owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(sunxi_cpufreq_cooling_of_match),
	}
};
module_platform_driver(sunxi_cpufreq_cooling_driver);
MODULE_DESCRIPTION("SUNXI cpufreq cooling driver");
MODULE_AUTHOR("QIn");
MODULE_LICENSE("GPL v2");


