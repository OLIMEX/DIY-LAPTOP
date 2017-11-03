/*
**********************************************************************************************************************
*
*						           the Embedded Secure Bootloader System
*
*
*						       Copyright(C), 2006-2014, Allwinnertech Co., Ltd.
*                                           All Rights Reserved
*
* File    :
*
* By      :
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/
#include "common.h"
#include "asm/arch/platform.h"

/*----------------------------------------------------------------------------*/
/*   Description of SCR (Secure Configuration Register)                       */
/*                                                                            */
/*   |  bit6 | bit5 | bit4 | bit3 | bit2 | bit1 | bit0  |                     */
/*   |  bit6 | AW   | FW   | EA   | FIQ  | IRQ  | NS    |                     */
/*                                                                            */
/*   AW  =0 the CPSR.A bit can be modified only in Secure state               */
/*       =1 the CPSR.A bit can be modified in any security state              */
/*                                                                            */
/*   FW  =0 the CPSR.F bit can be modified only in Secure state               */
/*       =1 the CPSR.F bit can be modified in any security state              */
/*                                                                            */
/*   EA  =0 Abort mode handles external aborts                                */
/*       =1 Monitor mode handles external aborts                              */
/*                                                                            */
/*   FIQ =0 FIQ mode entered when FIQ is taken                                */
/*       =1 Monitor mode entered when FIQ is taken                            */
/*                                                                            */
/*   IRQ =0 IRQ mode entered when IRQ is taken                                */
/*       =1 Monitor mode entered when IRQ is taken                            */
/*                                                                            */
/*   NS  =0 Secure state                                                      */
/*       =1 Non-secure state                                                  */
/*                                                                            */
/*----------------------------------------------------------------------------*/

.globl secure_switch_unsecure

secure_switch_unsecure:

	mrc p15, 0, r2, c1, c1, 2
	ldr r3, =(0xC00 | (0x03<<18))
	orr r2, r2, r3
	mcr p15, 0, r2, c1, c1, 2       @使能NSACR的CP11,CP10

	mov r4, r0
	cps #0x16		                @读取CPSR
	msr spsr_cxsf, #0x13

	mrc p15, 0, r5, c1, c1, 0
    tst r5, #1
    bne __switch_out

    mov r0, #0x31		           @r0 = (1<<4 | 1<<3 | 1<<0);
    mcr p15, 0, r0, c1, c1, 0

	mov r0, #0			           @清理掉branch
    mcr p15, 0, r0, c7, c5, 6

__switch_out:
	movs pc, r4


.globl secure_switch_other

secure_switch_other:

	stmfd sp!, {r2, lr}
	mov r2, r0
	mov r0, r1

	blx r2
	ldmfd sp!, {r2, pc}

