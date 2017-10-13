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

#ifndef _TZASC_SMC_H_
#define _TZASC_SMC_H_

#include <asm/arch/platform.h>

#define SMC_CONFIG_REG                (SUNXI_SMC_BASE + 0x0000)
#define SMC_ACTION_REG                (SUNXI_SMC_BASE + 0x0004)
#define SMC_LKDW_RANGE_REG            (SUNXI_SMC_BASE + 0x0008)
#define SMC_LKDW_SELECT_REG           (SUNXI_SMC_BASE + 0x000c)
#define SMC_INT_STATUS_REG            (SUNXI_SMC_BASE + 0x0010)
#define SMC_INT_CLEAR_REG             (SUNXI_SMC_BASE + 0x0014)

#define SMC_MASTER_BYPASS0_REG        (SUNXI_SMC_BASE + 0x0018)
#define SMC_MASTER_SECURITY0_REG      (SUNXI_SMC_BASE + 0x001c)
#define SMC_FAIL_ADDR_REG             (SUNXI_SMC_BASE + 0x0020)
#define SMC_FAIL_CTRL_REG             (SUNXI_SMC_BASE + 0x0028)
#define SMC_FAIL_ID_REG               (SUNXI_SMC_BASE + 0x002c)

#define SMC_SPECULATION_CTRL_REG      (SUNXI_SMC_BASE + 0x0030)
#define SMC_INVER_EN_REG              (SUNXI_SMC_BASE + 0x0034)

#define SMC_MST_ATTRI_REG             (SUNXI_SMC_BASE + 0x0048)
#define SMC_DRAM_EN_REG               (SUNXI_SMC_BASE + 0x0050)

#define SMC_DRAM_ILLEGAL_ACCESS0_REG  (SUNXI_SMC_BASE + 0x0058)

#define SMC_LOW_SADDR_REG             (SUNXI_SMC_BASE + 0x0060)
#define SMC_LOW_EADDR_REG             (SUNXI_SMC_BASE + 0x0068)

#define SMC_REGIN_SETUP_LOW_REG(x)    (SUNXI_SMC_BASE + 0x100 + 0x10*(x))
#define SMC_REGIN_SETUP_HIGH_REG(x)   (SUNXI_SMC_BASE + 0x104 + 0x10*(x))
#define SMC_REGIN_ATTRIBUTE_REG(x)    (SUNXI_SMC_BASE + 0x108 + 0x10*(x))


int sunxi_smc_config(uint dram_size, uint secure_region_size);
int sunxi_drm_config(u32 drm_start, u32 dram_size);

#endif    /*  #ifndef _TZASC_SMC_H_  */
