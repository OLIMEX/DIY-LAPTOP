/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * sunny <sunny@allwinnertech.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#define TEE_SMC_NS_IRQ_DONE	(0x0FFFFFF7)
#define TEE_SMC_NS_IRQ_CALL	(0x0FFFFFF6)

.global sunxi_smc_call
.func sunxi_smc_call
sunxi_smc_call:
        stmfd	sp!, {r4-r12, lr}	@ save reg state
        mrs     r4,  cpsr
        push    {r4}
        cpsid	f
        cpsid	i
sunxi_issue_smc:
        .arch_extension sec
	    smc	#0
	    cmp     r0, #TEE_SMC_NS_IRQ_CALL
	    beq	sunxi_smc_irq_handle
	    pop    {r4}
	    msr     cpsr_cxsf, r4
	    ldmfd	sp!, {r4-r12, pc}	@ restore saved regs and return
sunxi_smc_irq_handle:
	    /*
	     * RIQ raised while in secure world, unmask IRQ temporarily to
	     * receive the IRQ in as it's normally recieved.
	     */
	    cpsie	i
	    nop
	    nop
	    nop
	    nop
	    dsb
        isb
        cpsid	i
        ldr     r0, =TEE_SMC_NS_IRQ_DONE
	    b       sunxi_issue_smc            @ Goback to secure world
.endfunc
