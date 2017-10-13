/*
 * Allwinner AXP SoCs pinctrl driver.
 *
 * Copyright (C) 2012 sunny
 *
 * sunny <sunny@allwinnertech.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef __PINCTRL_AXP_H
#define __PINCTRL_AXP_H

#include <linux/kernel.h>
#include <linux/pinctrl/pinconf-sunxi.h>

#define AXP_PINCTRL_GPIO0	PINCTRL_PIN(0, "GPIO0")     /* axp8xx GPIO0 */
#define AXP_PINCTRL_GPIO1	PINCTRL_PIN(1, "GPIO1")     /* axp8xx GPIO1 */
#define AXP_PINCTRL_GPIO2	PINCTRL_PIN(2, "GPIO2")     /* axp8xx CHGLED : power supply for moto OUTPUT  */
#define AXP_PINCTRL_GPIO3	PINCTRL_PIN(3, "GPIO3")     /* axp8xx N_VBUSEN : OUTPUT */

/*
 * GPIO Registers.
 */

/*    AXP8XX
* GPIO0<-->GPIO0/LDO(PIN 37)
* GPIO1<-->GPIO1/LDO(PIN 31)
* GPIO2<-->CHGLED/MOTODRV(PIN 52)
* GPIO3<-->N_VBUSEN(PIN 11)
*/
#if defined (CONFIG_AW_AXP81X)
#define AXP_GPIO0_CFG                   (AXP81X_GPIO0_CTL)//0x90
#define AXP_GPIO1_CFG                   (AXP81X_GPIO1_CTL)//0x92
#define AXP_GPIO2_CFG                   (AXP81X_LDO_DC_EN2)//0x12
#define AXP_GPIO3_CFG                   (AXP81X_HOTOVER_CTL)//0x8f
#define AXP_GPIO3_STA                   (AXP81X_IPS_SET)//0x30
#define AXP_GPIO01_STATE                (AXP81X_GPIO01_SIGNAL)
#endif

#ifndef AXP_GPIO0_CFG
#define AXP_GPIO0_CFG                   (0)
#define AXP_GPIO1_CFG                   (0)
#define AXP_GPIO2_CFG                   (0)
#define AXP_GPIO3_CFG                   (0)
#define AXP_GPIO3_STA                   (0)
#define AXP_GPIO01_STATE                (0)
#endif

#define AXP_PIN_NAME_MAX_LEN	8

struct axp_desc_function {
	const char *name;
	u8         muxval;
};

struct axp_desc_pin {
	struct pinctrl_pin_desc	 pin;
	struct axp_desc_function *functions;
};

struct axp_pinctrl_desc {
	const struct axp_desc_pin *pins;
	int			  npins;
};

struct axp_pinctrl_function {
	const char	*name;
	const char	**groups;
	unsigned	ngroups;
};

struct axp_pinctrl_group {
	const char	*name;
	unsigned long	config;
	unsigned	pin;
};

struct axp_pinctrl {
	struct device                *dev;
	struct pinctrl_dev           *pctl_dev;
	struct gpio_chip             *gpio_chip;
	struct axp_pinctrl_desc	     *desc;
	struct axp_pinctrl_function  *functions;
	unsigned                     nfunctions;
	struct axp_pinctrl_group     *groups;
	unsigned                     ngroups;
};

#define AXP_PIN_DESC(_pin, ...)					\
	{							\
		.pin = _pin,					\
		.functions = (struct axp_desc_function[]){	\
			__VA_ARGS__, { } },			\
	}

#define AXP_FUNCTION(_val, _name)				\
	{							\
		.name = _name,					\
		.muxval = _val,					\
	}

#endif /* __PINCTRL_AXP_H */
