/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Copyright (C) 2014 Allwinner Technology Co., Ltd.
 *
 * Author: sunny <sunny@allwinnertech.com>
 */

#include <linux/delay.h>
#include <asm/cacheflush.h>
#include <asm/cpu_ops.h>
#include <asm/smp_plat.h>
#include <linux/reboot.h>
#include <asm/system_misc.h>

#ifdef CONFIG_SMP

#ifdef CONFIG_HOTPLUG_CPU
static cpumask_t dead_cpus;
#endif

#define SUN50I_PRCM_PBASE	(0x01F01400)
#define SUN50I_CPUCFG_PBASE	(0x01700000)
#define SUN50I_RCPUCFG_PBASE	(0x01F01C00)
#define SUN50I_TIMER_PBASE      (0x01C20C00)

#define SUNXI_CPU_PWR_CLAMP(cluster, cpu)         (0x140 + (cluster*4 + cpu)*0x04)
#define SUNXI_CLUSTER_PWROFF_GATING(cluster)      (0x100 + (cluster)*0x04)
#define SUNXI_CLUSTER_PWRON_RESET(cluster)        (0x30  + (cluster)*0x04)

#define SUNXI_DBG_REG0                            (0x20)
#define SUNXI_CLUSTER_CPU_STATUS(cluster)         (0x30 + (cluster)*0x4)
#define SUNXI_CPU_RST_CTRL(cluster)               (0x80 + (cluster)*0x4)
#define SUNXI_CLUSTER_CTRL0(cluster)              (0x00 + (cluster)*0x10)

#define SUNXI_CPU_RVBA_L(cpu)	(0xA0 + (cpu)*0x8)
#define SUNXI_CPU_RVBA_H(cpu)   (0xA4 + (cpu)*0x8)

#define SUNXI_CPU_IS_WFI_MODE(cluster, cpu) \
	(readl(sun50i_cpucfg_base + SUNXI_CLUSTER_CPU_STATUS(cluster)) & (1 << (16 + cpu)))

static void __iomem *sun50i_prcm_base;
static void __iomem *sun50i_cpucfg_base;
static void __iomem *sun50i_r_cpucfg_base;
static void __iomem *sun50i_watchdog_base;

static void sun50i_set_secondary_entry(unsigned long entry, unsigned int cpu)
{
	pr_debug("%s: secondary_entry is 0x%lx\n", __func__, entry);

	writel(entry, sun50i_cpucfg_base + SUNXI_CPU_RVBA_L(cpu));
	writel(0, sun50i_cpucfg_base + SUNXI_CPU_RVBA_H(cpu));
}

static void sun50i_set_AA32nAA64(unsigned int cluster, unsigned int cpu, bool is_aa64)
{
	unsigned int value;

	value = readl(sun50i_cpucfg_base + SUNXI_CLUSTER_CTRL0(cluster));
	value &= ~(1<<(cpu + 24));
	value |= (is_aa64 <<(cpu + 24));
	writel(value, sun50i_cpucfg_base + SUNXI_CLUSTER_CTRL0(cluster));
}

static void sun50i_power_switch_set(unsigned int cluster, unsigned int cpu, bool enable)
{
	if (enable) {
		if (0x00 == readl(sun50i_prcm_base + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
			pr_debug("%s: power switch enable already\n", __func__);
			return;
		}

		/* de-active cpu power clamp */
		writel(0xFE, sun50i_prcm_base + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		udelay(20);

		writel(0xF8, sun50i_prcm_base + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		udelay(10);

		writel(0xE0, sun50i_prcm_base + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		udelay(10);

		writel(0x80, sun50i_prcm_base + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		udelay(10);

		writel(0x00, sun50i_prcm_base + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		udelay(20);

		while (0x00 != readl(sun50i_prcm_base + SUNXI_CPU_PWR_CLAMP(cluster, cpu)));
	} else {
		if (0xFF == readl(sun50i_prcm_base + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
			pr_debug("%s: power switch disable already\n", __func__);
			return;
		}

		writel(0xFF, sun50i_prcm_base + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		udelay(30);

		while (0xFF != readl(sun50i_prcm_base + SUNXI_CPU_PWR_CLAMP(cluster, cpu)));
	}
}

static void sun50i_cpu_power_up(unsigned int cluster, unsigned int cpu)
{
	unsigned int value;

	pr_debug("sun50i power-up cluster-%d cpu-%d\n", cluster, cpu);

	/* Assert nCPUPORESET LOW */
	value  = readl(sun50i_cpucfg_base + SUNXI_CPU_RST_CTRL(cluster));
	value &= (~(1<<cpu));
	writel(value, sun50i_cpucfg_base + SUNXI_CPU_RST_CTRL(cluster));

	/* Assert cpu power-on reset */
	value  = readl(sun50i_r_cpucfg_base + SUNXI_CLUSTER_PWRON_RESET(cluster));
	value &= (~(1<<cpu));
	writel(value, sun50i_r_cpucfg_base + SUNXI_CLUSTER_PWRON_RESET(cluster));

	/* set AA32nAA64 to AA64 */
	sun50i_set_AA32nAA64(cluster, cpu, 1);

	/* Apply power to the PDCPU power domain. */
	sun50i_power_switch_set(cluster, cpu, 1);

	/* Release the core output clamps */
	value = readl(sun50i_prcm_base + SUNXI_CLUSTER_PWROFF_GATING(cluster));
	value &= (~(0x1<<cpu));
	writel(value, sun50i_prcm_base + SUNXI_CLUSTER_PWROFF_GATING(cluster));
	dsb(sy);
	udelay(1);

	/* Deassert cpu power-on reset */
	value  = readl(sun50i_r_cpucfg_base + SUNXI_CLUSTER_PWRON_RESET(cluster));
	value |= ((1<<cpu));
	writel(value, sun50i_r_cpucfg_base + SUNXI_CLUSTER_PWRON_RESET(cluster));

	/* Deassert core reset */
	value  = readl(sun50i_cpucfg_base + SUNXI_CPU_RST_CTRL(cluster));
	value |= (1<<cpu);
	writel(value, sun50i_cpucfg_base + SUNXI_CPU_RST_CTRL(cluster));

	/* Assert DBGPWRDUP HIGH */
	value = readl(sun50i_cpucfg_base + SUNXI_DBG_REG0);
	value |= (1<<cpu);
	writel(value, sun50i_cpucfg_base + SUNXI_DBG_REG0);

	pr_debug("sun50i power-up cluster-%d cpu-%d ok\n", cluster, cpu);
}

#ifdef CONFIG_HOTPLUG_CPU
static void sun50i_cpu_power_down(unsigned int cluster, unsigned int cpu)
{
	unsigned int value;

	pr_debug("sun50i power-down cluster-%d cpu-%d\n", cluster, cpu);

	/* step7: Deassert DBGPWRDUP LOW */
	value = readl(sun50i_cpucfg_base + SUNXI_DBG_REG0);
	value &= (~(1<<cpu));
	writel(value, sun50i_cpucfg_base + SUNXI_DBG_REG0);

	/* step8: Activate the core output clamps */
	value = readl(sun50i_prcm_base + SUNXI_CLUSTER_PWROFF_GATING(cluster));
	value |= (1 << cpu);
	writel(value, sun50i_prcm_base + SUNXI_CLUSTER_PWROFF_GATING(cluster));
	dsb(sy);
	udelay(1);

	/* step9: Assert nCPUPORESET LOW */
	value  = readl(sun50i_cpucfg_base + SUNXI_CPU_RST_CTRL(cluster));
	value &= (~(1<<cpu));
	writel(value, sun50i_cpucfg_base + SUNXI_CPU_RST_CTRL(cluster));

	/* step10: Assert cpu power-on reset */
	value  = readl(sun50i_r_cpucfg_base + SUNXI_CLUSTER_PWRON_RESET(cluster));
	value &= (~(1<<cpu));
	writel(value, sun50i_r_cpucfg_base + SUNXI_CLUSTER_PWRON_RESET(cluster));

	/* step11: Remove power from th e PDCPU power domain */
	sun50i_power_switch_set(cluster, cpu, 0);

	pr_debug("sun50i power-down cluster-%d cpu-%d ok.\n", cluster, cpu);
}
#endif

static volatile int sunxi_iomap_init = 0;
static void sunxi_cpu_iomap_init(void)
{
	sun50i_prcm_base     = ioremap(SUN50I_PRCM_PBASE, SZ_4K);
	sun50i_cpucfg_base   = ioremap(SUN50I_CPUCFG_PBASE, SZ_4K);
	sun50i_r_cpucfg_base = ioremap(SUN50I_RCPUCFG_PBASE, SZ_4K);
	sun50i_watchdog_base = ioremap(SUN50I_TIMER_PBASE, SZ_4K);
}

static void sunxi_sys_reset(char str, const char *cmd)
{
	writel(0x0, sun50i_watchdog_base + 0xA0);
	writel(1, sun50i_watchdog_base + 0xB4);
	writel((0x3 << 4), sun50i_watchdog_base + 0xB8);
	writel(0x01, sun50i_watchdog_base + 0xB8);
	while (1)
		;
}

static int __init sunxi_cpu_init(struct device_node *dn, unsigned int cpu)
{
	return 0;
}

static int __init sunxi_cpu_prepare(unsigned int cpu)
{
	return 0;
}

static int sunxi_cpu_boot(unsigned int cpu)
{
	if (sunxi_iomap_init == 0) {
		sunxi_cpu_iomap_init();
		sunxi_iomap_init = 1;
	}

	arm_pm_restart = sunxi_sys_reset;

	sun50i_set_secondary_entry(__pa(secondary_entry), cpu_logical_map(cpu));
	sun50i_cpu_power_up(0, cpu_logical_map(cpu));

	return 0;
}

#ifdef CONFIG_HOTPLUG_CPU
static int sunxi_cpu_disable(unsigned int cpu)
{
	cpumask_clear_cpu(cpu, &dead_cpus);
	return cpu == 0 ? -EPERM : 0;
}

static int sunxi_cpu_kill(unsigned int cpu)
{
	int k, tmp_cpu = get_cpu();

	put_cpu();
	pr_debug("%s: cpu%d try to kill cpu%d\n", __func__, tmp_cpu, cpu);

	for (k = 0; k < 1000; k++) {
		if (cpumask_test_cpu(cpu, &dead_cpus) && SUNXI_CPU_IS_WFI_MODE(0, cpu)) {
			/* power-off cpu */
			sun50i_cpu_power_down(0, cpu_logical_map(cpu));
			pr_debug("%s: cpu%d is killed!\n", __func__, cpu);
			return 1;
		}

		mdelay(1);
	}

	pr_err("%s: try to kill cpu%d failed!\n", __func__, cpu);
	return 0;
}

static void sunxi_cpu_die(unsigned int cpu)
{
	unsigned long sctlr, cpuectlr;

	if (sunxi_iomap_init == 0) {
		sunxi_cpu_iomap_init();
		sunxi_iomap_init = 1;
	}

	cpumask_set_cpu(cpu, &dead_cpus);

	/* step1: Disable the data cache */
	asm("mrs %0, SCTLR_EL1\n" : "=r" (sctlr));
	sctlr &= ~(0x1<<2);
	asm volatile("msr SCTLR_EL1, %0\n" : : "r" (sctlr));

	/* step2: Clean and invalidate all data from the L1 Data cache */
	flush_cache_all();

	/* step3: Disable data coherency with other cores in the cluster */
	asm("mrs %0, S3_1_c15_c2_1\n" : "=r" (cpuectlr));
	cpuectlr &= ~(0x1<<6);
	asm volatile("msr S3_1_c15_c2_1, %0\n" : : "r" (cpuectlr));

	/*
	 * step4: Execute an ISB instruction to ensure that
	 * all of the register changes from the previous steps
	 * have been committed
	*/
	isb();

	/*
	 * step5: Execute a DSB SY instruction to ensure that all cache,
	 * TLB and branch predictor maintenance operations issued
	 * by any core in the cluster device before the SMPEN bit
	 * was cleared have completed
	*/
	dsb(sy);

	/*
	 * step6: Execute a  WFI  instruction and wait until the STANDBYWFI output
	 * is asserted to indicate that the core is in idle and low power state
	*/
	while (1) {
		asm("wfi" : : : "memory", "cc");
	}
}
#endif

#ifdef CONFIG_ARM64_CPU_SUSPEND
static int sunxi_cpu_suspend(unsigned long index)
{
	return -EOPNOTSUPP;
}
#endif

const struct cpu_operations cpu_ops_sunxi = {
	.name		= "sunxi",
	.cpu_init	= sunxi_cpu_init,
	.cpu_prepare	= sunxi_cpu_prepare,
	.cpu_boot	= sunxi_cpu_boot,
#ifdef CONFIG_HOTPLUG_CPU
	.cpu_disable	= sunxi_cpu_disable,
	.cpu_die	= sunxi_cpu_die,
	.cpu_kill 	= sunxi_cpu_kill,
#endif
#ifdef CONFIG_ARM64_CPU_SUSPEND
	.cpu_suspend	= sunxi_cpu_suspend,
#endif
};

#endif
