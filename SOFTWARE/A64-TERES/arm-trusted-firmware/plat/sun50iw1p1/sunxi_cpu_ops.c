/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <arch.h>
#include <arch_helpers.h>
#include <platform.h>
#include <platform_def.h>
#include <mmio.h>
#include <debug.h>
#include <bakery_lock.h>
#include "sunxi_def.h"
#include "sunxi_private.h"

#define SUN50I_PRCM_PBASE	(0x01F01400)
#define SUN50I_CPUCFG_PBASE	(0x01700000)
#define SUN50I_RCPUCFG_PBASE	(0x01F01C00)
 
#define SUNXI_CPU_PWR_CLAMP(cluster, cpu)         (0x140 + (cluster*4 + cpu)*0x04)
#define SUNXI_CLUSTER_PWROFF_GATING(cluster)      (0x100 + (cluster)*0x04)
#define SUNXI_CLUSTER_PWRON_RESET(cluster)        (0x30  + (cluster)*0x04)
 
#define SUNXI_DBG_REG0                            (0x20)
#define SUNXI_CLUSTER_CPU_STATUS(cluster)         (0x30 + (cluster)*0x4)
#define SUNXI_CPU_RST_CTRL(cluster)               (0x80 + (cluster)*0x4)
#define SUNXI_CLUSTER_CTRL0(cluster)              (0x00 + (cluster)*0x10)
 
#define SUNXI_CPU_RVBA_L(cpu)	(0xA0 + (cpu)*0x8)
#define SUNXI_CPU_RVBA_H(cpu)   (0xA4 + (cpu)*0x8)
 
#define readl(x) mmio_read_32((x))
#define writel(v, a)	 mmio_write_32((a), (v))
 typedef unsigned int bool;
 
 static unsigned int sun50i_cpucfg_base = SUN50I_CPUCFG_PBASE;
 static unsigned int sun50i_prcm_base = SUN50I_PRCM_PBASE;
 static unsigned int sun50i_r_cpucfg_base = SUN50I_RCPUCFG_PBASE;
 extern bakery_lock_t plat_console_lock;
 void udelay(unsigned int delay)
 {
	 unsigned int i, j;
 
	 for (i=0; i<1000*delay; i++)
	 {
		 j+=i;
	 }
 }
 
 void sun50i_set_secondary_entry(unsigned long entry, unsigned int cpu)
 {
	 mmio_write_32(sun50i_cpucfg_base + SUNXI_CPU_RVBA_L(cpu) ,entry);
	 mmio_write_32(sun50i_cpucfg_base + SUNXI_CPU_RVBA_H(cpu), 0);
 }
 
 void sun50i_set_AA32nAA64(unsigned int cluster, unsigned int cpu, bool is_aa64)
 {
	 volatile unsigned int value;
 
	 value = readl(sun50i_cpucfg_base + SUNXI_CLUSTER_CTRL0(cluster));
	 value &= ~(1<<(cpu + 24));
	 value |= (is_aa64 <<(cpu + 24));
	 writel(value, sun50i_cpucfg_base + SUNXI_CLUSTER_CTRL0(cluster));
	 value = readl(sun50i_cpucfg_base + SUNXI_CLUSTER_CTRL0(cluster));
 }
 
 int  sun50i_power_switch_set(unsigned int cluster, unsigned int cpu, bool enable)
 {
	 if (enable) {
		 if (0x00 == readl(sun50i_prcm_base + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
			 NOTICE("%s: power switch enable already\n", __func__);
			 return 0;
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
			 NOTICE("%s: power switch disable already\n", __func__);
			 return 0;
		 }
 
		 writel(0xFF, sun50i_prcm_base + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		 udelay(30);
 
		 while (0xFF != readl(sun50i_prcm_base + SUNXI_CPU_PWR_CLAMP(cluster, cpu)));
	 }
	 return 0;
 }
 
 void sun50i_cpu_power_up(unsigned int cluster, unsigned int cpu)
 {
	 unsigned int value;
 
	 /* Assert nCPUPORESET LOW */
	 value	= readl(sun50i_cpucfg_base + SUNXI_CPU_RST_CTRL(cluster));
	 value &= (~(1<<cpu));
	 writel(value, sun50i_cpucfg_base + SUNXI_CPU_RST_CTRL(cluster));
	 udelay(10);
 
	 /* Assert cpu power-on reset */
	 value	= readl(sun50i_r_cpucfg_base + SUNXI_CLUSTER_PWRON_RESET(cluster));
	 value &= (~(1<<cpu));
	 writel(value, sun50i_r_cpucfg_base + SUNXI_CLUSTER_PWRON_RESET(cluster));
	 udelay(10);
 
	 /* set AA32nAA64 to AA64 */
	 sun50i_set_AA32nAA64(cluster, cpu, 1);
 
	 /* Apply power to the PDCPU power domain. */
	 sun50i_power_switch_set(cluster, cpu, 1);
 
	 /* Release the core output clamps */
	 value = readl(sun50i_prcm_base + SUNXI_CLUSTER_PWROFF_GATING(cluster));
	 value &= (~(0x1<<cpu));
	 writel(value, sun50i_prcm_base + SUNXI_CLUSTER_PWROFF_GATING(cluster));
	 udelay(20);
 
	 /* Deassert cpu power-on reset */
	 value	= readl(sun50i_r_cpucfg_base + SUNXI_CLUSTER_PWRON_RESET(cluster));
	 value |= ((1<<cpu));
	 writel(value, sun50i_r_cpucfg_base + SUNXI_CLUSTER_PWRON_RESET(cluster));
	 udelay(10);
 
	 /* Deassert core reset */
	 value	= readl(sun50i_cpucfg_base + SUNXI_CPU_RST_CTRL(cluster));
	 value |= (1<<cpu);
	 writel(value, sun50i_cpucfg_base + SUNXI_CPU_RST_CTRL(cluster));
	 udelay(10);
 
	 /* Assert DBGPWRDUP HIGH */
	 value = readl(sun50i_cpucfg_base + SUNXI_DBG_REG0);
	 value |= (1<<cpu);
	 writel(value, sun50i_cpucfg_base + SUNXI_DBG_REG0);
	 udelay(10);
	 bakery_lock_get(&plat_console_lock);
	 INFO("sun50i power-up cluster-%d cpu-%d ok\n", cluster, cpu);
	 bakery_lock_release(&plat_console_lock);
 }

void sun50i_cpu_power_down(unsigned int cluster, unsigned int cpu)
{
	unsigned int value;

	/* step7: Deassert DBGPWRDUP LOW */
	value = readl(sun50i_cpucfg_base + SUNXI_DBG_REG0);
	value &= (~(1<<cpu));
	writel(value, sun50i_cpucfg_base + SUNXI_DBG_REG0);
	udelay(10);

	/* step8: Activate the core output clamps */
	value = readl(sun50i_prcm_base + SUNXI_CLUSTER_PWROFF_GATING(cluster));
	value |= (1 << cpu);
	writel(value, sun50i_prcm_base + SUNXI_CLUSTER_PWROFF_GATING(cluster));
	udelay(20);

	/* step9: Assert nCPUPORESET LOW */
	value	= readl(sun50i_cpucfg_base + SUNXI_CPU_RST_CTRL(cluster));
	value &= (~(1<<cpu));
	writel(value, sun50i_cpucfg_base + SUNXI_CPU_RST_CTRL(cluster));
	udelay(10);

	/* step10: Remove power from th e PDCPU power domain */
	sun50i_power_switch_set(cluster, cpu, 0);
	bakery_lock_get(&plat_console_lock);
	INFO("sun50i power-down cluster-%d cpu-%d ok.\n", cluster, cpu);
	bakery_lock_release(&plat_console_lock);

}

void sunxi_cpu_die(unsigned int cpu)
{
	#if 0
        unsigned long sctlr, cpuectlr;

        /* step1: Disable the data cache */
        __asm("mrs %0, SCTLR_EL1\n" : "=r" (sctlr));
        sctlr &= ~(0x1<<2);
        __asm volatile("msr SCTLR_EL1, %0\n" : : "r" (sctlr));

        /* step2: Clean and invalidate all data from the L1 Data cache */
        //flush_cache_all();	
	dcsw_op_louis(DCCSW);

        /* step3: Disable data coherency with other cores in the cluster */
        __asm("mrs %0, S3_1_c15_c2_1\n" : "=r" (cpuectlr));
        cpuectlr &= ~(0x1<<6);
        __asm volatile("msr S3_1_c15_c2_1, %0\n" : : "r" (cpuectlr));

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
        dsb();
	#endif
}

