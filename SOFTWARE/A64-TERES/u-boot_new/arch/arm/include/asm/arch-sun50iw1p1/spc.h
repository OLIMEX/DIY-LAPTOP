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

#ifndef __SMTA_H__
#define __SMTA_H__


#include "platform.h"


#define SPC_DECPORT_STA_REG(n)       (SUNXI_SPC_BASE + (n) * 0x0C + 0x04)
#define SPC_DECPORT_SET_REG(n)       (SUNXI_SPC_BASE + (n) * 0x0C + 0x08)
#define SPC_DECPORT_CLR_REG(n)       (SUNXI_SPC_BASE + (n) * 0x0C + 0x0C)

#define SPC_STATUS_REG(n)      (SUNXI_SPC_BASE + (n) * 0x0C + 0x04)
#define SPC_SET_REG(n)         (SUNXI_SPC_BASE + (n) * 0x0C + 0x08)
#define SPC_CLEAR_REG(n)       (SUNXI_SPC_BASE + (n) * 0x0C + 0x0C)

void sunxi_spc_set_to_ns(uint type);
void sunxi_spc_set_to_s(uint type);
uint sunxi_spc_probe_status(uint type);

#endif /* __SMTA_H__ */
