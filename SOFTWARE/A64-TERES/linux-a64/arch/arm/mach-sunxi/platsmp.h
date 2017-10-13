/*
 * arch/arm/mach-sunxi/platsmp.h
 *
 * Copyright(c) 2013-2015 Allwinnertech Co., Ltd.
 *      http://www.allwinnertech.com
 *
 * Author: liugang <liugang@allwinnertech.com>
 *
 * sunxi smp ops header file
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __PLAT_SMP_H
#define __PLAT_SMP_H

#define get_nr_cores()					\
	({						\
		unsigned int __val;			\
		asm("mrc	p15, 1, %0, c9, c0, 2"	\
		    : "=r" (__val)			\
		    :					\
		    : "cc");				\
		((__val>>24) & 0x03) + 1;		\
	})

#define SUNXI_CPUCFG_PBASE			(0x01C25C00)
#define CPUCFG_CPUX_RST_CTRL(x)		(0x40 + (x)*0x40)
#define CPUCFG_CPUX_STATUS_REG(x)	(0x48 + (x)*0x40)
#define CPUCFG_GENER_CTRL_REG		(0x184)
#define CPUCFG_DEBUG_REG1			(0x1e4)

#define SUNXI_RTC_PBASE				(0x01C20400)
#define CPU_SOFT_ENTRY_REG0			(0x1e4)

extern void sunxi_cpu_die(unsigned int cpu);
extern int  sunxi_cpu_kill(unsigned int cpu);
extern int  sunxi_cpu_disable(unsigned int cpu);

extern void __iomem *sunxi_cpucfg_base;
extern void __iomem *sunxi_rtc_base;

static inline void sunxi_set_secondary_entry(void *entry)
{
	writel((u32)entry, sunxi_rtc_base + CPU_SOFT_ENTRY_REG0);
}

static inline int sunxi_is_wfi_mode(int cpu)
{
#ifdef CONFIG_EVB_PLATFORM
	return readl(sunxi_rtc_base + CPUCFG_CPUX_STATUS_REG(cpu)) & (1<<2);
#else
	return 1;
#endif
}

static inline void sunxi_enable_cpu(int cpu)
{
	unsigned int value;

	/*  Assert nCOREPORESET LOW and hold L1RSTDISABLE LOW.
	    Ensure DBGPWRDUP is held LOW to prevent any external
	    debug access to the processor.
	*/
	/* assert cpu core reset */
	writel(0, sunxi_cpucfg_base + CPUCFG_CPUX_RST_CTRL(cpu));

	/* L1RSTDISABLE hold low */
	value = readl(sunxi_cpucfg_base + CPUCFG_GENER_CTRL_REG);
	value &= ~(1<<cpu);
	writel(value, sunxi_cpucfg_base + CPUCFG_GENER_CTRL_REG);
	udelay(10);

	/* DBGPWRDUP hold low */
	value = readl(sunxi_cpucfg_base + CPUCFG_DEBUG_REG1);
	value &= ~(1<<cpu);
	writel(value, sunxi_cpucfg_base + CPUCFG_DEBUG_REG1);

	/* de-assert core reset */
	writel(3, sunxi_cpucfg_base + CPUCFG_CPUX_RST_CTRL(cpu));

	/* assert DBGPWRDUP signal */
	value = readl(sunxi_cpucfg_base + CPUCFG_DEBUG_REG1);
	value |= (1<<cpu);
	writel(value, sunxi_cpucfg_base + CPUCFG_DEBUG_REG1);
}

static inline void sunxi_disable_cpu(int cpu)
{
	unsigned int value;

	/* assert cpu core reset */
	writel(0, sunxi_cpucfg_base + CPUCFG_CPUX_RST_CTRL(cpu));

	/* DBGPWRDUP hold low */
	value = readl(sunxi_cpucfg_base + CPUCFG_DEBUG_REG1);
	value &= ~(1<<cpu);
	writel(value, sunxi_cpucfg_base + CPUCFG_DEBUG_REG1);

	/* power gating off */


	/* power switch off */

}

#endif /* __PLAT_SMP_H */
