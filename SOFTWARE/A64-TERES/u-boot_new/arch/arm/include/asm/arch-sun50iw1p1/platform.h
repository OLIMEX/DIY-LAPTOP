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

#ifndef __PLATFORM_H
#define __PLATFORM_H

#define SUNXI_SRAM_D_BASE                (0x00010000L)
#define SUNXI_SRAM_A2_BASE               (0x00040000L)
/* base address of modules */
#define SUNXI_DE_BASE                    (0x01000000L)
#define SUNXI_CORESIGHT_DEBUG_BASE       (0x01400000L)
#define SUNXI_CPU_MBIST_BASE             (0x01502000L)
#define SUNXI_CPUX_CFG_BASE              (0x01700000L)


#define SUNXI_SYSCRL_BASE			 (0x01c00000L)

#define SUNXI_DMA_BASE           		 (0x01c02000L)
#define SUNXI_NFC_BASE			         (0x01c03000L)
#define SUNXI_TSC_BASE			         (0x01c06000L)

#define SUNXI_TCON0_BASE			     (0x01c0c000L)
#define SUNXI_TCON1_BASE			     (0x01c0d000L)
#define SUNXI_VE_BASE				     (0x01c0e000L)

#define SUNXI_SMHC0_BASE			     (0x01c0f000L)
#define SUNXI_SMHC1_BASE			     (0x01c10000L)
#define SUNXI_SMHC2_BASE			     (0x01c11000L)

#define SUNXI_SID_BASE			         (0x01c14000L)
#define SUNXI_SS_BASE				 (0x01c15000L)

#define SUNXI_MSGBOX_BASE			 (0x01c17000L)
#define SUNXI_SPINLOCK_BASE		         (0x01c18000L)

#define SUNXI_USBOTG_BASE			     (0x01c19000L)
#define SUNXI_EHCI0_BASE			     (0x01c1a000L)
#define SUNXI_EHCI1_BASE			     (0x01c1b000L)

#define SUNXI_SMC_BASE			         (0x01c1e000L)

#define SUNXI_CCM_BASE			         (0x01c20000L)
#define SUNXI_PIO_BASE			         (0x01c20800L)
#define SUNXI_TIMER_BASE			     (0x01c20c00L)
#define SUNXI_SPDIF_BASE			     (0x01c21000L)
#define SUNXI_PWM03_BASE			     (0x01c21400L)

#define SUNXI_KEYADC_BASE			     (0x01c21800L)
#define SUNXI_DAUDIO0_BASE			     (0x01c22000L)
#define SUNXI_DAUDIO1_BASE			     (0x01c24000L)
#define SUNXI_DAUDIO2_BASE			     (0x01c28000L)

#define SUNXI_AC_BASE			         (0x01c22c00L)
#define SUNXI_SPC_BASE			         (0x01c23400L)
#define SUNXI_THC_BASE			         (0x01c25000L)

#define SUNXI_UART0_BASE			     (0x01c28000L)
#define SUNXI_UART1_BASE			     (0x01c28400L)
#define SUNXI_UART2_BASE			     (0x01c28800L)
#define SUNXI_UART3_BASE			     (0x01c28c00L)
#define SUNXI_UART4_BASE			     (0x01c29000L)

#define SUNXI_TWI0_BASE			         (0x01c2ac00L)
#define SUNXI_TWI1_BASE			         (0x01c2b000L)
#define SUNXI_TWI2_BASE			         (0x01c2b400L)
#define SUNXI_SCR_BASE			         (0x01c2c400L)

#define SUNXI_EMAC_BASE			         (0x01c30000L)
#define SUNXI_GPU_BASE			         (0x01c40000L)
#define SUNXI_HSTMR_BASE			 (0x01c60000L)

#define SUNXI_DRAMCOM_BASE		         (0x01c62000L)
#define SUNXI_DRAMCTL0_BASE		         (0x01c63000L)
#define SUNXI_DRAMPHY0_BASE		         (0x01c65000L)

#define SUNXI_SPI0_BASE			         (0x01c68000L)
#define SUNXI_SPI1_BASE			         (0x01c69000L)

#define ARMA9_SCU_BASE		             (0x01c80000L)
#define ARMA9_GIC_BASE		             (0x01c81000L)
#define ARMA9_CPUIF_BASE	             (0x01c82000L)

#define SUNXI_MIPI_DSI0_BASE		     (0x01ca0000L)
#define SUNXI_MIPI_DSIPHY_BASE		     (0x01ca1000L)

#define SUNXI_CSI0_BASE			         (0x01cb0000L)
#define SUNXI_DE_INTERLACED_BASE		 (0x01e00000L)
#define SUNXI_HDMI_BASE			         (0x01ee0000L)
#define HDMI_BASE                                SUNXI_HDMI_BASE

#define SUNXI_RTC_BASE			         (0x01f00000L)
#define SUNXI_RTMR01_BASE		         (0x01f00800L)
#define SUNXI_RINTC_BASE			 (0x01f00C00L)
#define SUNXI_RWDOG_BASE			 (0x01f01000L)
#define SUNXI_RPRCM_BASE			 (0x01f01400L)
#define SUNXI_RTWD_BASE			         (0x01f01800L)
#define SUNXI_RCPUCFG_BASE		         (0x01f01C00L)
#define SUNXI_RCIR_BASE			         (0x01f02000L)
#define SUNXI_RTWI_BASE			         (0x01f02400L)
#define SUNXI_RUART_BASE			 (0x01f02800L)
#define SUNXI_RPIO_BASE			         (0x01f02c00L)
#define SUNXI_RRSB_BASE			         (0x01f03400L)
#define SUNXI_RPWM_BASE			         (0x01f03800L)


#define SUNXI_CPUX_CFG_BASE_A32           (0x01700000)
#define RVBARADDR0_L		             (SUNXI_CPUX_CFG_BASE_A32+0xA0)
#define RVBARADDR0_H		             (SUNXI_CPUX_CFG_BASE_A32+0xA4)
#define RVBARADDR1_L		             (SUNXI_CPUX_CFG_BASE_A32+0xA8)
#define RVBARADDR1_H		             (SUNXI_CPUX_CFG_BASE_A32+0xAC)
#define RVBARADDR2_L		             (SUNXI_CPUX_CFG_BASE_A32+0xB0)
#define RVBARADDR2_H		             (SUNXI_CPUX_CFG_BASE_A32+0xB4)
#define RVBARADDR3_L		             (SUNXI_CPUX_CFG_BASE_A32+0xB8)
#define RVBARADDR3_H		             (SUNXI_CPUX_CFG_BASE_A32+0xBC)


#endif
