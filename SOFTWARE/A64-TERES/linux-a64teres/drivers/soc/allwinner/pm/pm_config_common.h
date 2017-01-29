#ifndef _PM_CONFIG_COMMON_H
#define _PM_CONFIG_COMMON_H

/*
* Copyright (c) 2011-2015 yanggq.young@allwinnertech.com
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License version 2 as published by
* the Free Software Foundation.
*/


#define SUNXI_SRAM_A1_PBASE			(0)  
#define SUNXI_SRAM_A2_PBASE			(0)
#define SUNXI_PIO_PBASE				(0)
#define SUNXI_R_PRCM_PBASE			(0)
#define SUNXI_TWI2_PBASE			(0)
#define SUNXI_MSGBOX_PBASE			(0)
#define SUNXI_SPINLOCK_PBASE			(0)
#define SUNXI_R_PIO_PBASE			(0)
#define SUNXI_R_CPUCFG_PBASE			(0)
#define SUNXI_UART0_PBASE			(0)
#define SUNXI_TWI0_PBASE			(0)
#define SUNXI_TWI1_PBASE			(0)
#define SUNXI_CPUCFG_P_REG0			(0)
#define SUNXI_CPUCFG_GENCTL			(0)
#define SUNXI_CPUX_PWR_CLAMP(x)			(0)
#define SUNXI_CPUX_PWR_CLAMP_STATUS(x)		(0) 
#define SUNXI_CPU_PWROFF_REG			(0)
#define SUNXI_RTC_PBASE		(0)
#define SUNXI_SRAMCTRL_PBASE    (0)
#define SUNXI_LRADC_PBASE       (0)
#define SUNXI_GIC_DIST_PBASE	(0)
#define SUNXI_GIC_CPU_PBASE	(0)
#define SUNXI_TIMER_PBASE		(0)
#define SUNXI_CCM_PBASE			(0)

#if 0
#define AW_GPIO_BASE_PA         (0)
#define AW_CCM_BASE             (0)
#endif

#define SUNXI_IRQ_TIMER0      (50)		    	
#define SUNXI_IRQ_TIMER1      (51)			
#define SUNXI_IRQ_LRADC       (0)
#define SUNXI_IRQ_NMI         (0)
#define SUNXI_IRQ_ALARM0      (0)
#define SUNXI_IRQ_USBOTG      (0)
#define SUNXI_IRQ_USBEHCI0    (0)
#define SUNXI_IRQ_USBOHCI0    (0)
#define SUNXI_IRQ_EINTA       (0)
#define SUNXI_IRQ_EINTB       (0)
#define SUNXI_IRQ_EINTG       (0)
#define SUNXI_IRQ_MBOX        (0)

#if 0
#define AW_IRQ_TIMER1	      (0)
#define AW_IRQ_TOUCHPANEL     (0)
#define AW_IRQ_LRADC          (0)
#define AW_IRQ_NMI            (0)
#define AW_IRQ_MBOX           (0)
#define AW_IRQ_ALARM          (0)
#define AW_IRQ_IR0            (0)
#define AW_IRQ_IR1            (0)
#define AW_IRQ_USBOTG         (0)
#define AW_IRQ_USBEHCI0       (0)
#define AW_IRQ_USBEHCI1       (0)
#define AW_IRQ_USBEHCI2       (0)
#define AW_IRQ_USBOHCI0       (0)
#define AW_IRQ_USBOHCI1       (0)
#define AW_IRQ_USBOHCI2       (0)
#define AW_IRQ_GPIOA          (0)
#define AW_IRQ_GPIOB          (0)
#define AW_IRQ_GPIOC          (0)
#define AW_IRQ_GPIOD          (0)
#define AW_IRQ_GPIOE          (0)
#define AW_IRQ_GPIOF			(0)
#define AW_IRQ_GPIOG			(0)
#define AW_IRQ_GPIOH			(0)
#define AW_IRQ_GPIOI			(0)
#define AW_IRQ_GPIOJ			(0)
#endif

#define SUNXI_BANK_SIZE 32
#define SUNXI_PA_BASE	0
#define SUNXI_PB_BASE	32
#define SUNXI_PC_BASE	64
#define SUNXI_PD_BASE	96
#define SUNXI_PE_BASE	128
#define SUNXI_PF_BASE	160
#define SUNXI_PG_BASE	192
#define SUNXI_PH_BASE	224
#define SUNXI_PI_BASE	256
#define SUNXI_PJ_BASE	288
#define SUNXI_PK_BASE	320
#define SUNXI_PL_BASE	352
#define SUNXI_PM_BASE	384
#define SUNXI_PN_BASE	416
#define SUNXI_PO_BASE	448
#define AXP_PIN_BASE	1024

#endif /*_PM_CONFIG_COMMON_H*/

