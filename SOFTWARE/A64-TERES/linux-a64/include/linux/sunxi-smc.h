/*
 * drivers/char/sunxi_sys_info/sunxi-smc.c
 *
 * Copyright(c) 2015-2016 Allwinnertech Co., Ltd.
 *         http://www.allwinnertech.com
 *
 * Author: sunny <superm@allwinnertech.com>
 *
 * allwinner sunxi soc chip version and chip id manager.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __SUNXI_SMC_H
#define __SUNXI_SMC_H

extern int invoke_smc_fn(u32 function_id, u64 arg0, u64 arg1, u64 arg2);
extern int sunxi_smc_readl(phys_addr_t addr);
extern int sunxi_smc_writel(u32 value, phys_addr_t addr);
extern int sunxi_smc_probe_secure(void);
#endif  /* __SUNXI_SMC_H */
