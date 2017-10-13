/*
 * Copyright (c) 2013-2015 liming@allwinnertech.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */
#ifndef _AXP_MFD_CORE_H
#define _AXP_MFD_CORE_H
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/module.h>

#include <linux/mfd/axp-mfd-81x.h>

#define AXP_MFD_ATTR(_name)				\
{							\
	.attr = { .name = #_name,.mode = 0644 },	\
	.show =  _name##_show,				\
	.store = _name##_store,				\
}

/* AXP battery charger data */
struct power_supply_info;

struct axp_supply_init_data {
	/* battery parameters */
	struct power_supply_info *battery_info;

	/* current and voltage to use for battery charging */
	u32 chgcur;
	u32 chgvol;
	u32 chgend;
	/*charger control*/
	bool chgen;
	bool limit_on;
	/*charger time */
	s32 chgpretime;
	s32 chgcsttime;

	/*adc sample time */
	u32 sample_time;

	/* platform callbacks for battery low and critical IRQs */
	void (*battery_low)(void);
	void (*battery_critical)(void);
};

struct axp_funcdev_info {
	s32		id;
	const char	*name;
	void	*platform_data;
};

struct axp_platform_data {
	s32 num_regl_devs;
	s32 num_sply_devs;
	struct axp_funcdev_info *regl_devs;
	struct axp_funcdev_info *sply_devs;
};

struct axp_dev {
	struct axp_platform_data *pdata;
	struct axp_mfd_chip_ops	*ops;
	struct device		*dev;

	s32 attrs_number;
	struct device_attribute *attrs;

	struct i2c_client *client;

	s32			type;
	u64			irqs_enabled;
	u32			irq_number;

	spinlock_t		spinlock;
	struct mutex		lock;
	struct work_struct	irq_work;

	struct blocking_notifier_head notifier_list;
	/* lists we belong to */
	struct list_head list; /* list of all axps */

};

struct axp_mfd_chip_ops {
	s32	(*init_chip)(struct axp_dev *);
	s32	(*enable_irqs)(struct axp_dev *, u64 irqs);
	s32	(*disable_irqs)(struct axp_dev *, u64 irqs);
	s32	(*read_irqs)(struct axp_dev *, u64 *irqs);
};

extern s32 axp_register_notifier(struct device *dev,
		struct notifier_block *nb, u64 irqs);
extern s32 axp_unregister_notifier(struct device *dev,
		struct notifier_block *nb, u64 irqs);


typedef enum AW_CHARGE_TYPE
{
	CHARGE_AC,
	CHARGE_USB_20,
	CHARGE_USB_30,
	CHARGE_MAX
} aw_charge_type;

/* NOTE: the functions below are not intended for use outside
 * of the AXP sub-device drivers
 */
extern s32 axp_write(struct device *dev, s32 reg, u8 val);
extern s32 axp_writes(struct device *dev, s32 reg, s32 len, u8 *val);
extern s32 axp_read(struct device *dev, s32 reg, u8 *val);
extern s32 axp_reads(struct device *dev, s32 reg, s32 len, u8 *val);
extern s32 axp_update(struct device *dev, s32 reg, u8 val, u8 mask);
extern s32 axp_set_bits(struct device *dev, s32 reg, u8 bit_mask);
extern s32 axp_clr_bits(struct device *dev, s32 reg, u8 bit_mask);
extern s32 axp_update_sync(struct device *dev, s32 reg, u8 val, u8 mask);
extern s32 axp_set_bits_sync(struct device *dev, s32 reg, u8 bit_mask);
extern s32 axp_clr_bits_sync(struct device *dev, s32 reg, u8 bit_mask);
extern struct axp_dev *axp_dev_lookup(s32 type);
s32 axp_register_mfd(struct axp_dev *dev);
void axp_unregister_mfd(struct axp_dev *dev);
extern void axp_reg_debug(s32 reg, s32 len, u8 *val);

/* NOTE: the functions below are used for outside
 * of the AXP drivers
 */
extern s32 axp_usbcur(aw_charge_type type);
extern s32 axp_usbvol(aw_charge_type type);
extern s32 axp_usb_det(void);
extern s32 axp_powerkey_get(void);
extern void axp_powerkey_set(s32 value);
extern u64 axp_read_power_sply(void);
extern s32 axp_read_bat_cap(void);
extern s32 axp_read_ac_chg(void);
#endif
