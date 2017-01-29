/*
 * drivers/devfreq/dramfreq/sunxi_dramfreq.h
 *
 * Copyright(c) 2013-2015 Allwinnertech Co., Ltd.
 *
 * Author: Pan Nan <pannan@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef __SUNXI_DRAMFREQ_H__
#define __SUNXI_DRAMFREQ_H__

#define SUNXI_DRAMFREQ_NORMAL       (300000)
#define SUNXI_DRAMFREQ_IDLE         (168000)

/* register define */
#define MC_WORK_MODE                (0x000)
#define MC_MDFSCR                   (0x100)
#define MC_MDFSMRMR                 (0x108)

#define PTR2                        (0x04c)
#define RFSHTMG                     (0x090)
#define VTFCR                       (0x0b8)
#define PGCR0                       (0x100)
#define ODTMAP                      (0x120)
#define DXnGCR0(x)                  (0x344 + 0x80 * (x))

#define CCM_PLL_DDR1_REG            (0x4C)
#define CCM_DRAM_CFG_REG            (0xF4)

enum DRAM_KEY_MASTER {
	MASTER_GPU,
	MASTER_CSI,
	MASTER_DE,
	MASTER_MAX,
};

enum DRAM_FREQ_LEVEL {
	LV_0,
	LV_1,
	LV_2,
	LV_3,
	LV_4,
	LV_END,
};

enum DRAM_FREQ_TREND {
	FREQ_DOWN,
	FREQ_UP,
};

enum DRAM_MDFS_MODE {
	DFS_MODE,
	CFS_MODE,
};

enum GOVERNOR_STATE {
	STATE_INIT,
	STATE_EXIT,
	STATE_RUNNING,
	STATE_PAUSE,
};

struct dram_para_t {
	unsigned int dram_clk;
	unsigned int dram_type;
	unsigned int dram_zq;
	unsigned int dram_odt_en;
	unsigned int dram_para1;
	unsigned int dram_para2;
	unsigned int dram_mr0;
	unsigned int dram_mr1;
	unsigned int dram_mr2;
	unsigned int dram_mr3;
	unsigned int dram_tpr0;
	unsigned int dram_tpr1;
	unsigned int dram_tpr2;
	unsigned int dram_tpr3;
	unsigned int dram_tpr4;
	unsigned int dram_tpr5;
	unsigned int dram_tpr6;
	unsigned int dram_tpr7;
	unsigned int dram_tpr8;
	unsigned int dram_tpr9;
	unsigned int dram_tpr10;
	unsigned int dram_tpr11;
	unsigned int dram_tpr12;
	unsigned int dram_tpr13;
};

struct sunxi_dramfreq {
	unsigned int max;
	unsigned int min;
#ifndef CONFIG_DEVFREQ_DRAM_FREQ_WITH_SOFT_NOTIFY
	unsigned int irq;
#endif
	unsigned int pause;
	unsigned int key_masters[MASTER_MAX];
	enum DRAM_MDFS_MODE mode;

#ifdef CONFIG_DEBUG_FS
	s64 dramfreq_set_us;
	s64 dramfreq_get_us;
#endif

	struct mutex lock;
	spinlock_t master_lock;
	struct dram_para_t dram_para;

	struct devfreq *devfreq;

	void __iomem *dramcom_base;
	void __iomem *dramctl_base;
	void __iomem *ccu_base;

	struct clk *clk_pll_ddr0;
	struct clk *clk_pll_ddr1;

	int (*governor_state_update)(enum GOVERNOR_STATE);
};

extern struct sunxi_dramfreq *dramfreq;

#ifdef CONFIG_DEVFREQ_DRAM_FREQ_WITH_SOFT_NOTIFY
extern int dramfreq_master_access(enum DRAM_KEY_MASTER master, bool access);
#endif

#endif /* __SUNXI_DRAMFREQ_H__ */
