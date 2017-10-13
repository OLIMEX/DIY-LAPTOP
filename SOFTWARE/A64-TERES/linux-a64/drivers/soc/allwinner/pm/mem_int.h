/*
*********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2006-2011, kevin.z China
*                                             All Rights Reserved
*
* File    : mem_int.h
* By      : gq.yang
* Version : v1.0
* Date    : 2012-11-3 20:13
* Descript: intterupt bsp for platform mem.
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/
#ifndef __MEM_INT_H__
#define __MEM_INT_H__
#include "pm.h"
#define GIC_400_ENABLE_LEN (0x40) //unit is byte. so in 32bit unit, the reg offset is 0-0x3c

/* define interrupt source */
enum interrupt_source_e{
	INT_SOURCE_TIMER0    = AW_IRQ_TIMER0,
	INT_SOURCE_TIMER1    = AW_IRQ_TIMER1,
	INT_SOURCE_TOUCHPNL  = AW_IRQ_TOUCHPANEL,
	INT_SOURCE_LRADC     = AW_IRQ_LRADC,
	INT_SOURCE_EXTNMI    = AW_IRQ_NMI,
	INT_SOURCE_MSG_BOX   = AW_IRQ_MBOX,
	INT_SOURCE_ALARM     = AW_IRQ_ALARM,
	INT_SOURCE_IR0	     = AW_IRQ_IR0,
	INT_SOURCE_IR1	     = AW_IRQ_IR1,
	INT_SOURCE_USBOTG    = AW_IRQ_USBOTG,
	INT_SOURCE_USBEHCI0  = AW_IRQ_USBEHCI0,
	INT_SOURCE_USBEHCI1  = AW_IRQ_USBEHCI1,
	INT_SOURCE_USBEHCI2  = AW_IRQ_USBEHCI2,
	INT_SOURCE_USBOHCI0  = AW_IRQ_USBOHCI0,
	INT_SOURCE_USBOHCI1  = AW_IRQ_USBOHCI1,
	INT_SOURCE_USBOHCI2  = AW_IRQ_USBOHCI2,
	INT_SOURCE_GPIOA     = AW_IRQ_GPIOA,
	INT_SOURCE_GPIOB     = AW_IRQ_GPIOB,
	INT_SOURCE_GPIOC     = AW_IRQ_GPIOC,
	INT_SOURCE_GPIOD     = AW_IRQ_GPIOD,
	INT_SOURCE_GPIOE     = AW_IRQ_GPIOE,
	INT_SOURCE_GPIOF     = AW_IRQ_GPIOF,
	INT_SOURCE_GPIOG     = AW_IRQ_GPIOG,
	INT_SOURCE_GPIOH     = AW_IRQ_GPIOH,
	INT_SOURCE_GPIOI     = AW_IRQ_GPIOI,
	INT_SOURCE_GPIOJ     = AW_IRQ_GPIOJ,
};

#define GIC_CPU_CTRL			0x00
#define GIC_CPU_PRIMASK			0x04
#define GIC_CPU_BINPOINT		0x08
#define GIC_CPU_INTACK			0x0c
#define GIC_CPU_EOI			0x10
#define GIC_CPU_RUNNINGPRI		0x14
#define GIC_CPU_HIGHPRI			0x18

#define GIC_DIST_CTRL			0x000
#define GIC_DIST_CTR			0x004
#define GIC_DIST_ENABLE_SET		0x100
#define GIC_DIST_ENABLE_CLEAR		0x180
#define GIC_DIST_PENDING_SET		0x200
#define GIC_DIST_PENDING_CLEAR		0x280
#define GIC_DIST_ACTIVE_BIT		0x300
#define GIC_DIST_PRI			0x400
#define GIC_DIST_TARGET			0x800
#define GIC_DIST_CONFIG			0xc00
#define GIC_DIST_SOFTINT		0xf00


extern __s32 mem_int_init(void);
extern __s32 mem_int_exit(void);
extern __s32 mem_enable_int(enum interrupt_source_e src);
extern __s32 mem_query_int(enum interrupt_source_e src);


#endif  //__MEM_INT_H__

