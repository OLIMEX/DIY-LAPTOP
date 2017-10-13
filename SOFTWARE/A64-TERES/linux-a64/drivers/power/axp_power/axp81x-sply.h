#ifndef	_LINUX_AXP81X_SPLY_H_
#define	_LINUX_AXP81X_SPLY_H_
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/mfd/axp-mfd.h>
#include <linux/power_supply.h>

/*      AXP81X      */
#define AXP81X_CHARGE_STATUS		AXP81X_STATUS
#define AXP81X_IN_CHARGE			(1 << 6)
#define AXP81X_PDBC			(0x32)
#define AXP81X_CHARGE_CONTROL1		AXP81X_CHARGE1
#define AXP81X_CHARGER_ENABLE		(1 << 7)
#define AXP81X_CHARGE_CONTROL2		AXP81X_CHARGE2
#define AXP81X_CHARGE_CONTROL3		AXP81X_CHARGE3
#define AXP81X_CHARGE_VBUS		AXP81X_IPS_SET
#define AXP81X_CHARGE_AC                AXP81X_CHARGE_AC_SET
#define AXP81X_CAP			(0xB9)
#define AXP81X_BATCAP0			(0xe0)
#define AXP81X_BATCAP1			(0xe1)
#define AXP81X_RDC0			(0xba)
#define AXP81X_RDC1			(0xbb)
#define AXP81X_WARNING_LEVEL		(0xe6)
#define AXP81X_ADJUST_PARA		(0xe8)
#define AXP81X_FAULT_LOG1		AXP81X_MODE_CHGSTATUS
#define AXP81X_FAULT_LOG_CHA_CUR_LOW	(1 << 2)
#define AXP81X_FAULT_LOG_BATINACT	(1 << 3)
#define AXP81X_FAULT_LOG_OVER_TEMP	(1 << 7)
#define AXP81X_FAULT_LOG2		AXP81X_INTSTS2
#define AXP81X_FAULT_LOG_COLD		(1 << 0)
#define AXP81X_FINISH_CHARGE		(1 << 2)
#define AXP81X_COULOMB_CONTROL		AXP81X_COULOMB_CTL
#define AXP81X_COULOMB_ENABLE		(1 << 7)
#define AXP81X_COULOMB_SUSPEND		(1 << 6)
#define AXP81X_COULOMB_CLEAR		(1 << 5)

#define AXP81X_ADC_CONTROL				AXP81X_ADC_EN
#define AXP81X_ADC_BATVOL_ENABLE				(1 << 7)
#define AXP81X_ADC_BATCUR_ENABLE				(1 << 6)
#define AXP81X_ADC_DCINVOL_ENABLE			(1 << 5)
#define AXP81X_ADC_DCINCUR_ENABLE			(1 << 4)
#define AXP81X_ADC_USBVOL_ENABLE				(1 << 3)
#define AXP81X_ADC_USBCUR_ENABLE				(1 << 2)
#define AXP81X_ADC_APSVOL_ENABLE				(1 << 1)
#define AXP81X_ADC_TSVOL_ENABLE				(1 << 0)
#define AXP81X_ADC_INTERTEM_ENABLE			(1 << 7)
#define AXP81X_ADC_GPIO0_ENABLE				(1 << 3)
#define AXP81X_ADC_GPIO1_ENABLE				(1 << 2)
#define AXP81X_ADC_GPIO2_ENABLE				(1 << 1)
#define AXP81X_ADC_GPIO3_ENABLE				(1 << 0)
#define AXP81X_ADC_CONTROL3				(0x84)
#define AXP81X_ADC_CONTROL4				(0x85)
#define AXP81X_VBATH_RES					(0x78)
#define AXP81X_VTS_RES					(0x58)
#define AXP81X_VBATL_RES					(0x79)
#define AXP81X_OCVBATH_RES				(0xBC)
#define AXP81X_OCVBATL_RES				(0xBD)
#define AXP81X_INTTEMP					(0x56)
#define AXP81X_DATA_BUFFER0				AXP81X_BUFFER1
#define AXP81X_DATA_BUFFER1				AXP81X_BUFFER2
#define AXP81X_DATA_BUFFER2				AXP81X_BUFFER3
#define AXP81X_DATA_BUFFER3				AXP81X_BUFFER4
#define AXP81X_DATA_BUFFER4				AXP81X_BUFFER5
#define AXP81X_DATA_BUFFER5				AXP81X_BUFFER6
#define AXP81X_DATA_BUFFER6				AXP81X_BUFFER7
#define AXP81X_DATA_BUFFER7				AXP81X_BUFFER8
#define AXP81X_DATA_BUFFER8				AXP81X_BUFFER9
#define AXP81X_DATA_BUFFER9				AXP81X_BUFFERA
#define AXP81X_DATA_BUFFERA				AXP81X_BUFFERB
#define AXP81X_DATA_BUFFERB				AXP81X_BUFFERC

#define AXP81X_CHARGE_VOLTAGE_LEVEL0                    (4100000)
#define AXP81X_CHARGE_VOLTAGE_LEVEL1                    (4150000)
#define AXP81X_CHARGE_VOLTAGE_LEVEL2                    (4200000)
#define AXP81X_CHARGE_VOLTAGE_LEVEL3                    (4350000)
#define AXP81X_CHARGE_CURRENT_MIN                       (200000)
#define AXP81X_CHARGE_CURRENT_MAX                       (2800000)
#define AXP81X_CHARGE_CURRENT_STEP                      (200000)
#define AXP81X_CHARGE_END_LEVEL0                        (10)
#define AXP81X_CHARGE_END_LEVEL1                        (20)
#define AXP81X_CHARGE_PRETIME_MIN                       (40)
#define AXP81X_CHARGE_PRETIME_MAX                       (70)
#define AXP81X_CHARGE_PRETIME_STEP                      (10)
#define AXP81X_CHARGE_FASTTIME_MIN                      (360)
#define AXP81X_CHARGE_FASTTIME_MAX                      (720)
#define AXP81X_CHARGE_FASTTIME_STEP                     (120)

#define AXP_CHG_ATTR(_name)					\
{								\
	.attr = { .name = #_name,.mode = 0644 },		\
	.show =  _name##_show,					\
	.store = _name##_store,					\
}

struct axp_adc_res {//struct change
	u16 vbat_res;
	u16 ocvbat_res;
	u16 ibat_res;
	u16 ichar_res;
	u16 idischar_res;
	u16 vac_res;
	u16 iac_res;
	u16 vusb_res;
	u16 iusb_res;
	u16 ts_res;
};

struct axp_charger {
	/*power supply sysfs*/
	struct power_supply batt;
	struct power_supply	ac;
	struct power_supply	usb;

	/*i2c device*/
	struct device *master;

	/* adc */
	struct axp_adc_res *adc;
	u32 sample_time;

	/*monitor*/
	struct delayed_work work;
	u32 interval;

	/*battery info*/
	struct power_supply_info *battery_info;

	/*charger control*/
	bool chgen;
	bool limit_on;
	u32 chgcur;
	u32 chgvol;
	u32 chgend;

	/*charger time */
	s32 chgpretime;
	s32 chgcsttime;

	/*external charger*/
	bool chgexten;
	s32 chgextcur;

	/* charger status */
	bool bat_det;
	bool is_on;
	bool is_finish;
	bool ac_not_enough;
	bool ac_det;
	bool usb_det;
	bool ac_valid;
	bool usb_valid;
	bool usb_adapter_valid;
	bool ext_valid;
	bool bat_current_direction;
	bool in_short;
	bool batery_active;
	bool low_charge_current;
	bool int_over_temp;
	u8 fault;
	s32 charge_on;

	s32 vbat;
	s32 ibat;
	s32 pbat;
	s32 vac;
	s32 iac;
	s32 vusb;
	s32 iusb;
	s32 ocv;

	s32 disvbat;
	s32 disibat;

	/*rest time*/
	s32 rest_vol;
	s32 ocv_rest_vol;
	s32 base_restvol;
	s32 rest_time;

	/*ic temperature*/
	s32 ic_temp;
	s32 bat_temp;

	/* chg current limit work*/
	struct delayed_work usbwork;

	/*irq*/
	struct notifier_block nb;

	/* platform callbacks for battery low and critical events */
	void (*battery_low)(void);
	void (*battery_critical)(void);
	s32 (*chg_usb_ac_current_set)(struct axp_charger *charger, aw_charge_type port_type, u32 current_limit);
	s32 (*chg_usb_ac_vol_set)(struct axp_charger *charger, aw_charge_type port_type, u32 vol_limit);

	/* timer for report ac/usb type */
	struct timer_list usb_status_timer;

	struct dentry *debug_file;

	spinlock_t charger_lock;
};

extern const struct axp_config_info axp81x_config;
extern struct class axppower_class;
extern struct axp_charger *axp_charger;
extern s32 vbus_curr_limit_debug;

extern s32 axp81x_chg_current_limit(u32 current_limit);
extern s32 axp81x_init(struct axp_charger *charger);
extern void axp81x_exit(struct axp_charger *charger);
extern void axp_charger_update_state(struct axp_charger *charger);
extern void axp_charger_update(struct axp_charger *charger, const struct axp_config_info *axp_config);
extern void axp_battery_update_vol(struct axp_charger *charger);
extern void axp_powerkey_set(s32 value);
extern s32 axp_powerkey_get(void);
extern s32 axp_irq_init(struct axp_charger *charger, struct platform_device *pdev);
extern void axp_irq_exit(struct axp_charger *charger);
extern s32 axp_chg_init(struct axp_charger *charger);
extern void axp_chg_exit(struct axp_charger *charger);
extern void axp_usbac_in(struct axp_charger *charger);
extern void axp_usbac_out(struct axp_charger *charger);
extern void axp_usbac_checkst(struct axp_charger *charger);
extern s32 axp_enable_irq(struct axp_charger *charger);
extern s32 axp_disable_irq(struct axp_charger *charger, u32 reg_count);
#endif

