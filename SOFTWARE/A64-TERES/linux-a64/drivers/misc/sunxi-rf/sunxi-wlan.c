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
#include <linux/interrupt.h>
#include <linux/rfkill.h>
#include <linux/regulator/consumer.h>
#include <linux/platform_device.h>
#include <linux/sys_config.h>

struct sunxi_wlan_platdata {
	int bus_index;
	struct regulator *wlan_power;
	struct regulator *io_regulator;
	struct clk 	*lpo;
	int gpio_wlan_regon;
	int gpio_wlan_hostwake;

	char *wlan_power_name;
	char *io_regulator_name;

	int power_state;
	struct platform_device *pdev;
};
static struct sunxi_wlan_platdata *wlan_data = NULL;

static int sunxi_wlan_on(struct sunxi_wlan_platdata *data, bool on_off);
static DEFINE_MUTEX(sunxi_wlan_mutex);

void sunxi_wlan_set_power(bool on_off)
{
	struct platform_device *pdev;
	int ret = 0;
	if(!wlan_data)
		return;

	pdev = wlan_data->pdev;
	mutex_lock(&sunxi_wlan_mutex);
	if(on_off != wlan_data->power_state){
		ret = sunxi_wlan_on(wlan_data, on_off);
		if(ret)
			dev_err(&pdev->dev,"set power failed\n");
	}
	mutex_unlock(&sunxi_wlan_mutex);
}
EXPORT_SYMBOL_GPL(sunxi_wlan_set_power);

int sunxi_wlan_get_bus_index(void)
{
	struct platform_device *pdev;
	if(!wlan_data)
		return -EINVAL;

	pdev = wlan_data->pdev;
	dev_info(&pdev->dev,"bus_index: %d\n",wlan_data->bus_index);
	return wlan_data->bus_index;
}
EXPORT_SYMBOL_GPL(sunxi_wlan_get_bus_index);

int sunxi_wlan_get_oob_irq(void)
{
	struct platform_device *pdev;
	int host_oob_irq = 0;
	if(!wlan_data || !gpio_is_valid(wlan_data->gpio_wlan_hostwake))
		return 0;

	pdev = wlan_data->pdev;

	host_oob_irq = gpio_to_irq(wlan_data->gpio_wlan_hostwake);
	if (IS_ERR_VALUE(host_oob_irq)) 
		dev_err(&pdev->dev,"map gpio [%d] to virq failed, errno = %d\n",
			wlan_data->gpio_wlan_hostwake,host_oob_irq);

	return host_oob_irq;
}
EXPORT_SYMBOL_GPL(sunxi_wlan_get_oob_irq);

int sunxi_wlan_get_oob_irq_flags(void)
{
	int oob_irq_flags;
	if(!wlan_data)
		return 0;

	oob_irq_flags = (IRQF_TRIGGER_HIGH | IRQF_SHARED | IRQF_NO_SUSPEND);

	return oob_irq_flags;
}
EXPORT_SYMBOL_GPL(sunxi_wlan_get_oob_irq_flags);

static int sunxi_wlan_on(struct sunxi_wlan_platdata *data, bool on_off)
{
	struct platform_device *pdev = data->pdev;
	struct device *dev = &pdev->dev;
	int ret = 0;

	if(!on_off && gpio_is_valid(data->gpio_wlan_regon))
		gpio_set_value(data->gpio_wlan_regon, 0);

	if(data->wlan_power_name){
		data->wlan_power = regulator_get(dev, data->wlan_power_name);
		if (!IS_ERR(data->wlan_power)) {
			if(on_off){
				ret = regulator_enable(data->wlan_power);
				if (ret < 0){
					dev_err(dev, "regulator wlan_power enable failed\n");
					regulator_put(data->wlan_power);
					return ret;
				}

				ret = regulator_get_voltage(data->wlan_power);
				if (ret < 0){
					dev_err(dev, "regulator wlan_power get voltage failed\n");
					regulator_put(data->wlan_power);
					return ret;
				}
				dev_info(dev, "check wlan wlan_power voltage: %d\n",ret);
			}else{
				ret = regulator_disable(data->wlan_power);
				if (ret < 0){
					dev_err(dev, "regulator wlan_power disable failed\n");
					regulator_put(data->wlan_power);
					return ret;
				}
			}
			regulator_put(data->wlan_power);
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
				dev_info(dev, "check wlan io_regulator voltage: %d\n",ret);
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

	if(on_off && gpio_is_valid(data->gpio_wlan_regon)){
		mdelay(10);
		gpio_set_value(data->gpio_wlan_regon, 1);
	}
	wlan_data->power_state = on_off;

	return 0;
}

static ssize_t power_state_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", wlan_data->power_state);
}

static ssize_t power_state_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long state;
	int err;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	err = kstrtoul(buf, 0, &state);
	if (err)
		return err;

	if (state > 1 )
		return -EINVAL;

	mutex_lock(&sunxi_wlan_mutex);
	if(state != wlan_data->power_state){
		err = sunxi_wlan_on(wlan_data, state);
		if(err)
			dev_err(dev,"set power failed\n");
	}
	mutex_unlock(&sunxi_wlan_mutex);

	return count;
}

static DEVICE_ATTR(power_state, S_IRUGO | S_IWUSR,
	power_state_show, power_state_store);

static int sunxi_wlan_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device *dev = &pdev->dev;
	struct sunxi_wlan_platdata *data;
	struct gpio_config config;
	u32 val;
	const char *power,*io_regulator;
	int ret = 0;

	data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	data->pdev = pdev;
	wlan_data = data;

	data->bus_index = -1;
	if (!of_property_read_u32(np, "wlan_busnum", &val)) {
		switch (val) {
		case 0:
		case 1:
		case 2:
			data->bus_index = val;
			break;
		default:
			dev_err(dev, "unsupported wlan_busnum (%u)\n", val);
			return -EINVAL;
		}
	}
	dev_info(dev, "wlan_busnum (%u)\n", val);

	if (of_property_read_string(np, "wlan_power", &power)) {
		dev_warn(dev, "Missing wlan_power.\n");
	}else{
		data->wlan_power_name = devm_kzalloc(dev, 64, GFP_KERNEL);
		if(!data->wlan_power_name)
			return -ENOMEM;
		else
			strcpy(data->wlan_power_name,power);
	}
	dev_info(dev, "wlan_power_name (%s)\n", data->wlan_power_name);

	if (of_property_read_string(np, "wlan_io_regulator", &io_regulator)) {
		dev_warn(dev, "Missing wlan_io_regulator.\n");
	}else{
		data->io_regulator_name = devm_kzalloc(dev, 64, GFP_KERNEL);
		if(!data->io_regulator_name)
			return -ENOMEM;
		else
			strcpy(data->io_regulator_name,io_regulator);
	}
	dev_info(dev, "io_regulator_name (%s)\n", data->io_regulator_name);

	data->gpio_wlan_regon = of_get_named_gpio_flags(np, "wlan_regon", 0, (enum of_gpio_flags *)&config);
	if (!gpio_is_valid(data->gpio_wlan_regon)) {
		dev_err(dev, "get gpio wlan_regon failed\n");
	}else{
		dev_info(dev,"wlan_regon gpio=%d  mul-sel=%d  pull=%d  drv_level=%d  data=%d\n",
				config.gpio,
				config.mul_sel,
				config.pull,
				config.drv_level,
				config.data);

		ret = devm_gpio_request(dev, data->gpio_wlan_regon, "wlan_regon");
		if (ret < 0) {
			dev_err(dev,"can't request wlan_regon gpio %d\n",
				data->gpio_wlan_regon);
			return ret;
		}

		ret = gpio_direction_output(data->gpio_wlan_regon, 0);
		if (ret < 0) {
			dev_err(dev,"can't request output direction wlan_regon gpio %d\n",
				data->gpio_wlan_regon);
			return ret;
		}
	}

	data->gpio_wlan_hostwake = of_get_named_gpio_flags(np, "wlan_hostwake", 0, (enum of_gpio_flags *)&config);
	if (!gpio_is_valid(data->gpio_wlan_hostwake)) {
		dev_err(dev, "get gpio wlan_hostwake failed\n");
	}else{
		dev_info(dev,"wlan_hostwake gpio=%d  mul-sel=%d  pull=%d  drv_level=%d  data=%d\n",
				config.gpio,
				config.mul_sel,
				config.pull,
				config.drv_level,
				config.data);

		ret = devm_gpio_request(dev, data->gpio_wlan_hostwake, "wlan_hostwake");
		if (ret < 0) {
			dev_err(dev,"can't request wlan_hostwake gpio %d\n",
				data->gpio_wlan_hostwake);
			return ret;
		}

		gpio_direction_input(data->gpio_wlan_hostwake);
		if (ret < 0) {
			dev_err(dev,"can't request input direction wlan_hostwake gpio %d\n",
				data->gpio_wlan_hostwake);
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

	device_create_file(dev, &dev_attr_power_state);
	data->power_state = 0;

	return 0;
}

static int sunxi_wlan_remove(struct platform_device *pdev)
{
	device_remove_file(&pdev->dev, &dev_attr_power_state);
	
	if (!IS_ERR_OR_NULL(wlan_data->lpo)) 
		clk_disable_unprepare(wlan_data->lpo);

	return 0;
}

static const struct of_device_id sunxi_wlan_ids[] = {
	{ .compatible = "allwinner,sunxi-wlan" },
	{ /* Sentinel */ }
};

static struct platform_driver sunxi_wlan_driver = {
	.probe		= sunxi_wlan_probe,
	.remove	= sunxi_wlan_remove,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= "sunxi-wlan",
		.of_match_table	= sunxi_wlan_ids,
	},
};

module_platform_driver(sunxi_wlan_driver);

MODULE_DESCRIPTION("sunxi wlan driver");
MODULE_LICENSE(GPL);