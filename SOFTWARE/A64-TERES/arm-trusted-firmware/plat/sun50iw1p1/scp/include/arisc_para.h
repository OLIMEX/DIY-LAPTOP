#ifndef __ARISC_PARA_H__
#define __ARISC_PARA_H__

#define ARISC_MACHINE_PAD    0
#define ARISC_MACHINE_HOMLET 1

/* arisc parameter size: 128byte */
/*
 * machine: machine id, pad = 0, homlet = 1;
 * message_pool_phys: message pool physical address;
 * message_pool_size: message pool size;
 */
#define SERVICES_DVFS_USED (1<<0)

/* FIXME: if you modify this struct, you should
 * sync this change with linux source,
 * by superm at 2015-05-15.
 */
typedef enum power_dm
{
	DM_CPUA = 0, /* 0  */
	DM_CPUB,     /* 1  */
	DM_DRAM,     /* 2  */
	DM_GPU,      /* 3  */
	DM_SYS,      /* 4  */
	DM_VPU,      /* 5  */
	DM_CPUS,     /* 6  */
	DM_DRAMPLL,  /* 7  */
	DM_ADC,      /* 8  */
	DM_PL,       /* 9  */
	DM_PM,       /* 10 */
	DM_IO,       /* 11 */
	DM_CPVDD,    /* 12 */
	DM_LDOIN,    /* 13 */
	DM_PLL,      /* 14 */
	DM_LPDDR,    /* 15 */
	DM_TEST,     /* 16 */
	DM_RES1,     /* 17 */
	DM_RES2,     /* 18 */
	DM_RES3,     /* 19 */
	DM_MAX,      /* 20 */
} power_dm_e;

typedef struct mem_cfg
{
	uintptr_t base;
	size_t size;
} mem_cfg_t;

typedef struct dev_cfg
{
	uintptr_t base;
	size_t size;
	uint32_t irq;
	int status;
} dev_cfg_t;

typedef struct cir_cfg
{
	uintptr_t base;
	size_t size;
	uint32_t irq;
	uint32_t power_key_code;
	uint32_t addr_code;
	int status;
} cir_cfg_t;

typedef struct pmu_cfg
{
	uint32_t pmu_bat_shutdown_ltf;
	uint32_t pmu_bat_shutdown_htf;
	uint32_t pmu_pwroff_vol;
	uint32_t power_start;
} pmu_cfg_t;

typedef struct power_cfg
{
	uint32_t powchk_used;
	uint32_t power_reg;
	uint32_t system_power;
} power_cfg_t;

typedef struct image_cfg
{
	uintptr_t base;
	size_t size;
} image_cfg_t;

typedef struct space_cfg
{
	uintptr_t sram_dst;
	uintptr_t sram_offset;
	size_t sram_size;
	uintptr_t dram_dst;
	uintptr_t dram_offset;
	size_t dram_size;
	uintptr_t para_dst;
	uintptr_t para_offset;
	size_t para_size;
	uintptr_t msgpool_dst;
	uintptr_t msgpool_offset;
	size_t msgpool_size;
	uintptr_t standby_dst;
	uintptr_t standby_offset;
	size_t standby_size;
} space_cfg_t;

typedef struct dram_para
{
	//normal configuration
	uint32_t dram_clk;
	uint32_t dram_type; //dram_type DDR2: 2 DDR3: 3 LPDDR2: 6 DDR3L: 31
	uint32_t dram_zq;
	uint32_t dram_odt_en;

	//control configuration
	uint32_t dram_para1;
	uint32_t dram_para2;

	//timing configuration
	uint32_t dram_mr0;
	uint32_t dram_mr1;
	uint32_t dram_mr2;
	uint32_t dram_mr3;
	uint32_t dram_tpr0;
	uint32_t dram_tpr1;
	uint32_t dram_tpr2;
	uint32_t dram_tpr3;
	uint32_t dram_tpr4;
	uint32_t dram_tpr5;
	uint32_t dram_tpr6;

	//reserved for future use
	uint32_t dram_tpr7;
	uint32_t dram_tpr8;
	uint32_t dram_tpr9;
	uint32_t dram_tpr10;
	uint32_t dram_tpr11;
	uint32_t dram_tpr12;
	uint32_t dram_tpr13;

}dram_para_t;

typedef struct arisc_freq_voltage
{
	uint32_t freq;       //cpu frequency
	uint32_t voltage;    //voltage for the frequency
	uint32_t axi_div;    //the divide ratio of axi bus
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

typedef struct arisc_para
{
	uint32_t message_pool_phys;
	uint32_t message_pool_size;
	uint32_t standby_base;
	uint32_t standby_size;
	uint32_t power_key_code;
	uint32_t addr_code;
	uint32_t suart_status;
	uint32_t pmu_bat_shutdown_ltf;
	uint32_t pmu_bat_shutdown_htf;
	uint32_t pmu_pwroff_vol;
	uint32_t power_start;
	uint32_t powchk_used;
	uint32_t power_reg;
	uint32_t system_power;
	struct dram_para dram_para;
	struct arisc_freq_voltage vf[ARISC_DVFS_VF_TABLE_MAX];
	uint32_t power_regu_tree[DM_MAX];
} arisc_para_t;

#define ARISC_PARA_SIZE (sizeof(struct arisc_para))

#endif /* __ARISC_PARA_H__ */
