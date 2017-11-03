/*
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * Configuration settings for the Allwinner A10-evb board.
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

#include<config.h>

_TEXT_BASE:
	.word	CONFIG_SYS_TEXT_BASE	/* sdram load addr from config.mk */


.globl lowlevel_init
lowlevel_init:
	ldr	sp, SRAM_STACK
	str	ip, [sp]	/* stash old link register */
	mov	ip, lr		/* save link reg across call */
	bl	s_init		/* go setup pll, mux, memory */
	ldr	ip, [sp]	/* restore save ip */
	mov	lr, ip		/* restore link reg */

	/* back to arch calling code */
	mov	pc, lr

	/* the literal pools origin */
	.ltorg

SRAM_STACK:
	.word LOW_LEVEL_SRAM_STACK
