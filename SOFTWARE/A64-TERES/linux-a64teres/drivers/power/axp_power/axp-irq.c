/*
 * Battery charger driver for AW-POWERS
 *
 * Copyright (C) 2014 ALLWINNERTECH.
 *  Ming Li <liming@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/input.h>
#include <linux/wakelock.h>
#include "axp-cfg.h"
#ifdef CONFIG_AW_AXP81X
#include "axp81x-sply.h"
#include "axp81x-common.h"
static const struct axp_config_info *axp_config = &axp81x_config;
static const u64 AXP_NOTIFIER_ON = AXP81X_NOTIFIER_ON;
#endif
#ifdef CONFIG_HAS_WAKELOCK
static struct wake_lock axp_wakeup_lock;
#endif
static  struct input_dev * powerkeydev;
static s32 axp_power_key = 0;

static DEFINE_SPINLOCK(axp_powerkey_lock);

void axp_powerkey_set(s32 value)
{
	spin_lock(&axp_powerkey_lock);
	axp_power_key = value;
	spin_unlock(&axp_powerkey_lock);
}
EXPORT_SYMBOL_GPL(axp_powerkey_set);

s32 axp_powerkey_get(void)
{
	s32 value;

	spin_lock(&axp_powerkey_lock);
	value = axp_power_key;
	spin_unlock(&axp_powerkey_lock);

	return value;
}
EXPORT_SYMBOL_GPL(axp_powerkey_get);


static void axp_change(struct axp_charger *charger)
{
	DBG_PSY_MSG(DEBUG_INT, "battery state change\n");
	axp_charger_update_state(charger);
	axp_charger_update(charger, axp_config);
	power_supply_changed(&charger->batt);
}

static void axp_presslong(struct axp_charger *charger)
{
	DBG_PSY_MSG(DEBUG_INT, "press long\n");
	input_report_key(powerkeydev, KEY_POWER, 1);
	input_sync(powerkeydev);
	ssleep(2);
	DBG_PSY_MSG(DEBUG_INT, "press long up\n");
	input_report_key(powerkeydev, KEY_POWER, 0);
	input_sync(powerkeydev);
}

static void axp_pressshort(struct axp_charger *charger)
{
	DBG_PSY_MSG(DEBUG_INT, "press short\n");
	input_report_key(powerkeydev, KEY_POWER, 1);
	input_sync(powerkeydev);
	msleep(100);
	input_report_key(powerkeydev, KEY_POWER, 0);
	input_sync(powerkeydev);
}

static void axp_keyup(struct axp_charger *charger)
{
	DBG_PSY_MSG(DEBUG_INT, "power key up\n");
	input_report_key(powerkeydev, KEY_POWER, 0);
	input_sync(powerkeydev);
}

static void axp_keydown(struct axp_charger *charger)
{
	DBG_PSY_MSG(DEBUG_INT, "power key down\n");
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_timeout(&axp_wakeup_lock, msecs_to_jiffies(20));
#endif
	input_report_key(powerkeydev, KEY_POWER, 1);
	input_sync(powerkeydev);
}

static void axp_capchange(struct axp_charger *charger)
{
	DBG_PSY_MSG(DEBUG_INT, "battery change\n");
	ssleep(2);
	axp_charger_update_state(charger);
	axp_charger_update(charger, axp_config);
	axp_battery_update_vol(charger);

	DBG_PSY_MSG(DEBUG_INT, "rest_vol = %d\n",charger->rest_vol);
	power_supply_changed(&charger->batt);
}

static s32 axp_clear_irq_state(struct axp_charger *charger, bool data, u32 event, u32 reg_count)
{
	u8 w[11];
	s32 ret = 0;
	u32 count = (reg_count*2) - 1;

	if(0 == data){
		w[0] = (u8) ((event) & 0xFF);
		w[1] = AXP_INTSTS2;
		w[2] = (u8) ((event >> 8) & 0xFF);
		w[3] = AXP_INTSTS3;
		w[4] = (u8) ((event >> 16) & 0xFF);
		w[5] = AXP_INTSTS4;
		w[6] = (u8) ((event >> 24) & 0xFF);
		w[7] = AXP_INTSTS5;
		w[8] = 0;
		w[9] = AXP_INTSTS6;
		w[10] = 0;
	} else {
		w[0] = 0;
		w[1] = AXP_INTSTS2;
		w[2] = 0;
		w[3] = AXP_INTSTS3;
		w[4] = 0;
		w[5] = AXP_INTSTS4;
		w[6] = 0;
		w[7] = AXP_INTSTS5;
		w[8] = (u8) ((event) & 0xFF);
		w[9] = AXP_INTSTS6;
		w[10] = (u8) ((event>> 8) & 0xFF);
	}

	ret = axp_writes(charger->master,AXP_INTSTS1,count,w);

	return ret;
}

static s32 axp_battery_event(struct notifier_block *nb, unsigned long event,
	 void *data)
{
	struct axp_charger *charger =
	container_of(nb, struct axp_charger, nb);
	s32 value;

	DBG_PSY_MSG(DEBUG_INT, "axp_battery_event enter...\n");
	if((bool)data==0){
		DBG_PSY_MSG(DEBUG_INT, "low 32bit status...\n");
		if(event & (AXP_IRQ_BATIN|AXP_IRQ_BATRE))
			axp_capchange(charger);
		if(event & (AXP_IRQ_BATINWORK|AXP_IRQ_BATOVWORK|AXP_IRQ_QBATINCHG|AXP_IRQ_BATINCHG
			|AXP_IRQ_QBATOVCHG|AXP_IRQ_BATOVCHG))
			axp_change(charger);
		if(event & (AXP_IRQ_ACOV|AXP_IRQ_USBOV|AXP_IRQ_CHAOV|AXP_IRQ_CHAST))
			axp_change(charger);
		if(event & (AXP_IRQ_ACIN|AXP_IRQ_USBIN)) {
#ifdef CONFIG_HAS_WAKELOCK
			wake_lock_timeout(&axp_wakeup_lock, msecs_to_jiffies(5000));
#endif
			axp_usbac_checkst(charger);
			axp_change(charger);
			axp_usbac_in(charger);
		}
		if(event & (AXP_IRQ_ACRE|AXP_IRQ_USBRE)) {
#ifdef CONFIG_HAS_WAKELOCK
			wake_lock_timeout(&axp_wakeup_lock, msecs_to_jiffies(5000));
#endif
			axp_usbac_checkst(charger);
			axp_change(charger);
			axp_usbac_out(charger);
		}
		if(event & AXP_IRQ_PEK_LONGTIME)
			axp_presslong(charger);
		if(event & AXP_IRQ_PEK_SHORTTIME)
			axp_pressshort(charger);
	} else {
		value = axp_powerkey_get();
		if (0 != value) {
			axp_powerkey_set(0);
		} else {
			if((event) & (AXP_IRQ_PEK_NEDGE>>32))
				axp_keydown(charger);
			if((event) & (AXP_IRQ_PEK_PEDGE>>32))
				axp_keyup(charger);
		}

		DBG_PSY_MSG(DEBUG_INT, "high 32bit status...\n");
	}
	axp_clear_irq_state(charger, (bool)data, event, 6);
	DBG_PSY_MSG(DEBUG_INT, "event = 0x%x\n",(s32) event);
	return 0;
}

s32 axp_disable_irq(struct axp_charger *charger, u32 reg_count)
{
	struct axp_dev *chip = dev_get_drvdata(charger->master);
	u8 irq_w[11];
	u32 count = (reg_count*2) - 1;
	s32 ret = 0;

#ifdef CONFIG_HAS_WAKELOCK
	if (wake_lock_active(&axp_wakeup_lock)) {
		printk(KERN_ERR "AXP:axp_wakeup_lock wakeup system\n");
		return -EPERM;
	}
#endif

	/*clear all irqs events*/
	irq_w[0] = 0xff;
	irq_w[1] = AXP_INTSTS2;
	irq_w[2] = 0xff;
	irq_w[3] = AXP_INTSTS3;
	irq_w[4] = 0xff;
	irq_w[5] = AXP_INTSTS4;
	irq_w[6] = 0xff;
	irq_w[7] = AXP_INTSTS5;
	irq_w[8] = 0xff;
	irq_w[9] = AXP_INTSTS6;
	irq_w[10] = 0xff;
	ret = axp_writes(charger->master,  AXP_INTSTS1,  count,  irq_w);
	if(0 != ret)
		printk("%s: clean axp irq state failed.\n", __func__);

#ifdef CONFIG_AXP_NMI_USED
	disable_nmi();
#endif

	/* close all irqs*/
	ret = chip->ops->disable_irqs(chip, AXP_NOTIFIER_ON);
	if(0 != ret)
		printk("%s: axp irq disable failed.\n", __func__);

	return 0;
}

s32 axp_enable_irq(struct axp_charger *charger)
{
	struct axp_dev *chip = dev_get_drvdata(charger->master);
	s32 ret = 0;

	ret = chip->ops->enable_irqs(chip, AXP_NOTIFIER_ON);
	if(0 != ret)
		printk("%s: axp irq enable failed.\n", __func__);

#ifdef CONFIG_AXP_NMI_USED
	enable_nmi();
#endif

	return ret;
}

s32 axp_irq_init(struct axp_charger *charger, struct platform_device *pdev)
{
	s32 ret = 0;

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&axp_wakeup_lock, WAKE_LOCK_SUSPEND, "axp_wakeup_lock");
#endif

	powerkeydev = input_allocate_device();
	if (!powerkeydev) {
		kfree(powerkeydev);
		return -ENODEV;
	}
	powerkeydev->name = pdev->name;
	powerkeydev->phys = "m1kbd/input2";
	powerkeydev->id.bustype = BUS_HOST;
	powerkeydev->id.vendor = 0x0001;
	powerkeydev->id.product = 0x0001;
	powerkeydev->id.version = 0x0100;
	powerkeydev->open = NULL;
	powerkeydev->close = NULL;
	powerkeydev->dev.parent = &pdev->dev;
	set_bit(EV_KEY, powerkeydev->evbit);
	set_bit(EV_REL, powerkeydev->evbit);
	set_bit(KEY_POWER, powerkeydev->keybit);
	ret = input_register_device(powerkeydev);
	if(ret)
		printk("Unable to Register the power key\n");

	charger->nb.notifier_call = axp_battery_event;
	ret = axp_register_notifier(charger->master, &charger->nb, AXP_NOTIFIER_ON);
	if (ret)
		goto err_notifier;

	return ret;

err_notifier:
	axp_unregister_notifier(charger->master, &charger->nb, AXP_NOTIFIER_ON);
	return ret;
}

void axp_irq_exit(struct axp_charger *charger)
{
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&axp_wakeup_lock);
#endif
	axp_unregister_notifier(charger->master, &charger->nb, AXP_NOTIFIER_ON);
	input_unregister_device(powerkeydev);
	kfree(powerkeydev);
	return;
}

