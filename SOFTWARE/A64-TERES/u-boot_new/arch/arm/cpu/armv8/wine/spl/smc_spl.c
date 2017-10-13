/*
**********************************************************************************************************************
*
*						           the Embedded Secure Bootloader System
*
*
*						       Copyright(C), 2006-2014, Allwinnertech Co., Ltd.
*                                           All Rights Reserved
*
* File    :
*
* By      :
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/
#include <common.h>
#include <asm/io.h>
#include <asm/arch/smc.h>

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static uint __tzasc_calc_2_power(uint data)
{
	uint ret = 0;

	do
	{
		data >>= 1;
		ret ++;
	}
	while(data);

	return (ret - 1);
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :  dram_start: e.g. 0x20000000
*
*    return        :  dram_size : Mbytes
*
*    note          :  secure_region_size: Mbytes
*
*
************************************************************************************************************
*/
int sunxi_smc_config(uint dram_size, uint secure_region_size)
{
	uint region_size, permission, region_start;

	//清除所有Master的bypass属性
	writel(0, SMC_MASTER_BYPASS0_REG);
	//设置所有Master为non-secure
	writel(0xffffffff, SMC_MASTER_SECURITY0_REG);
	//设置所有内存属性，允许安全/非安全访问
	region_size = (__tzasc_calc_2_power(dram_size*1024/32) + 0b001110)<<1;
	permission  = 0b1111<<28;	//设置允许安全模式和非安全模式下访问

	//设置fullmemory起始地址
	writel(0, SMC_REGIN_SETUP_LOW_REG(1));			//填入的是相对dram起始地址的偏移量
	//设置fullmemory访问属性
	writel(permission | region_size | 1 , SMC_REGIN_ATTRIBUTE_REG(1));

	//设置顶端16M起始地址
	region_size = (__tzasc_calc_2_power(secure_region_size*1024/32) + 0b001110)<<1;
	permission  = 0b1100<<28;	//设置只允许安全模式访问

	region_start = dram_size - secure_region_size;
	if(region_start <= (4 * 1024))		//表示不超过4G
	{
		//设置安全区域起始地址
		writel((region_start * 1024 * 1024) & 0xffff8000, SMC_REGIN_SETUP_LOW_REG(2));
	}
	else
	{
		unsigned long long long_regin_start;

		//设置安全区域起始地址
		long_regin_start = region_start;
		long_regin_start = long_regin_start * 1024 * 1024;
		writel(long_regin_start & 0xffff8000, SMC_REGIN_SETUP_LOW_REG(2));
	}
	//设置安全区域访问属性
	writel(permission | region_size | 1 , SMC_REGIN_ATTRIBUTE_REG(2));

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int sunxi_drm_config(u32 drm_start, u32 dram_size)
{
	return 0;
}

