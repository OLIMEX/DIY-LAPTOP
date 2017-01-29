/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __GIC_SUNXI_H_
#define __GIC_SUNXI_H_


#include "sunxi_def.h"
/*irq num*/
#define GIC_SRC_SPI(_n)                (32 + (_n))
#define AW_IRQ_UART0                   GIC_SRC_SPI(0)  /* 32 */
#define AW_IRQ_UART1                   GIC_SRC_SPI(1)  /* 33 */
#define AW_IRQ_UART2                   GIC_SRC_SPI(2)  /* 34 */
#define AW_IRQ_UART3                   GIC_SRC_SPI(3)  /* 35 */
#define AW_IRQ_UART4                   GIC_SRC_SPI(4)  /* 36 */

#define AW_IRQ_TWI0                    GIC_SRC_SPI(6)  /* 38 */
#define AW_IRQ_TWI1                    GIC_SRC_SPI(7)  /* 39 */
#define AW_IRQ_TWI2                    GIC_SRC_SPI(8)  /* 40 */

#define AW_IRQ_PBEINT                  GIC_SRC_SPI(11)  /* 43 */
#define AW_IRQ_OWA                     GIC_SRC_SPI(12)  /* 44*/
#define AW_IRQ_DAUDIO0                 GIC_SRC_SPI(13)  /* 45 */
#define AW_IRQ_DAUDIO1                 GIC_SRC_SPI(14)  /* 46 */
#define AW_IRQ_DAUDIO2                 GIC_SRC_SPI(15)  /* 47 */

#define AW_IRQ_PG_EINT                 GIC_SRC_SPI(17)  /* 49 */
#define AW_IRQ_TIMER0                  GIC_SRC_SPI(18)  /* 50 */
#define AW_IRQ_TIMER1                  GIC_SRC_SPI(19)  /* 51 */
#define AW_IRQ_PH_EINT                 GIC_SRC_SPI(21)  /* 53 */

#define AW_IRQ_WATCHDOG                GIC_SRC_SPI(25)  /* 57*/
#define AW_IRQ_NMI                     GIC_SRC_SPI(32)  /* 64*/


#define AW_IRQ_DMA                     GIC_SRC_SPI(50)  /* 82 */
#define AW_IRQ_HSTIMER                 GIC_SRC_SPI(51)  /* 83 */
#define AW_IRQ_SMC                     GIC_SRC_SPI(56)  /* 88 */
#define AW_IRQ_MMC0                    GIC_SRC_SPI(60)  /* 92 */
#define AW_IRQ_MMC1                    GIC_SRC_SPI(61)  /* 93 */
#define AW_IRQ_MMC2                    GIC_SRC_SPI(62)  /* 94 */
#define AW_IRQ_SPI0                    GIC_SRC_SPI(65)  /* 97 */
#define AW_IRQ_SPI1                    GIC_SRC_SPI(66)  /* 98 */
#define AW_IRQ_NAND                    GIC_SRC_SPI(70) /* 102 */
#define AW_IRQ_USB_OTG                 GIC_SRC_SPI(71) /* 103 */
#define AW_IRQ_USB_EHCI0               GIC_SRC_SPI(72) /* 104*/
#define AW_IRQ_USB_OHCI0               GIC_SRC_SPI(73) /* 105 */
#define AW_IRQ_USB_EHCI1               GIC_SRC_SPI(74) /* 106 */
#define AW_IRQ_USB_OHCI1               GIC_SRC_SPI(75) /* 107 */

#define AW_IRQ_CE0                     GIC_SRC_SPI(80) /* 112 */
#define AW_IRQ_TS                      GIC_SRC_SPI(81) /* 113 */
#define AW_IRQ_EMC                     GIC_SRC_SPI(82) /* 114 */
#define AW_IRQ_SCR                     GIC_SRC_SPI(83) /* 115 */
#define AW_IRQ_CSI                     GIC_SRC_SPI(84) /* 116 */
#define AW_IRQ_CSI_CCI                 GIC_SRC_SPI(85) /* 117 */
#define AW_IRQ_TCON0                   GIC_SRC_SPI(86) /* 118 */ 
#define AW_IRQ_TCON1                   GIC_SRC_SPI(87) /* 119 */
#define AW_IRQ_HDMI                    GIC_SRC_SPI(88) /* 120 */
#define AW_IRQ_MIPI_DSI                GIC_SRC_SPI(89) /* 121 */

#define AW_IRQ_DIT                     GIC_SRC_SPI(93) /* 125 */
#define AW_IRQ_CE1                     GIC_SRC_SPI(94) /* 126 */
#define AW_IRQ_DE                      GIC_SRC_SPI(95) /* 127 */
#define AW_IRQ_ROT                     GIC_SRC_SPI(96) /* 128 */
#define AW_IRQ_GPU_GP                  GIC_SRC_SPI(97) /* 129 */ 
#define AW_IRQ_GPU_GPMMU               GIC_SRC_SPI(98) /* 130 */ 
#define AW_IRQ_GPU_PP0                 GIC_SRC_SPI(99) /* 131 */ 
#define AW_IRQ_GPU_PP0MMU              GIC_SRC_SPI(100) /* 132 */ 
#define AW_IRQ_GPU_PMU                 GIC_SRC_SPI(101) /* 133 */ 
#define AW_IRQ_GPU_PP1                 GIC_SRC_SPI(102) /* 134 */ 
#define AW_IRQ_GPU_PPMMU1              GIC_SRC_SPI(103) /* 135 */ 

#define AW_IRQ_CTI0                   GIC_SRC_SPI(108) /* 140 */ 
#define AW_IRQ_CTI1                   GIC_SRC_SPI(109) /* 141 */ 
#define AW_IRQ_CTI2                   GIC_SRC_SPI(110) /* 142 */ 
#define AW_IRQ_CTI3                   GIC_SRC_SPI(111) /* 143 */ 
#define AW_IRQ_COMMTX0                GIC_SRC_SPI(112) /* 144 */ 
#define AW_IRQ_COMMTX1                GIC_SRC_SPI(113) /* 145 */ 
#define AW_IRQ_COMMTX2                GIC_SRC_SPI(114) /* 146 */ 
#define AW_IRQ_COMMTX3                GIC_SRC_SPI(115) /* 147 */ 
#define AW_IRQ_COMMRX0                GIC_SRC_SPI(116) /* 148 */ 
#define AW_IRQ_COMMRX1                GIC_SRC_SPI(117) /* 149 */ 
#define AW_IRQ_COMMRX2                GIC_SRC_SPI(118) /* 150 */ 
#define AW_IRQ_COMMRX3                GIC_SRC_SPI(119) /* 151 */ 
#define AW_IRQ_PMU0                   GIC_SRC_SPI(120) /* 152 */ 
#define AW_IRQ_PMU1                   GIC_SRC_SPI(121) /* 153 */ 
#define AW_IRQ_PMU2                   GIC_SRC_SPI(122) /* 154 */ 
#define AW_IRQ_PMU3                   GIC_SRC_SPI(123) /* 155 */ 
#define AW_IRQ_AXI_ERROR              GIC_SRC_SPI(124) /*156*/
#define GIC_IRQ_NUM                  (AW_IRQ_AXI_ERROR + 1)
/*irq num end*/



/* GIC registers */
#define GIC_DIST_CON		(ARMA9_GIC_BASE + 0x0000)
#define GIC_CON_TYPE		(ARMA9_GIC_BASE + 0x0004)
#define GIC_CON_IIDR		(ARMA9_GIC_BASE + 0x0008)

#define GIC_CON_IGRP(n)		(ARMA9_GIC_BASE + 0x0080 + (n)*4)

#define GIC_SET_EN(_n)		(ARMA9_GIC_BASE + 0x100 + 4 * (_n))
#define GIC_SET_EN0			GIC_SET_EN(0)	// 0x100
#define GIC_SET_EN1			GIC_SET_EN(1)   // 0x104
#define GIC_SET_EN2			GIC_SET_EN(2)   // 0x108
#define GIC_SET_EN3			GIC_SET_EN(3)   // 0x10c
#define GIC_SET_EN4			GIC_SET_EN(4)   // 0x110

#define GIC_CLR_EN(_n)		(ARMA9_GIC_BASE + 0x180 + 4 * (_n))
#define GIC_CLR_EN0			GIC_CLR_EN(0)	// 0x180
#define GIC_CLR_EN1			GIC_CLR_EN(1)   // 0x184
#define GIC_CLR_EN2			GIC_CLR_EN(2)   // 0x188
#define GIC_CLR_EN3			GIC_CLR_EN(3)   // 0x18c
#define GIC_CLR_EN4			GIC_CLR_EN(4)   // 0x190

#define GIC_PEND_SET(_n)	(ARMA9_GIC_BASE + 0x200 + 4 * (_n))
#define GIC_PEND_SET0		GIC_PEND_SET(0)	// 0x200
#define GIC_PEND_SET1		GIC_PEND_SET(1) // 0x204
#define GIC_PEND_SET2		GIC_PEND_SET(2) // 0x208
#define GIC_PEND_SET3		GIC_PEND_SET(3) // 0x20c
#define GIC_PEND_SET4		GIC_PEND_SET(4) // 0x210

#define GIC_PEND_CLR(_n)	(ARMA9_GIC_BASE + 0x280 + 4 * (_n))
#define GIC_PEND_CLR0		GIC_PEND_CLR(0)	// 0x280
#define GIC_PEND_CLR1		GIC_PEND_CLR(1) // 0x284
#define GIC_PEND_CLR2		GIC_PEND_CLR(2) // 0x288
#define GIC_PEND_CLR3		GIC_PEND_CLR(3) // 0x28c
#define GIC_PEND_CLR4		GIC_PEND_CLR(4) // 0x290

#define GIC_ACT_SET(_n)		(ARMA9_GIC_BASE + 0x300 + 4 * (_n))
#define GIC_ACT_SET0		GIC_ACT_SET(0)	// 0x300
#define GIC_ACT_SET1		GIC_ACT_SET(1)  // 0x304
#define GIC_ACT_SET2		GIC_ACT_SET(2)  // 0x308
#define GIC_ACT_SET3		GIC_ACT_SET(3)  // 0x30c
#define GIC_ACT_SET4		GIC_ACT_SET(4)  // 0x310

#define GIC_ACT_CLR(_n)		(ARMA9_GIC_BASE + 0x380 + 4 * (_n))
#define GIC_ACT_CLR0		GIC_ACT_CLR(0)	// 0x380
#define GIC_ACT_CLR1		GIC_ACT_CLR(1)  // 0x384
#define GIC_ACT_CLR2		GIC_ACT_CLR(2)  // 0x388
#define GIC_ACT_CLR3		GIC_ACT_CLR(3)  // 0x38c
#define GIC_ACT_CLR4		GIC_ACT_CLR(4)  // 0x390

#define GIC_SGI_PRIO(_n)	(ARMA9_GIC_BASE + 0x400 + 4 * (_n))
#define GIC_SGI_PRIO0		GIC_SGI_PRIO(0)	// 0x400
#define GIC_SGI_PRIO1		GIC_SGI_PRIO(1) // 0x404
#define GIC_SGI_PRIO2		GIC_SGI_PRIO(2) // 0x408
#define GIC_SGI_PRIO3		GIC_SGI_PRIO(3) // 0x40C

#define GIC_PPI_PRIO(_n)	(ARMA9_GIC_BASE + 0x410 + 4 * (_n))
#define GIC_PPI_PRIO0		GIC_PPI_PRIO(0)	// 0x410
#define GIC_PPI_PRIO1		GIC_PPI_PRIO(1) // 0x414
#define GIC_PPI_PRIO2		GIC_PPI_PRIO(2) // 0x418
#define GIC_PPI_PRIO3		GIC_PPI_PRIO(3) // 0x41C

#define GIC_SPI_PRIO(_n)	(ARMA9_GIC_BASE + 0x420 + 4 * (_n))
#define GIC_SPI_PRIO0		GIC_SPI_PRIO(0 )	// 0x420
#define GIC_SPI_PRIO1 		GIC_SPI_PRIO(1 )    // 0x424
#define GIC_SPI_PRIO2 		GIC_SPI_PRIO(2 )    // 0x428
#define GIC_SPI_PRIO3 		GIC_SPI_PRIO(3 )    // 0x42C
#define GIC_SPI_PRIO4 		GIC_SPI_PRIO(4 )    // 0x430
#define GIC_SPI_PRIO5 		GIC_SPI_PRIO(5 )    // 0x434
#define GIC_SPI_PRIO6 		GIC_SPI_PRIO(6 )    // 0x438
#define GIC_SPI_PRIO7 		GIC_SPI_PRIO(7 )    // 0x43C
#define GIC_SPI_PRIO8 		GIC_SPI_PRIO(8 )    // 0x440
#define GIC_SPI_PRIO9 		GIC_SPI_PRIO(9 )    // 0x444
#define GIC_SPI_PRIO10		GIC_SPI_PRIO(10)    // 0x448
#define GIC_SPI_PRIO11		GIC_SPI_PRIO(11)    // 0x44C
#define GIC_SPI_PRIO12		GIC_SPI_PRIO(12)    // 0x450
#define GIC_SPI_PRIO13		GIC_SPI_PRIO(13)    // 0x454
#define GIC_SPI_PRIO14		GIC_SPI_PRIO(14)    // 0x458
#define GIC_SPI_PRIO15		GIC_SPI_PRIO(15)    // 0x45C
#define GIC_SPI_PRIO16		GIC_SPI_PRIO(16)    // 0x460
#define GIC_SPI_PRIO17		GIC_SPI_PRIO(17)    // 0x464
#define GIC_SPI_PRIO18		GIC_SPI_PRIO(18)    // 0x468
#define GIC_SPI_PRIO19		GIC_SPI_PRIO(19)    // 0x46C
#define GIC_SPI_PRIO20		GIC_SPI_PRIO(20)    // 0x470
#define GIC_SPI_PRIO21		GIC_SPI_PRIO(21)    // 0x474
#define GIC_SPI_PRIO22		GIC_SPI_PRIO(22)    // 0x478
#define GIC_SPI_PRIO23		GIC_SPI_PRIO(23)    // 0x47C
#define GIC_SPI_PRIO24		GIC_SPI_PRIO(24)    // 0x480
#define GIC_SPI_PRIO25		GIC_SPI_PRIO(25)    // 0x484
#define GIC_SPI_PRIO26		GIC_SPI_PRIO(26)    // 0x488
#define GIC_SPI_PRIO27		GIC_SPI_PRIO(27)    // 0x48C
#define GIC_SPI_PRIO28		GIC_SPI_PRIO(28)    // 0x490
#define GIC_SPI_PRIO29		GIC_SPI_PRIO(29)    // 0x494
#define GIC_SPI_PRIO30		GIC_SPI_PRIO(30)    // 0x498
#define GIC_SPI_PRIO31		GIC_SPI_PRIO(31)    // 0x49C

#define GIC_SGI_PROC_TARG(_n)	(ARMA9_GIC_BASE + 0x800 + 4 * (_n))
#define GIC_SGI_PROC_TARG0		GIC_SGI_PROC_TARG(0)	// 0x800
#define GIC_SGI_PROC_TARG1		GIC_SGI_PROC_TARG(1)    // 0x804
#define GIC_SGI_PROC_TARG2		GIC_SGI_PROC_TARG(2)    // 0x808
#define GIC_SGI_PROC_TARG3		GIC_SGI_PROC_TARG(3)    // 0x80C

#define GIC_PPI_PROC_TARG(_n)	(ARMA9_GIC_BASE + 0x810 + 4 * (_n))
#define GIC_PPI_PROC_TARG0		GIC_PPI_PROC_TARG(0)	// 0x810
#define GIC_PPI_PROC_TARG1		GIC_PPI_PROC_TARG(1)    // 0x814
#define GIC_PPI_PROC_TARG2		GIC_PPI_PROC_TARG(2)    // 0x818
#define GIC_PPI_PROC_TARG3		GIC_PPI_PROC_TARG(3)    // 0x81C

#define GIC_SPI_PROC_TARG(_n)	(ARMA9_GIC_BASE + 0x820 + 4 * (_n))
#define GIC_SPI_PROC_TARG0 		GIC_SPI_PROC_TARG(0 )	// 0x820
#define GIC_SPI_PROC_TARG1 		GIC_SPI_PROC_TARG(1 )   // 0x824
#define GIC_SPI_PROC_TARG2 		GIC_SPI_PROC_TARG(2 )   // 0x828
#define GIC_SPI_PROC_TARG3 		GIC_SPI_PROC_TARG(3 )   // 0x82C
#define GIC_SPI_PROC_TARG4 		GIC_SPI_PROC_TARG(4 )   // 0x830
#define GIC_SPI_PROC_TARG5 		GIC_SPI_PROC_TARG(5 )   // 0x834
#define GIC_SPI_PROC_TARG6 		GIC_SPI_PROC_TARG(6 )   // 0x838
#define GIC_SPI_PROC_TARG7 		GIC_SPI_PROC_TARG(7 )   // 0x83C
#define GIC_SPI_PROC_TARG8 		GIC_SPI_PROC_TARG(8 )   // 0x840
#define GIC_SPI_PROC_TARG9 		GIC_SPI_PROC_TARG(9 )   // 0x844
#define GIC_SPI_PROC_TARG10		GIC_SPI_PROC_TARG(10)   // 0x848
#define GIC_SPI_PROC_TARG11		GIC_SPI_PROC_TARG(11)   // 0x84C
#define GIC_SPI_PROC_TARG12		GIC_SPI_PROC_TARG(12)   // 0x850
#define GIC_SPI_PROC_TARG13		GIC_SPI_PROC_TARG(13)   // 0x854
#define GIC_SPI_PROC_TARG14		GIC_SPI_PROC_TARG(14)   // 0x858
#define GIC_SPI_PROC_TARG15		GIC_SPI_PROC_TARG(15)   // 0x85C
#define GIC_SPI_PROC_TARG16		GIC_SPI_PROC_TARG(16)   // 0x860
#define GIC_SPI_PROC_TARG17		GIC_SPI_PROC_TARG(17)   // 0x864
#define GIC_SPI_PROC_TARG18		GIC_SPI_PROC_TARG(18)   // 0x868
#define GIC_SPI_PROC_TARG19		GIC_SPI_PROC_TARG(19)   // 0x86C
#define GIC_SPI_PROC_TARG20		GIC_SPI_PROC_TARG(20)   // 0x870
#define GIC_SPI_PROC_TARG21		GIC_SPI_PROC_TARG(21)   // 0x874
#define GIC_SPI_PROC_TARG22		GIC_SPI_PROC_TARG(22)   // 0x878
#define GIC_SPI_PROC_TARG23		GIC_SPI_PROC_TARG(23)   // 0x87C
#define GIC_SPI_PROC_TARG24		GIC_SPI_PROC_TARG(24)   // 0x880
#define GIC_SPI_PROC_TARG25		GIC_SPI_PROC_TARG(25)   // 0x884
#define GIC_SPI_PROC_TARG26		GIC_SPI_PROC_TARG(26)   // 0x888
#define GIC_SPI_PROC_TARG27		GIC_SPI_PROC_TARG(27)   // 0x88C
#define GIC_SPI_PROC_TARG28		GIC_SPI_PROC_TARG(28)   // 0x890
#define GIC_SPI_PROC_TARG29		GIC_SPI_PROC_TARG(29)   // 0x894
#define GIC_SPI_PROC_TARG30		GIC_SPI_PROC_TARG(30)   // 0x898
#define GIC_SPI_PROC_TARG31		GIC_SPI_PROC_TARG(31)   // 0x89C

#define GIC_IRQ_MOD_CFG(_n)		(ARMA9_GIC_BASE + 0xc00 + 4 * (_n))
#define GIC_IRQ_MOD_CFG0		GIC_IRQ_MOD_CFG(0)		// 0xc00 - SGI
#define GIC_IRQ_MOD_CFG1		GIC_IRQ_MOD_CFG(1)  	// 0xc04 - PPI
#define GIC_IRQ_MOD_CFG2		GIC_IRQ_MOD_CFG(2)  	// 0xc08 - SPI0 ~ 15
#define GIC_IRQ_MOD_CFG3		GIC_IRQ_MOD_CFG(3)  	// 0xc0C - SPI16 ~ 31
#define GIC_IRQ_MOD_CFG4		GIC_IRQ_MOD_CFG(4)  	// 0xc10 - SPI32 ~ 47
#define GIC_IRQ_MOD_CFG5		GIC_IRQ_MOD_CFG(5)  	// 0xc14 - SPI48 ~ 63
#define GIC_IRQ_MOD_CFG6		GIC_IRQ_MOD_CFG(6)  	// 0xc18 - SPI64 ~ 79
#define GIC_IRQ_MOD_CFG7		GIC_IRQ_MOD_CFG(7)  	// 0xc1C - SPI80 ~ 95
#define GIC_IRQ_MOD_CFG8		GIC_IRQ_MOD_CFG(8)  	// 0xc20 - SPI96 ~ 111
#define GIC_IRQ_MOD_CFG9		GIC_IRQ_MOD_CFG(9)  	// 0xc24 - SPI112 ~ 127

#define GIC_SOFT_IRQ_GEN		(ARMA9_GIC_BASE + 0xf00)	// 0xf00
#define GIC_SGI_PEND_SET(_n)	(ARMA9_GIC_BASE + 0xf10 + 4 * (_n))
#define GIC_SGI_PEND_SET0		GIC_SGI_PEND_SET(0)		// 0xf10
#define GIC_SGI_PEND_SET1		GIC_SGI_PEND_SET(1)		// 0xf14
#define GIC_SGI_PEND_SET2		GIC_SGI_PEND_SET(2)		// 0xf18
#define GIC_SGI_PEND_SET3		GIC_SGI_PEND_SET(3)		// 0xf1C
#define GIC_SGI_PEND_CLR(_n)	(ARMA9_GIC_BASE + 0xf10 + 4 * (_n))
#define GIC_SGI_PEND_CLR0		GIC_SGI_PEND_CLR(0)		// 0xf20
#define GIC_SGI_PEND_CLR1		GIC_SGI_PEND_CLR(1)		// 0xf24
#define GIC_SGI_PEND_CLR2		GIC_SGI_PEND_CLR(2)		// 0xf28
#define GIC_SGI_PEND_CLR3		GIC_SGI_PEND_CLR(3)		// 0xf2C


#define GIC_CPU_IF_CTRL			(ARMA9_CPUIF_BASE + 0x000)	// 0x8000
#define GIC_INT_PRIO_MASK		(ARMA9_CPUIF_BASE + 0x004) // 0x8004
#define GIC_BINARY_POINT		(ARMA9_CPUIF_BASE + 0x008) // 0x8008
#define GIC_INT_ACK_REG			(ARMA9_CPUIF_BASE + 0x00c) // 0x800c
#define GIC_END_INT_REG			(ARMA9_CPUIF_BASE + 0x010) // 0x8010
#define GIC_RUNNING_PRIO		(ARMA9_CPUIF_BASE + 0x014) // 0x8014
#define GIC_HIGHEST_PENDINT		(ARMA9_CPUIF_BASE + 0x018) // 0x8018
#define GIC_DEACT_INT_REG		(ARMA9_CPUIF_BASE + 0x1000)// 0x1000
#define GIC_AIAR_REG			(ARMA9_CPUIF_BASE + 0x020) // 0x8020
#define GIC_AEOI_REG			(ARMA9_CPUIF_BASE + 0x024) // 0x8024
#define GIC_AHIGHEST_PENDINT	(ARMA9_CPUIF_BASE + 0x028) // 0x8028


/* gic source list */
/* software generated interrupt */
#define GIC_SRC_SGI(_n)		(_n)
#define GIC_SRC_SGI0		GIC_SRC_SGI(0 )  // (0 )
#define GIC_SRC_SGI1        GIC_SRC_SGI(1 )  // (1 )
#define GIC_SRC_SGI2        GIC_SRC_SGI(2 )  // (2 )
#define GIC_SRC_SGI3        GIC_SRC_SGI(3 )  // (3 )
#define GIC_SRC_SGI4        GIC_SRC_SGI(4 )  // (4 )
#define GIC_SRC_SGI5        GIC_SRC_SGI(5 )  // (5 )
#define GIC_SRC_SGI6        GIC_SRC_SGI(6 )  // (6 )
#define GIC_SRC_SGI7        GIC_SRC_SGI(7 )  // (7 )
#define GIC_SRC_SGI8        GIC_SRC_SGI(8 )  // (8 )
#define GIC_SRC_SGI9        GIC_SRC_SGI(9 )  // (9 )
#define GIC_SRC_SGI10       GIC_SRC_SGI(10)  // (10)
#define GIC_SRC_SGI11       GIC_SRC_SGI(11)  // (11)
#define GIC_SRC_SGI12       GIC_SRC_SGI(12)  // (12)
#define GIC_SRC_SGI13       GIC_SRC_SGI(13)  // (13)
#define GIC_SRC_SGI14       GIC_SRC_SGI(14)  // (14)
#define GIC_SRC_SGI15       GIC_SRC_SGI(15)  // (15)
/* private peripheral interrupt */
#define GIC_SRC_PPI(_n)		(16 + (_n))
#define GIC_SRC_PPI0		GIC_SRC_PPI(0 )  // (16)
#define GIC_SRC_PPI1        GIC_SRC_PPI(1 )  // (17)
#define GIC_SRC_PPI2        GIC_SRC_PPI(2 )  // (18)
#define GIC_SRC_PPI3        GIC_SRC_PPI(3 )  // (19)
#define GIC_SRC_PPI4        GIC_SRC_PPI(4 )  // (20)
#define GIC_SRC_PPI5        GIC_SRC_PPI(5 )  // (21)
#define GIC_SRC_PPI6        GIC_SRC_PPI(6 )  // (22)
#define GIC_SRC_PPI7        GIC_SRC_PPI(7 )  // (23)
#define GIC_SRC_PPI8        GIC_SRC_PPI(8 )  // (24)
#define GIC_SRC_PPI9        GIC_SRC_PPI(9 )  // (25)
#define GIC_SRC_PPI10       GIC_SRC_PPI(10)  // (26)
#define GIC_SRC_PPI11       GIC_SRC_PPI(11)  // (27)
#define GIC_SRC_PPI12       GIC_SRC_PPI(12)  // (28)
#define GIC_SRC_PPI13       GIC_SRC_PPI(13)  // (29)
#define GIC_SRC_PPI14       GIC_SRC_PPI(14)  // (30)
#define GIC_SRC_PPI15       GIC_SRC_PPI(15)  // (31)
/* external peripheral interrupt */
#define GIC_SRC_SPI(_n)		(32 + (_n))


#endif

