/*
 * linux/drivers/devfreq/governor_adaptive.c
 *
 * Copyright(c) 2013-2015 Allwinnertech Co., Ltd.
 *
 * Author: Pan Nan <pannan@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sunxi_dramfreq.h>
#include "governor.h"

static ssize_t show_pause(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	if (!dramfreq)
		return sprintf(buf, "paras error\n");
	return sprintf(buf, "%u\n", dramfreq->pause);
}

static ssize_t store_pause(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	unsigned int value;
	int ret;

	if (!dramfreq) {
		pr_err("%s: paras error\n", __func__);
		ret = -EINVAL;
		goto out;
	}

	ret = sscanf(buf, "%u", &value);
	if (ret != 1)
		goto out;

	if (value && (!dramfreq->pause))
		dramfreq->governor_state_update(STATE_PAUSE);
	else if ((!value) && dramfreq->pause)
		dramfreq->governor_state_update(STATE_RUNNING);

	ret = count;

out:
	return ret;
}

static DEVICE_ATTR(pause, 0644, show_pause, store_pause);
static struct attribute *dev_entries[] = {
	&dev_attr_pause.attr,
	NULL,
};
static struct attribute_group dev_attr_group = {
	.name   = "adaptive",
	.attrs  = dev_entries,
};

static int devfreq_adaptive_func(struct devfreq *df, unsigned long *freq)
{
	if (!dramfreq) {
		pr_err("%s: paras error\n", __func__);
		return -EINVAL;
	}

	if (!dramfreq->pause) {
		if (dramfreq->key_masters[MASTER_DE] == 0 &&
			dramfreq->key_masters[MASTER_GPU] == 0 &&
			dramfreq->key_masters[MASTER_CSI] == 0) {
			*freq = SUNXI_DRAMFREQ_IDLE;
		} else if (dramfreq->key_masters[MASTER_DE] == 1 &&
			dramfreq->key_masters[MASTER_GPU] == 0 &&
			dramfreq->key_masters[MASTER_CSI] == 0) {
			*freq = SUNXI_DRAMFREQ_NORMAL;
		} else {
			*freq = df->max_freq;
		}
	} else {
		*freq = df->max_freq;
	}

	return 0;
}

static int adaptive_init(struct devfreq *devfreq)
{
	int ret = 0;

	if (!devfreq)
		return -EINVAL;

	ret = sysfs_create_group(&devfreq->dev.kobj, &dev_attr_group);
	if (ret)
		return ret;

	if (!dramfreq) {
		pr_err("%s: paras error\n", __func__);
		return -EINVAL;
	}

	dramfreq->governor_state_update(STATE_INIT);

	return 0;
}

static void adaptive_exit(struct devfreq *devfreq)
{
	if (!devfreq)
		return;

	sysfs_remove_group(&devfreq->dev.kobj, &dev_attr_group);

	if (!dramfreq) {
		pr_err("%s: paras error\n", __func__);
		return;
	}

	dramfreq->governor_state_update(STATE_EXIT);
}

static int devfreq_adaptive_handler(struct devfreq *devfreq,
			unsigned int event, void *data)
{
	int ret = 0;

	switch (event) {
	case DEVFREQ_GOV_START:
		ret = adaptive_init(devfreq);
		break;
	case DEVFREQ_GOV_STOP:
		adaptive_exit(devfreq);
		break;
	default:
		break;
	}

	return ret;
}

static struct devfreq_governor devfreq_adaptive = {
	.name = "adaptive",
	.get_target_freq = devfreq_adaptive_func,
	.event_handler = devfreq_adaptive_handler,
};

static int __init devfreq_adaptive_init(void)
{
	return devfreq_add_governor(&devfreq_adaptive);
}
late_initcall(devfreq_adaptive_init);

static void __exit devfreq_adaptive_exit(void)
{
	if (devfreq_remove_governor(&devfreq_adaptive))
		pr_err("%s: failed remove governor\n", __func__);

	return;
}
module_exit(devfreq_adaptive_exit);
MODULE_LICENSE("GPL");
