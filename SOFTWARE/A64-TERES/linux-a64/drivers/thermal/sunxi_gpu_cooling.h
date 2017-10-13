#ifndef SUNXI_GPU_COOLING_H
#define SUNXI_GPU_COOLING_H

#include <linux/thermal.h>

#define GPU_FREQ_TABLE_MAX	(10)

struct sunxi_gpu_cooling_device {
	struct device *dev;
	struct thermal_cooling_device *cool_dev;
	int (*cool) (int);
	u32 cooling_state;
	u32 state_num;
	u32 gpu_freq_limit;
	u32 gpu_freq_roof;
	u32 gpu_freq_floor;
	u32 freq_table[GPU_FREQ_TABLE_MAX];
	spinlock_t lock;
};

#endif /* SUNXI_GPU_COOLING_H */
