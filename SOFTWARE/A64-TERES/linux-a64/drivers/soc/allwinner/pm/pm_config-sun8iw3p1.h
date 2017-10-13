#ifndef _PM_CONFIG_SUN8IW3P1_H
#define _PM_CONFIG_SUN8IW3P1_H


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
#define STANDBY_STATUS_REG 	(0xf1f00100)
#define STANDBY_STATUS_REG_PA 	(0x01f00100)
#define STANDBY_STATUS_REG_NUM 	(4)

//module base addr
#define AW_LRADC01_BASE		(SUNXI_LRADC_PBASE)
#define AW_CCM_BASE		(SUNXI_CCM_PBASE)
#define AW_CCU_UART_PA		(0x01c2006C)
#define AW_CCU_UART_RESET_PA	(0x01c202D8)

//uart&jtag para
#define AW_JTAG_PH_GPIO_PA              (0x01c20800 + 0x00)            //jtag0: Pa0-Pa3,
#define AW_JTAG_PF_GPIO_PA              (0x01c20800 + 0xB4)             //jtag0: PF0,PF1,PF3,PF5        bitmap: 0x40,4044;

#define AW_UART_PH_GPIO_PA              (0x01c20800 + 0xb4)            //uart0: use pf
#define AW_UART_PF_GPIO_PA              (0x01c20800 + 0xB4)             //uart0: PF2,PF4,               bitmap: 0x04,0400;

#define AW_JTAG_PH_CONFIG_VAL_MASK      (0x0000ffff)
#define AW_JTAG_PH_CONFIG_VAL           (0x00003333)
#define AW_JTAG_PF_CONFIG_VAL_MASK      (0x00f0f0ff)
#define AW_JTAG_PF_CONFIG_VAL           (0x00303033)

#define AW_UART_PH_CONFIG_VAL_MASK      (0x000F0F00)
#define AW_UART_PH_CONFIG_VAL           (0x00030300)
#define AW_UART_PF_CONFIG_VAL_MASK      (0x000F0F00)
#define AW_UART_PF_CONFIG_VAL           (0x00030300)

#define AW_JTAG_GPIO_PA                 (AW_JTAG_PF_GPIO_PA)
#define AW_UART_GPIO_PA                 (AW_UART_PF_GPIO_PA)
#define AW_JTAG_CONFIG_VAL_MASK         AW_JTAG_PF_CONFIG_VAL_MASK
#define AW_JTAG_CONFIG_VAL              AW_JTAG_PF_CONFIG_VAL

#define AW_RTC_BASE		(SUNXI_RTC_PBASE)
#define AW_SRAMCTRL_BASE	(SUNXI_SRAMCTRL_PBASE)
#define GPIO_REG_LENGTH		((0x258+0x4)>>2)
#define CPUS_GPIO_REG_LENGTH	((0x218+0x4)>>2)
#define SRAM_REG_LENGTH		((0x94+0x4)>>2)
#define CCU_REG_LENGTH		((0x2d8+0x4)>>2)

//irq src no.
#define AW_IRQ_TIMER1		(SUNXI_IRQ_TIMER1	)
#define AW_IRQ_TOUCHPANEL	(0)
#define AW_IRQ_LRADC		(SUNXI_IRQ_LRADC        )
#define AW_IRQ_NMI			(SUNXI_IRQ_NMI          )
#define AW_IRQ_MBOX		(SUNXI_IRQ_MBOX         )

#define AW_IRQ_ALARM		(0)
#define AW_IRQ_IR0		(0)
#define AW_IRQ_IR1		(0)
#define AW_IRQ_USBOTG		(SUNXI_IRQ_USBOTG)
#define AW_IRQ_USBEHCI0		(SUNXI_IRQ_USBEHCI0)
#define AW_IRQ_USBEHCI1		(0)
#define AW_IRQ_USBEHCI2		(0)
#define AW_IRQ_USBOHCI0		(SUNXI_IRQ_USBOHCI0)
#define AW_IRQ_USBOHCI1		(0)
#define AW_IRQ_USBOHCI2		(0)

#define AW_IRQ_GPIOA		(SUNXI_IRQ_EINTA)
#define AW_IRQ_GPIOB		(SUNXI_IRQ_EINTB)
#define AW_IRQ_GPIOC		(0)
#define AW_IRQ_GPIOD		(0)
#define AW_IRQ_GPIOE		(0)
#define AW_IRQ_GPIOF		(0)
#define AW_IRQ_GPIOG		(SUNXI_IRQ_EINTG)
#define AW_IRQ_GPIOH		(0)
#define AW_IRQ_GPIOI		(0)
#define AW_IRQ_GPIOJ		(0)

#endif /*_PM_CONFIG_SUN8IW3P1_H*/

