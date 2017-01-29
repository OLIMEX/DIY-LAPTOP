#ifndef _PM_CONFIG_SUN8IW6P1_H
#define _PM_CONFIG_SUN8IW6P1_H


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
#if 1 //for real ic
	#define STANDBY_STATUS_REG_PA 	(SUNXI_R_PRCM_PBASE + 0x1f0)
	#define STANDBY_STATUS_REG 	IO_ADDRESS((STANDBY_STATUS_REG_PA))
#else
	#define STANDBY_STATUS_REG_PA 	(0x40000000 + 0x1f0)
	#define STANDBY_STATUS_REG 	(0xc0000000 + 0x1f0)
#endif
	#define STANDBY_STATUS_REG_NUM 	(4)				//reg1 - reg3 is available.


//module base reg
#define AW_LRADC01_BASE		(0x0)				//notice: fake addr
#define AW_CCM_BASE		(SUNXI_CCM_PBASE)
#define AW_CCM_MOD_BASE		(SUNXI_CCM_PBASE)
#define AW_CCM_PIO_BUS_GATE_REG_OFFSET  (0x68)
#define AW_CCU_UART_PA		(AW_CCM_BASE + 0x6C)			//uart0 gating: bit16, 0: mask, 1: pass
#define AW_CCU_UART_RESET_PA	(AW_CCM_BASE + 0x2D8)			//uart0 reset: bit16, 0: reset, 1: de_assert

//uart&jtag para
#define AW_JTAG_PH_GPIO_PA              (0x01c20800 + 0x24)            //jtag0: Pb0-Pb3,
#define AW_JTAG_PF_GPIO_PA              (0x01c20800 + 0xB4)             //jtag0: PF0,PF1,PF3,PF5        bitmap: 0x40,4044;

#define AW_UART_PH_GPIO_PA              (0x01c20800 + 0x28)            //uart0: use pB
#define AW_UART_PF_GPIO_PA              (0x01c20800 + 0xB4)             //uart0: PF2,PF4,               bitmap: 0x04,0400;

#define AW_JTAG_PH_CONFIG_VAL_MASK      (0x0000ffff)
#define AW_JTAG_PH_CONFIG_VAL           (0x00003333)
#define AW_JTAG_PF_CONFIG_VAL_MASK      (0x00f0f0ff)
#define AW_JTAG_PF_CONFIG_VAL           (0x00303033)

#define AW_UART_PH_CONFIG_VAL_MASK      (0x00000FF0)
#define AW_UART_PH_CONFIG_VAL           (0x00000220)
#define AW_UART_PF_CONFIG_VAL_MASK      (0x000F0F00)
#define AW_UART_PF_CONFIG_VAL           (0x00030300)

#define AW_JTAG_GPIO_PA                 (AW_JTAG_PF_GPIO_PA)
#define AW_UART_GPIO_PA                 (AW_UART_PF_GPIO_PA)
#define AW_JTAG_CONFIG_VAL_MASK         AW_JTAG_PF_CONFIG_VAL_MASK
#define AW_JTAG_CONFIG_VAL              AW_JTAG_PF_CONFIG_VAL

#define AW_SPINLOCK_BASE	(SUNXI_SPINLOCK_PBASE)
#define AW_RTC_BASE		(AW_SRAM_A1_BASE)		//notice: fake addr.
#define AW_SRAMCTRL_BASE	(SUNXI_SRAMCTRL_PBASE)
#define GPIO_REG_LENGTH		((0x31c+0x4)>>2)		//0x24 - 0x31c
#define CPUS_GPIO_REG_LENGTH	((0x300+0x4)>>2)		//
#define SRAM_REG_LENGTH		((0xf0+0x4)>>2)
#define CCU_REG_LENGTH		((0x2d8+0x4)>>2)
#define CCU_MOD_CLK_AHB1_RESET_SPINLOCK	    (AW_CCM_MOD_BASE + 0x2c4)	//bit22, 1:de-assert, 0: assert.
#define CCU_CLK_NRESET			    (0x1<<22)
#define CCU_MOD_CLK_AHB1_GATING_SPINLOCK    (AW_CCM_MOD_BASE + 0x64)	//bit22,1: pass; 0: mask
#define CCU_CLK_ON			    (0x1<<22)

//irq src no.
#define AW_IRQ_TIMER1		(SUNXI_IRQ_TIMER1	)
#define AW_IRQ_TOUCHPANEL	(0)
#define AW_IRQ_LRADC		(SUNXI_IRQ_LRADC        )
#define AW_IRQ_NMI		(SUNXI_IRQ_NMI          )
#define AW_IRQ_MBOX		(SUNXI_IRQ_MBOX         )

#define AW_IRQ_ALARM		(0)
#define AW_IRQ_IR0		(0)
#define AW_IRQ_IR1		(0)
#define AW_IRQ_USBOTG		(SUNXI_IRQ_USBOTG)
#define AW_IRQ_USBEHCI0		(SUNXI_IRQ_USBEHCI0)
#define AW_IRQ_USBEHCI1		(SUNXI_IRQ_USBEHCI1)
#define AW_IRQ_USBOHCI0		(SUNXI_IRQ_USBOHCI0)
#define AW_IRQ_USBOHCI1		(0)
#define AW_IRQ_USBEHCI2		(0)
#define AW_IRQ_USBOHCI2		(0)

#define AW_IRQ_GPIOA		(0)
#define AW_IRQ_GPIOB		(SUNXI_IRQ_EINTB)
#define AW_IRQ_GPIOC		(0)
#define AW_IRQ_GPIOD		(0)
#define AW_IRQ_GPIOE		(0)
#define AW_IRQ_GPIOF		(0)
#define AW_IRQ_GPIOG		(SUNXI_IRQ_EINTG)
#define AW_IRQ_GPIOH		(SUNXI_IRQ_EINTH)
#define AW_IRQ_GPIOI		(0)
#define AW_IRQ_GPIOJ		(0)

#endif /*_PM_CONFIG_SUN8IW6P1_H*/

