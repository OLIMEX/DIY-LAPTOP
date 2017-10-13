/*
 * at24.c - handle most I2C EEPROMs
 *
 * Copyright (C) 2005-2007 David Brownell
 * Copyright (C) 2008 Wolfram Sang, Pengutronix
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/mod_devicetable.h>
#include <linux/log2.h>
#include <linux/bitops.h>
#include <linux/jiffies.h>
#include <linux/of.h>
#include <linux/i2c.h>
#include <linux/i2c/at24.h>


#define EEPROM_ATTR(_name)       \
{									\
	.attr = { .name = #_name,.mode = 0444 },    \
	.show =  _name##_show,          \
}

struct i2c_client *this_client;

static const struct i2c_device_id at24_ids[] = {
	{ "24c16", 0 },
	{ /* END OF LIST */ }
};
MODULE_DEVICE_TABLE(i2c, at24_ids);

static int eeprom_i2c_rxdata(char *rxdata, int length)
{
	int ret;

	struct i2c_msg msgs[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= &rxdata[0],
		},
		{
			.addr	= this_client->addr,
			.flags	= I2C_M_RD,
			.len	= length,
			.buf	= &rxdata[1],
		},
	};

	ret = i2c_transfer(this_client->adapter, msgs, 2);
	if (ret < 0)
		pr_info("%s i2c read eeprom error: %d\n", __func__, ret);

	return ret;
}

static int eeprom_i2c_txdata(char *txdata, int length)
{
	int ret;

	struct i2c_msg msg[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= length,
			.buf	= txdata,
		},
	};

	ret = i2c_transfer(this_client->adapter, msg, 1);
	if (ret < 0)
		pr_err("%s i2c write eeprom error: %d\n", __func__, ret);

	return 0;
}

static ssize_t read_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
  	u8 rxdata[4];

	rxdata[0] = 0x1;
  	eeprom_i2c_rxdata(rxdata, 3);

	return sprintf(buf, "Read data : 0x%x, 0x%x, 0x%x, 0x%x\n",
				rxdata[0], rxdata[1], rxdata[2], rxdata[3]);
}

static ssize_t write_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	static u8 txdata[4] = {0x1, 0xAA, 0xBB, 0xCC};

	txdata[1]++;
	txdata[2]++;
	txdata[3]++;
	eeprom_i2c_txdata(txdata,4);

	return sprintf(buf, "Write data: 0x%x, 0x%x, 0x%x, 0x%x\n",
				txdata[0], txdata[1], txdata[2], txdata[3]);
}

static struct kobj_attribute read	= EEPROM_ATTR(read);
static struct kobj_attribute write	= EEPROM_ATTR(write);

static const struct attribute *test_attrs[] = {
	&read.attr,
	&write.attr,
	NULL,
};

static int at24_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int err;
	this_client = client;
	printk("1..at24_probe \n");
	err = sysfs_create_files(&client->dev.kobj,test_attrs);
	printk("2..at24_probe \n");
	if(err){
		printk("sysfs_create_files failed\n");
	}
	printk("3..at24_probe \n");
	return 0;
}

static int at24_remove(struct i2c_client *client)
{
	sysfs_remove_files(&client->dev.kobj,test_attrs);
	return 0;
}

/*-------------------------------------------------------------------------*/

static struct i2c_driver at24_driver = {
	.driver = {
		.name = "at24",
		.owner = THIS_MODULE,
	},
	.probe = at24_probe,
	.remove = at24_remove,
	.id_table = at24_ids,
};

static int __init at24_init(void)
{
	printk("%s    %d\n", __func__, __LINE__);
	
	return i2c_add_driver(&at24_driver);
}
module_init(at24_init);

static void __exit at24_exit(void)
{
	printk("%s()%d - \n", __func__, __LINE__);

	i2c_del_driver(&at24_driver);
}
module_exit(at24_exit);

MODULE_DESCRIPTION("24C16 simple test");
MODULE_AUTHOR("pannan");
MODULE_LICENSE("GPL");
