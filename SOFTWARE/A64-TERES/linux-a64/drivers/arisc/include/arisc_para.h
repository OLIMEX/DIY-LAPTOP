#ifndef __ARISC_PARA_H__
#define __ARISC_PARA_H__

#include <linux/power/axp_depend.h>
#define ARISC_MACHINE_PAD    0
#define ARISC_MACHINE_HOMLET 1

/* arisc parameter size: 128byte */
/*
 * machine: machine id, pad = 0, homlet = 1;
 * message_pool_phys: message pool physical address;
 * message_pool_size: message pool size;
 */
#define SERVICES_DVFS_USED (1<<0)

/* arisc clocksoure */
typedef enum arisc_clksrc
{
	CLKSRC_LOSC      = 0,
	CLKSRC_IOSC      = 1,
	CLKSRC_HOSC      = 2,
	CLKSRC_PERIPH0   = 3,
	CLKSRC_ARISC_MAX = 4,
} arisc_clksrc_e;

/* ddr clocksoure */
typedef enum ddr_clksrc
{
	CLKSRC_DDR0    = 0,
	CLKSRC_DDR1    = 1,
	CLKSRC_DDR_MAX = 2,
} ddr_clksrc_e;

typedef struct core_cfg
{
	struct device_node *np;
	struct clk *losc;
	struct clk *iosc;
	struct clk *hosc;
	struct clk *pllperiph0;
	u32 powchk_used;
	u32 power_reg;
	u32 system_power;
} core_cfg_t;

typedef struct dev_cfg
{
	struct device_node *np;
	struct clk *clk[ARISC_DEV_CLKSRC_NUM];
	void __iomem *vbase;
	phys_addr_t pbase;
	size_t size;
	u32 irq;
	int status;
} dev_cfg_t;

typedef struct dram_cfg
{
	struct device_node *np;
	struct clk *pllddr0;
	struct clk *pllddr1;
} dram_cfg_t;

typedef struct arisc_cfg
{
	struct core_cfg core;
	struct dram_cfg dram;
	struct dev_cfg suart;
	struct dev_cfg srsb;
	struct dev_cfg sjtag;
	u32 power_regu_tree[VCC_MAX_INDEX];
} arisc_cfg_t;
#endif /* __ARISC_PARA_H__ */
