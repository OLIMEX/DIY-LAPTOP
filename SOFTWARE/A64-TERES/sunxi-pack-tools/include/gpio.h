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

#ifndef __GPIO_H__
#define __GPIO_H__

#include "type_def.h"
//通用的，和GPIO相关的数据结构
typedef struct _normal_gpio_cfg
{
    unsigned char      port;                       //端口号
    unsigned char      port_num;                   //端口内编号
    char               mul_sel;                    //功能编号
    char               pull;                       //电阻状态
    char               drv_level;                  //驱动驱动能力
    char               data;                       //输出电平
    unsigned char      reserved[2];                //保留位，保证对齐
}
normal_gpio_cfg;

typedef struct _special_gpio_cfg
{
	unsigned char		port;				//端口号
	unsigned char		port_num;			//端口内编号
	char				mul_sel;			//功能编号
	char				data;				//输出电平
}special_gpio_cfg;

#endif    /*  #ifndef __GPIO_H__  */
