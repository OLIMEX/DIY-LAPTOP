#ifndef SUNXI_THS_H
#define SUNXI_THS_H

#define MAX_CHN (4)
#define THERMAL_DATA_DELAY	(500)

#define SUNXI_THS_NAME "sunxi_ths_sensor"
#define SUNXI_THS_COMBINE_NAME "sunxi_ths_combine_sensor"

struct sunxi_ths_controller;

enum combine_type{
	COMBINE_MAX_TEMP = 0,
	COMBINE_AVG_TMP,
	COMBINE_MIN_TMP,
};

struct sunxi_ths_controller_ops {
	int (*suspend)(struct sunxi_ths_controller *);
	int (*resume)(struct sunxi_ths_controller *);
	int (*get_temp)(struct sunxi_ths_controller *,u32 id, long *temp);
};

struct sunxi_ths_controller{
	struct device *dev;
	struct sunxi_ths_controller_ops *ops;
	atomic_t initialize;
	atomic_t usage;
	atomic_t is_suspend;
	struct mutex lock;
	struct list_head combine_list;
	struct list_head node;
	void *data;
};

struct sunxi_ths_combine_disc{
	u32  combine_cnt;
	enum combine_type type;
	u32  combine_chn[MAX_CHN];
	struct sunxi_ths_controller *controller;
};

struct sunxi_ths_sensor{
	struct platform_device *pdev;
	u32 sensor_id;
	long last_temp;
	struct sunxi_ths_combine_disc *combine;
	struct thermal_zone_device *tz;
	u32 emulate;
	atomic_t is_suspend;
	struct input_dev *ths_input_dev;
	atomic_t input_delay;
	atomic_t input_enable;
	struct delayed_work input_work;
	struct mutex input_enable_mutex;
	struct list_head node;
};

struct sunxi_ths_controller *
sunxi_ths_controller_register(struct device *dev ,struct sunxi_ths_controller_ops *ops, void *data);
void sunxi_ths_controller_unregister(struct sunxi_ths_controller *controller);

#endif /* SUNXI_THS_H */
