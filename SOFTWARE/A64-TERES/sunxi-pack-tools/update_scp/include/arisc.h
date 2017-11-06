#ifndef _ARISC_H
#define _ARISC_H

#define DVFS_VF_TABLE_MAX               (16)
#define IR_NUM_KEY_SUP                  (16)            /* the number of IR code support */

typedef unsigned int u32;

typedef enum power_dm
{
	DM_CPUA = 0,		/* 0  */
	DM_CPUB,		/* 1  */
	DM_DRAM,		/* 2  */
	DM_GPU,			/* 3  */
	DM_SYS,			/* 4  */
	DM_VPU,			/* 5  */
	DM_CPUS,		/* 6  */
	DM_DRAMPLL,		/* 7  */
	DM_ADC,			/* 8  */
	DM_PL,			/* 9  */
	DM_PM,			/* 10 */
	DM_IO,			/* 11 */
	DM_CPVDD,		/* 12 */
	DM_LDOIN,		/* 13 */
	DM_PLL,			/* 14 */
	DM_LPDDR,		/* 15 */
	DM_TEST,		/* 16 */
	DM_RES1,		/* 17 */
	DM_RES2,		/* 18 */
	DM_RES3,		/* 19 */
	DM_MAX,			/* 20 */
} power_dm_e;

typedef struct ir_code
{
	u32 key_code;
	u32 addr_code;
} ir_code_t;

typedef struct ir_key
{
	u32 num;
	ir_code_t ir_code_depot[IR_NUM_KEY_SUP];
} ir_key_t;

typedef struct dram_para
{
	//normal configuration
	unsigned int        dram_clk;
	unsigned int        dram_type;      //dram_type         DDR2: 2             DDR3: 3     LPDDR2: 6   LPDDR3: 7   DDR3L: 31
	//unsigned int        lpddr2_type;  //LPDDR2 type       S4:0    S2:1    NVM:2
	unsigned int        dram_zq;        //do not need
	unsigned int        dram_odt_en;

	//control configuration
	unsigned int        dram_para1;
	unsigned int        dram_para2;

	//timing configuration
	unsigned int        dram_mr0;
	unsigned int        dram_mr1;
	unsigned int        dram_mr2;
	unsigned int        dram_mr3;
	unsigned int        dram_tpr0;  //DRAMTMG0
	unsigned int        dram_tpr1;  //DRAMTMG1
	unsigned int        dram_tpr2;  //DRAMTMG2
	unsigned int        dram_tpr3;  //DRAMTMG3
	unsigned int        dram_tpr4;  //DRAMTMG4
	unsigned int        dram_tpr5;  //DRAMTMG5
	unsigned int        dram_tpr6;  //DRAMTMG8

	//reserved for future use
	unsigned int        dram_tpr7;
	unsigned int        dram_tpr8;
	unsigned int        dram_tpr9;
	unsigned int        dram_tpr10;
	unsigned int        dram_tpr11;
	unsigned int        dram_tpr12;
	unsigned int        dram_tpr13;
}dram_para_t;

typedef struct freq_voltage
{
	u32 freq;       //cpu frequency
	u32 voltage;    //voltage for the frequency
	u32 axi_div;    //the divide ratio of AXI bus
} freq_voltage_t;

typedef struct box_start_os_cfg
{
	u32 used;
	u32 start_type;
	u32 irkey_used;
	u32 pmukey_used;
	u32 pmukey_num;
	u32 led_power;
	u32 led_state;
} box_start_os_cfg_t;

typedef struct arisc_para
{
	u32 message_pool_phys;
	u32 message_pool_size;
	u32 standby_base;
	u32 standby_size;
	u32 suart_status;
	u32 pmu_bat_shutdown_ltf;
	u32 pmu_bat_shutdown_htf;
	u32 pmu_pwroff_vol;
	u32 power_mode;
	u32 power_start;
	u32 powchk_used;
	u32 power_reg;
	u32 system_power;
	struct ir_key ir_key;
	struct dram_para dram_para;
	struct freq_voltage vf[DVFS_VF_TABLE_MAX];
	u32 power_regu_tree[DM_MAX];
	struct box_start_os_cfg start_os;
} arisc_para_t;

#endif /* _ARISC_H */
