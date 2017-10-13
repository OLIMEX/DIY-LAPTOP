/*
 * linux/arch/arm/mach-sunxi/sun9i-chip.c
 *
 * Copyright(c) 2014-2016 Allwinnertech Co., Ltd.
 *         http://www.allwinnertech.com
 *
 * Author: sunny <sunny@allwinnertech.com>
 *
 * allwinner sunxi soc chip version and chip id manager.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __SUNXI_MACH_SUNXI_CHIP_H
#define __SUNXI_MACH_SUNXI_CHIP_H

extern unsigned int sunxi_get_soc_ver(void);
extern int sunxi_get_soc_chipid(u8 *chipid);
extern int sunxi_get_soc_chipid_str(char *chipid);
extern int sunxi_get_pmu_chipid(u8 *chipid);
extern int sunxi_get_serial(u8 *serial);
extern unsigned int sunxi_get_soc_bin(void);
extern int sunxi_soc_is_secure(void);

#endif  /* __SUNXI_MACH_SUNXI_CHIP_H */
