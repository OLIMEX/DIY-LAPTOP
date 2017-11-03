#ifndef __LINUX_AXP_CFG_H_
#define __LINUX_AXP_CFG_H_
#include <asm/types.h>

#define AXP22                   (22)
#define AXP15                   (15)

#define BATRDC			100		//initial rdc

#define ABS(x)			((x) >0 ? (x) : -(x) )

#define INTCHGCUR		300000		//set initial charging current limite
#define SUSCHGCUR		1000000		//set suspend charging current limite
#define RESCHGCUR		INTCHGCUR	//set resume charging current limite
#define CLSCHGCUR		SUSCHGCUR	//set shutdown charging current limite
#define INTCHGVOL		4200000		//set initial charing target voltage
#define INTCHGENDRATE		10		//set initial charing end current rate
#define INTCHGENABLED		1		//set initial charing enabled
#define INTADCFREQ		25		//set initial adc frequency
#define INTADCFREQC		100		//set initial coulomb adc coufrequency
#define INTCHGPRETIME		50		//set initial pre-charging time
#define INTCHGCSTTIME		480		//set initial pre-charging time
#define BATMAXVOL		4200000		//set battery max design volatge
#define BATMINVOL		3500000		//set battery min design volatge

#define OCVREG0			0x00		//2.99V
#define OCVREG1			0x00		//3.13V
#define OCVREG2			0x00		//3.27V
#define OCVREG3			0x00		//3.34V
#define OCVREG4			0x00		//3.41V
#define OCVREG5			0x00		//3.48V
#define OCVREG6			0x00		//3.52V
#define OCVREG7			0x00		//3.55V
#define OCVREG8			0x04		//3.57V
#define OCVREG9			0x05		//3.59V
#define OCVREGA			0x06		//3.61V
#define OCVREGB			0x07		//3.63V
#define OCVREGC			0x0a		//3.64V
#define OCVREGD			0x0d		//3.66V
#define OCVREGE			0x1a		//3.7V
#define OCVREGF			0x24		//3.73V
#define OCVREG10		0x29		//3.77V
#define OCVREG11		0x2e		//3.78V
#define OCVREG12		0x32		//3.8V
#define OCVREG13		0x35		//3.84V
#define OCVREG14		0x39		//3.85V
#define OCVREG15		0x3d		//3.87V
#define OCVREG16		0x43		//3.91V
#define OCVREG17		0x49		//3.94V
#define OCVREG18		0x4f		//3.98V
#define OCVREG19		0x54		//4.01V
#define OCVREG1A		0x58		//4.05V
#define OCVREG1B		0x5c		//4.08V
#define OCVREG1C		0x5e		//4.1V
#define OCVREG1D		0x60		//4.12V
#define OCVREG1E		0x62		//4.14V
#define OCVREG1F		0x64		//4.15V

struct axp_config_info{
	u32	pmu_used;
	u32	pmu_twi_addr;
	u32	pmu_twi_id;
	u32	pmu_irq_id;
	u32	pmu_battery_rdc;
	u32	pmu_battery_cap;
	u32	pmu_batdeten;
	u32     pmu_chg_ic_temp;
	u32	pmu_runtime_chgcur;
	u32	pmu_suspend_chgcur;
	u32	pmu_shutdown_chgcur;
	u32	pmu_init_chgvol;
	u32	pmu_init_chgend_rate;
	u32	pmu_init_chg_enabled;
	u32	pmu_init_bc_en;
	u32	pmu_init_adc_freq;
	u32	pmu_init_adcts_freq;
	u32	pmu_init_chg_pretime;
	u32	pmu_init_chg_csttime;
	u32	pmu_batt_cap_correct;
	u32	pmu_chg_end_on_en;
	u32	ocv_coulumb_100;

	u32	pmu_bat_para1;
	u32	pmu_bat_para2;
	u32	pmu_bat_para3;
	u32	pmu_bat_para4;
	u32	pmu_bat_para5;
	u32	pmu_bat_para6;
	u32	pmu_bat_para7;
	u32	pmu_bat_para8;
	u32	pmu_bat_para9;
	u32	pmu_bat_para10;
	u32	pmu_bat_para11;
	u32	pmu_bat_para12;
	u32	pmu_bat_para13;
	u32	pmu_bat_para14;
	u32	pmu_bat_para15;
	u32	pmu_bat_para16;
	u32	pmu_bat_para17;
	u32	pmu_bat_para18;
	u32	pmu_bat_para19;
	u32	pmu_bat_para20;
	u32	pmu_bat_para21;
	u32	pmu_bat_para22;
	u32	pmu_bat_para23;
	u32	pmu_bat_para24;
	u32	pmu_bat_para25;
	u32	pmu_bat_para26;
	u32	pmu_bat_para27;
	u32	pmu_bat_para28;
	u32	pmu_bat_para29;
	u32	pmu_bat_para30;
	u32	pmu_bat_para31;
	u32	pmu_bat_para32;

	u32	pmu_ac_vol;
	u32	pmu_ac_cur;
	u32	pmu_usbpc_vol;
	u32	pmu_usbpc_cur;
	u32	pmu_pwroff_vol;
	u32	pmu_pwron_vol;
	u32	pmu_powkey_off_time;
	u32	pmu_powkey_off_en;
	u32     pmu_powkey_off_delay_time;
	u32	pmu_powkey_off_func;
	u32	pmu_powkey_long_time;
	u32	pmu_powkey_on_time;
	u32	pmu_pwrok_time;
	u32     pmu_reset_shutdown_en;
	u32	pmu_battery_warning_level1;
	u32	pmu_battery_warning_level2;
	u32	pmu_restvol_adjust_time;
	u32	pmu_ocv_cou_adjust_time;
	u32	pmu_chgled_func;
	u32	pmu_chgled_type;
	u32	pmu_vbusen_func;
	u32	pmu_reset;
	u32	pmu_IRQ_wakeup;
	u32	pmu_hot_shutdown;
	u32	pmu_inshort;
	int	power_start;

	u32	pmu_bat_temp_enable;
	u32	pmu_bat_charge_ltf;
	u32	pmu_bat_charge_htf;
	u32	pmu_bat_shutdown_ltf;
	u32	pmu_bat_shutdown_htf;
	u32	pmu_bat_temp_para1;
	u32	pmu_bat_temp_para2;
	u32	pmu_bat_temp_para3;
	u32	pmu_bat_temp_para4;
	u32	pmu_bat_temp_para5;
	u32	pmu_bat_temp_para6;
	u32	pmu_bat_temp_para7;
	u32	pmu_bat_temp_para8;
	u32	pmu_bat_temp_para9;
	u32	pmu_bat_temp_para10;
	u32	pmu_bat_temp_para11;
	u32	pmu_bat_temp_para12;
	u32	pmu_bat_temp_para13;
	u32	pmu_bat_temp_para14;
	u32	pmu_bat_temp_para15;
	u32	pmu_bat_temp_para16;
};

enum {
	DEBUG_SPLY = 1U << 0,
	DEBUG_REGU = 1U << 1,
	DEBUG_INT = 1U << 2,
	DEBUG_CHG = 1U << 3,
};

#define AXP_LDOIO_ID_START      30
#define AXP_DCDC_ID_START       40

#ifdef CONFIG_AXP_NMI_USED
extern void clear_nmi_status(void);
extern void disable_nmi(void);
extern void enable_nmi(void);
extern void set_nmi_trigger(u32 trigger);
#endif

#ifdef CONFIG_AW_AXP81X
extern s32 axp_debug;
#define DBG_PSY_MSG(level_mask, fmt, arg...)	if (unlikely(axp_debug & level_mask)) \
	printk(KERN_DEBUG fmt , ## arg)
extern void axp81x_power_off(void);
#endif

extern s32 axp_fetch_sysconfig_para(char * pmu_type, struct axp_config_info *axp_config);
extern s32  axp_device_tree_parse(char * pmu_type, struct axp_config_info *axp_config);
#endif
