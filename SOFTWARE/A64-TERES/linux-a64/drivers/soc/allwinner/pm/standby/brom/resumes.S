/***********************************************************************************************************************
;                             Copyright(C), 2012-2015, Newbie Microelectronic Co., Ltd.
;                                                  All Rights Reserved
;
; File Name : resume1.S
;
; Author : yanggq
;
; Version : 1.1.0
;
; Date : 2012.7.11
;
; Description :
;
; Functions list : none.
;
; Others : None at present.
;
;
; History :
;
;  <Author>        <time>       <version>      <description>
;
; gq.Yang       2012.7.11      1.1.0        build the file
;
***********************************************************************************************************************/
#include <linux/linkage.h>
#include <linux/threads.h>
#include <asm/asm-offsets.h>
#include <asm/assembler.h>
#include <asm/glue-cache.h>
#include <asm/glue-proc.h>
#include <asm/cache.h>

#include "resumes_assembler.h"

#define  ARMV7_USR_MODE         0x10
#define  ARMV7_FIQ_MODE         0x11
#define  ARMV7_IRQ_MODE         0x12
#define  ARMV7_SVC_MODE         0x13
#define  ARMV7_MON_MODE         0x16
#define  ARMV7_ABT_MODE         0x17
#define  ARMV7_UND_MODE         0x1b
#define  ARMV7_SYSTEM_MODE      0x1f
#define  ARMV7_MODE_MASK        0x1f
#define  ARMV7_FIQ_MASK         0x40
#define  ARMV7_IRQ_MASK         0x80

#define   ARMV7_THUMB_MASK      (1<<5)
#define   ARMV7_END_MASK        (1<<9)

#define   ARMV7_IT_MASK         ((0x3f<<10)|(0x03<<25))
#define   ARMV7_GE_MASK         (0x0f<<16)
#define   ARMV7_JAVA_MASK       (0x01<<24)

#define   ARMV7_QFLAG_BIT       (1 << 27)
#define   ARMV7_CC_V_BIT        (1 << 28)
#define   ARMV7_CC_C_BIT        (1 << 29)
#define   ARMV7_CC_Z_BIT        (1 << 30)
#define   ARMV7_CC_N_BIT        (1 << 31)
#define   ARMV7_CC_E_BIT        (1 << 9 )

#define   ARMV7_C1_M_BIT        (1 << 0 )
#define   ARMV7_C1_A_BIT        (1 << 1 )
#define   ARMV7_C1_C_BIT        (1 << 2 )
#define   ARMV7_C1_Z_BIT        (1 << 11)
#define   ARMV7_C1_I_BIT        (1 << 12)

	/* PRESERVE8 */
	/* AREA	Monitor_Code, CODE, ALIGN=5, READONLY */

	/* Defines used in the code */
#define	Mode_MON	(0x16)
#define	Mode_SVC        (0x13)
#define	NS_BIT          (0x1)

	/* SCR Bit Masks */
	/* Bit masks for SCR bit settings (CP15-C1 SCR register) */
	/* For details see ARM ARM Security Extensions Supplement (DDI 0309B) */
#define SCR_BIT_0_NS                	(0x01)
#define SCR_BIT_1_IRQ_INTO_MON      	(0x02)
#define SCR_BIT_2_FIQ_INTO_MON      	(0x04)
#define SCR_BIT_3_EA_INTO_MON       	(0x08)
#define SCR_BIT_4_FW_MODIFY_ENABLE  	(0x10)
#define SCR_BIT_5_AW_MODIFY_ENABLE  	(0x20)
#define SCR_BIT_4_FW_MODIFY_DISABLE 	(0x00)
#define SCR_BIT_5_AW_MODIFY_DISABLE 	(0x00)

	// Bit masks for NSACR bit setting
#define NSACR_ACTLR_ENABLE     	(0x1<<18)
#define NSACR_L2ECTLR_ENABLE   	(0x1<<17)
#define NS_ACCESS_CP10_ENABLE	(0x1<<10)
#define NS_ACCESS_CP11_ENABLE	(0x1<<11)

	/* config SCR:  secure configuration register
	 * affect:      CPSR, mode entry, secure or non-secure state.
	 * config:      bit4,bit5: FW, AW; F bit Writable, A bit Writable -> 1;
	 *              can be modified in any security state.
	 *              in normal world, irq is take care by normal os
	 *              fiq is take care by FIQ mode.?
	 */
#define SCR_NS                  (SCR_BIT_0_NS | SCR_BIT_4_FW_MODIFY_ENABLE  | \
				SCR_BIT_5_AW_MODIFY_ENABLE)

	/* config nsacr: non-secure Access Control Register
	 * affect:      access to ACTLR.SMP, auxiliary	Ctrol Register.
	 *              access to L2ECTLR.AXI, L2 Extended Control Register.
	 *              access to CPACR.ASEDIS, Coprocessor Access Control Register
	 * config:      bit17, bit18: NS_L2ERR, NS_SMP -> 1; to allow non-secure write.
	 * config:      bit10, bit11: NS_ACCESS_CP10_ENABLE, NS_ACCESS_CP11_ENABLE ->1;
	 *                to allow non-secure write.
	 */
 #define NSACR			(NSACR_ACTLR_ENABLE | NSACR_L2ECTLR_ENABLE | \
				NS_ACCESS_CP10_ENABLE | NS_ACCESS_CP11_ENABLE)

	.globl cpu_brom_addr
	.text
	.arm
/**********************the begin of initializing system**********************/
	.globl _start
_start:
#if defined(CONFIG_ARCH_SUN8IW3P1) || defined(CONFIG_ARCH_SUN8IW5P1) || defined(CONFIG_ARCH_SUN8IW7P1)
	ldr     r0, =0x01c00024
	ldr     r1, [r0]
	/* 1 -> bit15 to enable write */
	orr     r1, #0x8000
	str     r1, [r0]
	/* read version */
	ldr     r1, [r0]
	lsr     r2, r1, #16
	ldr     r3, =0x1680
	cmp     r2, r3
	beq     config_80

	/* setting to 0x22223a22 */
config_50:
	ldr     r0, =0x01c00044
	ldr     r1,[r0]
	orr     r1,#0x1800
	str     r1,[r0]
	b       config_end

	/* setting to 0x222222e2 */
config_80:
	ldr     r0, =0x01c00044
	ldr     r1,[r0]
	orr     r1,#0xc0
	str     r1,[r0]

config_end:
#endif

	//ldr r0, =(SUNXI_SRAM_A1_PBASE + SRAMA1_PARA_OFFSET)
	ldr     r1, =(0x0 + 0x400)
#ifdef CONFIG_SUNXI_TRUSTZONE
	/* config banked gic */
	ldr     r1, [r0, #REGS_NUM]
	mov     r2, r0
	add     r2, r2, #REGS_OFFST
regs_resume:
	ldr     r3, [r2]
	ldr     r4, [r2, #0x04]
	str     r4, [r3]
	add     r2, r2, #8
	sub     r1, r1, #4
	cmp     r1, #0
	bne     regs_resume

	/* config TZPC, make sure rtc&alarm region is non-secure */
	//ldr     r4, =0x01c23408
	//ldr     r5, =0x2
	//str     r5,[r4]

	/* config cci */

	/* before restore secure mmu, u need to re-init reg base address */
	/* same as restore_secure_mmu_state(&(st_arisc_para.saved_secure_mmu_state)) */
	/* CR0 */
	ldr     r1, [r0, #CP15_CR0]
	mcr     p15, 2, r1, c0, c0, 0
	/* CR1 */
	ldr     r1, [r0, #CP15_CR1]
	mcr     p15, 0, r1, c1, c0, 2
	/* CR3 */
	ldr     r1, [r0, #CP15_DACR]
	mcr     p15, 0, r1, c3, c0, 0

	/* CR2 */
	/* when translate 0x0000,0000, use ttb0,
	 * while ttb0 shoudbe the same with ttb1
	 */
	ldr     r1, [r0, #CP15_TTB0R]
	mcr     p15, 0, r1, c2, c0, 0
	ldr     r1, [r0, #CP15_TTB1R]
	mcr     p15, 0, r1, c2, c0, 1
	ldr     r1, [r0, #CP15_TTBCR]
	mcr     p15, 0, r1, c2, c0, 2
	/* CR1 */
	/*cr: will effect visible addr space*/
	ldr     r1, [r0, #CP15_CR1]
	mcr     p15, 0, r1, c1, c0, 0
	/*read id reg*/
	mrc     p15, 0, r3, c0, c0, 0
	mov     r3, r3
	mov     r3, r3
	isb

	/*
	 * switch2normal
	 *
	 * This is called when the Secure world wishes to move to the Normal world.
	 * On entry: Must be in secure state.
	 */

	/* switch to normal world */
	/*mov monitor_vector to r1*/
	ldr     r1, [r0, #MONITOR_VEC]
	ldr     r2, [r0]
	mov     r0, r2

	/* config monitor vector */
	mcr     p15, 0, r1, c12, c0, 1

	/* config NSACR */
	mrc     p15, 0, r4, c1, c1, 2   /* Read */
	ldr     r2, =NSACR
	orr     r4, r2                  /* r2 can be a 32-bit -value */
	mcr     p15, 0, r4, c1, c1, 2   /* Write */

	/* Switch to Monitor mode */
	cps     #Mode_MON               /* Move to Monitor mode after saving Secure state */

	/* Set up execption return information */
	msr     spsr_cxsf, #Mode_SVC    /* Set SPSR to be SVC mode */

	/* Switch to Normal world */
	mrc     p15, 0, r4, c1, c1, 0   /* Read Secure Configuration Register data */
	orr     r4, #SCR_NS             /* Set NS bit */
	mcr     p15, 0, r4, c1, c1, 0   /* Write Secure Configuration Register data */

	/* ----------------------- */
	cps     #Mode_SVC
	ldr     R0, =0x0FFFFFF1
	.arch_extension sec
	smc     #0
#endif

	mrs     r0, cpsr
	bic     r0, r0, #ARMV7_MODE_MASK
	orr     r0, r0, #ARMV7_SVC_MODE
	orr     r0, r0, #( ARMV7_IRQ_MASK | ARMV7_FIQ_MASK )    @ After reset, ARM automaticly disables IRQ and FIQ, and runs in SVC mode.
	bic     r0, r0, #ARMV7_CC_E_BIT                         @ set little-endian
	msr     cpsr_c, r0

	/* config smp */

	/* configure memory system : disable MMU,cache and write buffer;
	 * set little_endian */
	mrc     p15, 0, r0, c1, c0
	bic     r0, r0, #( ARMV7_C1_M_BIT | ARMV7_C1_C_BIT )  @ disable MMU, data cache
	bic     r0, r0, #( ARMV7_C1_I_BIT | ARMV7_C1_Z_BIT )  @ disable instruction cache, disable flow prediction
	bic     r0, r0, #( ARMV7_C1_A_BIT)                    @ disable align
	mcr     p15, 0, r0, c1, c0

	dsb
	isb

#if defined(CONFIG_ARCH_SUN9IW1P1)
	mrc     p15, 0, r0, c0, c0, 5   @ Read CPU ID register
	ubfx    r0, r0, #8, #4          @ cluster
	cmp     r0, #1                  @ A15 cluster ?
	bne     A7f

	/* config A15 */
	@sun9i platform-specific Cortex-A15 setup.
	mrc     p15, 1, r1, c15, c0, 4      @ ACTLR2
	orr     r1, r1, #(0x1<<31)          @ Enable CPU regional clock gates
	mcr     p15, 1, r1, c15, c0, 4

	mrc     p15, 1, r1, c15, c0, 0      @ L2ACTLR
	orr     r1, r1, #(0x1<<26)          @ Enables L2, GIC, and Timer regional clock gates
	mcr     p15, 1, r1, c15, c0, 0

	mrc     p15, 1, r1, c15, c0, 0      @ L2ACTLR
	orr     r1, r1, #(0x1<<3)           @ Disables clean/evict from being pushed to external
	mcr     p15, 1, r1, c15, c0, 0

	mrc     p15, 1, r1, c9, c0, 2
	bic     r1, r1, #(0x7<<0)           @ L2 data ram latency
	orr     r1, r1, #(0x3<<0)
	mcr     p15, 1, r1, c9, c0, 2
	dsb
	isb
A7f:
#endif

	dsb
	isb

/************************the end of initializing system*********************/
	/* jumpt to cpu_resume */
	/*
	 * MMU is off so we need to get to various variables in a
	 * position independent way.
	 */
	//adr	r5, 3f
	//ldmia	r5, {r0, r6}
	//add	r0, r5, r0              @ r0 = cpu_brom_addr[0]
	//add	r6, r5, r6              @ r6 = cpu_brom_addr[1]
	//dmb
	//bx r0

	/* Clear general purpose registers*/
	mov     r2,  #0
	mov     r3,  #0
	mov     r4,  #0
	mov     r5,  #0
	mov     r6,  #0
	mov     r7,  #0
	mov     r8,  #0
	mov     r9,  #0
	mov     r10, #0
	mov     r11, #0
	mov     r12, #0
	/* Clear local monitor, Not strictly required in here as not using LDREX/STREX.
	 * However, architecturally should execute CLREX on a context switch
	 */
	clrex
	//ldr     r1, =(SUNXI_SRAM_A1_PBASE + SRAMA1_PARA_OFFSET)
	ldr     r1, =(0x0 + 0x400)
	ldr     r0, [r1, #RESUME_ADDR]
	dmb
	movs    pc, r0

 	b .                             @ infinite loop

 	.align	2
3: 	.word	cpu_brom_addr - .

	.type	cpu_brom_addr, #object
ENTRY(cpu_brom_addr)
	.space	8

	.end
