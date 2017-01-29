/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __SMC_H__
#define __SMC_H__

#include <asm/io.h>


u32 smc_readl(ulong addr);
void smc_writel(u32 val,ulong addr);


extern u32 __sunxi_smc_call(ulong arg0, ulong arg1, ulong arg2, ulong arg3);

u32 sunxi_smc_call(ulong arg0, ulong arg1, ulong arg2, ulong arg3,ulong pResult);
int arm_svc_version(u32* major, u32* minor);
int arm_svc_call_count(void);
int arm_svc_uuid(u32 *uuid);
int arm_svc_run_os(ulong kernel, ulong fdt, ulong arg2);
u32 arm_svc_read_sec_reg(ulong reg);
int arm_svc_write_sec_reg(u32 val,ulong reg);
int arm_svc_arisc_startup(ulong cfg_base);
int arm_svc_arisc_wait_ready(void);
u32 arm_svc_arisc_read_pmu(ulong addr);
int arm_svc_arisc_write_pmu(ulong addr,u32 value);

int smc_init(void);



#endif
