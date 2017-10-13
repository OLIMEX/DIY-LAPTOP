/*
 * include/linux/pinctrl/pinconf-sunxi.h
 *
 * Copyright(c) 2013-2015 Allwinnertech Co., Ltd. 
 *         http://www.allwinnertech.com
 *
 * Author: sunny <sunny@allwinnertech.com>
 *
 * allwinner sunxi pinconfig portions of the pinctrl subsystem.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __LINUX_PINCTRL_PINCONF_SUNXI_H
#define __LINUX_PINCTRL_PINCONF_SUNXI_H

/**
 * enum sunxi_pincfg_type - possible pin configuration types supported.
 * @SUNXI_PINCFG_TYPE_FUNC: Function configuration.
 * @SUNXI_PINCFG_TYPE_DAT : Pin value configuration.
 * @SUNXI_PINCFG_TYPE_PUD : Pull up/down configuration.
 * @SUNXI_PINCFG_TYPE_DRV : Drive strength configuration.
 */
enum sunxi_pincfg_type {
	SUNXI_PINCFG_TYPE_FUNC,
	SUNXI_PINCFG_TYPE_DAT,
	SUNXI_PINCFG_TYPE_PUD,
	SUNXI_PINCFG_TYPE_DRV
};
/*
 * pin configuration (pull up/down and drive strength) type and its value are
 * packed together into a 32-bits. The upper 16-bits represent the configuration
 * type and the lower 16-bits hold the value of the configuration type.
 */
#define SUNXI_PINCFG_PACK(type, value)	(((value) << 16) | (type & 0xFFFF))
#define SUNXI_PINCFG_UNPACK_TYPE(cfg)	((cfg) & 0xFFFF)
#define SUNXI_PINCFG_UNPACK_VALUE(cfg)	(((cfg) & 0xFFFF0000) >> 16)

#endif /* __LINUX_PINCTRL_PINCONF_SUNXI_H */
