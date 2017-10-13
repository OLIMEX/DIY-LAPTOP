/*
 * Battery charger driver for allwinnertech AXP81X
 *
 * Copyright (C) 2014 ALLWINNERTECH.
 *  Ming Li <liming@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kthread.h>

#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/mfd/axp-mfd.h>
#include <asm/div64.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include "axp-cfg.h"
#include "axp81x-sply.h"

struct axp_charger *axp_charger;

#ifdef CONFIG_HAS_EARLYSUSPEND
static struct early_suspend axp_early_suspend;
#endif

static enum power_supply_property axp_battery_props[] = {
	POWER_SUPPLY_PROP_MODEL_NAME,
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TEMP,
};

static enum power_supply_property axp_ac_props[] = {
	POWER_SUPPLY_PROP_MODEL_NAME,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
};

static enum power_supply_property axp_usb_props[] = {
	POWER_SUPPLY_PROP_MODEL_NAME,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
};

static void axp_battery_check_status(struct axp_charger *charger,
            union power_supply_propval *val)
{
	if (charger->bat_det){
		if (charger->ext_valid){
			if( charger->rest_vol == 100)
				val->intval = POWER_SUPPLY_STATUS_FULL;
			else if(charger->charge_on)
				val->intval = POWER_SUPPLY_STATUS_CHARGING;
			else
				val->intval = POWER_SUPPLY_STATUS_NOT_CHARGING;
		}else
			val->intval = POWER_SUPPLY_STATUS_DISCHARGING;
	}else
		val->intval = POWER_SUPPLY_STATUS_FULL;
}

static void axp_battery_check_health(struct axp_charger *charger,
            union power_supply_propval *val)
{
	if (charger->fault & AXP81X_FAULT_LOG_BATINACT)
		val->intval = POWER_SUPPLY_HEALTH_DEAD;
	else if (charger->fault & AXP81X_FAULT_LOG_OVER_TEMP)
		val->intval = POWER_SUPPLY_HEALTH_OVERHEAT;
	else if (charger->fault & AXP81X_FAULT_LOG_COLD)
		val->intval = POWER_SUPPLY_HEALTH_COLD;
	else
		val->intval = POWER_SUPPLY_HEALTH_GOOD;
}

static s32 axp_battery_get_property(struct power_supply *psy,
           enum power_supply_property psp,
           union power_supply_propval *val)
{
	struct axp_charger *charger;
	s32 ret = 0;

	charger = container_of(psy, struct axp_charger, batt);

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		axp_battery_check_status(charger, val);
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		axp_battery_check_health(charger, val);
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = charger->battery_info->technology;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
		val->intval = charger->battery_info->voltage_max_design;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
		val->intval = charger->battery_info->voltage_min_design;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		//val->intval = charger->ocv * 1000;
		val->intval = charger->vbat * 1000;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = charger->ibat * 1000;
		break;
	case POWER_SUPPLY_PROP_MODEL_NAME:
		val->strval = charger->batt.name;
	break;
	case POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN:
		val->intval = charger->battery_info->energy_full_design;
		//  DBG_PSY_MSG("POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN:%d\n",val->intval);
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = charger->rest_vol;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
	{
	/* in order to get hardware state, we must update charger state now.
	 * by sunny at 2012-12-23 11:06:15.
	 */
		axp_charger_update_state(charger);
		val->intval = charger->bat_current_direction;
		if (charger->bat_temp > 50 || -5 < charger->bat_temp)
			val->intval = 0;
		break;
	}
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = charger->bat_det;
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval =  charger->bat_temp * 10;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static s32 axp_ac_get_property(struct power_supply *psy,
           enum power_supply_property psp,
           union power_supply_propval *val)
{
	struct axp_charger *charger;
	s32 ret = 0;

	charger = container_of(psy, struct axp_charger, ac);

	switch(psp){
	case POWER_SUPPLY_PROP_MODEL_NAME:
		val->strval = charger->ac.name;break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = (charger->ac_valid || charger->usb_adapter_valid);break;
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = (charger->ac_valid || charger->usb_adapter_valid);break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = charger->vac * 1000;break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = charger->iac * 1000;break;
	default:
		ret = -EINVAL;break;
	}
	return ret;
}

static s32 axp_usb_get_property(struct power_supply *psy,
           enum power_supply_property psp,
           union power_supply_propval *val)
{
	struct axp_charger *charger;
	s32 ret = 0;

	charger = container_of(psy, struct axp_charger, usb);

	switch(psp){
	case POWER_SUPPLY_PROP_MODEL_NAME:
		val->strval = charger->usb.name;break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = charger->usb_valid;break;
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = charger->usb_valid;break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = charger->vusb * 1000;break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = charger->iusb * 1000;break;
	default:
		ret = -EINVAL;break;
	}
	return ret;
}

static char *supply_list[] = {
	"battery",
};

static void axp_battery_setup_psy(struct axp_charger *charger)
{
	struct power_supply *batt = &charger->batt;
	struct power_supply *ac = &charger->ac;
	struct power_supply *usb = &charger->usb;
	struct power_supply_info *info = charger->battery_info;

	batt->name = "battery";
	batt->use_for_apm = info->use_for_apm;
	batt->type = POWER_SUPPLY_TYPE_BATTERY;
	batt->get_property = axp_battery_get_property;
	batt->properties = axp_battery_props;
	batt->num_properties = ARRAY_SIZE(axp_battery_props);

	ac->name = "ac";
	ac->type = POWER_SUPPLY_TYPE_MAINS;
	ac->get_property = axp_ac_get_property;
	ac->supplied_to = supply_list,
	ac->num_supplicants = ARRAY_SIZE(supply_list),
	ac->properties = axp_ac_props;
	ac->num_properties = ARRAY_SIZE(axp_ac_props);

	usb->name = "usb";
	usb->type = POWER_SUPPLY_TYPE_USB;
	usb->get_property = axp_usb_get_property;
	usb->supplied_to = supply_list,
	usb->num_supplicants = ARRAY_SIZE(supply_list),
	usb->properties = axp_usb_props;
	usb->num_properties = ARRAY_SIZE(axp_usb_props);
};

#if defined CONFIG_HAS_EARLYSUSPEND
static void axp_earlysuspend(struct early_suspend *h)
{
	DBG_PSY_MSG(DEBUG_SPLY, "======early suspend=======\n");
}
static void axp_lateresume(struct early_suspend *h)
{
	s32 value;

	value = axp_powerkey_get();
	if (0 != value)
		axp_powerkey_set(0);

	DBG_PSY_MSG(DEBUG_SPLY, "======late resume=======\n");
}
#endif

static void axp_charging_monitor(struct work_struct *work)
{
	struct axp_charger *charger;
	u8 temp_val[4];
	static s32 pre_rest_vol = 0;
	static s32 pre_bat_curr_dir = 0;
	u64 power_sply = 0;

	charger = container_of(work, struct axp_charger, work.work);
	axp_charger_update_state(charger);
	axp_charger_update(charger, &axp81x_config);

	if (axp_debug & DEBUG_SPLY) {
		DBG_PSY_MSG(DEBUG_SPLY, "charger->ic_temp = %d\n",charger->ic_temp);
		DBG_PSY_MSG(DEBUG_SPLY, "charger->bat_temp = %d\n",charger->bat_temp);
		DBG_PSY_MSG(DEBUG_SPLY, "charger->vbat = %d\n",charger->vbat);
		DBG_PSY_MSG(DEBUG_SPLY, "charger->ibat = %d\n",charger->ibat);
		DBG_PSY_MSG(DEBUG_SPLY, "charger->ocv = %d\n",charger->ocv);
		DBG_PSY_MSG(DEBUG_SPLY, "charger->disvbat = %d\n",charger->disvbat);
		DBG_PSY_MSG(DEBUG_SPLY, "charger->disibat = %d\n",charger->disibat);
		power_sply = charger->disvbat * charger->disibat;
		if (0 != power_sply)
			power_sply = power_sply/1000;
		DBG_PSY_MSG(DEBUG_SPLY, "power_sply = %lld mW\n",power_sply);
		axp_reads(charger->master,0xba,2,temp_val);
		DBG_PSY_MSG(DEBUG_SPLY, "Axp Rdc = %d\n",(((temp_val[0] & 0x1f) <<8) + temp_val[1])*10742/10000);
		axp_reads(charger->master,0xe0,2,temp_val);
		DBG_PSY_MSG(DEBUG_SPLY, "Axp batt_max_cap = %d\n",(((temp_val[0] & 0x7f) <<8) + temp_val[1])*1456/1000);
		axp_reads(charger->master,0xe2,2,temp_val);
		DBG_PSY_MSG(DEBUG_SPLY, "Axp coulumb_counter = %d\n",(((temp_val[0] & 0x7f) <<8) + temp_val[1])*1456/1000);
		axp_read(charger->master,0xb8,temp_val);
		DBG_PSY_MSG(DEBUG_SPLY, "Axp REG_B8 = %x\n",temp_val[0]);
		DBG_PSY_MSG(DEBUG_SPLY, "charger->is_on = %d\n",charger->is_on);
		DBG_PSY_MSG(DEBUG_SPLY, "charger->bat_current_direction = %d\n",charger->bat_current_direction);
		DBG_PSY_MSG(DEBUG_SPLY, "charger->charge_on = %d\n",charger->charge_on);
		DBG_PSY_MSG(DEBUG_SPLY, "charger->ext_valid = %d\n",charger->ext_valid);
	}

	axp_battery_update_vol(charger);

	/* if battery volume changed, inform uevent */
	if((charger->rest_vol - pre_rest_vol) || (charger->bat_current_direction != pre_bat_curr_dir)){
		axp_reads(charger->master,0xe2,2,temp_val);
		axp_reads(charger->master,0xe4,2,(temp_val+2));
		DBG_PSY_MSG(DEBUG_SPLY, "battery vol change: %d->%d \n", pre_rest_vol, charger->rest_vol);
		DBG_PSY_MSG(DEBUG_SPLY, "for test %d %d %d %d %d %d\n",charger->vbat,charger->ocv,charger->ibat,
			(temp_val[2] & 0x7f),(temp_val[3] & 0x7f),(((temp_val[0] & 0x7f) <<8) + temp_val[1])*1456/1000);
		pre_rest_vol = charger->rest_vol;
		pre_bat_curr_dir = charger->bat_current_direction;
		power_supply_changed(&charger->batt);
	}
	/* reschedule for the next time */
	schedule_delayed_work(&charger->work, charger->interval);
}

static s32 axp_battery_probe(struct platform_device *pdev)
{
	struct axp_charger *charger;
	struct axp_supply_init_data *pdata = pdev->dev.platform_data;
	s32 ret;

	if (pdata == NULL)
		return -EINVAL;
	if (pdata->chgcur > AXP81X_CHARGE_CURRENT_MAX ||
	pdata->chgvol < AXP81X_CHARGE_VOLTAGE_LEVEL0 ||
	pdata->chgvol > AXP81X_CHARGE_VOLTAGE_LEVEL3){
		printk("charger milliamp is too high or target voltage is over range\n");
		return -EINVAL;
	}
	if (pdata->chgpretime < AXP81X_CHARGE_PRETIME_MIN || pdata->chgpretime >AXP81X_CHARGE_PRETIME_MAX ||
	pdata->chgcsttime < AXP81X_CHARGE_FASTTIME_MIN || pdata->chgcsttime > AXP81X_CHARGE_FASTTIME_MAX){
		printk("prechaging time or constant current charging time is over range\n");
		return -EINVAL;
	}
	charger = kzalloc(sizeof(*charger), GFP_KERNEL);
	if (charger == NULL)
		return -ENOMEM;

	spin_lock_init(&charger->charger_lock);

	spin_lock(&charger->charger_lock);
	charger->master 	= pdev->dev.parent;
	charger->chgcur		= pdata->chgcur;
	charger->chgvol     	= pdata->chgvol;
	charger->chgend       	= pdata->chgend;
	charger->sample_time  	= pdata->sample_time;
	charger->chgen        	= pdata->chgen;
	charger->chgpretime   	= pdata->chgpretime;
	charger->chgcsttime 	= pdata->chgcsttime;
	charger->battery_info 	= pdata->battery_info;
	charger->disvbat	= 0;
	charger->disibat	= 0;
	spin_unlock(&charger->charger_lock);

	axp_charger = charger;

	ret = axp81x_init(charger);
	if (ret) {
		goto err_ps_register;
	}

	axp_battery_setup_psy(charger);
	ret = power_supply_register(&pdev->dev, &charger->batt);
	if (ret)
		goto err_ps_register;

	ret = power_supply_register(&pdev->dev, &charger->ac);
	if (ret){
		power_supply_unregister(&charger->batt);
		goto err_ps_register;
	}

	ret = power_supply_register(&pdev->dev, &charger->usb);
	if (ret){
		power_supply_unregister(&charger->ac);
		power_supply_unregister(&charger->batt);
		goto err_ps_register;
	}

	platform_set_drvdata(pdev, charger);

	axp_charger_update_state(charger);
	axp_battery_update_vol(charger);

	spin_lock(&charger->charger_lock);
	charger->interval = msecs_to_jiffies(10 * 1000);
	spin_unlock(&charger->charger_lock);
	INIT_DELAYED_WORK(&charger->work, axp_charging_monitor);
	schedule_delayed_work(&charger->work, charger->interval);

	axp_chg_init(charger);
	ret = axp_irq_init(charger, pdev);
	if(ret){
		printk("cat notaxp_charger_create_attrs!!!===\n ");
		return ret;
	}
	axp_usbac_checkst(charger);

#ifdef CONFIG_HAS_EARLYSUSPEND
	axp_early_suspend.suspend = axp_earlysuspend;
	axp_early_suspend.resume = axp_lateresume;
	axp_early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 2;
	register_early_suspend(&axp_early_suspend);
#endif

	class_register(&axppower_class);

	return ret;
err_ps_register:
	axp81x_exit(charger);
	cancel_delayed_work_sync(&charger->work);
	kfree(charger);
	return ret;
}

static s32 axp_battery_remove(struct platform_device *dev)
{
	struct axp_charger *charger = platform_get_drvdata(dev);

	class_unregister(&axppower_class);
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&axp_early_suspend);
#endif
	axp_irq_exit(charger);
	axp_chg_exit(charger);
	cancel_delayed_work_sync(&charger->work);
	power_supply_unregister(&charger->usb);
	power_supply_unregister(&charger->ac);
	power_supply_unregister(&charger->batt);
	kfree(charger);
	return 0;
}


static s32 axp81x_suspend(struct platform_device *dev, pm_message_t state)
{
	struct axp_charger *charger = platform_get_drvdata(dev);
	/*s32 ret;

	ret = axp_disable_irq(charger, 6);
	if (ret < 0)
		return ret;*/
	schedule_delayed_work(&charger->usbwork, 0);
	flush_delayed_work(&charger->usbwork);
	cancel_delayed_work_sync(&charger->work);
	cancel_delayed_work_sync(&charger->usbwork);
	axp81x_chg_current_limit(axp81x_config.pmu_suspend_chgcur);
	return 0;
}

static s32 axp81x_resume(struct platform_device *dev)
{
	struct axp_charger *charger = platform_get_drvdata(dev);
	s32 pre_rest_vol;

	/*axp_enable_irq(charger);*/
	pre_rest_vol = charger->rest_vol;
	axp_charger_update_state(charger);
	axp_charger_update(charger, &axp81x_config);
	axp_battery_update_vol(charger);
	if(charger->rest_vol - pre_rest_vol){
		printk("battery vol change: %d->%d \n", pre_rest_vol, charger->rest_vol);
		pre_rest_vol = charger->rest_vol;
		axp_write(charger->master,AXP81X_DATA_BUFFER1,charger->rest_vol | 0x80);
	}
	axp81x_chg_current_limit(axp81x_config.pmu_runtime_chgcur);
	power_supply_changed(&charger->batt);
	power_supply_changed(&charger->ac);
	power_supply_changed(&charger->usb);
	schedule_delayed_work(&charger->work, charger->interval);
	schedule_delayed_work(&charger->usbwork, msecs_to_jiffies(7 * 1000));
	return 0;
}

static void axp81x_shutdown(struct platform_device *dev)
{
	struct axp_charger *charger = platform_get_drvdata(dev);

	cancel_delayed_work_sync(&charger->work);
	axp81x_chg_current_limit(axp81x_config.pmu_shutdown_chgcur);
	return;
}

static struct platform_driver axp_battery_driver = {
	.driver = {
		.name = "axp81x-supplyer",
		.owner  = THIS_MODULE,
	},
	.probe = axp_battery_probe,
	.remove = axp_battery_remove,
	.suspend = axp81x_suspend,
	.resume = axp81x_resume,
	.shutdown = axp81x_shutdown,
};

static s32 axp_battery_init(void)
{
	s32 ret =0;

	ret = platform_driver_register(&axp_battery_driver);
	return ret;
}

static void axp_battery_exit(void)
{
	platform_driver_unregister(&axp_battery_driver);
}

device_initcall(axp_battery_init);
module_exit(axp_battery_exit);

MODULE_DESCRIPTION("AXP81X battery driver");
MODULE_AUTHOR("Ming Li");
MODULE_LICENSE("GPL");
