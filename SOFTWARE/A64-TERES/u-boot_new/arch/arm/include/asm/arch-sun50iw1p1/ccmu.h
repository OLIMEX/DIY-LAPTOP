/*
 * (C) Copyright 2007-2015
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
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef __CCMU_H
#define __CCMU_H


#include "platform.h"

/* pll list */
#define CCMU_PLL_CPUX_CTRL_REG            (SUNXI_CCM_BASE + 0x00)
#define CCMU_PLL_AUDIO_CTRL_REG           (SUNXI_CCM_BASE + 0x08)
#define CCMU_PLL_VIDEO0_CTRL_REG          (SUNXI_CCM_BASE + 0x10)
#define CCMU_PLL_VE_CTRL_REG              (SUNXI_CCM_BASE + 0x18)
#define CCMU_PLL_DDR0_CTRL_REG            (SUNXI_CCM_BASE + 0x20)
#define CCMU_PLL_PERIPH0_CTRL_REG         (SUNXI_CCM_BASE + 0x28)
#define CCMU_PLL_PERIPH1_CTRL_REG         (SUNXI_CCM_BASE + 0x2C)
#define CCMU_PLL_VIDEO1_CTRL_REG          (SUNXI_CCM_BASE + 0x30)
#define CCMU_PLL_GPU_CTRL_REG             (SUNXI_CCM_BASE + 0x38)
#define CCMU_PLL_MIPI_CTRL_REG            (SUNXI_CCM_BASE + 0x40)
#define CCMU_PLL_HSIC_CTRL_REG            (SUNXI_CCM_BASE + 0x44)
#define CCMU_PLL_DE_CTRL_REG              (SUNXI_CCM_BASE + 0x48)
#define CCMU_PLL_DDR1_CTRL_REG            (SUNXI_CCM_BASE + 0x4C)

//new mode for pll lock
#define CCMU_PLL_LOCK_CTRL_REG            (SUNXI_CCM_BASE + 0x320)
#define LOCK_EN_PLL_CPUX       (1<<0)
#define LOCK_EN_PLL_AUDIO      (1<<1)
#define LOCK_EN_PLL_VIDEO0     (1<<2)
#define LOCK_EN_PLL_VE         (1<<3)
#define LOCK_EN_PLL_DDR0       (1<<4)
#define LOCK_EN_PLL_PERIPH0    (1<<5)
#define LOCK_EN_PLL_VIDEO1     (1<<6)
#define LOCK_EN_PLL_GPU        (1<<7)
#define LOCK_EN_PLL_MIPI       (1<<8)
#define LOCK_EN_PLL_HSIC       (1<<9)
#define LOCK_EN_PLL_DE         (1<<10)
#define LOCK_EN_PLL_DDR1       (1<<11)
#define LOCK_EN_PLL_PERIPH1    (1<<12)
#define LOCK_EN_NEW_MODE       (1<<28)



/* cfg list */
#define CCMU_CPUX_AXI_CFG_REG             (SUNXI_CCM_BASE + 0x50)
#define CCMU_AHB1_APB1_CFG_REG            (SUNXI_CCM_BASE + 0x54)
#define CCMU_APB2_CFG_GREG                (SUNXI_CCM_BASE + 0x58)
#define CCMU_AHB2_CFG_GREG                (SUNXI_CCM_BASE + 0x5C)

/* gate list */
#define CCMU_BUS_CLK_GATING_REG0          (SUNXI_CCM_BASE + 0x60)
#define CCMU_BUS_CLK_GATING_REG1          (SUNXI_CCM_BASE + 0x64)
#define CCMU_BUS_CLK_GATING_REG2          (SUNXI_CCM_BASE + 0x68)
#define CCMU_BUS_CLK_GATING_REG3          (SUNXI_CCM_BASE + 0x6C)
#define CCMU_BUS_CLK_GATING_REG4          (SUNXI_CCM_BASE + 0x70)

/* module list */
#define CCMU_NAND0_CLK_REG                (SUNXI_CCM_BASE + 0x80)
#define CCMU_SDMMC0_CLK_REG               (SUNXI_CCM_BASE + 0x88)
#define CCMU_SDMMC1_CLK_REG               (SUNXI_CCM_BASE + 0x8C)
#define CCMU_SDMMC2_CLK_REG               (SUNXI_CCM_BASE + 0x90)

#define CCMU_CE_CLK_REG                   (SUNXI_CCM_BASE + 0x9C)

#define CCMU_USBPHY_CLK_REG               (SUNXI_CCM_BASE + 0xCC)
#define CCMU_DRAM_CLK_REG                 (SUNXI_CCM_BASE + 0xF4)
#define CCMU_PLL_DDR_CFG_REG              (SUNXI_CCM_BASE + 0xF8)
#define CCMU_MBUS_RST_REG                 (SUNXI_CCM_BASE + 0xFC)
#define CCMU_DRAM_CLK_GATING_REG          (SUNXI_CCM_BASE + 0x100)

#define CCMU_AVS_CLK_REG                  (SUNXI_CCM_BASE + 0x144)
#define CCMU_MBUS_CLK_REG                 (SUNXI_CCM_BASE + 0x15C)

/*gate rst list*/
#define CCMU_BUS_SOFT_RST_REG0            (SUNXI_CCM_BASE + 0x2C0)
#define CCMU_BUS_SOFT_RST_REG1            (SUNXI_CCM_BASE + 0x2C4)
#define CCMU_BUS_SOFT_RST_REG2            (SUNXI_CCM_BASE + 0x2C8)
#define CCMU_BUS_SOFT_RST_REG3            (SUNXI_CCM_BASE + 0x2D0)
#define CCMU_BUS_SOFT_RST_REG4            (SUNXI_CCM_BASE + 0x2D8)

#endif

