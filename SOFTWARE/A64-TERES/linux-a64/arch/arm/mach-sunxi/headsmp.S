/*
 * Secondary CPU startup routine source file.
 *
 * Copyright (c) Allwinner.  All rights reserved.
 *
 * Sugar <shuge@allwinnertech.com>
 *
 * Interface functions needed for the SMP. This file is based on arm
 * realview smp platform.
 * Copyright (c) 2003 ARM Limited.
 *
 * This program is free software,you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/linkage.h>
#include <linux/init.h>

	__CPUINIT

ENTRY(sunxi_secondary_startup)
	msr	cpsr_fsxc, #0xd3
	mov	r0, #0
	ldr	r1, =0x104d
	mrc p15, 0, r2, c0, c0, 5	 @ get core id
	and r2, r2, #0x3
	adr r3, cpus_boot_entry
	add r3, r2, lsl #2
	ldr r3, [r3]
	bx	r3
ENDPROC(sunxi_secondary_startup)

	.global cpus_boot_entry	 @ boot entry for every core
cpus_boot_entry:
	.space	CONFIG_NR_CPUS*4
