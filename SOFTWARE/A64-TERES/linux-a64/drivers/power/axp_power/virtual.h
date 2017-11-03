#ifndef _LINUX_AXP_VIRTUAL_H_
#define _LINUX_AXP_VIRTUAL_H_
#include <linux/regulator/of_regulator.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>

/*
 * struct regulator
 *
 * One for each consumer device.
 */
struct regulator {
	struct device *dev;
	struct list_head list;
	unsigned int always_on:1;
	unsigned int bypass:1;
	int uA_load;
	int min_uV;
	int max_uV;
	char *supply_name;
	struct device_attribute dev_attr;
	struct regulator_dev *rdev;
	struct dentry *debugfs;
};

struct virtual_consumer_data {
	struct mutex lock;
	struct regulator *regulator;
	int enabled;
	int min_uV;
	int max_uV;
	int min_uA;
	int max_uA;
	unsigned int mode;
};

static void update_voltage_constraints(struct virtual_consumer_data *data)
{
	int ret = 0;

	if (data->min_uV && data->max_uV
	    && data->min_uV <= data->max_uV) {
		ret = regulator_set_voltage(data->regulator,
					    data->min_uV, data->max_uV);
		if (ret != 0) {
			printk(KERN_ERR "regulator_set_voltage() failed: %d\n",
			       ret);
			return;
		}
	}

	if (data->min_uV && data->max_uV ) {
		ret = data->regulator->rdev->desc->ops->enable(data->regulator->rdev);
		if (ret != 0)
			printk(KERN_ERR "regulator_enable() failed: %d\n",
				ret);
	}

	if (!(data->min_uV && data->max_uV)) {
		ret = data->regulator->rdev->desc->ops->disable(data->regulator->rdev);
		if (ret != 0)
			printk(KERN_ERR "regulator_disable() failed: %d\n",
				ret);
	}
}

static void update_current_limit_constraints(struct virtual_consumer_data
						*data)
{
	int ret;

	if (data->max_uA
	    && data->min_uA <= data->max_uA) {
		ret = regulator_set_current_limit(data->regulator,
					data->min_uA, data->max_uA);
		if (ret != 0) {
			pr_err("regulator_set_current_limit() failed: %d\n",
			       ret);
			return;
		}
	}

	if (data->max_uA && !data->enabled) {
		ret = regulator_enable(data->regulator);
		if (ret == 0)
			data->enabled = 1;
		else
			printk(KERN_ERR "regulator_enable() failed: %d\n",
				ret);
	}

	if (!(data->min_uA && data->max_uA) && data->enabled) {
		ret = regulator_disable(data->regulator);
		if (ret == 0)
			data->enabled = 0;
		else
			printk(KERN_ERR "regulator_disable() failed: %d\n",
				ret);
	}
}

static ssize_t show_min_uV(struct device *dev,
			   struct device_attribute *attr, char *buf)
{
	struct virtual_consumer_data *data = dev_get_drvdata(dev);
	data->min_uV = regulator_get_voltage(data->regulator);
	return sprintf(buf, "%d\n", data->min_uV);
}

static ssize_t set_min_uV(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct virtual_consumer_data *data = dev_get_drvdata(dev);
	long val;

	if (strict_strtol(buf, 10, &val) != 0)
		return count;

	mutex_lock(&data->lock);

	data->min_uV = val;
	update_voltage_constraints(data);

	mutex_unlock(&data->lock);

	return count;
}

static ssize_t show_max_uV(struct device *dev,
			   struct device_attribute *attr, char *buf)
{
	struct virtual_consumer_data *data = dev_get_drvdata(dev);
	return sprintf(buf, "%d\n", data->max_uV);
}

static ssize_t set_max_uV(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct virtual_consumer_data *data = dev_get_drvdata(dev);
	long val;

	if (strict_strtol(buf, 10, &val) != 0)
		return count;

	mutex_lock(&data->lock);

	data->min_uV = regulator_get_voltage(data->regulator);
	data->max_uV = val;
	update_voltage_constraints(data);

	mutex_unlock(&data->lock);

	return count;
}

static ssize_t show_min_uA(struct device *dev,
			   struct device_attribute *attr, char *buf)
{
	struct virtual_consumer_data *data = dev_get_drvdata(dev);
	return sprintf(buf, "%d\n", data->min_uA);
}

static ssize_t set_min_uA(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct virtual_consumer_data *data = dev_get_drvdata(dev);
	long val;

	if (strict_strtol(buf, 10, &val) != 0)
		return count;

	mutex_lock(&data->lock);

	data->min_uA = val;
	update_current_limit_constraints(data);

	mutex_unlock(&data->lock);

	return count;
}

static ssize_t show_max_uA(struct device *dev,
			   struct device_attribute *attr, char *buf)
{
	struct virtual_consumer_data *data = dev_get_drvdata(dev);
	return sprintf(buf, "%d\n", data->max_uA);
}

static ssize_t set_max_uA(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct virtual_consumer_data *data = dev_get_drvdata(dev);
	long val;

	if (strict_strtol(buf, 10, &val) != 0)
		return count;

	mutex_lock(&data->lock);

	data->max_uA = val;
	update_current_limit_constraints(data);

	mutex_unlock(&data->lock);

	return count;
}

static ssize_t show_mode(struct device *dev,
			 struct device_attribute *attr, char *buf)
{
	struct virtual_consumer_data *data = dev_get_drvdata(dev);

	switch (data->mode) {
	case REGULATOR_MODE_FAST:
		return sprintf(buf, "fast\n");
	case REGULATOR_MODE_NORMAL:
		return sprintf(buf, "normal\n");
	case REGULATOR_MODE_IDLE:
		return sprintf(buf, "idle\n");
	case REGULATOR_MODE_STANDBY:
		return sprintf(buf, "standby\n");
	default:
		return sprintf(buf, "unknown\n");
	}
}

static ssize_t set_mode(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct virtual_consumer_data *data = dev_get_drvdata(dev);
	unsigned int mode;
	int ret;

	if (strncmp(buf, "fast", strlen("fast")) == 0)
		mode = REGULATOR_MODE_FAST;
	else if (strncmp(buf, "normal", strlen("normal")) == 0)
		mode = REGULATOR_MODE_NORMAL;
	else if (strncmp(buf, "idle", strlen("idle")) == 0)
		mode = REGULATOR_MODE_IDLE;
	else if (strncmp(buf, "standby", strlen("standby")) == 0)
		mode = REGULATOR_MODE_STANDBY;
	else {
		dev_err(dev, "Configuring invalid mode\n");
		return count;
	}

	mutex_lock(&data->lock);
	ret = regulator_set_mode(data->regulator, mode);
	if (ret == 0)
		data->mode = mode;
	else
		dev_err(dev, "Failed to configure mode: %d\n", ret);
	mutex_unlock(&data->lock);

	return count;
}

static DEVICE_ATTR(min_microvolts, 0644, show_min_uV, set_min_uV);
static DEVICE_ATTR(max_microvolts, 0644, show_max_uV, set_max_uV);
static DEVICE_ATTR(min_microamps, 0644, show_min_uA, set_min_uA);
static DEVICE_ATTR(max_microamps, 0644, show_max_uA, set_max_uA);
static DEVICE_ATTR(mode, 0644, show_mode, set_mode);

static struct device_attribute *attributes_virtual[] = {
	&dev_attr_min_microvolts,
	&dev_attr_max_microvolts,
	&dev_attr_min_microamps,
	&dev_attr_max_microamps,
	&dev_attr_mode,
};
#endif

