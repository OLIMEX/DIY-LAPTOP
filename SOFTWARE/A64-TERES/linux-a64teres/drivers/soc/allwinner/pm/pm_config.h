#ifndef _PM_CONFIG_H
#define _PM_CONFIG_H


/*
* Copyright (c) 2011-2015 yanggq.young@allwinnertech.com
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License version 2 as published by
* the Free Software Foundation.
*/

#include "pm_def_i.h"
#ifndef CONFIG_ARCH_SUN8IW10P1
#include "pm_config_common.h"
#endif
#include "asm-generic/sizes.h"
//#include <generated/autoconf.h>

//hardware resource description
#ifdef CONFIG_ARCH_SUN8IW1P1
#include "pm_config-sun8iw1p1.h"
#elif defined CONFIG_ARCH_SUN8IW3P1
#include "pm_config-sun8iw3p1.h"
#elif defined CONFIG_ARCH_SUN8IW5P1
#include "pm_config-sun8iw5p1.h"
#elif defined CONFIG_ARCH_SUN8IW6P1
#include "pm_config-sun8iw6p1.h"
#elif defined CONFIG_ARCH_SUN8IW8P1
#include "pm_config-sun8iw8p1.h"
#elif defined CONFIG_ARCH_SUN8IW10P1
#include "pm_config-sun8iw10p1.h"
#elif defined CONFIG_ARCH_SUN9IW1P1
#include "pm_config-sun9iw1p1.h"
#elif defined CONFIG_ARCH_SUN50IW1P1
#include "pm_config-sun50iw1p1.h"
#endif

//#define CHECK_IC_VERSION

//#define RETURN_FROM_RESUME0_WITH_MMU    //suspend: 0xf000, resume0: 0xc010, resume1: 0xf000
//#define RETURN_FROM_RESUME0_WITH_NOMMU // suspend: 0x0000, resume0: 0x4010, resume1: 0x0000
//#define DIRECT_RETURN_FROM_SUSPEND //not support yet
#define ENTER_SUPER_STANDBY    //suspend: 0xf000, resume0: 0x4010, resume1: 0x0000
//#define ENTER_SUPER_STANDBY_WITH_NOMMU //not support yet, suspend: 0x0000, resume0: 0x4010, resume1: 0x0000
//#define WATCH_DOG_RESET

//NOTICE: only need one definiton
#define RESUME_FROM_RESUME1

#ifdef CONFIG_ARCH_SUN4I 
	#define PERMANENT_REG 		(0xf1c20d20)
	#define PERMANENT_REG_PA 	(0x01c20d20)
	#define STANDBY_STATUS_REG 		(0xf1c20d20)
	#define STANDBY_STATUS_REG_PA 		(0x01c20d20)
#elif defined(CONFIG_ARCH_SUN5I)
	#define PERMANENT_REG 		(0xF1c0123c)
	#define PERMANENT_REG_PA 	(0x01c0123c)
	#define STANDBY_STATUS_REG 		(0xf0000740)
	#define STANDBY_STATUS_REG_PA 		(0x00000740)
	//notice: the address is located in the last word of (DRAM_BACKUP_BASE_ADDR + DRAM_BACKUP_SIZE)
	#define SUN5I_STANDBY_STATUS_REG 	(DRAM_BACKUP_BASE_ADDR + (DRAM_BACKUP_SIZE<<2) -0x4)
	#define SUN5I_STANDBY_STATUS_REG_PA 	(DRAM_BACKUP_BASE_ADDR_PA + (DRAM_BACKUP_SIZE<<2) -0x4)
#endif


#if defined(CONFIG_ARCH_SUN8I) || defined(CONFIG_ARCH_SUN9IW1P1)
#define CORTEX_A7
#endif


/**********************************************platform separator *****************************************/
#ifdef CONFIG_ARCH_SUN8I
/**start address for function run in sram*/
#define SRAM_FUNC_START    	(0xf0000000)
#define SRAM_FUNC_START_PA 	(0x00000000)
//for mem mapping
#define MEM_SW_VA_SRAM_BASE 	(0x00000000)
#define MEM_SW_PA_SRAM_BASE 	(0x00000000)
//dram area
#define DRAM_BASE_ADDR      	(0xc0000000)
#define DRAM_BASE_ADDR_PA	(0x40000000)
#define DRAM_TRANING_SIZE   	(16)

#define CPU_CLK_REST_DEFAULT_VAL	(0x00010000)	//SRC is HOSC.

#endif


#ifdef CONFIG_ARCH_SUN4I
#define INT_REG_LENGTH		((0x90+0x4)>>2)
#define GPIO_REG_LENGTH		((0x218+0x4)>>2)
#define SRAM_REG_LENGTH		((0x94+0x4)>>2)
#elif defined CONFIG_ARCH_SUN5I
#define INT_REG_LENGTH		((0x94+0x4)>>2)
#define GPIO_REG_LENGTH		((0x218+0x4)>>2)
#define SRAM_REG_LENGTH		((0x94+0x4)>>2)
#endif


//interrupt src definition.
#define AW_IRQ_TIMER0		(SUNXI_IRQ_TIMER0	)

//platform independant src config.

#define AW_SRAM_A1_BASE		(SUNXI_SRAM_A1_PBASE)
#define AW_SRAM_A2_BASE		(SUNXI_SRAM_A2_PBASE)
#define AW_PIO_BASE		(SUNXI_PIO_PBASE)

#ifdef CONFIG_ARCH_SUN8IW8P1
#else
#define AW_R_PRCM_BASE		(SUNXI_R_PRCM_PBASE)
#define AW_TWI2_BASE		(SUNXI_TWI2_PBASE)
#define AW_MSGBOX_BASE		(SUNXI_MSGBOX_PBASE)
#define AW_SPINLOCK_BASE	(SUNXI_SPINLOCK_PBASE)
#define AW_R_PIO_BASE		(SUNXI_R_PIO_PBASE)
#endif

#define AW_R_CPUCFG_BASE	(SUNXI_R_CPUCFG_PBASE)
#define AW_UART0_BASE		(SUNXI_UART0_PBASE)

#define AW_TWI0_BASE		(SUNXI_TWI0_PBASE)
#define AW_TWI1_BASE		(SUNXI_TWI1_PBASE)
#define AW_CPUCFG_P_REG0	(SUNXI_CPUCFG_P_REG0)
#define AW_CPUCFG_GENCTL	(SUNXI_CPUCFG_GENCTL)
#define AW_CPUX_PWR_CLAMP(x)	(SUNXI_CPUX_PWR_CLAMP(x))
#define AW_CPUX_PWR_CLAMP_STATUS(x)	(SUNXI_CPUX_PWR_CLAMP_STATUS(x))
#define AW_CPU_PWROFF_REG	(SUNXI_CPU_PWROFF_REG)

#define SRAM_CTRL_REG1_ADDR_PA			0x01c00004
#define SRAM_CTRL_REG1_ADDR_VA			IO_ADDRESS(SRAM_CTRL_REG1_ADDR_PA)

#define RUNTIME_CONTEXT_SIZE			(14) //r0-r13

#define DRAM_COMPARE_DATA_ADDR			(0xc0100000) //1Mbytes offset
#define DRAM_COMPARE_SIZE			(0x10000) //?


#define __AC(X,Y)				(X##Y)
#define _AC(X,Y)				__AC(X,Y)
#define _AT(T,X)				((T)(X))
#define UL(x)					_AC(x, UL)

#define SUSPEND_FREQ				(720000)	//720M
#define SUSPEND_DELAY_MS			(10)


/**-----------------------------stack point address in sram-----------------------------------------*/
#define SP_IN_SRAM		0xf0003ffc //16k
#define SP_IN_SRAM_PA		0x00003ffc //16k
#define SP_IN_SRAM_START	(SRAM_FUNC_START_PA | 0x3c00) //15k  

#endif /*_PM_CONFIG_H*/

