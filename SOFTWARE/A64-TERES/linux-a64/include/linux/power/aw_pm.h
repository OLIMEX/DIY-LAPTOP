/*
*******************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2006-2011, kevin.z China
*                                             All Rights Reserved
*
* File    : pm.h
* By      : kevin.z
* Version : v1.0
* Date    : 2011-5-27 14:08
* Descript: power manager
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/
#ifndef __AW_PM_H__
#define __AW_PM_H__

#include "axp_depend.h"
#include <linux/types.h>

#define BITMAP(bit) (0x1<<bit)

/**max device number of pmu*/
#define PMU_MAX_DEVS        2

/**
*@name PMU command
*@{
*/
#define AW_PMU_SET          0x10
#define AW_PMU_VALID        0x20
/**
*@}
*/

/*
* the wakeup source of main cpu: cpu0
*/
#define CPU0_WAKEUP_MSGBOX      (1<<0)  /* external interrupt, pmu event for ex.    */
#define CPU0_WAKEUP_KEY         (1<<1)  /* key event    */
#define CPU0_WAKEUP_EXINT	(1<<2)
#define CPU0_WAKEUP_IR		(1<<3)
#define CPU0_WAKEUP_ALARM	(1<<4)
#define CPU0_WAKEUP_USB		(1<<5)
#define CPU0_WAKEUP_TIMEOUT	(1<<6)
#define CPU0_WAKEUP_PIO		(1<<7)


//the wakeup source of assistant cpu: cpus
#define CPUS_WAKEUP_LOWBATT     (1<<12 )
#define CPUS_WAKEUP_USB         (1<<13 )
//#define CPUS_WAKEUP_AC          (1<<14 )
#define CPUS_WAKEUP_ASCEND      (1<<15 )
#define CPUS_WAKEUP_DESCEND     (1<<16 )
#define CPUS_WAKEUP_SHORT_KEY   (1<<17 )
#define CPUS_WAKEUP_LONG_KEY    (1<<18 )
#define CPUS_WAKEUP_IR          (1<<19 )
#define CPUS_WAKEUP_ALM0        (1<<20)
#define CPUS_WAKEUP_ALM1        (1<<21)
#define CPUS_WAKEUP_TIMEOUT     (1<<22)
#define CPUS_WAKEUP_GPIO        (1<<23)
#define CPUS_WAKEUP_USBMOUSE    (1<<24)
#define CPUS_WAKEUP_LRADC       (1<<25)
//null for 1<<26
#define CPUS_WAKEUP_CODEC       (1<<27)
#define CPUS_WAKEUP_BAT_TEMP    (1<<28)
#define CPUS_WAKEUP_FULLBATT    (1<<29)
#define CPUS_WAKEUP_HMIC        (1<<30)
#define CPUS_WAKEUP_POWER_EXP   (1<<31)
#define CPUS_WAKEUP_KEY         (CPUS_WAKEUP_SHORT_KEY | CPUS_WAKEUP_LONG_KEY)

//for format all the wakeup gpio into one word.
#define GPIO_PL_MAX_NUM		    (11)    //0-11
#define GPIO_PM_MAX_NUM		    (11)    //0-11
#define GPIO_AXP_MAX_NUM	    (7)	    //0-7

#define WAKEUP_GPIO_PL(num)         (1 << (num))
#define WAKEUP_GPIO_PM(num)         (1 << (num + 12))
#define WAKEUP_GPIO_AXP(num)        (1 << (num + 24))
#define WAKEUP_GPIO_GROUP(group)    (1 << (group - 'A'))

#if defined(CONFIG_ARCH_SUN8IW6P1) || defined(CONFIG_ARCH_SUN8IW8P1) || defined(CONFIG_ARCH_SUN50IW1P1) \
	|| defined(CONFIG_ARCH_SUN8IW10P1)
#define IO_NUM (2)
#elif defined(CONFIG_ARCH_SUN9IW1P1)
#define PLL_NUM (12)
#define BUS_NUM (15)
#define IO_NUM (2)
#else
#define PLL_NUM (11)
#define BUS_NUM (6)
#define IO_NUM (2)
#endif

#if defined(CONFIG_ARCH_SUN9IW1P1)
typedef struct pll_para{
	int n;
	int p;
	int divi; /* input_div */
	int divo; /* output_div */
}pll_para_t;
#elif defined(CONFIG_ARCH_SUN8IW6P1) || defined(CONFIG_ARCH_SUN8IW8P1) \
	|| defined(CONFIG_ARCH_SUN8IW10P1)|| defined(CONFIG_ARCH_SUN50IW1P1)
typedef struct pll_para{
	unsigned int factor1;
	unsigned int factor2;
	unsigned int factor3;
	unsigned int factor4;
}pll_para_t;
#else
typedef struct pll_para{
	unsigned int n;	//for a83, pll6, n
	unsigned int k; //		div1
	unsigned int m; //		div2
	unsigned int p;
}pll_para_t;
#endif

typedef struct bus_para{
	unsigned int src;
	unsigned int pre_div;
	unsigned int div_ratio;
	unsigned int n;
	unsigned int m;
}bus_para_t;

//for bitmap macro definition
#define PM_PLL_C0	(0)
#define PM_PLL_C1	(1)
#define PM_PLL_AUDIO	(2)
#define PM_PLL_VIDEO0	(3)
#define PM_PLL_VE	(4)
#define PM_PLL_DRAM	(5)
#define PM_PLL_PERIPH	(6)
#define PM_PLL_GPU	(7)
#define PM_PLL_HSIC	(8)
#define PM_PLL_DE	(9)
#define PM_PLL_VIDEO1	(10)
#define PLL_PERIPH1	(11)
#define PLL_DRAM1	(12)
#define PLL_MIPI	(13)
#define PLL_NUM		(14)

#define BUS_C0      (0)
#define BUS_C1      (1)
#define BUS_AXI0    (2)
#define BUS_AXI1    (3)
#define BUS_AHB1    (4)
#define BUS_AHB2    (5)
#define BUS_APB1    (6)
#define BUS_APB2    (7)
#define BUS_NUM	    (8)

#define OSC_HOSC_BIT	    (3)
#define OSC_LOSC_BIT	    (2)
#define OSC_LDO1_BIT	    (1)
#define OSC_LDO0_BIT	    (0)

typedef enum clk_src
{
	CLK_SRC_NONE = 0x0, //invalid source clock id

	CLK_SRC_LOSC,   //LOSC, 33/50/67:32768Hz, 73:16MHz/512=31250
	CLK_SRC_IOSC,   //InternalOSC,  33/50/67:700KHZ, 73:16MHz
	CLK_SRC_HOSC,   //HOSC, 24MHZ clock
	CLK_SRC_AXI,    //AXI clock
	CLK_SRC_16M,    //16M for the backdoor

	CLK_SRC_PLL1,   //PLL1 clock
	CLK_SRC_PLL2,   //PLL2 clock
	CLK_SRC_PLL3,   //PLL3 clock
	CLK_SRC_PLL4,   //PLL4 clock
	CLK_SRC_PLL5,   //PLL5 clock
	CLK_SRC_PLL6,   //PLL6 clock
	CLK_SRC_PLL7,   //PLL7 clock
	CLK_SRC_PLL8,   //PLL8 clock
	CLK_SRC_PLL9,   //PLL9 clock
	CLK_SRC_PLL10,  //PLL10 clock
	CLK_SRC_PLL11,  //PLL10 clock

	CLK_SRC_CPUS,   //cpus clock
	CLK_SRC_C0,     //cluster0 clock
	CLK_SRC_C1,     //cluster1 clock
	CLK_SRC_AXI0,   //AXI0 clock
	CLK_SRC_AXI1,   //AXI0 clock
	CLK_SRC_AHB0,   //AHB0 clock
	CLK_SRC_AHB1,   //AHB1 clock
	CLK_SRC_AHB2,   //AHB2 clock
	CLK_SRC_APB0,   //APB0 clock
	CLK_SRC_APB1,   //APB1 clock
	CLK_SRC_APB2,   //APB2 clock
} clk_src_e;

typedef struct pwr_dm_state{
    /* 
     * for state bitmap: 
     * bitx = 1: keep state.
     * bitx = 0: mean close corresponding power src.
     */
    unsigned int state;	
    unsigned int sys_mask;	//bitx=1, the corresponding state is effect, 
				//otherwise, the corresponding power is in charge in device driver.
    
    // sys_mask&state		: bitx=1, mean the power is on, for the "on" state power, u need care about the voltage.;
    // ((~sys_mask)|state)		: bitx=0, mean the power is close;
    // pwr_dm_state bitmap
    
    // actually: we care about the pwr_dm voltage, 
    // such as: we want to keep the vdd_sys at 1.0v at standby period.
    //		we actually do not care how to do it.
    //		it can be sure that cpus can do it with the pmu's help.
    unsigned short volt[VCC_MAX_INDEX]; //unsigned short is 16bit width.  
}pwr_dm_state_t;

typedef struct pm_dram_para{
    unsigned int selfresh_flag; //selfresh_flag must be compatible with vdd_sys pwr state.
    unsigned int crc_en;
    unsigned int crc_start;
    unsigned int crc_len;
}pm_dram_para_t;

typedef struct cpus_clk_para{
    unsigned int cpus_id;
}cpus_para_t;

typedef struct io_state_config{
    unsigned int paddr;
    unsigned int value_mask; //specify the effect bit.
    unsigned int value;
}io_state_config_t;

typedef struct soc_io_para{
    /*
     *	hold: mean before power off vdd_sys, whether hold gpio pad or not.
     *	this flag only effect: when vdd_sys is powered_off;
     *	this flag only affect hold&unhold operation.
     *	the recommended hold sequence is as follow: 
     *	backup_io_cfg -> cfg_io(enter_low_power_mode) -> hold -> assert vdd_sys_reset -> poweroff vdd_sys.
     *  the recommended unhold sequence is as follow: 
     *  poweron vdd_sys -> de_assert vdd_sys -> restore_io_cfg -> unhold.
     * */
    unsigned int hold_flag;

    
    /*
     * note: only specific bit mark by value_mask is effect.
     * IO_NUM: only uart and jtag needed care.
     * */
    io_state_config_t io_state[IO_NUM];
}soc_io_para_t;

typedef struct cpux_clk_para{
    /*
     * Hosc: losc: ldo1: ldo0
     * the osc bit map is as follow:
     * bit3: bit2: bit1:bit0
     * Hosc: losc: ldo1: ldo0
     */
    int osc_en;

    /*
     * for a83, pll bitmap as follow:
     * bit7:	    bit6:	bit5:	    bit4:	bit3:		bit2:		bit1:	    bit0
     * pll8(gpu): pll6(periph): pll5(dram): pll4(ve): pll3(video):	pll2(audio):	c1cpux:	    c0cpux
     *									pll11(video1):	pll10(de):  pll9(hsic)
     * */

    /* for disable bitmap:
     * bitx = 0: close
     * bitx = 1: do not care.
     */
    int init_pll_dis;
	 
    /* for enable bitmap:
     * bitx = 0: do not care.
     * bitx = 1: open
     */
    int exit_pll_en;

    /*
     * set corresponding bit if it's pll factors need to be set some value.
     */
    int pll_change;

    /*
     * fill in the enabled pll freq factor sequently. unit is khz pll6: 0x90041811
     * factor n/m/k/p already do the pretreatment of the minus one
     */
    pll_para_t pll_factor[PLL_NUM];

    /*
     * **********A31************
     * at a31 platform, the clk relationship is as follow:
     * pll1->cpu -> axi
     *	     -> atb/apb
     * ahb1 -> apb1
     * apb2
     * so, at a31, only cpu:ahb1:apb2 need be cared.
     *
     * *********A80************
     * at a83 platform, the clk relationship is as follow:
     * c1 -> axi1
     * c0 -> axi0
     * gtbus
     * ahb0
     * ahb1
     * ahb2
     * apb0
     * apb1
     * so, at a80, c1:c0:gtbus:ahb0:ahb1:ahb2:apb0:apb1 need be cared.
     *
     * *********A83************
     * at a83 platform, the clk relationship is as follow:
     * c1 -> axi1
     * c0 -> axi0
     * ahb1 -> apb1
     * apb2
     * ahb2
     * so, at a83, only c1:c0:ahb1:apb2:ahb2 need be cared.
     *
     * normally, only clk src need be cared.
     * the bus bitmap as follow:
     * for a83, bus_en:  
     * bit4: bit3: bit2: bit1: bit0
     * ahb2: apb2: ahb1: c1:   c0
     *
     * for a31, bus_en: 
     * bit2:bit1:bit0
     * cpu:ahb1:apb2
     *
     * for a80, bus_en: 
     * bit7: bit6:  bit5:  bit4: bit3: bit2: bit1: bit0
     * c1:   c0:    gtbus: ahb0: ahb1: ahb2: apb0: apb1
     */
    int bus_change;

    /*
     * bus_src: ahb1, apb2 src;
     * option:  pllx:axi:hosc:losc
     */
    bus_para_t bus_factor[BUS_NUM];

}cpux_clk_para_t;

typedef struct soc_pwr_dep{
    /*
     * id of scene_lock
     */
    unsigned int id;
    
    pwr_dm_state_t soc_pwr_dm_state;
    pm_dram_para_t soc_dram_state; 
    soc_io_para_t soc_io_state;
    cpux_clk_para_t cpux_clk_state;

}soc_pwr_dep_t;

typedef struct extended_standby{
    /*
     * id of extended standby
     */
    unsigned int id;
    unsigned int pmu_id;	//for: 808 || 809+806 || 803 || 813
				//support 4 pmu: each pmu_id range is: 0-255;
				//pmu_id <--> pmu_name have directly mapping, u can get the pmu_name from sys_config.fex files.;
				//bitmap as follow:
				//pmu1: 0-7 bit; pmu2: 8-15 bit; pmu3: 16-23 bit; pmu4: 24-31 bit
				//

    unsigned int soc_id;        // a33, a80, a83,...,
                                // for compatible reason, different soc, each bit have different meaning.

    pwr_dm_state_t soc_pwr_dm_state;
    pm_dram_para_t soc_dram_state; 
    soc_io_para_t soc_io_state;
    cpux_clk_para_t cpux_clk_state;
}extended_standby_t;


#define CPUS_ENABLE_POWER_EXP   (1<<31)
#define CPUS_WAKEUP_POWER_STA   (1<<1 )
#define CPUS_WAKEUP_POWER_CSM   (1<<0 )
typedef struct sst_power_info_para
{
	/*
	 * define for sun9iw1p1
	 * power_reg bit0 ~ 7 AXP_main REG10, bit 8~15 AXP_main REG12
	 * power_reg bit16~23 AXP_slav REG10, bit24~31 AXP_slav REG11
	 *
	 * AXP_main REG10: 0-off, 1-on
	 * bit7   bit6   bit5   bit4   bit3   bit2   bit1   bit0
	 * aldo2  aldo1  dcdc5  dcdc4  dcdc3  dcdc2  dcdc1  dc5ldo
	 *
	 * REG12: bit0~5:0-off, 1-on, bit6~7: 0-on, 1-off, dc1sw's power come from dcdc1
	 * bit7   bit6   bit5   bit4   bit3   bit2   bit1   bit0
	 * dc1sw  swout  aldo3  dldo2  dldo1  eldo3  eldo2  eldo1
	 *
	 * AXP_slave REG10: 0-off, 1-on. dcdc b&c is not used, ignore them.
	 * bit7   bit6   bit5   bit4   bit3   bit2   bit1   bit0
	 * aldo3  aldo2  aldo1  dcdce  dcdcd  dcdcc  dcdcb  dcdca
	 *
	 * AXP_slave REG11: 0-off, 1-on
	 * bit7   bit6   bit5   bit4   bit3   bit2   bit1   bit0
	 * swout  cldo3  cldo2  cldo1  bldo4  bldo3  bldo2  bldo1
	 */
	/*
	 * define for sun8iw5p1
	 * power_reg0 ~ 7 AXP_main REG10,  8~15 AXP_main REG12
	 * power_reg16~32 null
	 *
	 * AXP_main REG10: 0-off, 1-on
	 * bit7   bit6	 bit5   bit4   bit3   bit2   bit1   bit0
	 * aldo2  aldo1  dcdc5  dcdc4  dcdc3  dcdc2  dcdc1  dc5ldo
	 *
	 * REG12: 0-off, 1-on
	 * bit7   bit6   bit5   bit4   bit3   bit2   bit1   bit0
	 * dc1sw  dldo4  dldo3  dldo2  dldo1  eldo3  eldo2  eldo1
	 *
	 * REG13: bit16 aldo3, 0-off, 1-on
	 * REG90: bit17 gpio0/ldo, 0-off, 1-on
	 * REG92: bit18 gpio1/ldo, 0-off, 1-on
	 */
	unsigned int enable;		/* enable bit */
	unsigned int power_reg;		/* registers of power state should be */
	signed int system_power;	/* the max power of system, signed, power mabe negative when charge */
} sst_power_info_para_t;

typedef	struct super_standby_para
{
	unsigned int event;			//cpus wakeup event types
	unsigned int resume_code_src; 		//cpux resume code src
	unsigned int resume_code_length; 	//cpux resume code length
	unsigned int resume_entry; 		//cpux resume entry
	unsigned int timeout;			//wakeup after timeout seconds
	unsigned int gpio_enable_bitmap;
	unsigned int cpux_gpiog_bitmap;
	unsigned int pextended_standby;
} super_standby_para_t;

typedef	struct normal_standby_para
{
	unsigned int event;		//cpus wakeup event types
	unsigned int timeout;		//wakeup after timeout seconds
	unsigned int gpio_enable_bitmap;
	unsigned int cpux_gpiog_bitmap;
	unsigned int pextended_standby;
} normal_standby_para_t;


//define cpus wakeup src
#define CPUS_MEM_WAKEUP              (CPUS_WAKEUP_LOWBATT | CPUS_WAKEUP_USB |  \
						CPUS_WAKEUP_DESCEND | CPUS_WAKEUP_ASCEND | CPUS_WAKEUP_ALM0 | CPUS_WAKEUP_GPIO)
#define CPUS_BOOTFAST_WAKEUP         (CPUS_WAKEUP_LOWBATT | CPUS_WAKEUP_LONG_KEY |CPUS_WAKEUP_ALM0|CPUS_WAKEUP_USB )

/*used in normal standby*/
#define CPU0_MEM_WAKEUP              (CPU0_WAKEUP_MSGBOX | CPU0_WAKEUP_EXINT)
#define CPU0_BOOTFAST_WAKEUP         (CPU0_WAKEUP_MSGBOX)


/**
*@brief struct of pmu device arg
*/
struct aw_pmu_arg{
    unsigned int  twi_port;		/**<twi port for pmu chip   */
    unsigned char dev_addr;		/**<address of pmu device   */
    unsigned int soc_power_tree[VCC_MAX_INDEX];     /* power tree struct point */
};


/**
*@brief struct of standby
*/
struct aw_standby_para{
	unsigned int event;		/**<event type for system wakeup    */
	unsigned int axp_event;		/**<axp event type for system wakeup    */
	unsigned int debug_mask;	/* debug mask */
	signed int   timeout;		/**<time to power off system from now, based on second */
	unsigned int gpio_enable_bitmap;
	unsigned int cpux_gpiog_bitmap;
	unsigned int pextended_standby;
};


/**
*@brief struct of power management info
*/
struct aw_pm_info{
    struct aw_standby_para		standby_para;   /* standby parameter            */
    struct aw_pmu_arg			pmu_arg;        /**<args used by main function  */
};

typedef struct sst_dram_info
{
	unsigned int dram_crc_enable;
	unsigned int dram_crc_src;
	unsigned int dram_crc_len;
	unsigned int dram_crc_error;
	unsigned int dram_crc_total_count;
	unsigned int dram_crc_error_count;
} sst_dram_info_t;

typedef struct standby_info_para
{
	sst_power_info_para_t power_state; /* size 3W=12B */
	sst_dram_info_t dram_state; /*size 6W=24B */
} standby_info_para_t;

typedef enum event_cpu_id
{
    CPUS_ID,
    CPU0_ID,
} event_cpu_id_e;

typedef struct standby_space_cfg
{
    struct device_node *np;
    phys_addr_t standby_mem_base;
    phys_addr_t extended_standby_mem_base;
    phys_addr_t mem_offset;
    size_t mem_size;
} standby_space_cfg_t;

extern unsigned int parse_wakeup_gpio_group_map(char *s, unsigned int size, unsigned int gpio_map);

extern unsigned int parse_wakeup_event(char *s, unsigned int size, unsigned int event, event_cpu_id_e cpu_id);

extern unsigned int parse_wakeup_gpio_map(char *s, unsigned int size, unsigned int gpio_map);

extern unsigned int show_gpio_config(char *s, unsigned int size);

#endif /* __AW_PM_H__ */

