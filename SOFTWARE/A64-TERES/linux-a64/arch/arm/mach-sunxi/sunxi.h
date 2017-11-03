/*
 * Generic definitions for Allwinner SunXi SoCs
 *
 * Copyright (C) 2012 Maxime Ripard
 *
 * Maxime Ripard <maxime.ripard@free-electrons.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef __MACH_SUNXI_H
#define __MACH_SUNXI_H

#define SUNXI_REGS_PHYS_BASE	0x01c00000
#define SUNXI_REGS_VIRT_BASE	IOMEM(0xf1c00000)
#define SUNXI_REGS_SIZE		(SZ_2M + SZ_1M)

#ifdef CONFIG_ARCH_SUN8IW10P1
#define SUNXI_SRAM_A1_VBASE     (0xf0000000)
#define SUNXI_SRAM_A1_PBASE     (0x00000000)
#define SUNXI_SRAM_A1_SIZE      (0x4000)
#endif
#ifdef CONFIG_SMP
extern struct smp_operations sunxi_smp_ops;
#endif

#endif /* __MACH_SUNXI_H */
