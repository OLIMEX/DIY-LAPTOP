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

#ifndef __CCMU_H
#define __CCMU_H
#include <linux/types.h>
#include "platform.h"

#define CCMU_PLL_CPUX_CTRL_REG          (CCM_BASE + 0x00)
#define CCMU_PLL2_AUDIO_CTRL 		(CCM_BASE+0x008)
#define CCMU_PLL3_VIDEO_CTRL    	(CCM_BASE+0x010)
#define CCMU_PLL4_VE_CTRL     		(CCM_BASE+0x018)
#define CCMU_PLL5_DDR_CTRL  		(CCM_BASE+0x020)
#define CCMU_PLL_PERIPH0_CTRL_REG       (CCM_BASE+0x028)
#define CCMU_PLL7_VIDEO1_CTRL		(CCM_BASE+0x030)
#define CCMU_PLL8_GPU_CTRL  		(CCM_BASE+0x038)
#define CCMU_MIPI_PLL_CTRL		(CCM_BASE+0x040)
#define CCMU_HSIC_PLL_CTRL		(CCM_BASE+0x040)
#define CCMU_DE_PLL_CTRL		(CCM_BASE+0x048)
#define CCMU_PLL_DDR1_CTRL       	(CCM_BASE+0x04C)//--new



/* cfg list */
#define CCMU_CPUX_AXI_CFG_REG             (CCM_BASE + 0x50)
#define CCMU_AHB1_APB1_CFG_REG            (CCM_BASE + 0x54)
#define CCMU_APB2_CFG_GREG                (CCM_BASE + 0x58)
#define CCMU_AHB2_CFG_GREG                (CCM_BASE + 0x5C)

#define CCMU_BUS_CLK_GATING_REG0		(CCM_BASE+0x060)
#define CCMU_BUS_CLK_GATING_REG1		(CCM_BASE+0x064)
#define CCMU_BUS_CLK_GATING_REG2		(CCM_BASE+0x068)
#define CCMU_BUS_CLK_GATING_REG3		(CCM_BASE+0x06c)
#define CCMU_BUS_CLK_GATING_REG4		(CCM_BASE+0x070)

#define CCMU_NAND0_SCLK_CTRL		(CCM_BASE+0x080)
#define CCMU_NAND1_SCLK_CTRL		(CCM_BASE+0x084)

#define CCMU_SDMMC0_CLK_REG		(CCM_BASE+0x088)
#define CCMU_SDMMC1_CLK_REG		(CCM_BASE+0x08c)
#define CCMU_SDMMC2_CLK_REG		(CCM_BASE+0x090)
#define CCMU_SDMMC3_CLK_REG		(CCM_BASE+0x094)
#define CCMU_TS_SCLK_CTRL		(CCM_BASE+0x098)
#define CCMU_SS_SCLK_CTRL		(CCM_BASE+0x09c)
#define CCMU_SPI0_SCLK_CTRL		(CCM_BASE+0x0a0)
#define CCMU_SPI1_SCLK_CTRL		(CCM_BASE+0x0a4)
#define CCMU_SPI2_SCLK_CTRL		(CCM_BASE+0x0a8)
#define CCMU_SPI3_SCLK_CTRL		(CCM_BASE+0x0ac)
#define CCMU_I2S0_SCLK_CTRL		(CCM_BASE+0x0b0)
#define CCMU_I2S1_SCLK_CTRL		(CCM_BASE+0x0b4)

#define CCMU_SPDIF_SCLK_CTRL		(CCM_BASE+0x0c0)
           
#define CCMU_USBPHY_SCLK_CTRL	(CCM_BASE+0x0cc)
           
#define CCMU_MDFS_CLK_CTRL		(CCM_BASE+0x0f0)
#define CCMU_DRAMCLK_CFG_CTRL	(CCM_BASE+0x0f4)
#define CCMU_DDR_CFG_CTRL	    (CCM_BASE+0x0f8) //--new
#define CCMU_MBUS_RST_REG   (CCM_BASE+0x0fC ) //--new

#define CCMU_BUS_SOFT_RST_REG0     (CCM_BASE + 0x02c0)
#define CCMU_BUS_SOFT_RST_REG1     (CCM_BASE + 0x02c4)
#define CCMU_BUS_SOFT_RST_REG2     (CCM_BASE + 0x02d0)
#define CCMU_BUS_SOFT_RST_REG3     (CCM_BASE + 0x02d8)

#define CCMU_DRAMCLK_GATE_CTRL	(CCM_BASE+0x0100)
#define CCMU_BE0_SCLK_CTRL		(CCM_BASE+0x0104)
#define CCMU_BE1_SCLK_CTRL		(CCM_BASE+0x0108)
#define CCMU_FE0_SCLK_CTRL		(CCM_BASE+0x010c)
#define CCMU_FE1_SCLK_CTRL		(CCM_BASE+0x0110)
#define CCMU_MP_SCLK_CTRL		(CCM_BASE+0x0114)
#define CCMU_LCD0C0_SCLK_CTRL	(CCM_BASE+0x0118)
#define CCMU_LCD1C0_SCLK_CTRL	(CCM_BASE+0x011c)

#define CCMU_LCD0C1_SCLK_CTRL	(CCM_BASE+0x012c)
#define CCMU_LCD1C1_SCLK_CTRL	(CCM_BASE+0x0130)
#define CCMU_CSI0_SCLK_CTRL		(CCM_BASE+0x0134)
#define CCMU_CSI1_SCLK_CTRL		(CCM_BASE+0x0138)
#define CCMU_VE_SCLK_CTRL		(CCM_BASE+0x013c)
#define CCMU_CODEC_SCLK_CTRL		(CCM_BASE+0x0140)
#define CCMU_AVS_SCLK_CTRL		(CCM_BASE+0x0144)

#define CCMU_HDMI_SCLK_CTRL		(CCM_BASE+0x0150)

#define CCMU_MBUS_CLK_REG		(CCM_BASE+0x015c)
#define CCMU_MBUS_SCLK_CTRL1		(CCM_BASE+0x0160)

#define CCMU_MIPIDSI_SCLK_CTRL	(CCM_BASE+0x0168)
#define CCMU_MIPICSI0_SCLK_CTRL	(CCM_BASE+0x016c)

#define CCMU_DRC0_SCLK_CTRL		(CCM_BASE+0x0180)
#define CCMU_DRC1_SCLK_CTRL		(CCM_BASE+0x0184)
#define CCMU_DEU0_SCLK_CTRL		(CCM_BASE+0x0188)
#define CCMU_DEU1_SCLK_CTRL		(CCM_BASE+0x018c)

#define CCMU_GPU_CORECLK_CTRL	(CCM_BASE+0x01A0)
#define CCMU_GPU_MEMCLK_CTRL		(CCM_BASE+0x01A4)
#define CCMU_GPU_HYDCLK_CTRL		(CCM_BASE+0x01A8)

#define CCMU_PLL_STABLE_REG		(CCM_BASE+0x0200)
#define CCMU_MCLK_STABLE_REG		(CCM_BASE+0x0204)

#define CCMU_PPL1_BIAS_REG		(CCM_BASE+0x0220)
#define CCMU_PPL2_BIAS_REG		(CCM_BASE+0x0224)
#define CCMU_PPL3_BIAS_REG		(CCM_BASE+0x0228)
#define CCMU_PPL4_BIAS_REG		(CCM_BASE+0x022C)
#define CCMU_PPL5_BIAS_REG		(CCM_BASE+0x0230)
#define CCMU_PPL6_BIAS_REG		(CCM_BASE+0x0234)
#define CCMU_PPL7_BIAS_REG		(CCM_BASE+0x0238)
#define CCMU_PPL8_BIAS_REG		(CCM_BASE+0x023C)
#define CCMU_MIPIPLL_BIAS_REG	(CCM_BASE+0x0240)
#define CCMU_DDR1_BIAS_REG		(CCM_BASE+0x024C)


#define CCMU_PPL1_TUNE_REG		(CCM_BASE+0x0250)

#define CCMU_PPL5_TUNE_REG		(CCM_BASE+0x0260)

#define CCMU_MIPIPLL_TUNE_REG	(CCM_BASE+0x0270)

#define CCMU_PPL1_PAT_REG		(CCM_BASE+0x0280)
#define CCMU_PPL2_PAT_REG		(CCM_BASE+0x0284)
#define CCMU_PPL3_PAT_REG		(CCM_BASE+0x0288)
#define CCMU_PPL4_PAT_REG		(CCM_BASE+0x028C)
#define CCMU_PPL5_PAT_REG		(CCM_BASE+0x0290)

#define CCMU_PPL7_PAT_REG		(CCM_BASE+0x0298)
#define CCMU_PPL8_PAT_REG		(CCM_BASE+0x029C)

#define CCMU_MIPIPLL_PAT_REG		(CCM_BASE+0x02A0)
#define CCMU_DDR1_PAT_REG0		(CCM_BASE+0x02AC)
#define CCMU_DDR1_PAT_REG1		(CCM_BASE+0x02B0)



#define CCMU_AHB1_RST_REG0		(CCM_BASE+0x02C0)
#define CCMU_AHB1_RST_REG1		(CCM_BASE+0x02C4)
#define CCMU_AHB1_RST_REG2		(CCM_BASE+0x02C8)

#define CCMU_APB1_RST_REG		(CCM_BASE+0x02D0)
#define CCMU_APB2_RST_REG		(CCM_BASE+0x02D8)


#define CCMU_CLK_OUTA_REG		(CCM_BASE+0x0300)
#define CCMU_CLK_OUTB_REG		(CCM_BASE+0x0304)
#define CCMU_CLK_OUTC_REG		(CCM_BASE+0x0308)

/* cmmu pll ctrl bit field */
#define CCM_PLL_STABLE_FLAG		(1U << 28)

/* clock ID */
#define AXI_BUS		(0)
#define AHB1_BUS0	(1)
#define AHB1_BUS1	(2)
#define AHB1_LVDS	(3)
#define APB1_BUS0	(4)
#define APB2_BUS0	(5)
/* axi */
#define DRAMMEM_CKID	((AXI_BUS << 8) | 0)
/* ahb1 branc0 */
#define USBOHCI2_CKID	((AHB1_BUS0 << 8) | 31)
#define USBOHCI1_CKID	((AHB1_BUS0 << 8) | 30)
#define USBOHCI0_CKID	((AHB1_BUS0 << 8) | 29)
#define USBEHCI2_CKID	((AHB1_BUS0 << 8) | 28)
#define USBEHCI1_CKID	((AHB1_BUS0 << 8) | 27)
#define USBEHCI0_CKID	((AHB1_BUS0 << 8) | 26)
#define USB_OTG_CKID	((AHB1_BUS0 << 8) | 24)
#define SPI3_CKID		((AHB1_BUS0 << 8) | 23)
#define SPI2_CKID		((AHB1_BUS0 << 8) | 22)
#define SPI1_CKID		((AHB1_BUS0 << 8) | 21)
#define SPI0_CKID		((AHB1_BUS0 << 8) | 20)
#define HSTMR_CKID		((AHB1_BUS0 << 8) | 19)
#define TS_CKID			((AHB1_BUS0 << 8) | 18)
#define GMAC_CKID		((AHB1_BUS0 << 8) | 17)
#define DRAMREG_CKID	((AHB1_BUS0 << 8) | 14)
#define NAND0_CKID		((AHB1_BUS0 << 8) | 13)
#define NAND1_CKID		((AHB1_BUS0 << 8) | 12)
#define SDC3_CKID		((AHB1_BUS0 << 8) | 11)
#define SDC2_CKID		((AHB1_BUS0 << 8) | 10)
#define SDC1_CKID		((AHB1_BUS0 << 8) | 9 )
#define SDC0_CKID		((AHB1_BUS0 << 8) | 8 )
#define DMA_CKID		((AHB1_BUS0 << 8) | 6 )
#define SS_CKID			((AHB1_BUS0 << 8) | 5 )
#define MIPIDSI_CKID	((AHB1_BUS0 << 8) | 1 )
#define MIPICSI_CKID	((AHB1_BUS0 << 8) | 0 )
/* ahb1 branc1 */
#define DRC1_CKID		((AHB1_BUS1 << 8) | 26)
#define DRC0_CKID		((AHB1_BUS1 << 8) | 25)
#define DEU1_CKID		((AHB1_BUS1 << 8) | 24)
#define DEU0_CKID		((AHB1_BUS1 << 8) | 23)
#define SPINLOCK_CKID	((AHB1_BUS1 << 8) | 22)
#define MSGBOX_CKID		((AHB1_BUS1 << 8) | 21)
#define GPU_CKID		((AHB1_BUS1 << 8) | 20)
#define MP_CKID			((AHB1_BUS1 << 8) | 18)
#define FE1_CKID		((AHB1_BUS1 << 8) | 15)
#define FE0_CKID		((AHB1_BUS1 << 8) | 14)
#define BE1_CKID		((AHB1_BUS1 << 8) | 13)
#define BE0_CKID		((AHB1_BUS1 << 8) | 12)
#define HDMI_CKID		((AHB1_BUS1 << 8) | 11)
#define CSI1_CKID		((AHB1_BUS1 << 8) | 9)
#define CSI0_CKID		((AHB1_BUS1 << 8) | 8)
#define LCD1_CKID		((AHB1_BUS1 << 8) | 5)
#define LCD0_CKID		((AHB1_BUS1 << 8) | 4)
#define VE_CKID			((AHB1_BUS1 << 8) | 0)
/* ahb1 special for lvds */
#define LVDS_CKID		((AHB1_LVDS << 8) | 0)

/* apb1  */
#define IIS1_CKID		((APB1_BUS0 << 8) | 13)
#define IIS0_CKID		((APB1_BUS0 << 8) | 12)
#define KP_CKID			((APB1_BUS0 << 8) | 10)
#define GPADC_CKID		((APB1_BUS0 << 8) | 8)
#define PIO_CKID		((APB1_BUS0 << 8) | 5)
#define SPDIF_CKID		((APB1_BUS0 << 8) | 1)
#define CODEC_CKID		((APB1_BUS0 << 8) | 0)
/* apb2  */
#define UART4_CKID		((APB2_BUS0 << 8) | 20)
#define UART3_CKID		((APB2_BUS0 << 8) | 19)
#define UART2_CKID		((APB2_BUS0 << 8) | 18)
#define UART1_CKID		((APB2_BUS0 << 8) | 17)
#define UART0_CKID		((APB2_BUS0 << 8) | 16)
#define STWI_CKID		((APB2_BUS0 << 8) | 4)
#define TWI3_CKID		((APB2_BUS0 << 8) | 3)
#define TWI2_CKID		((APB2_BUS0 << 8) | 2)
#define TWI1_CKID		((APB2_BUS0 << 8) | 1)
#define TWI0_CKID		((APB2_BUS0 << 8) | 0)

#define CPUCLK_SRC_32K		(0)
#define CPUCLK_SRC_24M		(1)
#define CPUCLK_SRC_PLL1		(2)

#define AHB1CLK_SRC_LOSC	(0)
#define AHB1CLK_SRC_24M		(1)
#define AHB1CLK_SRC_AXI		(2)
#define AHB1CLK_SRC_PLL6D	(3)

#define APB2CLK_SRC_LOSC	(0)
#define APB2CLK_SRC_24M		(1)
#define APB2CLK_SRC_PLL6	(2)

#define MBUSCLK_SRC_24M		(0)
#define MBUSCLK_SRC_PLL6	(1)
#define MBUSCLK_SRC_PLL5	(2)


#endif
