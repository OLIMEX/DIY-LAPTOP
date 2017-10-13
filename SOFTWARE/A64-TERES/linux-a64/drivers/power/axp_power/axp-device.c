#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include "axp-cfg.h"

static struct of_device_id axp_device_match[] = {
	{ .compatible = "allwinner,", .data = NULL },
	{ }
};

static s32 axp_device_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	const struct of_device_id *device;
	struct axp_config_info *axp_config = NULL;

	device = of_match_device(axp_device_match, &pdev->dev);
	if (!device)
		return -ENODEV;
	axp_config = (struct axp_config_info *)device->data;
	if (NULL == axp_config) {
		return -ENOENT;
	}

	if (!of_device_is_available(node)) {
		axp_config->pmu_used = 0;
		printk("%s: pmu_used = 0", __func__);
		return -EPERM;
	} else
		axp_config->pmu_used = 1;

	if (of_property_read_u32(node, "pmu_twi_id", &axp_config->pmu_twi_id))
		axp_config->pmu_twi_id = 0;

	if (of_property_read_u32(node, "pmu_twi_addr", &axp_config->pmu_twi_addr))
		axp_config->pmu_twi_addr = 0x34;

	if (of_property_read_u32(node, "pmu_irq_id", &axp_config->pmu_irq_id))
		axp_config->pmu_irq_id = 0;

	if (of_property_read_u32(node, "pmu_battery_rdc", &axp_config->pmu_battery_rdc))
		axp_config->pmu_battery_rdc = BATRDC;

	if (of_property_read_u32(node, "pmu_battery_cap", &axp_config->pmu_battery_cap))
		axp_config->pmu_battery_cap = 4000;

	if (of_property_read_u32(node, "pmu_batdeten", &axp_config->pmu_batdeten))
		axp_config->pmu_batdeten = 1;

	if (of_property_read_u32(node, "pmu_chg_ic_temp", &axp_config->pmu_chg_ic_temp))
		axp_config->pmu_chg_ic_temp = 0;

	if (of_property_read_u32(node, "pmu_runtime_chgcur", &axp_config->pmu_runtime_chgcur))
		axp_config->pmu_runtime_chgcur = INTCHGCUR / 1000;
		axp_config->pmu_runtime_chgcur = axp_config->pmu_runtime_chgcur * 1000;

	if (of_property_read_u32(node, "pmu_suspend_chgcur", &axp_config->pmu_suspend_chgcur))
		axp_config->pmu_suspend_chgcur = 1200;
		axp_config->pmu_suspend_chgcur = axp_config->pmu_suspend_chgcur * 1000;

	if (of_property_read_u32(node, "pmu_shutdown_chgcur", &axp_config->pmu_shutdown_chgcur))
		axp_config->pmu_shutdown_chgcur = 1200;
		axp_config->pmu_shutdown_chgcur = axp_config->pmu_shutdown_chgcur *1000;

	if (of_property_read_u32(node, "pmu_init_chgvol", &axp_config->pmu_init_chgvol))
		axp_config->pmu_init_chgvol = INTCHGVOL / 1000;
		axp_config->pmu_init_chgvol = axp_config->pmu_init_chgvol * 1000;

	if (of_property_read_u32(node, "pmu_init_chgend_rate", &axp_config->pmu_init_chgend_rate))
		axp_config->pmu_init_chgend_rate = INTCHGENDRATE;

	if (of_property_read_u32(node, "pmu_init_chg_enabled", &axp_config->pmu_init_chg_enabled))
		axp_config->pmu_init_chg_enabled = 1;

	if (of_property_read_u32(node, "pmu_init_bc_en", &axp_config->pmu_init_bc_en))
		axp_config->pmu_init_bc_en = 0;

	if (of_property_read_u32(node, "pmu_init_adc_freq", &axp_config->pmu_init_adc_freq))
		axp_config->pmu_init_adc_freq = INTADCFREQ;

	if (of_property_read_u32(node, "pmu_init_adcts_freq", &axp_config->pmu_init_adcts_freq))
		axp_config->pmu_init_adcts_freq = INTADCFREQC;

	if (of_property_read_u32(node, "pmu_init_chg_pretime", &axp_config->pmu_init_chg_pretime))
		axp_config->pmu_init_chg_pretime = INTCHGPRETIME;

	if (of_property_read_u32(node, "pmu_init_chg_csttime", &axp_config->pmu_init_chg_csttime))
		axp_config->pmu_init_chg_csttime = INTCHGCSTTIME;

	if (of_property_read_u32(node, "pmu_batt_cap_correct", &axp_config->pmu_batt_cap_correct))
		axp_config->pmu_batt_cap_correct = 1;

	if (of_property_read_u32(node, "pmu_chg_end_on_en", &axp_config->pmu_chg_end_on_en))
		axp_config->pmu_chg_end_on_en = 0;

	if (of_property_read_u32(node, "ocv_coulumb_100", &axp_config->ocv_coulumb_100))
		axp_config->ocv_coulumb_100 = 0;

	if (of_property_read_u32(node, "pmu_bat_para1", &axp_config->pmu_bat_para1))
		axp_config->pmu_bat_para1 = OCVREG0;

	if (of_property_read_u32(node, "pmu_bat_para2", &axp_config->pmu_bat_para2))
		axp_config->pmu_bat_para2 = OCVREG1;

	if (of_property_read_u32(node, "pmu_bat_para3", &axp_config->pmu_bat_para3))
		axp_config->pmu_bat_para3 = OCVREG2;

	if (of_property_read_u32(node, "pmu_bat_para4", &axp_config->pmu_bat_para4))
		axp_config->pmu_bat_para4 = OCVREG3;

	if (of_property_read_u32(node, "pmu_bat_para5", &axp_config->pmu_bat_para5))
		axp_config->pmu_bat_para5 = OCVREG4;

	if (of_property_read_u32(node, "pmu_bat_para6", &axp_config->pmu_bat_para6))
		axp_config->pmu_bat_para6 = OCVREG5;

	if (of_property_read_u32(node, "pmu_bat_para7", &axp_config->pmu_bat_para7))
		axp_config->pmu_bat_para7 = OCVREG6;

	if (of_property_read_u32(node, "pmu_bat_para8", &axp_config->pmu_bat_para8))
		axp_config->pmu_bat_para8 = OCVREG7;

	if (of_property_read_u32(node, "pmu_bat_para9", &axp_config->pmu_bat_para9))
		axp_config->pmu_bat_para9 = OCVREG8;

	if (of_property_read_u32(node, "pmu_bat_para10", &axp_config->pmu_bat_para10))
		axp_config->pmu_bat_para10 = OCVREG9;

	if (of_property_read_u32(node, "pmu_bat_para11", &axp_config->pmu_bat_para11))
		axp_config->pmu_bat_para11 = OCVREGA;

	if (of_property_read_u32(node, "pmu_bat_para12", &axp_config->pmu_bat_para12))
		axp_config->pmu_bat_para12 = OCVREGB;

	if (of_property_read_u32(node, "pmu_bat_para13", &axp_config->pmu_bat_para13))
		axp_config->pmu_bat_para13 = OCVREGC;

	if (of_property_read_u32(node, "pmu_bat_para14", &axp_config->pmu_bat_para14))
		axp_config->pmu_bat_para14 = OCVREGD;

	if (of_property_read_u32(node, "pmu_bat_para15", &axp_config->pmu_bat_para15))
		axp_config->pmu_bat_para15 = OCVREGE;

	if (of_property_read_u32(node, "pmu_bat_para16", &axp_config->pmu_bat_para16))
		axp_config->pmu_bat_para16 = OCVREGF;

	//Add 32 Level OCV para 20121128 by evan
	if (of_property_read_u32(node, "pmu_bat_para17", &axp_config->pmu_bat_para17))
		axp_config->pmu_bat_para17 = OCVREG10;

	if (of_property_read_u32(node, "pmu_bat_para18", &axp_config->pmu_bat_para18))
		axp_config->pmu_bat_para18 = OCVREG11;

	if (of_property_read_u32(node, "pmu_bat_para19", &axp_config->pmu_bat_para19))
		axp_config->pmu_bat_para19 = OCVREG12;

	if (of_property_read_u32(node, "pmu_bat_para20", &axp_config->pmu_bat_para20))
		axp_config->pmu_bat_para20 = OCVREG13;

	if (of_property_read_u32(node, "pmu_bat_para21", &axp_config->pmu_bat_para21))
		axp_config->pmu_bat_para21 = OCVREG14;

	if (of_property_read_u32(node, "pmu_bat_para22", &axp_config->pmu_bat_para22))
		axp_config->pmu_bat_para22 = OCVREG15;

	if (of_property_read_u32(node, "pmu_bat_para23", &axp_config->pmu_bat_para23))
		axp_config->pmu_bat_para23 = OCVREG16;

	if (of_property_read_u32(node, "pmu_bat_para24", &axp_config->pmu_bat_para24))
		axp_config->pmu_bat_para24 = OCVREG17;

	if (of_property_read_u32(node, "pmu_bat_para25", &axp_config->pmu_bat_para25))
		axp_config->pmu_bat_para25 = OCVREG18;

	if (of_property_read_u32(node, "pmu_bat_para26", &axp_config->pmu_bat_para26))
		axp_config->pmu_bat_para26 = OCVREG19;

	if (of_property_read_u32(node, "pmu_bat_para27", &axp_config->pmu_bat_para27))
		axp_config->pmu_bat_para27 = OCVREG1A;

	if (of_property_read_u32(node, "pmu_bat_para28", &axp_config->pmu_bat_para28))
		axp_config->pmu_bat_para28 = OCVREG1B;

	if (of_property_read_u32(node, "pmu_bat_para29", &axp_config->pmu_bat_para29))
		axp_config->pmu_bat_para29 = OCVREG1C;

	if (of_property_read_u32(node, "pmu_bat_para30", &axp_config->pmu_bat_para30))
		axp_config->pmu_bat_para30 = OCVREG1D;

	if (of_property_read_u32(node, "pmu_bat_para31", &axp_config->pmu_bat_para31))
		axp_config->pmu_bat_para31 = OCVREG1E;

	if (of_property_read_u32(node, "pmu_bat_para32", &axp_config->pmu_bat_para32))
		axp_config->pmu_bat_para32 = OCVREG1F;

	if (of_property_read_u32(node, "pmu_ac_vol", &axp_config->pmu_ac_vol))
		axp_config->pmu_ac_vol = 4400;

	if (of_property_read_u32(node, "pmu_usbpc_vol", &axp_config->pmu_usbpc_vol))
		axp_config->pmu_usbpc_vol = 4400;

	if (of_property_read_u32(node, "pmu_ac_cur", &axp_config->pmu_ac_cur))
		axp_config->pmu_ac_cur = 0;

	if (of_property_read_u32(node, "pmu_usbpc_cur", &axp_config->pmu_usbpc_cur))
		axp_config->pmu_usbpc_cur = 0;

	if (of_property_read_u32(node, "pmu_pwroff_vol", &axp_config->pmu_pwroff_vol))
		axp_config->pmu_pwroff_vol = 3300;

	if (of_property_read_u32(node, "pmu_pwron_vol", &axp_config->pmu_pwron_vol))
		axp_config->pmu_pwron_vol = 2900;
	if (of_property_read_u32(node, "pmu_powkey_off_time", &axp_config->pmu_powkey_off_time))
		axp_config->pmu_powkey_off_time = 6000;

	//offlevel restart or not 0:not restart 1:restart
	if (of_property_read_u32(node, "pmu_powkey_off_func", &axp_config->pmu_powkey_off_func))
		axp_config->pmu_powkey_off_func   = 0;

	//16's power restart or not 0:not restart 1:restart
	if (of_property_read_u32(node, "pmu_powkey_off_en", &axp_config->pmu_powkey_off_en))
		axp_config->pmu_powkey_off_en   = 1;

	if (of_property_read_u32(node, "pmu_powkey_off_delay_time", &axp_config->pmu_powkey_off_delay_time))
		axp_config->pmu_powkey_off_delay_time   = 0;

	if (of_property_read_u32(node, "pmu_powkey_long_time", &axp_config->pmu_powkey_long_time))
		axp_config->pmu_powkey_long_time = 1500;

	if (of_property_read_u32(node, "pmu_pwrok_time", &axp_config->pmu_pwrok_time))
		axp_config->pmu_pwrok_time    = 64;

	if (of_property_read_u32(node, "pmu_powkey_on_time", &axp_config->pmu_powkey_on_time))
		axp_config->pmu_powkey_on_time = 1000;

	if (of_property_read_u32(node, "pmu_reset_shutdown_en", &axp_config->pmu_reset_shutdown_en))
		axp_config->pmu_reset_shutdown_en = 0;

	if (of_property_read_u32(node, "pmu_battery_warning_level1", &axp_config->pmu_battery_warning_level1))
		axp_config->pmu_battery_warning_level1 = 15;

	if (of_property_read_u32(node, "pmu_battery_warning_level2", &axp_config->pmu_battery_warning_level2))
		axp_config->pmu_battery_warning_level2 = 0;

	if (of_property_read_u32(node, "pmu_restvol_adjust_time", &axp_config->pmu_restvol_adjust_time))
		axp_config->pmu_restvol_adjust_time = 30;

	if (of_property_read_u32(node, "pmu_ocv_cou_adjust_time", &axp_config->pmu_ocv_cou_adjust_time))
		axp_config->pmu_ocv_cou_adjust_time = 60;

	if (of_property_read_u32(node, "pmu_chgled_func", &axp_config->pmu_chgled_func))
		axp_config->pmu_chgled_func = 0;

	if (of_property_read_u32(node, "pmu_chgled_type", &axp_config->pmu_chgled_type))
		axp_config->pmu_chgled_type = 0;

	if (of_property_read_u32(node, "pmu_vbusen_func", &axp_config->pmu_vbusen_func))
		axp_config->pmu_vbusen_func = 1;

	if (of_property_read_u32(node, "pmu_reset", &axp_config->pmu_reset))
		axp_config->pmu_reset = 0;

	if (of_property_read_u32(node, "pmu_IRQ_wakeup", &axp_config->pmu_IRQ_wakeup))
		axp_config->pmu_IRQ_wakeup = 0;

	if (of_property_read_u32(node, "pmu_hot_shutdown", &axp_config->pmu_hot_shutdown))
		axp_config->pmu_hot_shutdown = 1;

	if (of_property_read_u32(node, "pmu_inshort", &axp_config->pmu_inshort))
		axp_config->pmu_inshort = 0;

	if (of_property_read_u32(node, "power_start", &axp_config->power_start))
		axp_config->power_start = 0;

	if (of_property_read_u32(node, "pmu_bat_temp_enable", &axp_config->pmu_bat_temp_enable))
		axp_config->pmu_bat_temp_enable = 0;

	if (of_property_read_u32(node, "pmu_bat_charge_ltf", &axp_config->pmu_bat_charge_ltf))
		axp_config->pmu_bat_charge_ltf = 0xA5;

	if (of_property_read_u32(node, "pmu_bat_charge_htf", &axp_config->pmu_bat_charge_htf))
		axp_config->pmu_bat_charge_htf = 0x1F;

	if (of_property_read_u32(node, "pmu_bat_shutdown_ltf", &axp_config->pmu_bat_shutdown_ltf))
		axp_config->pmu_bat_shutdown_ltf = 0xFC;

	if (of_property_read_u32(node, "pmu_bat_shutdown_htf", &axp_config->pmu_bat_shutdown_htf))
		axp_config->pmu_bat_shutdown_htf = 0x16;

	if (of_property_read_u32(node, "pmu_bat_temp_para1", &axp_config->pmu_bat_temp_para1))
		axp_config->pmu_bat_temp_para1 = 0;

	if (of_property_read_u32(node, "pmu_bat_temp_para2", &axp_config->pmu_bat_temp_para2))
		axp_config->pmu_bat_temp_para2 = 0;

	if (of_property_read_u32(node, "pmu_bat_temp_para3", &axp_config->pmu_bat_temp_para3))
		axp_config->pmu_bat_temp_para3 = 0;

	if (of_property_read_u32(node, "pmu_bat_temp_para4", &axp_config->pmu_bat_temp_para4))
		axp_config->pmu_bat_temp_para4 = 0;

	if (of_property_read_u32(node, "pmu_bat_temp_para5", &axp_config->pmu_bat_temp_para5))
		axp_config->pmu_bat_temp_para5 = 0;

	if (of_property_read_u32(node, "pmu_bat_temp_para6", &axp_config->pmu_bat_temp_para6))
		axp_config->pmu_bat_temp_para6 = 0;

	if (of_property_read_u32(node, "pmu_bat_temp_para7", &axp_config->pmu_bat_temp_para7))
		axp_config->pmu_bat_temp_para7 = 0;

	if (of_property_read_u32(node, "pmu_bat_temp_para8", &axp_config->pmu_bat_temp_para8))
		axp_config->pmu_bat_temp_para8 = 0;

	if (of_property_read_u32(node, "pmu_bat_temp_para9", &axp_config->pmu_bat_temp_para9))
		axp_config->pmu_bat_temp_para9 = 0;

	if (of_property_read_u32(node, "pmu_bat_temp_para10", &axp_config->pmu_bat_temp_para10))
		axp_config->pmu_bat_temp_para10 = 0;

	if (of_property_read_u32(node, "pmu_bat_temp_para11", &axp_config->pmu_bat_temp_para11))
		axp_config->pmu_bat_temp_para11 = 0;

	if (of_property_read_u32(node, "pmu_bat_temp_para12", &axp_config->pmu_bat_temp_para12))
		axp_config->pmu_bat_temp_para12 = 0;

	if (of_property_read_u32(node, "pmu_bat_temp_para13", &axp_config->pmu_bat_temp_para13))
		axp_config->pmu_bat_temp_para13 = 0;

	if (of_property_read_u32(node, "pmu_bat_temp_para14", &axp_config->pmu_bat_temp_para14))
		axp_config->pmu_bat_temp_para14 = 0;

	if (of_property_read_u32(node, "pmu_bat_temp_para15", &axp_config->pmu_bat_temp_para15))
		axp_config->pmu_bat_temp_para15 = 0;

	if (of_property_read_u32(node, "pmu_bat_temp_para16", &axp_config->pmu_bat_temp_para16))
		axp_config->pmu_bat_temp_para16 = 0;

        return 0;

}

static struct platform_driver axp_device_driver = {
	.probe = axp_device_probe,
	.driver = {
		.name  = "axp-device",
		.owner = THIS_MODULE,
		.of_match_table = axp_device_match,
	},
};
s32  axp_device_tree_parse(char * pmu_type, struct axp_config_info *axp_config)
{
	s32 ret;

	strcat(axp_device_match[0].compatible, pmu_type);

	axp_device_match[0].data = (void *)axp_config;

	ret = platform_driver_register(&axp_device_driver);

	return ret;
}

