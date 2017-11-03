
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/keyboard.h>
#include <linux/ioport.h>
#include <linux/timer.h>
#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/sys_config.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/of_regulator.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif


#define DEV_NAME "Lid Switch"

//log
#define DEBUG_NONE 0
#define DEBUG_ERROR 1
#define DEBUG_WARN 2
#define DEBUG_INFO 3
#define DEBUG_DEBUG 4
#define TAG DEV_NAME

static int debug_level = DEBUG_DEBUG;

#define LOG(level, fmt, arg...) \
    if(unlikely(level <= debug_level)) \
        printk("%s: "fmt"\n", TAG, ##arg)

#define LOGE(fmt, arg...) LOG(DEBUG_ERROR, "(E) "fmt, ##arg)
#define LOGW(fmt, arg...) LOG(DEBUG_WARN, "(W) "fmt, ##arg)
#define LOGI(fmt, arg...) LOG(DEBUG_INFO, "(I) "fmt, ##arg)
#define LOGD(fmt, arg...) LOG(DEBUG_DEBUG, "(D) "fmt, ##arg)

#define LOGF() LOGD("%s", __func__)


#define HALL_OPEN 0
#define HALL_CLOSE 1
#define HALL_UNKNOW 2

struct hall_data {
    int gpio;
    int irq;//irq number
    int pos;
    struct input_dev *input;
    struct work_struct work;
#ifdef HALL_STATE_LOCK
    spinlock_t state_lock;
    int screen_state;
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
    struct early_suspend early_suspend;
#endif
    char* power_name;
    struct regulator* power_regu;
    int power_vol;
};


static int hall_startup(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	int ret = 0;
    struct hall_data *hdata = NULL;
    struct gpio_config gpio_cfg;

	np = pdev->dev.of_node;
	if (!of_device_is_available(np)) {
		LOGE("%s: gpio hall is disable", __func__);
		return -EPERM;
	}

	hdata = kzalloc(sizeof(struct hall_data), GFP_KERNEL);
	if (IS_ERR_OR_NULL(hdata)) {
		LOGE("not enough memory for hall data");
		return -ENOMEM;
	}

    if (of_property_read_u32(np, "hall_pos", &hdata->pos)) {
        LOGW("get hall pos failed, use default value (0)");
        hdata->pos = 0;
        //return -EBUSY;
    }
    hdata->gpio = of_get_named_gpio_flags(np, "hall_gpio", 0, (enum of_gpio_config*)&gpio_cfg);
    if(!gpio_is_valid(hdata->gpio)) {
        LOGE("get hall gpio failed!!");
        ret = -EINVAL;
        goto fail_gpio;
    }
    hdata->irq = gpio_to_irq(hdata->gpio);
    if(hdata->irq < 0) {
        LOGE("get irq failed!!");
        ret = -EINVAL;
        goto fail_irq;
    }

    if(of_property_read_string(np, "hall_power", &hdata->power_name)) {
        LOGE("get hall power failed!!");
        hdata->power_name = NULL;
    }

    if(of_property_read_u32(np, "hall_power_vol", &hdata->power_vol)) {
        LOGE("get hall power vol failed!!");
    }

    LOGD("gpio=%d", hdata->gpio);
    LOGD("irq=%d", hdata->irq);
    LOGD("power_name=%s, voltage=%d", hdata->power_name, hdata->power_vol);

#ifdef HALL_STATE_LOCK
    hdata->screen_state = HALL_UNKNOW;
#endif

    platform_set_drvdata(pdev, hdata);

    LOGI("get hall info success!");

    return ret;

fail_irq:
fail_gpio:
    kfree(hdata);
	return ret;
}

static void hall_handler(struct work_struct *work)
{
    struct hall_data *hdata = container_of(work, struct hall_data, work);
    int key, val, state;

    //LOGI("%s start", __func__);

    val = gpio_get_value(hdata->gpio);
    state = (val ^ hdata->pos ? HALL_OPEN : HALL_CLOSE);

#ifdef HALL_STATE_LOCK
    spin_lock(&hdata->state_lock);
    if (hdata->screen_state == state) {
        LOGW("get the same state, do nothing");
        spin_unlock(&hdata->state_lock);
        return;
    }
    hdata->screen_state = state;
    spin_unlock(&hdata->state_lock);
#endif


    if(val ^ hdata->pos) {
        LOGI("HALL_OPEN");
        state = HALL_OPEN;
    } else {
        LOGI("HALL_CLOSE");
        state = HALL_CLOSE;
    }

    input_report_switch(hdata->input, SW_LID, state);
    input_sync(hdata->input);

    /*
    request_irq(hdata->irq, hall_isr, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, hdata);
    */
    //LOGI("%s end", __func__);
}


static irqreturn_t hall_isr(int irq, void *data)
{
    struct hall_data *hdata = (struct hall_data *)data;
    LOGF();
    //disable_irq_nosync(hdata->irq);
    schedule_work(&hdata->work);
    return IRQ_HANDLED;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void hall_early_suspend(struct early_suspend *h)
{
    struct hall_data *hdata = container_of(h, struct hall_data, early_suspend);

    LOGF();

#ifdef HALL_STATE_LOCK
    spin_lock(&hdata->state_lock);
    hdata->screen_state = HALL_CLOSE;
    spin_unlock(&hdata->state_lock);
#endif

}

static void hall_late_resume(struct early_suspend *h)
{
    struct hall_data *hdata = container_of(h, struct hall_data, early_suspend);

    LOGF();

#ifdef HALL_STATE_LOCK
    spin_lock(&hdata->state_lock);
    hdata->screen_state = HALL_OPEN;
    spin_unlock(&hdata->state_lock);
#endif

}
#endif

static void hall_power_get(struct hall_data *hdata)
{
    LOGF();
    hdata->power_regu = regulator_get(NULL, hdata->power_name);
}
static void hall_power_put(struct hall_data *hdata)
{
    if(!IS_ERR_OR_NULL(hdata->power_regu)) {
        LOGF();
        regulator_put(hdata->power_regu);
    }
}

static void hall_power_on(struct hall_data *hdata)
{
    if(!IS_ERR_OR_NULL(hdata->power_regu)) {
        LOGF();
        regulator_set_voltage(hdata->power_regu, (int)(hdata->power_vol)*1000, (int)(hdata->power_vol)*1000);
        regulator_enable(hdata->power_regu);
    }
}

static void hall_power_off(struct hall_data *hdata)
{
    if(!IS_ERR_OR_NULL(hdata->power_regu)) {
        LOGF();
        regulator_disable(hdata->power_regu);
    }
}


static int hall_probe(struct platform_device *pdev)
{
	int err =0;
    struct hall_data *hdata;
    struct input_dev *input;

    LOGF();

    //get platform data
	if (pdev->dev.of_node) {
		/* get dt and sysconfig */
		err = hall_startup(pdev);
        if( err < 0)
            goto fail1;
	} else {
		LOGE("gpio hall device tree err!");
		return -EBUSY;
	}

    hdata = platform_get_drvdata(pdev);

    //power on
    hall_power_get(hdata);
    hall_power_on(hdata);

    //alloc input device
	input = input_allocate_device();
	if (!input) {
		LOGE("not enough memory for input device");
		err = -ENOMEM;
		goto fail1;
	}

    input_set_capability(input, EV_SW, SW_LID);
    input->name = DEV_NAME;
	err = input_register_device(input);
    if (err) {
        LOGE("input register faild!!");
		goto fail2;
    }
    hdata->input = input;
    LOGI("input register ok");

#ifdef HALL_STATE_LOCK
    spin_lock_init(&hdata->state_lock);
#endif

    INIT_WORK(&(hdata->work), hall_handler);
    if (request_irq(hdata->irq, hall_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "hall", hdata)) {
		LOGE("request irq failure.");
		err = -EBUSY;
		goto fail3;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
    hdata->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
    hdata->early_suspend.suspend = hall_early_suspend;
    hdata->early_suspend.resume = hall_late_resume;
    register_early_suspend(&hdata->early_suspend);
#endif

#ifdef CONFIG_PM
	//keyboard_pm_domain.ops.suspend = sunxi_keyboard_suspend;
	//keyboard_pm_domain.ops.resume = sunxi_keyboard_resume;
	//sunxikbd_dev->dev.pm_domain = &keyboard_pm_domain;
#endif

	LOGI("hall probe success");

	return 0;

fail3:
    input_unregister_device(input);
fail2:
	input_free_device(input);
fail1:
	if(hdata)
		kfree(hdata);

    hall_power_off(hdata);
    hall_power_put(hdata);

	LOGE("init failed!!");

	return err;
}

static int hall_remove(struct platform_device *pdev)
{
    struct hall_data *hdata = platform_get_drvdata(pdev);
    LOGI("before free irq");
	free_irq(hdata->irq, hdata);
    LOGI("after free irq");
	input_unregister_device(hdata->input);
	if(hdata)
		kfree(hdata);

    hall_power_off(hdata);
    hall_power_put(hdata);
	return 0;
}

static const struct of_device_id hall_ids[] = {
    { .compatible = "allwinner,hall" },
    {}
};

static int hall_suspend(struct device *dev)
{
    struct hall_data *hdata = dev_get_drvdata(dev);

    LOGF();

#ifdef HALL_STATE_LOCK
    spin_lock(&hdata->state_lock);
    hdata->screen_state = HALL_CLOSE;
    spin_unlock(&hdata->state_lock);
#endif

    return 0;

}

static int hall_resume(struct device *dev)
{
    struct hall_data *hdata = dev_get_drvdata(dev);

    LOGF();

#ifdef HALL_STATE_LOCK
    spin_lock(&hdata->state_lock);
    hdata->screen_state = HALL_OPEN;
    spin_unlock(&hdata->state_lock);
#endif

    return 0;
}


static const struct dev_pm_ops hall_pm_ops = {
	.suspend = hall_suspend,
	.resume = hall_resume,
};

static struct platform_driver hall_driver = {
	.probe  = hall_probe,
	.remove = hall_remove,
	.driver = {
		.name   = "hall",
		.owner  = THIS_MODULE,
		.of_match_table = hall_ids,
        //.pm = &hall_pm_ops,
	},
};

module_platform_driver(hall_driver);

MODULE_AUTHOR("Mitko Gamishev <hehopmajieh@debian.bg>");
MODULE_DESCRIPTION("Hall device with one GPIO");
MODULE_LICENSE("GPL");

