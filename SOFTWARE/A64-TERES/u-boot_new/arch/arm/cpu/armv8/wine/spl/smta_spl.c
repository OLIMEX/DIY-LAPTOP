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
#include <asm/arch/smta.h>

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
void sunxi_smta_set_to_ns(uint type)
{
	writel(0xbe, SMTA_SET_REG(0));
	writel(0x7f, SMTA_SET_REG(1));
	writel(0x10, SMTA_SET_REG(2));
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
void sunxi_smta_set_to_s(uint type)
{
	u8  sub_type0, sub_type1;

	sub_type0 = type & 0xff;
	sub_type1 = (type>>8) & 0xff;

	if(sub_type0)
	{
		writel(sub_type0, SMTA_CLEAR_REG(0));
	}
	if(sub_type1)
	{
		writel(sub_type1, SMTA_CLEAR_REG(1));
	}
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
uint sunxi_smta_probe_status(uint type)
{
	if(type < 8)
	{
		return (readl(SMTA_STATUS_REG(0)) >> type) & 1;
	}
	if(type < 16)
	{
		return (readl(SMTA_STATUS_REG(1)) >> type) & 1;
	}

	return 0;
}

