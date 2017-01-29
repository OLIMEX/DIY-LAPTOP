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
#include <asm/arch/cpu.h>
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
uint rtc_region_probe_fel_flag(void)
{
	uint fel_flag, reg_value;
	int  i;

    fel_flag = readl(RTC_GENERAL_PURPOSE_REG(2));
    printf("fel flag  = 0x%x\n", fel_flag);
	for(i=0;i<8;i++)
	{
		reg_value = readl(RTC_GENERAL_PURPOSE_REG(i));
		printf("rtc[%d] value = 0x%x\n", i, reg_value);
	}

	return fel_flag;
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
void rtc_region_clear_fel_flag(void)
{
    writel(0, RTC_GENERAL_PURPOSE_REG(2));
}




