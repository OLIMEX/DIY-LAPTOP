#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/input.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/thermal.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#ifdef CONFIG_PM
#include <linux/pm.h>
#endif
#include "sunxi_ths.h"

#define thsprintk(level_mask, fmt, arg...)	if (unlikely(thermal_debug_mask & level_mask)) \
	printk(fmt , ## arg)

enum {
	DEBUG_INIT = 1U << 0,
	DEBUG_INT = 1U << 1,
	DEBUG_DATA_INFO = 1U << 2,
	DEBUG_SUSPEND = 1U << 3,
	DEBUG_ERR = 1U << 4,
};

static u32 thermal_debug_mask = 0;

#define THERMAL_DATA_DELAY	(500)

static LIST_HEAD(controller_list);
static DEFINE_MUTEX(controller_list_lock);

static const char * const combine_types[] = {
	[COMBINE_MAX_TEMP]	= "max",
	[COMBINE_AVG_TMP]	= "avg",
	[COMBINE_MIN_TMP]	= "min",
};

static enum combine_type get_combine_type(const char *t)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(combine_types); i++)
		if (!strcasecmp(t, combine_types[i])) {
			return i;
		}
	return 0;
}

static ssize_t sunxi_ths_input_delay_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct sunxi_ths_sensor *sensor = dev_get_drvdata(dev);
	thsprintk(DEBUG_DATA_INFO, "%d, %s\n", atomic_read(&sensor->input_delay), __FUNCTION__);
	return sprintf(buf, "%d\n", atomic_read(&sensor->input_delay));
}

static ssize_t sunxi_ths_input_delay_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct sunxi_ths_sensor *sensor = dev_get_drvdata(dev);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if (data > THERMAL_DATA_DELAY)
		data = THERMAL_DATA_DELAY;
	atomic_set(&sensor->input_delay, (unsigned int) data);

	return count;
}

static ssize_t sunxi_ths_input_enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct sunxi_ths_sensor *sensor = dev_get_drvdata(dev);
	thsprintk(DEBUG_DATA_INFO, "%d, %s\n", atomic_read(&sensor->input_enable), __FUNCTION__);
	return sprintf(buf, "%d\n", atomic_read(&sensor->input_enable));
}

static void sunxi_ths_input_set_enable(struct device *dev, int enable)
{
	struct sunxi_ths_sensor *sensor = dev_get_drvdata(dev);
	int pre_enable = atomic_read(&sensor->input_enable);

	mutex_lock(&sensor->input_enable_mutex);
	if (enable) {
		if (pre_enable == 0) {
			schedule_delayed_work(&sensor->input_work,
				msecs_to_jiffies(atomic_read(&sensor->input_delay)));
			atomic_set(&sensor->input_enable, 1);
		}

	} else {
		if (pre_enable == 1) {
			cancel_delayed_work_sync(&sensor->input_work);
			atomic_set(&sensor->input_enable, 0);
		}
	}
	mutex_unlock(&sensor->input_enable_mutex);
}

static ssize_t sunxi_ths_input_enable_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if ((data == 0)||(data==1)) {
		sunxi_ths_input_set_enable(dev,data);
	}

	return count;
}

static ssize_t sunxi_ths_show_emu(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct sunxi_ths_sensor *sensor = dev_get_drvdata(dev);
	thsprintk(DEBUG_DATA_INFO, "%d, %s\n", sensor->emulate, __FUNCTION__);
	return sprintf(buf, "%d\n", sensor->emulate);
}

static ssize_t sunxi_ths_set_emu(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct sunxi_ths_sensor *sensor = dev_get_drvdata(dev);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	sensor->emulate= (unsigned int) data;

	return count;
}

static ssize_t sunxi_ths_set_emutemp(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct sunxi_ths_sensor *sensor = dev_get_drvdata(dev);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	sensor->last_temp= data;

	return count;
}

static DEVICE_ATTR(delay, S_IRUGO|S_IWUSR|S_IWGRP,
		sunxi_ths_input_delay_show, sunxi_ths_input_delay_store);
static DEVICE_ATTR(enable, S_IRUGO|S_IWUSR|S_IWGRP,
		sunxi_ths_input_enable_show, sunxi_ths_input_enable_store);
static DEVICE_ATTR(emulate, S_IRUGO|S_IWUSR|S_IWGRP,
		sunxi_ths_show_emu, sunxi_ths_set_emu);
static DEVICE_ATTR(temperature, S_IRUGO|S_IWUSR|S_IWGRP,
		NULL, sunxi_ths_set_emutemp);

static struct attribute *sunxi_ths_input_attributes[] = {
	&dev_attr_delay.attr,
	&dev_attr_enable.attr,
	&dev_attr_emulate.attr,
	&dev_attr_temperature.attr,
	NULL
};

static struct attribute_group sunxi_ths_input_attribute_group = {
	.attrs = sunxi_ths_input_attributes
};

static void ths_combine_input_work_func(struct work_struct *work)
{
	static long tempetature = 5;
	struct sunxi_ths_sensor *sensor = container_of((struct delayed_work *)work,
			struct sunxi_ths_sensor, input_work);
	unsigned long delay = msecs_to_jiffies(atomic_read(&sensor->input_delay));

	thermal_zone_get_temp(sensor->tz, &tempetature);
	input_report_abs(sensor->ths_input_dev, ABS_MISC, tempetature);
	input_sync(sensor->ths_input_dev);
	thsprintk(DEBUG_DATA_INFO, "%s: temperature %ld\n", __func__, tempetature);

	schedule_delayed_work(&sensor->input_work, delay);
}

static int sunxi_combine_input_init(struct sunxi_ths_sensor *sensor)
{
	int err = 0;

	sensor->ths_input_dev = input_allocate_device();
	if (IS_ERR_OR_NULL(sensor->ths_input_dev)) {
		printk(KERN_ERR "ths combine: not enough memory for input device\n");
		err = -ENOMEM;
		goto fail1;
	}

	sensor->ths_input_dev->name = "sunxi-ths";
	sensor->ths_input_dev->phys = "sunxiths/input0";
	sensor->ths_input_dev->id.bustype = BUS_HOST;
	sensor->ths_input_dev->id.vendor = 0x0001;
	sensor->ths_input_dev->id.product = 0x0001;
	sensor->ths_input_dev->id.version = 0x0100;

	input_set_capability(sensor->ths_input_dev, EV_ABS, ABS_MISC);
	input_set_abs_params(sensor->ths_input_dev, ABS_MISC, -50, 180, 0, 0);

	err = input_register_device(sensor->ths_input_dev);
	if (0 < err) {
		pr_err("%s: could not register input device\n", __func__);
		input_free_device(sensor->ths_input_dev);
		goto fail2;
	}

	INIT_DELAYED_WORK(&sensor->input_work, ths_combine_input_work_func);

	mutex_init(&sensor->input_enable_mutex);
	atomic_set(&sensor->input_enable, 0);
	atomic_set(&sensor->input_delay, THERMAL_DATA_DELAY);
	input_set_drvdata(sensor->ths_input_dev,sensor);

	err = sysfs_create_group(&sensor->ths_input_dev->dev.kobj,
						 &sunxi_ths_input_attribute_group);
	if (err < 0)
	{
		pr_err("%s: sysfs_create_group err\n", __func__);
		goto fail3;
	}

	return err;
fail3:
	input_unregister_device(sensor->ths_input_dev);
fail2:
	kfree(sensor->ths_input_dev);
fail1:
	return err;
}

static void sunxi_combine_input_exit(struct sunxi_ths_sensor *sensor)
{
	//sysfs_remove_group(&data->ths_input_dev->dev.kobj, &sunxi_ths_input_attribute_group);
	cancel_delayed_work_sync(&sensor->input_work);
	input_unregister_device(sensor->ths_input_dev);
}

static int sunxi_combine_get_temp(void *data, long *temperature)
{
	struct sunxi_ths_sensor *sensor = data;
	struct sunxi_ths_controller *controller = sensor->combine->controller;
	int i, ret, is_suspend, emulate;
	u32 chn;
	long temp = 0, taget;

	emulate = sensor->emulate;
	is_suspend = atomic_read(&sensor->is_suspend);

	if((!is_suspend) && (!emulate)){
		switch(sensor->combine->type){
		case COMBINE_MAX_TEMP:
			for(i = 0, taget = -40; i < sensor->combine->combine_cnt; i++){
				chn = sensor->combine->combine_chn[i];
				ret = controller->ops->get_temp(controller, chn, &temp);
				if(ret)
					return ret;
				if(temp > taget)
					taget = temp;
			}
			break;
		case COMBINE_AVG_TMP:
			for(i = 0, taget = 0; i < sensor->combine->combine_cnt; i++){
				chn = sensor->combine->combine_chn[i];
				ret = controller->ops->get_temp(controller, chn, &temp);
				if(ret)
					return ret;
				taget += temp;
			}
			do_div(taget, sensor->combine->combine_cnt);
			break;
		case COMBINE_MIN_TMP:
			for(i = 0, taget = 180; i < sensor->combine->combine_cnt; i++){
				chn = sensor->combine->combine_chn[i];
				ret = controller->ops->get_temp(controller, chn, &temp);
				if(ret)
					return ret;
				if(temp < taget)
					taget = temp;
			}
			break;
		default:
			break;
		}
		*temperature = taget;
		sensor->last_temp = taget;
	}else{
		*temperature = sensor->last_temp;
	}
	thsprintk(DEBUG_DATA_INFO, "%s: get temp %ld\n", __func__, (*temperature));
	return 0;
}

static struct sunxi_ths_controller *
combine_find_controller(struct device_node *np)
{
	struct sunxi_ths_controller *controller;
	struct device_node *pnp = of_get_parent(np);
	if (IS_ERR_OR_NULL(pnp)) {
		pr_err("ths combine: get prent err\n");
		return NULL;
	}
	mutex_lock(&controller_list_lock);
	list_for_each_entry(controller, &controller_list, node){
		if(pnp == controller->dev->of_node){
			mutex_unlock(&controller_list_lock);
			return controller;
		}
	}
	mutex_unlock(&controller_list_lock);
	return NULL;
}

struct sunxi_ths_controller *
sunxi_ths_controller_register(struct device *dev ,struct sunxi_ths_controller_ops *ops, void *data)
{
	struct sunxi_ths_controller *controller = NULL;
	struct device_node *np = dev->of_node;
	struct device_node *combine_np = NULL;
	struct platform_device *combine_dev;
	controller = kzalloc(sizeof(*controller), GFP_KERNEL);
	if (IS_ERR_OR_NULL(controller)) {
		pr_err("ths controller: not enough memory for controller data\n");
		return NULL;
	}
	controller->dev = dev;
	controller->ops = ops;
	controller->data = data;
	atomic_set(&controller->is_suspend, 0);
	atomic_set(&controller->usage, 0);
	mutex_init(&controller->lock);
	INIT_LIST_HEAD(&controller->combine_list);
	mutex_lock(&controller_list_lock);
	list_add_tail(&controller->node, &controller_list);
	mutex_unlock(&controller_list_lock);
	for_each_child_of_node(np, combine_np) {
		combine_dev = kzalloc(sizeof(*combine_dev), GFP_KERNEL);
		combine_dev->dev.of_node = combine_np;
		combine_dev->name = SUNXI_THS_COMBINE_NAME;
		combine_dev->id = PLATFORM_DEVID_NONE;
		platform_device_register(combine_dev);
	}
	return controller;
}
EXPORT_SYMBOL(sunxi_ths_controller_register);

void sunxi_ths_controller_unregister(struct sunxi_ths_controller *controller)
{
	struct sunxi_ths_sensor *sensor;
	struct platform_device *pdev;
	list_for_each_entry(sensor, &controller->combine_list, node) {
		pdev = sensor->pdev;
		platform_device_unregister(pdev);
		kfree(pdev);
	}
	mutex_lock(&controller_list_lock);
	list_del(&controller->node);
	mutex_unlock(&controller_list_lock);
	kfree(controller);
}
EXPORT_SYMBOL(sunxi_ths_controller_unregister);

static int sunxi_combine_parse(struct sunxi_ths_sensor *sensor)
{
	struct device_node *np =NULL;
	struct sunxi_ths_combine_disc *combine;
	int i;
	const char *type = NULL;
	combine = kzalloc(sizeof(*combine), GFP_KERNEL);
	if (IS_ERR_OR_NULL(combine)) {
		pr_err("ths combine: not enough memory for combine data\n");
		return -ENOMEM;
	}
	np = sensor->pdev->dev.of_node;
	combine->controller = combine_find_controller(np);
	if(!combine->controller){
		pr_err("sensor find no controller\n");
		goto parse_fail;
	}
	if (of_property_read_u32(np, "combine_cnt", &combine->combine_cnt)) {
		pr_err("%s: get combine cnt failed\n", __func__);
		goto parse_fail;
	}
	if (of_property_read_string(np, "combine_type", &type)){
		pr_err("%s: get combine type failed\n", __func__);
		goto parse_fail;
	}else{
		combine->type = get_combine_type(type);
	}
	for(i = 0; i < combine->combine_cnt; i++){
		if (of_property_read_u32_index(np, "combine_chn",
					i, &(combine->combine_chn[i]))) {
			pr_err("node combine chn get failed!\n");
			goto parse_fail;
		}
	}
	sensor->combine = combine;
	list_add_tail(&sensor->node, &combine->controller->combine_list);
	return 0;
parse_fail:
	kfree(combine);
	return -ENOMEM;
}

static int sunxi_combine_probe(struct platform_device *pdev)
{
	int err = 0;
	struct sunxi_ths_sensor *sensor;

	thsprintk(DEBUG_INIT, "sunxi ths sensor probe start !\n");

	if (!pdev->dev.of_node) {
		pr_err("sunxi ths sensor register err!\n");
		return -EBUSY;
	}
	sensor = kzalloc(sizeof(*sensor), GFP_KERNEL);
	if (IS_ERR_OR_NULL(sensor)) {
		pr_err("ths combine: not enough memory for sensor data\n");
		return -EBUSY;
	}
	sensor->pdev = pdev;

	err = sunxi_combine_parse(sensor);
	if(err)
		goto fail;
	atomic_add(1, &sensor->combine->controller->usage);
	sensor->tz = thermal_zone_of_sensor_register(&pdev->dev,
				0, sensor, sunxi_combine_get_temp, NULL);
	if(IS_ERR(sensor->tz)){
		pr_err("sunxi ths sensor register err!\n");
		goto fail1;
	}
	sunxi_combine_input_init(sensor);

	dev_set_drvdata(&pdev->dev, sensor);
	atomic_set(&sensor->is_suspend, 0);

	if(sensor->tz->ops->set_mode)
		sensor->tz->ops->set_mode(sensor->tz, THERMAL_DEVICE_ENABLED);
	else
		thermal_zone_device_update(sensor->tz);

	thsprintk(DEBUG_INIT, "ths probe end!\n");
	return 0;
fail1:
	kfree(sensor->combine);
fail:
	kfree(sensor);
	return -EBUSY;
}

static int sunxi_combine_remove(struct platform_device *pdev)
{
	struct sunxi_ths_sensor *sensor;

	sensor = dev_get_drvdata(&pdev->dev);
	thermal_zone_of_sensor_unregister(&pdev->dev, sensor->tz);
	sunxi_combine_input_exit(sensor);
	kfree(sensor->combine);
	kfree(sensor);
	return 0;
}

#ifdef CONFIG_PM
static int sunxi_combine_suspend(struct device *dev)
{
	struct sunxi_ths_sensor *sensor = dev_get_drvdata(dev);
	struct sunxi_ths_controller *controller = sensor->combine->controller;

	thsprintk(DEBUG_SUSPEND, "enter: sunxi_ths_suspend. \n");
	mutex_lock(&sensor->input_enable_mutex);
	if (atomic_read(&sensor->input_enable)== 1) {
		cancel_delayed_work_sync(&sensor->input_work);
	}
	mutex_unlock(&sensor->input_enable_mutex);
	atomic_set(&sensor->is_suspend, 1);
	if(atomic_sub_return(1, &controller->usage)== 0)
		if(controller->ops->suspend)
			return controller->ops->suspend(controller);

	return 0;
}

static int sunxi_combine_resume(struct device *dev)
{
	struct sunxi_ths_sensor *sensor = dev_get_drvdata(dev);
	struct sunxi_ths_controller *controller = sensor->combine->controller;

	thsprintk(DEBUG_SUSPEND, "enter: sunxi_ths_resume. \n");

	if(atomic_add_return(1, &controller->usage)== 1)
		if(controller->ops->resume)
			controller->ops->resume(controller);

	atomic_set(&sensor->is_suspend, 0);
	mutex_lock(&sensor->input_enable_mutex);
	if (atomic_read(&sensor->input_enable)== 1) {
		schedule_delayed_work(&sensor->input_work,
		msecs_to_jiffies(atomic_read(&sensor->input_delay)));
	}
	mutex_unlock(&sensor->input_enable_mutex);

	return 0;
}

static const struct dev_pm_ops sunxi_combine_pm_ops = {
	.suspend        = sunxi_combine_suspend,
	.resume         = sunxi_combine_resume,
};
#endif

static struct platform_driver sunxi_combine_driver = {
	.probe  = sunxi_combine_probe,
	.remove = sunxi_combine_remove,
	.driver = {
		.name   = SUNXI_THS_COMBINE_NAME,
		.owner  = THIS_MODULE,
#ifdef CONFIG_PM
		.pm	= &sunxi_combine_pm_ops,
#endif
	},
};

static int __init sunxi_ths_combine_init(void)
{
	return platform_driver_register(&sunxi_combine_driver);
}

static void __exit sunxi_ths_combine_exit(void)
{
	platform_driver_unregister(&sunxi_combine_driver);
	mutex_destroy(&controller_list_lock);
}

module_init(sunxi_ths_combine_init);
module_exit(sunxi_ths_combine_exit);
module_param_named(debug_mask, thermal_debug_mask, int, 0644);
MODULE_DESCRIPTION("SUNXI combine thermal sensor driver");
MODULE_AUTHOR("QIn");
MODULE_LICENSE("GPL v2");

