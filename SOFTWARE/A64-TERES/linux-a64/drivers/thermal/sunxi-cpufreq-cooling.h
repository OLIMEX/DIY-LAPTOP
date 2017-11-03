#ifndef __SUNXI_CPUFREQ_COOLING_H__
#define __SUNXI_CPUFREQ_COOLING_H__

#include <linux/thermal.h>
#include <linux/cpufreq.h>

#define CPUFREQ_COOLING_START		0
#define CPUFREQ_COOLING_STOP		1


#define NOTIFY_INVALID NULL
#define INVALID_FREQ    (-1)
#define CLUSTER_MAX	(2)

struct sunxi_cpufreq_cooling_table
{
    u32 cluster_freq[CLUSTER_MAX];
};

struct sunxi_cpufreq_cooling_device {
	s32 id;
	struct thermal_cooling_device *cool_dev;
	u32 cooling_state;
	u32 cluster_freq_limit[CLUSTER_MAX];
	u32 cluster_freq_roof[CLUSTER_MAX];
	u32 cluster_freq_floor[CLUSTER_MAX];
	struct cpumask cluster_cpus[CLUSTER_MAX];
	struct sunxi_cpufreq_cooling_table * tbl;
	u32 tbl_num;
	u32 cluster_num;
	struct notifier_block cpufreq_notifer;
	struct list_head node;
	spinlock_t lock;
};

int sunxi_cpufreq_update_state(struct sunxi_cpufreq_cooling_device *cooling_device);

#endif /* __SUNXI_CPUFREQ_COOLING_H__ */

