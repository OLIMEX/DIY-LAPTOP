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
#include "common.h"
#include "malloc.h"
#include <private_toc.h>

extern sbrom_toc0_config_t *toc0_config;
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
int BOOT_NandGetPara(void *param, uint size)
{
    memcpy( (void *)param, toc0_config->storage_data, size);

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
__u8  *get_page_buf( void )
{

	return (__u8 *)malloc(16 * 1024);
}

/*******************************************************************************
*函数名称: g_mod
*函数原型：uint32 g_mod( __u32 dividend, __u32 divisor, __u32 *quot_p )
*函数功能: 从nand flash的某一块中找到一个完好备份将其载入到RAM中。如果成功，返
*          回OK；否则，返回ERROR。
*入口参数: dividend          输入。被除数
*          divisor           输入。除数
*          quot_p            输出。商
*返 回 值: 余数
*******************************************************************************/
__u32 g_mod( __u32 dividend, __u32 divisor, __u32 *quot_p )
{
	if( divisor == 0 )
	{
		*quot_p = 0;
		return 0;
	}
	if( divisor == 1 )
	{
		*quot_p = dividend;
		return 0;
	}

	for( *quot_p = 0; dividend >= divisor; ++(*quot_p) )
		dividend -= divisor;
	return dividend;
}

