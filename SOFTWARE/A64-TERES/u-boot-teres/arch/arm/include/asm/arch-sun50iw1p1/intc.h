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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.         See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __SUNXI_INTC_H__
#define __SUNXI_INTC_H__

#ifndef FPGA_PLATFORM	//chip irq mapping

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

#define GIC_IRQ_NUM                    (AW_IRQ_AXI_ERROR + 1)


#else
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

#define GIC_IRQ_NUM                    (AW_IRQ_AXI_ERROR + 1)

#endif	//fpga irq mapping

extern int arch_interrupt_init (void);
extern int arch_interrupt_exit (void);
extern int irq_enable(int irq_no);
extern int irq_disable(int irq_no);
extern void irq_install_handler (int irq, interrupt_handler_t handle_irq, void *data);
extern void irq_free_handler(int irq);

#endif
