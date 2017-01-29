/*
 * driver/pinctrl/pinctrl-axp.c
 *
 * Copyright(c) 2013-2015 Allwinnertech Co., Ltd.
 *         http://www.allwinnertech.com
 *
 * Author: sunny <sunny@allwinnertech.com>
 *
 * allwinner axp pinctrl driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/io.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pinctrl/machine.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/pinconf-generic.h>
#include <linux/pinctrl/pinmux.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/sys_config.h>
#include <linux/mfd/axp-mfd.h>
#include <linux/slab.h>
#include <linux/mfd/core.h>
#include <linux/seq_file.h>
#include <linux/i2c.h>
#include "../core.h"
#include "pinctrl-axp.h"

#define MODULE_NAME "axp-pinctrl"

static s32 pin_debug = 0;
#define PIN_MSG(fmt, arg...)	if (unlikely(pin_debug)) \
	printk(KERN_DEBUG fmt , ## arg)

static const struct axp_desc_pin axp_pins[] = {
	AXP_PIN_DESC(AXP_PINCTRL_GPIO0,
		     AXP_FUNCTION(0x0, "gpio_in"),
		     AXP_FUNCTION(0x1, "gpio_out")),
	AXP_PIN_DESC(AXP_PINCTRL_GPIO1,
		     AXP_FUNCTION(0x0, "gpio_in"),
		     AXP_FUNCTION(0x1, "gpio_out")),
	AXP_PIN_DESC(AXP_PINCTRL_GPIO2,
		     AXP_FUNCTION(0x0, "gpio_in"),
		     AXP_FUNCTION(0x1, "gpio_out")),
	AXP_PIN_DESC(AXP_PINCTRL_GPIO3,
		     AXP_FUNCTION(0x0, "gpio_in"),
		     AXP_FUNCTION(0x1, "gpio_out")),
};

#define AXP_NUM_GPIOS   ARRAY_SIZE(axp_pins)


static struct axp_pinctrl_desc axp_pinctrl_pins_desc = {
	.pins  = axp_pins,
	.npins = AXP_NUM_GPIOS,
};

/* axp driver porting interface */
static int axp_gpio_get_data(int gpio)
{
	uint8_t ret;
	struct axp_dev *axp = NULL;

	PIN_MSG("%s enter... gpio = %d\n", __func__, gpio);

#ifdef CONFIG_AW_AXP22
	axp = axp_dev_lookup(AXP22);
#elif defined(CONFIG_AW_AXP81X)
	axp = axp_dev_lookup(AXP81X);
#endif
	if (NULL == axp) {
		printk("%s: axp data is null\n", __func__);
		return -ENXIO;
	}
	switch(gpio){
		case 0:axp_read(axp->dev,AXP_GPIO01_STATE,&ret);ret &= 0x1;break;
		case 1:axp_read(axp->dev,AXP_GPIO01_STATE,&ret);ret &= 0x2;break;
		case 2:printk("This IO is not an input,no return value!");return -ENXIO;
		case 3:printk("This IO is not an input,no return value!");return -ENXIO;
		case 4:printk("This IO is not an input,no return value!");return -ENXIO;
		case 5:printk("This IO is not an input,no return value!");return -ENXIO;
		default:return -ENXIO;
	}
	return ret;
}

static int axp_gpio_set_data(int gpio, int value)
{
	struct axp_dev *axp = NULL;

	PIN_MSG("%s enter... gpio = %d, value = %d\n", __func__, gpio, value);

	if ((0 <= gpio) && (4 >= gpio)) {
#ifdef CONFIG_AW_AXP22
		axp = axp_dev_lookup(AXP22);
#elif defined(CONFIG_AW_AXP81X)
		axp = axp_dev_lookup(AXP81X);
#endif
		if (NULL == axp) {
			printk("%s: %d axp data is null\n", __func__, __LINE__);
			return -ENXIO;
		}
	} else {
		return -ENXIO;
	}

	if(value){//high
		switch(gpio) {
			case 0:
				return axp_update_sync(axp->dev,AXP_GPIO0_CFG,0x01,0x07);
			case 1:
				return axp_update_sync(axp->dev,AXP_GPIO1_CFG,0x01,0x07);
			case 2:
				return axp_set_bits(axp->dev,AXP_GPIO2_CFG,0x80);
			case 3:
				axp_clr_bits(axp->dev,AXP_GPIO3_CFG, 0x10);
				return axp_set_bits(axp->dev,AXP_GPIO3_STA,0x04);
			default:break;
		}
	}else{//low
		switch(gpio){
			case 0:
				return axp_update_sync(axp->dev,AXP_GPIO0_CFG,0x0,0x07);
			case 1:
				return axp_update_sync(axp->dev,AXP_GPIO1_CFG,0x0,0x07);
			case 2:
				return axp_clr_bits(axp->dev,AXP_GPIO2_CFG,0x80);
			case 3:
				axp_clr_bits(axp->dev,AXP_GPIO3_CFG, 0x10);
				return axp_clr_bits(axp->dev,AXP_GPIO3_STA,0x04);
			default:break;
		}
	}
	return -ENXIO;
}

static int axp_pmx_set(int gpio, int mux)
{
	struct axp_dev *axp = NULL;

	PIN_MSG("%s enter... gpio = %d, mux = %d\n", __func__, gpio, mux);

#ifdef CONFIG_AW_AXP22
	axp = axp_dev_lookup(AXP22);
#elif defined(CONFIG_AW_AXP81X)
	axp = axp_dev_lookup(AXP81X);
#endif

	if (NULL == axp) {
		printk("%s: %d axp data is null\n", __func__, __LINE__);
		return -ENXIO;
	}
	if(mux == 1){//output
		switch(gpio){
			case 0: return axp_clr_bits_sync(axp->dev,AXP_GPIO0_CFG, 0x06);
			case 1: return axp_clr_bits_sync(axp->dev,AXP_GPIO1_CFG, 0x06);
			case 2: return 0;
			case 3: return axp_clr_bits(axp->dev,AXP_GPIO3_CFG, 0x10);
			default:return -ENXIO;
		}
	}
	else if(mux == 0){//input
		switch(gpio){
			case 0: axp_clr_bits_sync(axp->dev,AXP_GPIO0_CFG,0x05);
					return axp_set_bits_sync(axp->dev,AXP_GPIO0_CFG,0x02);
			case 1: axp_clr_bits_sync(axp->dev,AXP_GPIO1_CFG,0x05);
					return axp_set_bits_sync(axp->dev,AXP_GPIO1_CFG,0x02);
			case 2:	printk("This IO can not config as an input!");
				return -EINVAL;
			case 3:	printk("This IO can not config as an input!");
				return -EINVAL;
			default:return -ENXIO;
		}
	}
	return -ENXIO;
}

static int axp_pmx_get(int gpio)
{
	struct axp_dev *axp = NULL;
	uint8_t data;

	PIN_MSG("%s enter... gpio = %d\n", __func__, gpio);

#ifdef CONFIG_AW_AXP22
	axp = axp_dev_lookup(AXP22);
#elif defined(CONFIG_AW_AXP81X)
	axp = axp_dev_lookup(AXP81X);
#endif

	if (NULL == axp) {
		printk("%s: %d axp data is null\n", __func__, __LINE__);
		return -ENXIO;
	}

	switch(gpio){
	case 0:
		axp_read(axp->dev, AXP_GPIO0_CFG, &data);
		if (0 == (data & 0x06))
			return 1;
		else if (0x02 == (data & 0x07))
			return 0;
		else
			return -ENXIO;
	case 1:
		axp_read(axp->dev, AXP_GPIO1_CFG, &data);
		if (0 == (data & 0x06))
			return 1;
		else if (0x02 == (data & 0x07))
			return 0;
		else
			return -ENXIO;
	case 2:
		return 1;
	case 3:
		axp_read(axp->dev, AXP_GPIO3_CFG, &data);
		if (0 == (data & 0x10))
			return 1;
		else
			return 0;
	default:return -ENXIO;
	}
	return -ENXIO;
}

static int axp_gpio_request(struct gpio_chip *chip, unsigned offset)
{
	return pinctrl_request_gpio(chip->base + offset);
}

static void axp_gpio_free(struct gpio_chip *chip, unsigned offset)
{
	pinctrl_free_gpio(chip->base + offset);
}

static int axp_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	return pinctrl_gpio_direction_input(chip->base + offset);
}

static int axp_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	//struct axp_pinctrl *pc = dev_get_drvdata(chip->dev);

	return axp_gpio_get_data(offset);
}

static void axp_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	//struct axp_pinctrl *pc = dev_get_drvdata(chip->dev);

	axp_gpio_set_data(offset, value);
}

static int axp_gpio_direction_output(struct gpio_chip *chip,
		unsigned offset, int value)
{
	axp_gpio_set(chip, offset, value);
	return pinctrl_gpio_direction_output(chip->base + offset);
}

static int axp_pinctrl_gpio_of_xlate(struct gpio_chip *gc,
				const struct of_phandle_args *gpiospec,
				u32 *flags)
{
	struct gpio_config *config = NULL;
	int pin, base;

	PIN_MSG("%s enter... gpiospec->args[0] = %d, gpiospec->args[1] = %d\n",
		__func__, gpiospec->args[0], gpiospec->args[1]);

	base = AXP_PIN_BASE;
	pin = base + gpiospec->args[1];

	pin = pin-gc->base;
	if (pin > gc->ngpio)
		return -EINVAL;

	if (flags){
		config = (struct gpio_config *)flags;
		config->gpio = base + gpiospec->args[1];
		config->mul_sel = gpiospec->args[2];
		config->drv_level = gpiospec->args[3];
		config->pull = gpiospec->args[4];
		config->data = gpiospec->args[5];
	}

	return pin;
}

static struct gpio_chip axp_gpio_chip = {
	.label            = MODULE_NAME,
	.owner            = THIS_MODULE,
	.request          = axp_gpio_request,
	.free             = axp_gpio_free,
	.direction_input  = axp_gpio_direction_input,
	.direction_output = axp_gpio_direction_output,
	.get              = axp_gpio_get,
	.set              = axp_gpio_set,
	.of_xlate         = axp_pinctrl_gpio_of_xlate,
	.of_gpio_n_cells  = 6,
	.base             = AXP_PIN_BASE,
	.ngpio            = AXP_NUM_GPIOS,
	.can_sleep        = 0,
};

static struct axp_pinctrl_group *
axp_pinctrl_find_group_by_name(struct axp_pinctrl *pctl, const char *group)
{
	int i;

	for (i = 0; i < pctl->ngroups; i++) {
		struct axp_pinctrl_group *grp = pctl->groups + i;

		if (!strcmp(grp->name, group))
			return grp;
	}

	return NULL;
}

static struct axp_desc_function *
axp_pinctrl_desc_find_function_by_name(struct axp_pinctrl *pctl,
				       const char *pin_name,
				       const char *func_name)
{
	int i;
	for (i = 0; i < pctl->desc->npins; i++) {
		const struct axp_desc_pin *pin = pctl->desc->pins + i;
		if (!strcmp(pin->pin.name, pin_name)) {
			struct axp_desc_function *func = pin->functions;
			while (func->name) {
				if (!strcmp(func->name, func_name)) {
					return func;
				}
				func++;
			}
		}
	}
	return NULL;
}

static int axp_pinctrl_get_groups_count(struct pinctrl_dev *pctldev)
{
	struct axp_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	return pctl->ngroups;
}

static const char *axp_pinctrl_get_group_name(struct pinctrl_dev *pctldev,
                                              unsigned selector)
{
	struct axp_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	return pctl->groups[selector].name;
}

static int axp_pinctrl_get_group_pins(struct pinctrl_dev *pctldev,
                                      unsigned selector,
                                      const unsigned **pins,
                                      unsigned *num_pins)
{
	struct axp_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	*pins = (unsigned *)&pctl->groups[selector].pin;
	*num_pins = 1;

	return 0;
}

static int axp_pctrl_dt_node_to_map(struct pinctrl_dev *pctldev,
				      struct device_node *node,
				      struct pinctrl_map **map,
				      unsigned *num_maps)
{
	struct axp_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);
	unsigned long *pinconfig;
	struct property *prop;
	const char *function;
	const char *group;
	int ret, nmaps, i = 0;
	u32 val;

	PIN_MSG("%s enter...\n", __func__);

	*map = NULL;
	*num_maps = 0;

	ret = of_property_read_string(node, "allwinner,function", &function);
	if (ret) {
		dev_err(pctl->dev,
			"missing allwinner,function property in node %s\n",
			node->name);
		return -EINVAL;
	}

	nmaps = of_property_count_strings(node, "allwinner,pins") * 2;
	if (nmaps < 0) {
		dev_err(pctl->dev,
			"missing allwinner,pins property in node %s\n",
			node->name);
		return -EINVAL;
	}

	*map = kmalloc(nmaps * sizeof(struct pinctrl_map), GFP_KERNEL);
	if (!map)
		return -ENOMEM;

	of_property_for_each_string(node, "allwinner,pins", prop, group) {
		struct axp_pinctrl_group *grp =
			axp_pinctrl_find_group_by_name(pctl, group);
		int configlen = 0;

		if (!grp) {
			dev_err(pctl->dev, "unknown pin %s", group);
			continue;
		}

		if (!axp_pinctrl_desc_find_function_by_name(pctl,
							      grp->name,
							      function)) {
			dev_err(pctl->dev, "unsupported function %s on pin %s",
				function, group);
			continue;
		}

		(*map)[i].type = PIN_MAP_TYPE_MUX_GROUP;
		(*map)[i].data.mux.group = group;
		(*map)[i].data.mux.function = function;

		i++;

		(*map)[i].type = PIN_MAP_TYPE_CONFIGS_GROUP;
		(*map)[i].data.configs.group_or_pin = group;

		if (of_find_property(node, "allwinner,drive", NULL))
			configlen++;
		if (of_find_property(node, "allwinner,pull", NULL))
			configlen++;

		pinconfig = kzalloc(configlen * sizeof(*pinconfig), GFP_KERNEL);

		if (!of_property_read_u32(node, "allwinner,drive", &val)) {
		}

		if (!of_property_read_u32(node, "allwinner,pull", &val)) {
		}

		(*map)[i].data.configs.configs = pinconfig;
		(*map)[i].data.configs.num_configs = configlen;

		i++;
	}

	*num_maps = nmaps;

	return 0;
}

static void axp_pctrl_dt_free_map(struct pinctrl_dev *pctldev,
				    struct pinctrl_map *map,
				    unsigned num_maps)
{
	int i;

	for (i = 0; i < num_maps; i++) {
		if (map[i].type == PIN_MAP_TYPE_CONFIGS_GROUP)
			kfree(map[i].data.configs.configs);
	}

	kfree(map);
}

static struct pinctrl_ops axp_pinctrl_ops = {
	.dt_node_to_map   = axp_pctrl_dt_node_to_map,
	.dt_free_map      = axp_pctrl_dt_free_map,
	.get_groups_count = axp_pinctrl_get_groups_count,
	.get_group_name   = axp_pinctrl_get_group_name,
	.get_group_pins   = axp_pinctrl_get_group_pins,
};

static int axp_pmx_get_functions_count(struct pinctrl_dev *pctldev)
{
	struct axp_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	return pctl->nfunctions;
}

static const char *axp_pmx_get_function_name(struct pinctrl_dev *pctldev,
		                             unsigned selector)
{
	struct axp_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	return pctl->functions[selector].name;
}

static int axp_pmx_get_function_groups(struct pinctrl_dev *pctldev,
		                       unsigned function,
		                       const char * const **groups,
		                       unsigned * const num_groups)
{
	struct axp_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	*groups     = pctl->functions[function].groups;
	*num_groups = pctl->functions[function].ngroups;

	return 0;
}

static int axp_pmx_enable(struct pinctrl_dev *pctldev,
		          unsigned function,
		          unsigned group)
{
	struct axp_pinctrl           *pctl = pinctrl_dev_get_drvdata(pctldev);
	struct axp_pinctrl_group      *g  = pctl->groups + group;
	struct axp_pinctrl_function *func = pctl->functions + function;
	struct axp_desc_function *desc    =
	axp_pinctrl_desc_find_function_by_name(pctl, g->name, func->name);
	if (!desc) {
		return -EINVAL;
	}

	PIN_MSG("%s enter...\n", __func__);

	axp_pmx_set(g->pin, desc->muxval);

	return 0;
}

static void axp_pmx_disable(struct pinctrl_dev *pctldev,
		          unsigned function,
		          unsigned group)
{
	return ;
}

static int axp_pmx_gpio_set_direction(struct pinctrl_dev *pctldev,
		                      struct pinctrl_gpio_range *range,
		                      unsigned offset,
		                      bool input)
{
	struct axp_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);
	struct axp_desc_function *desc;
	char        pin_name[AXP_PIN_NAME_MAX_LEN];
	const char *func;
	int         ret;

	PIN_MSG("%s enter...\n", __func__);

	ret = sprintf(pin_name, "GPIO%d", offset);
	if (!ret) {
		goto error;
	}
	if (input) {
		func = "gpio_in";
	} else {
		func = "gpio_out";
	}
	desc = axp_pinctrl_desc_find_function_by_name(pctl, pin_name, func);
	if (!desc) {
		ret = -EINVAL;
		goto error;
	}
	axp_pmx_set(offset, desc->muxval);
	ret = 0;
error:
	return ret;
}

static struct pinmux_ops axp_pmx_ops = {
	.get_functions_count = axp_pmx_get_functions_count,
	.get_function_name   = axp_pmx_get_function_name,
	.get_function_groups = axp_pmx_get_function_groups,
	.enable              = axp_pmx_enable,
	.disable	     = axp_pmx_disable,
	.gpio_set_direction  = axp_pmx_gpio_set_direction,
};

static int axp_pinconf_get(struct pinctrl_dev *pctldev,
		           unsigned pin,
			   unsigned long *config)
{
	struct axp_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);
	int                 data;

	PIN_MSG("%s enter... pin = %d\n", __func__, pin);

	switch (SUNXI_PINCFG_UNPACK_TYPE(*config)) {
	case SUNXI_PINCFG_TYPE_DAT:
		data = axp_gpio_get_data(pin);
		*config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT, data);
               pr_debug("axp pconf get pin [%s] data [%d]\n",
		         pin_get_name(pctl->pctl_dev, pin), data);
		break;
	case SUNXI_PINCFG_TYPE_FUNC:
		data = axp_pmx_get(pin);
		*config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC, data);
               pr_debug("axp pconf get pin [%s] funcs [%d]\n",
		         pin_get_name(pctl->pctl_dev, pin), data);
		break;
	default:
               pr_debug("invalid axp pconf type for get\n");
		return -EINVAL;
	}
	return 0;
}

static int axp_pinconf_set(struct pinctrl_dev *pctldev,
		           unsigned pin,
			   unsigned long *pin_config,
			   unsigned num_configs)
{
	struct axp_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);
	unsigned long config = (unsigned long)pin_config;
	int                  data;
	int                  func;

	PIN_MSG("%s enter... pin = %d\n", __func__, pin);

	switch (SUNXI_PINCFG_UNPACK_TYPE(config)) {
	case SUNXI_PINCFG_TYPE_DAT:
		data = SUNXI_PINCFG_UNPACK_VALUE(config);
		axp_gpio_set_data(pin, data);
               pr_debug("axp pconf set pin [%s] data to [%d]\n",
		         pin_get_name(pctl->pctl_dev, pin), data);
		break;
	case SUNXI_PINCFG_TYPE_FUNC:
		func = SUNXI_PINCFG_UNPACK_VALUE(config);
		axp_pmx_set(pin, func);
               pr_debug("axp pconf set pin [%s] func to [%d]\n",
		         pin_get_name(pctl->pctl_dev, pin), func);
	       break;
	default:
               pr_debug("invalid axp pconf type for set\n");
		return -EINVAL;
	}
	return 0;
}

static int axp_pinconf_group_get(struct pinctrl_dev *pctldev,
				 unsigned group,
				 unsigned long *config)
{
	struct axp_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	*config = pctl->groups[group].config;
	return 0;
}

static int axp_pinconf_group_set(struct pinctrl_dev *pctldev,
				 unsigned group,
				 unsigned long *configs,
				 unsigned num_configs)
{
	struct axp_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);
	struct axp_pinctrl_group *g = &pctl->groups[group];
	int i = 0;

	for (i = 0; i < num_configs; i++) {
		/* cache the config value */
		g->config = configs[i];
	}
	return 0;
}

static struct pinconf_ops axp_pinconf_ops = {
	.pin_config_get       = axp_pinconf_get,
	.pin_config_set       = axp_pinconf_set,
	.pin_config_group_get = axp_pinconf_group_get,
	.pin_config_group_set = axp_pinconf_group_set,
};

static struct pinctrl_desc axp_pinctrl_desc = {
	.name    = MODULE_NAME,
	.pctlops = &axp_pinctrl_ops,
	.pmxops  = &axp_pmx_ops,
	.confops = &axp_pinconf_ops,
	.owner   = THIS_MODULE,
};

static int axp_pinctrl_add_function(struct axp_pinctrl *pctl, const char *name)
{
	struct axp_pinctrl_function *func = pctl->functions;
	while (func->name) {
		/* function already there */
		if (strcmp(func->name, name) == 0) {
			func->ngroups++;
			return -EEXIST;
		}
		func++;
	}
	func->name = name;
	func->ngroups = 1;

	pctl->nfunctions++;

	return 0;
}

static struct axp_pinctrl_function *
axp_pinctrl_find_function_by_name(struct axp_pinctrl *pctl, const char *name)
{
	int    i;
	struct axp_pinctrl_function *func = pctl->functions;
	for (i = 0; i < pctl->nfunctions; i++) {
		if (!func[i].name) {
			break;
		}
		if (!strcmp(func[i].name, name)) {
			return func + i;
		}
	}
	return NULL;
}


static int axp_pinctrl_build_state(struct platform_device *pdev)
{
	int    i;
	struct axp_pinctrl *pctl = platform_get_drvdata(pdev);

	pctl->ngroups = pctl->desc->npins;

	/* Allocate groups */
	pctl->groups = devm_kzalloc(&pdev->dev,
				    pctl->ngroups * sizeof(*pctl->groups),
				    GFP_KERNEL);
	if (!pctl->groups) {
		return -ENOMEM;
	}
	for (i = 0; i < pctl->desc->npins; i++) {
		const struct axp_desc_pin *pin = pctl->desc->pins + i;
		struct axp_pinctrl_group *group = pctl->groups + i;

		group->name = pin->pin.name;
		group->pin = pin->pin.number;
	}
	/*
	 * We suppose that we won't have any more functions than pins,
	 * we'll reallocate that later anyway
	 */
	pctl->functions = devm_kzalloc(&pdev->dev,
				pctl->desc->npins * sizeof(*pctl->functions),
				GFP_KERNEL);
	if (!pctl->functions) {
		return -ENOMEM;
	}

	/* Count functions and their associated groups */
	for (i = 0; i < pctl->desc->npins; i++) {
		const struct axp_desc_pin *pin = pctl->desc->pins + i;
		struct axp_desc_function *func = pin->functions;
		while (func->name) {
			axp_pinctrl_add_function(pctl, func->name);
			func++;
		}
	}
	pctl->functions = krealloc(pctl->functions,
				pctl->nfunctions * sizeof(*pctl->functions),
				GFP_KERNEL);

	for (i = 0; i < pctl->desc->npins; i++) {
		const struct axp_desc_pin *pin = pctl->desc->pins + i;
		struct axp_desc_function *func = pin->functions;

		while (func->name) {
			struct axp_pinctrl_function *func_item;
			const char **func_grp;

			func_item = axp_pinctrl_find_function_by_name(pctl, func->name);
			if (!func_item) {
				return -EINVAL;
			}

			if (!func_item->groups) {
				func_item->groups =
					devm_kzalloc(&pdev->dev,
						     func_item->ngroups * sizeof(*func_item->groups),
						     GFP_KERNEL);
				if (!func_item->groups)
					return -ENOMEM;
			}

			func_grp = func_item->groups;
			while (*func_grp)
				func_grp++;

			*func_grp = pin->pin.name;
			func++;
		}
	}

	return 0;
}

static struct of_device_id axp_pinctrl_match[] = {
	{ .compatible = "allwinner,axp-pinctrl", .data = (void *)&axp_pinctrl_pins_desc },
	{}
};
MODULE_DEVICE_TABLE(of, axp_pinctrl_match);

static int axp_pinctrl_probe(struct platform_device *pdev)
{
	const struct of_device_id *device;

	struct device           *dev = &pdev->dev;
	struct axp_pinctrl      *pctl;
	struct pinctrl_pin_desc *pins;
	int    ret;
	int    i, rc;

	/* allocate and initialize axp-pinctrl */
	pr_debug("axp_pinctrl_probe enter...\n");
	pctl = devm_kzalloc(dev, sizeof(*pctl), GFP_KERNEL);
	if (!pctl) {
		dev_err(dev, "allocate memory for axp-pinctrl structure failed\n");
		return -ENOMEM;
	}
	platform_set_drvdata(pdev, pctl);
	pctl->dev  = dev;

	device = of_match_device(axp_pinctrl_match, &pdev->dev);
	if (!device)
		return -ENODEV;
	pctl->desc = (struct axp_pinctrl_desc *)device->data;

	ret = axp_pinctrl_build_state(pdev);
	if (ret) {
		dev_err(&pdev->dev, "dt probe failed: %d\n", ret);
		return ret;
	}
	/* register axp pinctrl */
	pins = devm_kzalloc(&pdev->dev,
			    pctl->desc->npins * sizeof(*pins),
			    GFP_KERNEL);
	if (!pins) {
		return -ENOMEM;
	}
	for (i = 0; i < pctl->desc->npins; i++) {
		pins[i] = pctl->desc->pins[i].pin;
	}
	axp_pinctrl_desc.name = dev_name(&pdev->dev);
	axp_pinctrl_desc.owner = THIS_MODULE;
	axp_pinctrl_desc.pins = pins;
	axp_pinctrl_desc.npins = pctl->desc->npins;
	pctl->pctl_dev = pinctrl_register(&axp_pinctrl_desc, dev, pctl);
	if (!pctl->pctl_dev) {
		rc = gpiochip_remove(pctl->gpio_chip);
		if (rc < 0)
			return rc;
		return -EINVAL;
	}

	/* initialize axp-gpio-chip */
	pctl->gpio_chip      = &axp_gpio_chip;
	pctl->gpio_chip->dev = dev;
	ret = gpiochip_add(pctl->gpio_chip);
	if (ret) {
		dev_err(dev, "could not add GPIO chip\n");
		return ret;
	}
	for (i = 0; i < pctl->desc->npins; i++) {
		const struct axp_desc_pin *pin = pctl->desc->pins + i;

		ret = gpiochip_add_pin_range(pctl->gpio_chip, dev_name(&pdev->dev),
					     pin->pin.number,
					     pin->pin.number, 1);
		if (ret) {
			dev_err(dev, "could not add GPIO pin range\n");
			rc = gpiochip_remove(pctl->gpio_chip);
			if (rc < 0)
				return rc;
			return ret;
		}
	}

	pr_debug("axp pinctrl driver probe ok\n");
	return 0;
}

static int axp_pinctrl_remove(struct platform_device *pdev)
{
	struct axp_pinctrl *pc = platform_get_drvdata(pdev);
	int rc;

	pinctrl_unregister(pc->pctl_dev);
	rc = gpiochip_remove(pc->gpio_chip);
	if (rc < 0)
		return rc;

	return 0;
}

static struct platform_driver axp_pinctrl_driver = {
	.probe = axp_pinctrl_probe,
	.remove = axp_pinctrl_remove,
	.driver = {
		.name = MODULE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = axp_pinctrl_match,
	},
};

static int __init axp_pinctrl_init(void)
{
	int ret;
	ret = platform_driver_register(&axp_pinctrl_driver);
	if (IS_ERR_VALUE(ret)) {
		pr_debug("register axp platform driver failed, errno %d\n", ret);
		return -EINVAL;
	}
	return 0;
}
postcore_initcall(axp_pinctrl_init);

MODULE_AUTHOR("sunny");
MODULE_DESCRIPTION("allwinner axp pin control driver");
MODULE_LICENSE("GPLv2");
