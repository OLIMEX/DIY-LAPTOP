/*
*********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2006-2011, kevin.z China
*                                             All Rights Reserved
*
* File    : mem_gpio.h
* By      : 
* Version : v1.0
* Date    : 2011-5-31 14:34
* Descript:
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/
#ifndef __MEM_GPIO_H__
#define __MEM_GPIO_H__
#include "pm.h"

struct gpio_state{
	__u32 gpio_reg_back[GPIO_REG_LENGTH];
};
__s32 mem_gpio_init(void);
__s32 mem_gpio_save(struct gpio_state *pgpio_state);
__s32 mem_gpio_restore(struct gpio_state *pgpio_state);

#if defined(CONFIG_ARCH_SUN9IW1P1) || defined(CONFIG_ARCH_SUN8IW6P1)  || defined(CONFIG_ARCH_SUN8IW8P1) 
void config_gpio_clk(__u32 mmu_flag);
#endif

#endif  /* __MEM_GPIO_H__ */

