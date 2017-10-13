#ifndef _PM_CONFIG_SUN9IW1P1_H
#define _PM_CONFIG_SUN9IW1P1_H


/*
* Copyright (c) 2011-2015 yanggq.young@allwinnertech.com
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License version 2 as published by
* the Free Software Foundation.
*/

#include "pm_def_i.h"
#include "mach/platform.h"
#include "mach/memory.h"
#include "asm-generic/sizes.h"
//#include <generated/autoconf.h>
#include "mach/irqs.h"

//debug reg
#define STANDBY_STATUS_REG_PA 	(SUNXI_R_PRCM_PBASE + 0x1f0)
#define STANDBY_STATUS_REG 	IO_ADDRESS((STANDBY_STATUS_REG_PA))
#define STANDBY_STATUS_REG_NUM 	(4)				//reg1 - reg3 is available.

//module base reg
#define AW_LRADC01_BASE		(SUNXI_LRADC01_PBASE)
#define AW_CCM_BASE		(SUNXI_CCM_PLL_PBASE)
#define AW_CCM_MOD_BASE		(SUNXI_CCM_MOD_PBASE)
#define AW_SPINLOCK_BASE	(SUNXI_SPINLOCK_PBASE)

#define AW_CCU_UART_PA		(0x06000400 + 0x194)		//bit16: uart0;
#define AW_CCU_UART_RESET_PA	(0x06000400 + 0x1b4)		//bit16: uart0;
#define AW_CCM_PIO_BUS_GATE_REG_OFFSET  (0x190)

//Notice: jtag&uart_ph use the same addr, on sun9i platform.
#define AW_JTAG_PH_GPIO_PA		(0x06000800 + 0x100)		//jtag0: PH8-PH11,	bitmap: 0x2222
#define AW_UART_PF_GPIO_PA		(0x06000800 + 0xB4)		//uart0: PF2,PF4,		bitmap: 0x04,0400;

#define AW_UART_PH_GPIO_PA		(0x06000800 + 0x100)		//uart0: PH12,PH13,	bitmap: 0x22,0000
#define AW_JTAG_PF_GPIO_PA		(0x06000800 + 0xB4)		//jtag0: PF0,PF1,PF3,PF5	bitmap: 0x40,4044;

#define AW_JTAG_PH_CONFIG_VAL_MASK	(0x0000ffff)
#define AW_JTAG_PH_CONFIG_VAL		(0x00002222)
#define AW_JTAG_PF_CONFIG_VAL_MASK	(0x00f0f0ff)
#define AW_JTAG_PF_CONFIG_VAL		(0x00404044)

#define AW_UART_PH_CONFIG_VAL_MASK	(0x00ff0000)
#define AW_UART_PH_CONFIG_VAL		(0x00220000)
#define AW_UART_PF_CONFIG_VAL_MASK	(0x000F0F00)
#define AW_UART_PF_CONFIG_VAL		(0x00040400)

#define AW_JTAG_GPIO_PA			(AW_JTAG_PF_GPIO_PA)	
#define AW_UART_GPIO_PA			(AW_UART_PF_GPIO_PA)	
#define AW_JTAG_CONFIG_VAL_MASK		AW_JTAG_PF_CONFIG_VAL_MASK
#define AW_JTAG_CONFIG_VAL		AW_JTAG_PF_CONFIG_VAL

#define AW_RTC_BASE		(AW_SRAM_A1_BASE)		//notice: fake addr.
#define AW_SRAMCTRL_BASE	(SUNXI_SYS_CTRL_PBASE)
#define GPIO_REG_LENGTH		((0x324+0x4)>>2)
#define CPUS_GPIO_REG_LENGTH	((0x304+0x4)>>2)
#define SRAM_REG_LENGTH		((0xF0+0x4)>>2)
#define CCU_REG_LENGTH		((0x184+0x4)>>2)
#define CCU_MOD_REG_LENGTH	((0x1B4+0x4)>>2)
#define CCU_MOD_CLK_AHB1_RESET_SPINLOCK	    (AW_CCM_MOD_BASE + 0x1a4)
#define CCU_CLK_NRESET			    (0x1<<22)
#define CCU_MOD_CLK_AHB1_GATING_SPINLOCK    (AW_CCM_MOD_BASE + 0x184)
#define CCU_CLK_ON			    (0x1<<22)

/**start address for function run in sram*/
#define SRAM_FUNC_START    	(0xf0010000)
#define SRAM_FUNC_START_PA 	(0x00010000)
//for mem mapping
#define MEM_SW_VA_SRAM_BASE 	(0x00010000)
#define MEM_SW_PA_SRAM_BASE 	(0x00010000)
#define CPU_PLL_REST_DEFAULT_VAL	(0x02001100)
#define CPU_CLK_REST_DEFAULT_VAL	(0x00000000)	//SRC is HOSC.

//interrupt src definition
#define AW_IRQ_TIMER1		(0)
#define AW_IRQ_TOUCHPANEL	(0)
#define AW_IRQ_LRADC		(0)
#define AW_IRQ_NMI			(0)
#define AW_IRQ_MBOX		(SUNXI_IRQ_MBOX         )

#define AW_IRQ_ALARM		(0)
#define AW_IRQ_IR0		(0)
#define AW_IRQ_IR1		(0)

#define AW_IRQ_USBOTG		(SUNXI_IRQ_USB_OTG)
#define AW_IRQ_USBEHCI0		(SUNXI_IRQ_USB_EHCI0)
#define AW_IRQ_USBOHCI0		(SUNXI_IRQ_USB_OHCI0)
#define AW_IRQ_USBEHCI1		(SUNXI_IRQ_USB_EHCI1)
#define AW_IRQ_USBOHCI1		(SUNXI_IRQ_USB_OHCI1)
#define AW_IRQ_USBEHCI2		(SUNXI_IRQ_USB_EHCI2)
#define AW_IRQ_USBOHCI2		(SUNXI_IRQ_USB_OHCI2)


#define AW_IRQ_GPIOA		(0)
#define AW_IRQ_GPIOB		(SUNXI_IRQ_EINTB)
#define AW_IRQ_GPIOC		(0)
#define AW_IRQ_GPIOD		(0)
#define AW_IRQ_GPIOE		(0)
#define AW_IRQ_GPIOF		(0)
#define AW_IRQ_GPIOG		(SUNXI_IRQ_EINTG)
#define AW_IRQ_GPIOH		(0)
#define AW_IRQ_GPIOI		(0)
#define AW_IRQ_GPIOJ		(0)
#endif /*_PM_CONFIG_SUN9IW1P1_H*/

