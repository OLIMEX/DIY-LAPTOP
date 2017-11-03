/*
 * Copyright (C) 2013 Allwinnertech, kevin.z.m <kevin@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Adjustable factor-based clock implementation
 */
#ifndef __MACH_SUNXI_CLK_SUN8IW10_H
#define __MACH_SUNXI_CLK_SUN8IW10_H

#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/io.h>
#include "clk-factors.h"
/* register list */
#define PLL_CPU             0x0000
#define PLL_AUDIO           0x0008
#define PLL_VIDEO0          0x0010
#define PLL_DDR0            0x0020
#define PLL_PERIPH0         0x0028
#define PLL_VIDEO1          0x0030
#define PLL_24M             0X0034
#define PLL_PERIPH1         0x0044
#define PLL_DE              0x0048
#define PLL_DDR1            0x004c


#define CPU_CFG             0x0050
#define AHB1_CFG            0x0054
#define APB2_CFG            0x0058
#define BUS_GATE0           0x0060
#define BUS_GATE1           0x0064
#define BUS_GATE2           0x0068
#define BUS_GATE3           0x006c
#define BUS_GATE4           0x0070
#define GPADC_CFG           0x0078
#define THS_CFG             0x007c
#define NAND_CFG            0x0080
#define SD0_CFG             0x0088
#define SD1_CFG             0x008c
#define SD2_CFG             0x0090
#define SD3_CFG             0x0094
#define SPI0_CFG            0x00A0
#define SPI1_CFG            0x00A4
#define SPI2_CFG            0x00A8
#define I2S0_CFG            0x00B0
#define I2S1_CFG            0x00B4
#define SPDIF_CFG           0x00C0
#define DSD_CFG             0x00C4
#define DMIC_CFG            0x00C8
#define USB_CFG             0x00CC
#define PLL_DDR_CFG         0x00F0
#define DRAM_CFG            0x00F4
#define DDR1_CFG            0x00F8
#define MBUS_RST            0x00FC
#define DRAM_GATE           0x0100

#define DE_CFG              0x0104
#define EE_CFG              0x0108
#define EDMA_CFG            0x010C
#define TCON_CFG            0x0118
#define CSI_MISC            0x0130
#define CSI_CFG             0x0134
#define ADDA_CFG            0x0140
#define WLAN_CFG            0x0148
#define MBUS_CFG            0x015C

#define	PLL_LOCK            0x0200
#define	CPU_LOCK            0x0204

#define PLL_CPUPAT          0x0280
#define PLL_AUDIOPAT        0x0284
#define PLL_VIDEO0PAT       0x0288
#define PLL_VEDEO1PAT       0x0298
#define PLL_PERI1PAT        0x02A4
#define PLL_DEPAT           0x02A8
#define PLL_DDR0PAT0        0x02AC
#define PLL_DDR0PAT1        0x02B0
#define PLL_DDR1PAT0        0x02B4
#define PLL_DDR1PAT1        0x02B8

#define PLL_CLK_CTRL        0x0320

#define BUS_RST0            0x02C0
#define BUS_RST1            0x02C4
#define BUS_RST2            0x02D0
#define BUS_RST3            0x02D8

#define SUNXI_CLK_MAX_REG   0x0324

#define LOSC_OUT_GATE       0x01C20460

#define F_N8X7_M0X4(nv,mv) FACTOR_ALL(nv,8,7,0,0,0,mv,0,4,0,0,0,0,0,0,0,0,0)
#define F_N8X5_K4X2(nv,kv) FACTOR_ALL(nv,8,5,kv,4,2,0,0,0,0,0,0,0,0,0,0,0,0)
#define F_N8X7_M0X2(nv,mv) FACTOR_ALL(nv,8,7,0,0,0,mv,0,2,0,0,0,0,0,0,0,0,0)
#define F_N8X5_K4X2_M0X2(nv,kv,mv) FACTOR_ALL(nv,8,5,kv,4,2,mv,0,2,0,0,0,0,0,0,0,0,0)
#define F_N8X5_K4X2_M0X2_P16x2(nv,kv,mv,pv) \
               FACTOR_ALL(nv,8,5, \
                          kv,4,2, \
                          mv,0,2, \
                          pv,16,2, \
                          0,0,0,0,0,0)
#define F_N8X7_N116X5_M0X2_M14x4(nv,kv,mv,pv) \
               FACTOR_ALL(nv,8,7, \
                          kv,16,5, \
                          mv,0,2, \
                          pv,4,4, \
                          0,0,0,0,0,0)


#define PLLCPU(n,k,m,p,freq)    {F_N8X5_K4X2_M0X2_P16x2(n, k, m, p),  freq}
#define PLLVIDEO0(n,m,freq)     {F_N8X7_M0X4( n, m),  freq}
#define PLLDDR0(n,k,m,freq)     {F_N8X7_M0X2( n, m),  freq}
#define PLLPERIPH0(n,k,freq)    {F_N8X5_K4X2( n, k),  freq}
#define PLLPERIPH1(n,k,freq)    {F_N8X5_K4X2( n, k),  freq}
#define PLLVIDEO1(n,m,freq)     {F_N8X7_M0X4( n, m),  freq}
#define PLL24M(n,n1,m,m1,freq)  {F_N8X7_N116X5_M0X2_M14x4(n, n1, m, m1),  freq}
#define PLLDE(n,m,freq)         {F_N8X7_M0X4( n, m),  freq}
#define PLLDDR1(n,m,freq)       {F_N8X7_M0X2(n,m),  freq}

#endif
