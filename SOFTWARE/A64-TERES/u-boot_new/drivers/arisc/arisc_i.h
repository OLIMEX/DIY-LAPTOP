/*
 *  arch/arm/mach-sun6i/arisc/arisc_i.h
 *
 * Copyright (c) 2012 Allwinner.
 * 2012-05-01 Written by sunny (sunny@allwinnertech.com).
 * 2012-10-01 Written by superm (superm@allwinnertech.com).
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

#ifndef __ARISC_I_H__
#define __ARISC_I_H__

#include <common.h>
#include <fdt_support.h>
#include <fdtdec.h>
#include <libfdt.h>
#include <private_uboot.h>
#include <linux/types.h>
#include <linux/err.h>
#include <asm/io.h>
#include <asm/errno.h>

#define ARISC_DVFS_VF_TABLE_MAX         (16)

/*
 * debug level define,
 * level 0 : dump debug information--none;
 * level 1 : dump debug information--error;
 * level 2 : dump debug information--error+warning;
 * level 3 : dump debug information--error+warning+information;
 * extern void printk(const char *, ...);
 */
#define ARISC_DEBUG_ON
#ifdef ARISC_DEBUG_ON
/* debug levels */
#define DEBUG_LEVEL_INF    ((u32)1 << 0)
#define DEBUG_LEVEL_LOG    ((u32)1 << 1)
#define DEBUG_LEVEL_WRN    ((u32)1 << 2)
#define DEBUG_LEVEL_ERR    ((u32)1 << 3)

#define ARISC_INF(format, args...)                          \
	if(DEBUG_LEVEL_INF & (0xf0 >> (arisc_debug_level +1)))  \
		tick_printf("[ARISC] :"format, ##args);

#define ARISC_LOG(format, args...)                                      \
	if(DEBUG_LEVEL_LOG & (0xf0 >> (arisc_debug_level +1)))	\
		tick_printf("[ARISC] :"format, ##args);

#define ARISC_WRN(format, args...)                          \
	if(DEBUG_LEVEL_WRN & (0xf0 >> (arisc_debug_level +1)))  \
		printf("[ARISC WARING] :"format, ##args);

#define ARISC_ERR(format, args...)                          \
	if(DEBUG_LEVEL_ERR & (0xf0 >> (arisc_debug_level +1)))  \
		printf("[ARISC ERROR] :"format, ##args);

#else /* ARISC_DEBUG_ON */
#define ARISC_INF(...)
#define ARISC_WRN(...)
#define ARISC_ERR(...)
#define ARISC_LOG(...)

#endif /* ARISC_DEBUG_ON */

typedef struct mem_cfg
{
	phys_addr_t base;
	size_t size;
} mem_cfg_t;

typedef struct mem_cfg_64
{
	u64 base;
	u64 size;
} mem_cfg_64_t;

typedef struct dev_cfg
{
	phys_addr_t base;
	size_t size;
	u32 irq;
	int status;
} dev_cfg_t;

typedef struct dev_cfg_64
{
	u64 base;
	u64 size;
	u32 irq;
	int status;
} dev_cfg_64_t;

typedef struct cir_cfg
{
	phys_addr_t base;
	size_t size;
	u32 irq;
	u32 power_key_code;
	u32 addr_code;
	int status;
} cir_cfg_t;

typedef struct cir_cfg_64
{
	u64 base;
	u64 size;
	u32 irq;
	u32 power_key_code;
	u32 addr_code;
	int status;
} cir_cfg_64_t;

typedef struct pmu_cfg
{
	u32 pmu_bat_shutdown_ltf;
	u32 pmu_bat_shutdown_htf;
	u32 pmu_pwroff_vol;
	u32 power_start;
} pmu_cfg_t;

typedef struct power_cfg
{
	u32 powchk_used;
	u32 power_reg;
	u32 system_power;
} power_cfg_t;

typedef struct image_cfg
{
	phys_addr_t base;
	size_t size;
} image_cfg_t;

typedef struct image_cfg_64
{
	u64 base;
	u64 size;
} image_cfg_64_t;

typedef struct space_cfg
{
	phys_addr_t sram_dst;
	phys_addr_t sram_offset;
	size_t sram_size;
	phys_addr_t dram_dst;
	phys_addr_t dram_offset;
	size_t dram_size;
	phys_addr_t para_dst;
	phys_addr_t para_offset;
	size_t para_size;
	phys_addr_t msgpool_dst;
	phys_addr_t msgpool_offset;
	size_t msgpool_size;
	phys_addr_t standby_dst;
	phys_addr_t standby_offset;
	size_t standby_size;
} space_cfg_t;

typedef struct space_cfg_64
{
	u64 sram_dst;
	u64 sram_offset;
	u64 sram_size;
	u64 dram_dst;
	u64 dram_offset;
	u64 dram_size;
	u64 para_dst;
	u64 para_offset;
	u64 para_size;
	u64 msgpool_dst;
	u64 msgpool_offset;
	u64 msgpool_size;
	u64 standby_dst;
	u64 standby_offset;
	u64 standby_size;
} space_cfg_64_t;

typedef struct dram_para
{
	//normal configuration
	u32 dram_clk;
	u32 dram_type; //dram_type DDR2: 2 DDR3: 3 LPDDR2: 6 DDR3L: 31
	u32 dram_zq;
	u32 dram_odt_en;

	//control configuration
	u32 dram_para1;
	u32 dram_para2;

	//timing configuration
	u32 dram_mr0;
	u32 dram_mr1;
	u32 dram_mr2;
	u32 dram_mr3;
	u32 dram_tpr0;
	u32 dram_tpr1;
	u32 dram_tpr2;
	u32 dram_tpr3;
	u32 dram_tpr4;
	u32 dram_tpr5;
	u32 dram_tpr6;

	//reserved for future use
	u32 dram_tpr7;
	u32 dram_tpr8;
	u32 dram_tpr9;
	u32 dram_tpr10;
	u32 dram_tpr11;
	u32 dram_tpr12;
	u32 dram_tpr13;

}dram_para_t;

typedef struct arisc_freq_voltage
{
	u32 freq;       //cpu frequency
	u32 voltage;    //voltage for the frequency
	u32 axi_div;    //the divide ratio of axi bus
} arisc_freq_voltage_t;

typedef struct dts_cfg
{
	struct dram_para dram_para;
	struct arisc_freq_voltage vf[ARISC_DVFS_VF_TABLE_MAX];
	struct space_cfg space;
	struct image_cfg image;
	struct mem_cfg prcm;
	struct mem_cfg cpuscfg;
	struct dev_cfg msgbox;
	struct dev_cfg hwspinlock;
	struct dev_cfg s_uart;
	struct dev_cfg s_rsb;
	struct dev_cfg s_jtag;
	struct cir_cfg s_cir;
	struct pmu_cfg pmu;
	struct power_cfg power;
} dts_cfg_t;

typedef struct dts_cfg_64
{
	struct dram_para dram_para;
	struct arisc_freq_voltage vf[ARISC_DVFS_VF_TABLE_MAX];
	struct space_cfg_64 space;
	struct image_cfg_64 image;
	struct mem_cfg_64 prcm;
	struct mem_cfg_64 cpuscfg;
	struct dev_cfg_64 msgbox;
	struct dev_cfg_64 hwspinlock;
	struct dev_cfg_64 s_uart;
	struct dev_cfg_64 s_rsb;
	struct dev_cfg_64 s_jtag;
	struct cir_cfg_64 s_cir;
	struct pmu_cfg pmu;
	struct power_cfg power;
} dts_cfg_64_t;

#endif  //__ARISC_I_H__
