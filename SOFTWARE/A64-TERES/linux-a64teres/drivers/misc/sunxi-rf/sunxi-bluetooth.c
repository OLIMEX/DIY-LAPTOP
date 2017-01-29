#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <linux/clk.h>
#include <linux/rfkill.h>
#include <linux/regulator/consumer.h>
#include <linux/platform_device.h>
#include <linux/sys_config.h>

struct sunxi_bt_platdata {
	struct regulator *bt_power;
	struct regulator *io_regulator;
	struct clk 	*lpo;
	int gpio_bt_rst;
	char *bt_power_name;
	char *io_regulator_name;

	int power_state;
	struct rfkill *rfkill;
	struct platform_device *pdev;
};

static int sunxi_bt_on(struct sunxi_bt_platdata *data, bool on_off)
{
	struct platform_device *pdev = data->pdev;
	struct device *dev = &pdev->dev;
	int ret = 0;

	if(!on_off && gpio_is_valid(data->gpio_bt_rst))
		gpio_set_value(data->gpio_bt_rst, 0);

	if(data->bt_power_name){
		data->bt_power = regulator_get(dev, data->bt_power_name);
		if (!IS_ERR(data->bt_power)) {
			if(on_off){
				ret = regulator_enable(data->bt_power);
				if (ret < 0){
					dev_err(dev, "regulator bt_power enable failed\n");
					regulator_put(data->bt_power);
					return ret;
				}

				ret = regulator_get_voltage(data->bt_power);
				if (ret < 0){
					dev_err(dev, "regulator bt_power get voltage failed\n");
					regulator_put(data->bt_power);
					return ret;
				}
				dev_info(dev, "check bluetooth bt_power voltage: %d\n",ret);
			}else{
				ret = regulator_disable(data->bt_power);
				if (ret < 0){
					dev_err(dev, "regulator bt_power disable failed\n");
					regulator_put(data->bt_power);
					return ret;
				}
			}
			regulator_put(data->bt_power);
		}
	}

	if(data->io_regulator_name){
		data->io_regulator = regulator_get(dev, data->io_regulator_name);
		if (!IS_ERR(data->io_regulator)) {
			if(on_off){
				ret = regulator_enable(data->io_regulator);
				if (ret < 0){
					dev_err(dev, "regulator io_regulator enable failed\n");
					regulator_put(data->io_regulator);
					return ret;
				}

				ret = regulator_get_voltage(data->io_regulator);
				if (ret < 0){
					dev_err(dev, "regulator io_regulator get voltage failed\n");
					regulator_put(data->io_regulator);
					return ret;
				}
				dev_info(dev, "check bluetooth io_regulator voltage: %d\n",ret);
			}else{
				ret = regulator_disable(data->io_regulator);
				if (ret < 0){
					dev_err(dev, "regulator io_regulator disable failed\n");
					regulator_put(data->io_regulator);
					return ret;
				}
			}
			regulator_put(data->io_regulator);
		}
	}

	if(on_off && gpio_is_valid(data->gpio_bt_rst)){
		mdelay(10);
		gpio_set_value(data->gpio_bt_rst, 1);
	}
	data->power_state = on_off;

	return 0;
}

static int sunxi_bt_set_block(void *data, bool blocked)
{
	struct sunxi_bt_platdata *platdata = data;
	struct platform_device *pdev = platdata->pdev;
	int ret;

	if(blocked != platdata->power_state){
		dev_warn(&pdev->dev, "block state already is %d\n",blocked);
		return 0;
	}

	dev_info(&pdev->dev, "set block: %d\n",blocked);
	ret = sunxi_bt_on(platdata,!blocked);
	if(ret){
		dev_err(&pdev->dev, "set block failed\n");
		return ret;
	}

	return 0;
}

static const struct rfkill_ops sunxi_bt_rfkill_ops = {
	.set_block = sunxi_bt_set_block,
};

static int sunxi_bt_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device *dev = &pdev->dev;
	struct sunxi_bt_platdata *data;
	struct gpio_config config;
	const char *power,*io_regulator;
	int ret = 0;

	data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;
	
	data->pdev = pdev;
	if (of_property_read_string(np, "bt_power", &power)) {
		dev_warn(dev, "Missing bt_power.\n");
	}else{
		data->bt_power_name = devm_kzalloc(dev, 64, GFP_KERNEL);
		if(!data->bt_power_name)
			return -ENOMEM;
		else
			strcpy(data->bt_power_name,power);
	}
	dev_info(dev, "bt_power_name (%s)\n", data->bt_power_name);

	if (of_property_read_string(np, "bt_io_regulator", &io_regulator)) {
		dev_warn(dev, "Missing bt_io_regulator.\n");
	}else{
		data->io_regulator_name = devm_kzalloc(dev, 64, GFP_KERNEL);
		if(!data->io_regulator_name)
			return -ENOMEM;
		else
			strcpy(data->io_regulator_name,io_regulator);
	}
	dev_info(dev, "io_regulator_name (%s)\n", data->io_regulator_name);

	data->gpio_bt_rst = of_get_named_gpio_flags(np, "bt_rst_n", 0, (enum of_gpio_flags *)&config);
	if (!gpio_is_valid(data->gpio_bt_rst)) {
		dev_err(dev, "get gpio bt_rst failed\n");
	}else{
		dev_info(dev,"bt_rst gpio=%d  mul-sel=%d  pull=%d  drv_level=%d  data=%d\n",
				config.gpio,
				config.mul_sel,
				config.pull,
				config.drv_level,
				config.data);

		ret = devm_gpio_request(dev, data->gpio_bt_rst, "bt_rst");
		if (ret < 0) {
			dev_err(dev,"can't request bt_rst gpio %d\n",
				data->gpio_bt_rst);
			return ret;
		}

		ret = gpio_direction_output(data->gpio_bt_rst, 0);
		if (ret < 0) {
			dev_err(dev,"can't request output direction bt_rst gpio %d\n",
				data->gpio_bt_rst);
			return ret;
		}
	}

	data->lpo = devm_clk_get(dev, NULL);
	if (IS_ERR_OR_NULL(data->lpo)){
		dev_warn(dev, "clk not config\n");
	}else{
		ret = clk_prepare_enable(data->lpo);
		if (ret < 0) 
			dev_warn(dev,"can't enable clk\n");
	}

	data->rfkill = rfkill_alloc("sunxi-bt", dev, RFKILL_TYPE_BLUETOOTH,
				&sunxi_bt_rfkill_ops, data);
	if (!data->rfkill) {
		ret = -ENOMEM;
		goto failed_alloc;
	}

	rfkill_set_states(data->rfkill, true, false);

	ret = rfkill_register(data->rfkill);
	if (ret){
		goto fail_rfkill;
	}
	platform_set_drvdata(pdev, data);

	data->power_state = 0;
	return 0;

fail_rfkill:
	if (data->rfkill) 
		rfkill_destroy(data->rfkill);
failed_alloc:
	if (!IS_ERR_OR_NULL(data->lpo))
		clk_disable_unprepare(data->lpo);

	return ret;
}

static int sunxi_bt_remove(struct platform_device *pdev)
{
	struct sunxi_bt_platdata *data = platform_get_drvdata(pdev);
	struct rfkill *rfk = data->rfkill;
	
	platform_set_drvdata(pdev, NULL);
	
	if(rfk){
		rfkill_unregister(rfk);
		rfkill_destroy(rfk);
	}

	if (!IS_ERR_OR_NULL(data->lpo))
		clk_disable_unprepare(data->lpo);

	return 0;
}

static const struct of_device_id sunxi_bt_ids[] = {
	{ .compatible = "allwinner,sunxi-bt" },
	{ /* Sentinel */ }
};

static struct platform_driver sunxi_bt_driver = {
	.probe	= sunxi_bt_probe,
	.remove	= sunxi_bt_remove,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= "sunxi-bt",
		.of_match_table	= sunxi_bt_ids,
	},
};

module_platform_driver(sunxi_bt_driver);

MODULE_DESCRIPTION("sunxi bluetooth driver");
MODULE_LICENSE(GPL);