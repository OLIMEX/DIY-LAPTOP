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
#include <common.h>
#include <asm/io.h>
#include <smc.h>
#include <sunxi_board.h>

DECLARE_GLOBAL_DATA_PTR;

#define ARM_SVC_CALL_COUNT          0x8000ff00
#define ARM_SVC_UID                 0x8000ff01
//0x8000ff02 reserved
#define ARM_SVC_VERSION             0x8000ff03
#define ARM_SVC_RUNNSOS             0x8000ff04

#define ARM_SVC_READ_SEC_REG        0x8000ff05
#define ARM_SVC_WRITE_SEC_REG       0x8000ff06

//arisc
#define ARM_SVC_ARISC_STARTUP       0x8000ff10
#define ARM_SVC_ARISC_WAIT_READY    0x8000ff11
#define ARM_SVC_ARISC_READ_PMU      0x8000ff12
#define ARM_SVC_ARISC_WRITE_PMU     0x8000ff13

/*
*note pResult is will 
*/
u32 sunxi_smc_call(ulong arg0, ulong arg1, ulong arg2, ulong arg3, ulong pResult)
{
	u32 ret = 0;
	static u32 result[4] = {0};
	ret = __sunxi_smc_call(arg0,arg1,arg2,arg3);
	if(pResult != 0)
	{
		__asm volatile("str r0,[%0]": : "r" (result+0));
		__asm volatile("str r1,[%0]": : "r" (result+1));
		__asm volatile("str r2,[%0]": : "r" (result+2));
		__asm volatile("str r3,[%0]": : "r" (result+3));
		__asm volatile("": :  : "memory");
		*(u32*)pResult = (u32)result;
	}
	return ret;
}

int arm_svc_version(u32* major, u32* minor)
{
	u32 *pResult = NULL;
	int ret = 0;
	ret = sunxi_smc_call(ARM_SVC_VERSION, 0, 0, 0, (ulong)&pResult);
	if(ret < 0)
	{
		return ret;
	}

	*major = pResult[0];
	*minor = pResult[1];
	return ret;
}

int arm_svc_call_count(void)
{
	return sunxi_smc_call(ARM_SVC_CALL_COUNT, 0, 0, 0,0);
}

int arm_svc_uuid(u32 *uuid)
{
	return sunxi_smc_call(ARM_SVC_UID, 0, 0, 0, (ulong)uuid);
}

int arm_svc_run_os(ulong kernel, ulong fdt, ulong arg2)
{
	return sunxi_smc_call(ARM_SVC_RUNNSOS, kernel, fdt, arg2,0);
}

u32 arm_svc_read_sec_reg(ulong reg)
{
	return sunxi_smc_call(ARM_SVC_READ_SEC_REG, reg, 0, 0, 0);
}

int arm_svc_write_sec_reg(u32 val,ulong reg)
{
	sunxi_smc_call(ARM_SVC_WRITE_SEC_REG, reg, val, 0,0);
	return 0;

}

int arm_svc_arisc_startup(ulong cfg_base)
{
	return sunxi_smc_call(ARM_SVC_ARISC_STARTUP,cfg_base, 0, 0,0);
}

int arm_svc_arisc_wait_ready(void)
{
	return sunxi_smc_call(ARM_SVC_ARISC_WAIT_READY,0, 0, 0,0);
}

u32 arm_svc_arisc_read_pmu(ulong addr)
{
	return sunxi_smc_call(ARM_SVC_ARISC_READ_PMU,addr, 0, 0, 0);
}

int arm_svc_arisc_write_pmu(ulong addr,u32 value)
{
	return sunxi_smc_call(ARM_SVC_ARISC_WRITE_PMU,addr, value, 0, 0);
}


static __inline u32 smc_readl_normal(ulong addr)
{
	return readl(addr);
}
static __inline int smc_writel_normal(u32 value, ulong addr)
{
	writel(value, addr);
	return 0;
}

u32 (* smc_readl_pt)(ulong addr) = smc_readl_normal;
int (* smc_writel_pt)(u32 value, ulong addr) = smc_writel_normal;

u32 smc_readl(ulong addr)
{
	return smc_readl_pt(addr);
}

void smc_writel(u32 val,ulong addr)
{
	smc_writel_pt(val,addr);
}


int smc_init(void)
{
	if(sunxi_probe_secure_monitor())
	{
		smc_readl_pt = arm_svc_read_sec_reg;
		smc_writel_pt = arm_svc_write_sec_reg;
	}
	else
	{
		printf("uboot:normal mode\n");
	}
	return 0;
}


