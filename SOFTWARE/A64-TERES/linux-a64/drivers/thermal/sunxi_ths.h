#ifndef SUNXI_THS_H
#define SUNXI_THS_H

#define THS_CLK		 (4000000)

#define THERMAL_DATA_DELAY	(500)

#define THS_INTS_DATA0		(0x100)
#define THS_INTS_DATA1		(0x200)
#define THS_INTS_DATA2		(0x400)
#define THS_INTS_DATA3		(0x800)
#define THS_INTS_SHT0		(0x010)
#define THS_INTS_SHT1		(0x020)
#define THS_INTS_SHT2		(0x040)
#define THS_INTS_SHT3		(0x080)
#define THS_INTS_ALARM0		(0x001)
#define THS_INTS_ALARM1		(0x002)
#define THS_INTS_ALARM2		(0x004)
#define THS_INTS_ALARM3		(0x008)

#define SUNXI_THS_NAME "sunxi_ths_sensor"

#define thsprintk(level_mask, fmt, arg...)	if (unlikely(thermal_debug_mask & level_mask)) \
	printk(fmt , ## arg)

enum {
	DEBUG_INIT = 1U << 0,
	DEBUG_INT = 1U << 1,
	DEBUG_DATA_INFO = 1U << 2,
	DEBUG_SUSPEND = 1U << 3,
	DEBUG_ERR = 1U << 4,
};

enum ths_mode{
	MAX_TEMP = 0,
	AVG_TMP,
	MIN_TMP,
};

struct sunxi_ths_data {
	void __iomem *base_addr;
	struct platform_device *pdev;
	struct clk *mclk;
	struct clk *pclk;
	int irq_num;
	int int_temp;
	int sensor_cnt;
	enum ths_mode mode;
	struct sunxi_ths_sensor_ops *ops;
	struct thermal_zone_device *tz;
	struct work_struct  irq_work;
	struct input_dev *ths_input_dev;
	atomic_t input_delay;
	atomic_t input_enable;
	struct delayed_work input_work;
	struct mutex input_enable_mutex;
	void *data;
#ifdef CONFIG_PM
	struct dev_pm_domain ths_pm_domain;
#endif
};

struct sunxi_ths_sensor_ops {
	int (*init_reg)(struct sunxi_ths_data *);
	int (*clear_reg)(struct sunxi_ths_data *);
	int (*enable)(struct sunxi_ths_data *);
	int (*disable)(struct sunxi_ths_data *);
	long (*get_temp)(struct sunxi_ths_data *, u32);
	int (*get_int)(struct sunxi_ths_data *);
	void (*clear_int)(struct sunxi_ths_data *);
};

#endif //SUNXI_THS_H
