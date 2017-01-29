/*
 * Sunxi platform smp source file.
 * It contains platform specific fucntions needed for the linux smp kernel.
 *
 * Copyright (c) Allwinner.  All rights reserved.
 * Sugar (shuge@allwinnertech.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/delay.h>
#include <asm/cacheflush.h>
#include <asm/io.h>

#include "platsmp.h"

extern void sunxi_secondary_startup(void);
extern void *cpus_boot_entry[NR_CPUS];
extern void secondary_startup(void);

static DEFINE_SPINLOCK(boot_lock);
void __iomem *sunxi_cpucfg_base;
void __iomem *sunxi_rtc_base;

static void sunxi_set_cpus_boot_entry(int cpu, void *entry)
{
	if (cpu < NR_CPUS) {
		cpus_boot_entry[cpu] = (void *)(virt_to_phys(entry));
		smp_wmb();
		__cpuc_flush_dcache_area(cpus_boot_entry, NR_CPUS * 4);
		outer_clean_range(__pa(&cpus_boot_entry), __pa(&cpus_boot_entry + 1));
	}
}

static void sunxi_smp_iomap_init(void)
{
	sunxi_cpucfg_base = ioremap(SUNXI_CPUCFG_PBASE, SZ_1K);
	sunxi_rtc_base = ioremap(SUNXI_RTC_PBASE, SZ_1K);
	pr_debug("cpucfg_base=0x%p rtc_base=0x%p\n", sunxi_cpucfg_base, sunxi_rtc_base);
}

static void sunxi_smp_init_cpus(void)
{
	unsigned int i, ncores;
	ncores = get_nr_cores();

	pr_debug("[%s] ncores=%d\n", __func__, ncores);

	 /* Limit possible CPUs to defconfig */
	if (ncores > nr_cpu_ids) {
		pr_warn("SMP: %u CPUs physically present. Only %d configured.",
			ncores, nr_cpu_ids);
		ncores = nr_cpu_ids;
	}

	for (i = 0; i < ncores; i++)
		set_cpu_possible(i, true);

	sunxi_smp_iomap_init();
	pr_debug("[%s] done\n", __func__);
}

static void sunxi_smp_prepare_cpus(unsigned int max_cpus)
{
	sunxi_set_secondary_entry((void *)(virt_to_phys(sunxi_secondary_startup)));
	pr_debug("[%s] done\n", __func__);
}

/*
 * Boot a secondary CPU, and assign it the specified idle task.
 * This also gives us the initial stack to use for this CPU.
 */
int sunxi_smp_boot_secondary(unsigned int cpu, struct task_struct *idle)
{
	spin_lock(&boot_lock);
	sunxi_set_cpus_boot_entry(cpu, secondary_startup);
	sunxi_enable_cpu(cpu);

	/*
	 * Now the secondary core is starting up let it run its
	 * calibrations, then wait for it to finish
	 */
	spin_unlock(&boot_lock);
	pr_debug("[%s] done\n", __func__);

	return 0;
}

struct smp_operations sunxi_smp_ops __initdata = {
	.smp_init_cpus		= sunxi_smp_init_cpus,
	.smp_prepare_cpus	= sunxi_smp_prepare_cpus,
	.smp_boot_secondary	= sunxi_smp_boot_secondary,
#ifdef CONFIG_HOTPLUG_CPU
	.cpu_die			= sunxi_cpu_die,
	.cpu_kill			= sunxi_cpu_kill,
	.cpu_disable		= sunxi_cpu_disable,
#endif
};
