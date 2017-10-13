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

u32 thermal_debug_mask = 0;

extern struct sunxi_ths_sensor_ops sunxi_ths_ops;
static struct sunxi_ths_data *ths_data;
static struct workqueue_struct *thermal_wq;
static long save_tmp = 20;
static unsigned int ths_suspending = 0;
static unsigned int ths_emu = 0;

static ssize_t sunxi_ths_input_delay_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	thsprintk(DEBUG_DATA_INFO, "%d, %s\n", atomic_read(&ths_data->input_delay), __FUNCTION__);
	return sprintf(buf, "%d\n", atomic_read(&ths_data->input_delay));

}

static ssize_t sunxi_ths_input_delay_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if (data > THERMAL_DATA_DELAY)
		data = THERMAL_DATA_DELAY;
	atomic_set(&ths_data->input_delay, (unsigned int) data);

	return count;
}

static ssize_t sunxi_ths_input_enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	thsprintk(DEBUG_DATA_INFO, "%d, %s\n", atomic_read(&ths_data->input_enable), __FUNCTION__);
	return sprintf(buf, "%d\n", atomic_read(&ths_data->input_enable));
}

static void sunxi_ths_input_set_enable(struct device *dev, int enable)
{
	int pre_enable = atomic_read(&ths_data->input_enable);

	mutex_lock(&ths_data->input_enable_mutex);
	if (enable) {
		if (pre_enable == 0) {
			schedule_delayed_work(&ths_data->input_work,
				msecs_to_jiffies(atomic_read(&ths_data->input_delay)));
			atomic_set(&ths_data->input_enable, 1);
		}

	} else {
		if (pre_enable == 1) {
			cancel_delayed_work_sync(&ths_data->input_work);
			atomic_set(&ths_data->input_enable, 0);
		}
	}
	mutex_unlock(&ths_data->input_enable_mutex);
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
	thsprintk(DEBUG_DATA_INFO, "%d, %s\n", ths_emu, __FUNCTION__);
	return sprintf(buf, "%d\n", ths_emu);
}

static ssize_t sunxi_ths_set_emu(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	ths_emu = (unsigned int) data;

	return count;
}

static ssize_t sunxi_ths_set_emutemp(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	save_tmp = data;

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

static void sunxi_ths_input_work_func(struct work_struct *work)
{
	static long tempetature = 5;
	struct sunxi_ths_data *data = container_of((struct delayed_work *)work,
			struct sunxi_ths_data, input_work);
	unsigned long delay = msecs_to_jiffies(atomic_read(&data->input_delay));

	thermal_zone_get_temp(data->tz, &tempetature);
	input_report_abs(data->ths_input_dev, ABS_MISC, tempetature);
	input_sync(data->ths_input_dev);
	thsprintk(DEBUG_DATA_INFO, "%s: temperature %ld\n", __func__, tempetature);

	schedule_delayed_work(&data->input_work, delay);
}

static int sunxi_ths_input_init(struct sunxi_ths_data *data)
{
	int err = 0;

	data->ths_input_dev = input_allocate_device();
	if (IS_ERR_OR_NULL(data->ths_input_dev)) {
		printk(KERN_ERR "temp_dev: not enough memory for input device\n");
		err = -ENOMEM;
		goto fail1;
	}

	data->ths_input_dev->name = "sunxi-ths";
	data->ths_input_dev->phys = "sunxiths/input0";
	data->ths_input_dev->id.bustype = BUS_HOST;
	data->ths_input_dev->id.vendor = 0x0001;
	data->ths_input_dev->id.product = 0x0001;
	data->ths_input_dev->id.version = 0x0100;

	input_set_capability(data->ths_input_dev, EV_ABS, ABS_MISC);
	input_set_abs_params(data->ths_input_dev, ABS_MISC, -50, 180, 0, 0);

	err = input_register_device(data->ths_input_dev);
	if (0 < err) {
		pr_err("%s: could not register input device\n", __func__);
		input_free_device(data->ths_input_dev);
		goto fail2;
	}

	INIT_DELAYED_WORK(&data->input_work, sunxi_ths_input_work_func);

	mutex_init(&data->input_enable_mutex);
	atomic_set(&data->input_enable, 0);
	atomic_set(&data->input_delay, THERMAL_DATA_DELAY);

	err = sysfs_create_group(&data->ths_input_dev->dev.kobj,
						 &sunxi_ths_input_attribute_group);
	if (err < 0)
	{
		pr_err("%s: sysfs_create_group err\n", __func__);
		goto fail3;
	}

	return err;
fail3:
	input_unregister_device(data->ths_input_dev);
fail2:
	kfree(data->ths_input_dev);
fail1:
	return err;

}

static void sunxi_ths_input_exit(struct sunxi_ths_data *data)
{
	//sysfs_remove_group(&data->ths_input_dev->dev.kobj, &sunxi_ths_input_attribute_group);
	input_unregister_device(data->ths_input_dev);
}

static void ths_enable(void)
{
	ths_data->ops->enable(ths_data);
}

static void ths_disable(void)
{
	ths_data->ops->disable(ths_data);
}

static void ths_clk_cfg(void)
{
	unsigned long rate = 0;

	rate = clk_get_rate(ths_data->pclk);
	thsprintk(DEBUG_INIT, "%s: get ths_clk_source rate %dHZ\n", __func__, (__u32)rate);

	if(clk_set_parent(ths_data->mclk, ths_data->pclk))
		pr_err("%s: set ths_clk parent to ths_clk_source failed!\n", __func__);

	if (clk_set_rate(ths_data->mclk, THS_CLK)) {
		pr_err("set ths clock freq to 4M failed!\n");
	}
	rate = clk_get_rate(ths_data->mclk);
	thsprintk(DEBUG_INIT, "%s: get ths_clk rate %dHZ\n", __func__, (__u32)rate);

	if (clk_prepare_enable(ths_data->mclk)) {
			pr_err("try to enable ths_clk failed!\n");
	}

	return;
}

static void ths_clk_uncfg(void)
{

	if(NULL == ths_data->mclk || IS_ERR(ths_data->mclk)) {
		pr_err("ths_clk handle is invalid, just return!\n");
		return;
	} else {
		clk_disable_unprepare(ths_data->mclk);
		clk_put(ths_data->mclk);
		ths_data->mclk = NULL;
	}

	if(NULL == ths_data->pclk || IS_ERR(ths_data->pclk)) {
		pr_err("ths_clk_source handle is invalid, just return!\n");
		return;
	} else {
		clk_put(ths_data->pclk);
		ths_data->pclk = NULL;
	}
	return;
}

static void ths_irq_work_func(struct work_struct *work)
{
	thsprintk(DEBUG_INT, "%s enter\n", __func__);
	thermal_zone_device_update(ths_data->tz);
	return;
}

static irqreturn_t sunxi_ths_irq(int irq, void *dev_id)
{
	u32 intsta;
	thsprintk(DEBUG_INT, "THS IRQ Serve\n");

	intsta = ths_data->ops->get_int(ths_data);

	ths_data->ops->clear_int(ths_data);

	if (intsta & (THS_INTS_SHT0|THS_INTS_SHT1|THS_INTS_SHT2|THS_INTS_SHT3)){
		queue_work(thermal_wq, &ths_data->irq_work);
	}

	return IRQ_HANDLED;
}

static void ths_sensor_init(void)
{
	thsprintk(DEBUG_INIT, "ths_sensor_init: ths setup start!!\n");

	ths_data->ops->init_reg(ths_data);

	thsprintk(DEBUG_INIT, "ths_sensor_init: ths setup end!!\n");
	return;
}

static void ths_sensor_exit(void)
{
	thsprintk(DEBUG_INIT, "ths_sensor_exit: ths exit start!!\n");

	ths_data->ops->clear_reg(ths_data);

	thsprintk(DEBUG_INIT, "ths_sensor_exit: ths exir end!!\n");
	return;
}

static int sunxi_ths_startup(struct platform_device *pdev)
{
	struct device_node *np =NULL;
	int ret = 0;
	
	ths_data = kzalloc(sizeof(*ths_data), GFP_KERNEL);
	if (IS_ERR_OR_NULL(ths_data)) {
		pr_err("ths_data: not enough memory for ths data\n");
		return -ENOMEM;
	}

	np = pdev->dev.of_node;
	
	ths_data->base_addr= of_iomap(np, 0);
	if (NULL == ths_data->base_addr) {
		pr_err("%s:Failed to ioremap() io memory region.\n",__func__);
		ret = -EBUSY;
	}else
		thsprintk(DEBUG_INIT, "ths base: %p !\n", ths_data->base_addr);
	ths_data->irq_num= irq_of_parse_and_map(np, 0);
	if (0 == ths_data->irq_num) {
		pr_err("%s:Failed to map irq.\n", __func__);
		ret = -EBUSY;
	}else
		thsprintk(DEBUG_INIT, "ths irq num: %d !\n", ths_data->irq_num);
	if (of_property_read_u32(np, "sensor_num", &ths_data->sensor_cnt)) {
		pr_err("%s: get sensor_num failed\n", __func__);
		ret =  -EBUSY;
	}
	if (of_property_read_u32(np, "int_temp", &ths_data->int_temp)) {
		pr_err("%s: get int temp failed\n", __func__);
		ths_data->int_temp = 120;
	}
	ths_data->pclk = of_clk_get(np, 0);
	ths_data->mclk = of_clk_get(np, 1);
	if (NULL==ths_data->pclk||IS_ERR(ths_data->pclk)
		||NULL==ths_data->mclk||IS_ERR(ths_data->mclk)) {
		pr_err("%s:Failed to get clk.\n", __func__);
		ret = -EBUSY;
	}

	ths_data->ops = &sunxi_ths_ops;

	return ret;
}

int sunxi_get_sensor_temp(u32 sensor_num, long *temperature)
{
	long temp = 0;
	int ret = -1;
	if(sensor_num < ths_data->sensor_cnt){
		temp = ths_data->ops->get_temp(ths_data, sensor_num);
		if((temp > -20) && (temp < 180 )){
			*temperature = temp;
			ret = 0;
		}
	}
	return ret;
}
EXPORT_SYMBOL(sunxi_get_sensor_temp);

static int sunxi_ths_get_temp(void *data, long *temperature)
{
	struct sunxi_ths_data *pdata = data;
	u32 i ;
	long temp = 0, taget;
	if (IS_ERR(pdata))
		return PTR_ERR(pdata);
	if((!ths_suspending) && (!ths_emu)){
		switch(pdata->mode){
		case MAX_TEMP:
			for(i = 0, taget = -20; i < pdata->sensor_cnt; i++){
				temp = pdata->ops->get_temp(pdata, i);
				if(temp > taget)
					taget = temp;
			}
			break;
		case AVG_TMP:
			for(i = 0, taget = 0; i < pdata->sensor_cnt; i++){
				temp = pdata->ops->get_temp(pdata, i);
				taget += temp;
			}
			do_div(taget, pdata->sensor_cnt);
			break;
		case MIN_TMP:
			for(i = 0, taget = 180; i < pdata->sensor_cnt; i++){
				temp = pdata->ops->get_temp(pdata, i);
				if(temp < taget)
					taget = temp;
			}
			break;
		default:
			break;
		}
		*temperature = taget;
		save_tmp = taget;
	}else{
		*temperature = save_tmp;
	}
	thsprintk(DEBUG_DATA_INFO, "%s: get temp %ld\n", __func__, (*temperature));
	return 0;
}

static int sunxi_ths_probe(struct platform_device *pdev)
{
	int err = 0;
	
	thsprintk(DEBUG_INIT, "sunxi ths sensor probe start !\n");

	if (pdev->dev.of_node) {
		// get dt and sysconfig
		err = sunxi_ths_startup(pdev);
	}else{
		pr_err("sunxi ths device tree err!\n");
		return -EBUSY;
	}

	ths_data->tz = thermal_zone_of_sensor_register(&pdev->dev,
						   0,
						   ths_data,
						   sunxi_ths_get_temp, NULL);
	if(IS_ERR(ths_data->tz)){
		pr_err("sunxi ths sensor register err!\n");
		goto err_allocate_device;
	}

	platform_set_drvdata(pdev, ths_data);
	ths_clk_cfg();
	ths_sensor_init();
	sunxi_ths_input_init(ths_data);
	
	INIT_WORK(&ths_data->irq_work, ths_irq_work_func);
	thermal_wq = create_singlethread_workqueue("thermal_wq");
	if (!thermal_wq) {
		pr_err(KERN_ALERT "Creat thermal_wq failed.\n");
		goto err_allocate_device;
	}
	flush_workqueue(thermal_wq);
	
	if (request_irq(ths_data->irq_num, sunxi_ths_irq, 0, "Thermal Sensor",
			ths_data->tz)) {
		pr_err("%s: request irq fail.\n", __func__);
		err = -EBUSY;
		goto err_request_irq;
	}
	
	ths_enable();
	/* enable here */
	if(ths_data->tz->ops->set_mode)
		ths_data->tz->ops->set_mode(ths_data->tz, THERMAL_DEVICE_ENABLED);
	else
		thermal_zone_device_update(ths_data->tz);
	
	thsprintk(DEBUG_INIT, "ths probe end!\n");
	return 0;

err_request_irq:
	platform_set_drvdata(pdev, NULL);
	sunxi_ths_input_exit(ths_data);
err_allocate_device:
	if(ths_data)
		kfree(ths_data);
	return err;

}

static int sunxi_ths_remove(struct platform_device *pdev)
{
	cancel_delayed_work_sync(&ths_data->input_work);
	ths_disable();
	free_irq(ths_data->irq_num, ths_data->tz);
	ths_sensor_exit();
	ths_clk_uncfg();
	sunxi_ths_input_exit(ths_data);
	kfree(ths_data);
	return 0;
}

#ifdef CONFIG_OF
/* Translate OpenFirmware node properties into platform_data */
static struct of_device_id sunxi_ths_of_match[] = {
	{ .compatible = "allwinner,thermal_sensor", },
	{ },
};
MODULE_DEVICE_TABLE(of, sunxi_ths_of_match);
#else /* !CONFIG_OF */
#endif

#ifdef CONFIG_PM
static int sunxi_ths_suspend(struct device *dev)
{
	thsprintk(DEBUG_SUSPEND, "enter: sunxi_ths_suspend. \n");

	mutex_lock(&ths_data->input_enable_mutex);
	if (atomic_read(&ths_data->input_enable)== 1) {
		cancel_delayed_work_sync(&ths_data->input_work);
	}
	mutex_unlock(&ths_data->input_enable_mutex);
	ths_disable();
	disable_irq_nosync(ths_data->irq_num);
	ths_suspending = 1;
	ths_sensor_exit();
	if(NULL == ths_data->mclk || IS_ERR(ths_data->mclk)) {
		thsprintk(DEBUG_SUSPEND,"ths_clk handle is invalid\n");
	} else {
		clk_disable_unprepare(ths_data->mclk);
	}
	return 0;
}

static int sunxi_ths_resume(struct device *dev)
{
	thsprintk(DEBUG_SUSPEND, "enter: sunxi_ths_resume. \n");
	clk_prepare_enable(ths_data->mclk);
	ths_sensor_init();
	enable_irq(ths_data->irq_num);
	ths_enable();

	mutex_lock(&ths_data->input_enable_mutex);
	if (atomic_read(&ths_data->input_enable)== 1) {
		schedule_delayed_work(&ths_data->input_work,
		msecs_to_jiffies(atomic_read(&ths_data->input_delay)));
	}
	mutex_unlock(&ths_data->input_enable_mutex);
	ths_suspending = 0;

	return 0;
}

static const struct dev_pm_ops sunxi_ths_pm_ops = {
	.suspend        = sunxi_ths_suspend,
	.resume         = sunxi_ths_resume,
};
#endif

static struct platform_driver sunxi_ths_driver = {
	.probe  = sunxi_ths_probe,
	.remove = sunxi_ths_remove,
	.driver = {
		.name   = SUNXI_THS_NAME,
		.owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(sunxi_ths_of_match),
#ifdef CONFIG_PM
		.pm	= &sunxi_ths_pm_ops,
#endif
	},
};
module_platform_driver(sunxi_ths_driver);
module_param_named(debug_mask, thermal_debug_mask, int, 0644);
MODULE_DESCRIPTION("SUNXI thermal sensor driver");
MODULE_AUTHOR("QIn");
MODULE_LICENSE("GPL v2");


