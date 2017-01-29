/*
 * debugfs driver for allwinnertech AXP
 *
 * Copyright (C) 2014 allwinnertech Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>

extern u64 axp_read_power_sply(void);
static struct dentry *my_axpdbg_root;
static struct power_supply_data {
	atomic_t delay;
	struct delayed_work work;
} supply_data;

static void power_sply_work_func(struct work_struct *work)
{
	u64 power_sply = 0;
	struct power_supply_data *supply_data = container_of((struct delayed_work *)work,
			struct power_supply_data, work);
	u64 delay = msecs_to_jiffies(atomic_read(&supply_data->delay));
	power_sply = axp_read_power_sply();
	printk(KERN_ERR "power_sply = %lld mW\n", power_sply);
	schedule_delayed_work(&supply_data->work, delay);
}

static s32 axpdbg_power_sply_open(struct inode * inode, struct file * file)
{
	printk("%s: enter\n", __func__);
	return 0;
}
static s32 axpdbg_power_sply_release(struct inode * inode, struct file * file)
{
	printk("%s: enter\n", __func__);
	return 0;
}

static ssize_t axpdbg_power_sply_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	s32 var;
	u64 delay = 1000;
	char data_buf[4] = {0};

	printk("%s: enter\n", __func__);

	if (copy_from_user(&data_buf, buf, count))
		return -EFAULT;

	var = simple_strtoul(data_buf, NULL, 10);

	if(var) {
		atomic_set(&supply_data.delay, var);
		delay = msecs_to_jiffies(var);
		schedule_delayed_work(&supply_data.work, delay);
	} else {
		atomic_set(&supply_data.delay, 0);
		cancel_delayed_work_sync(&supply_data.work);
	}

	return count;
}

static const struct file_operations power_sply_ops = {
	.write       = axpdbg_power_sply_write,
	.open        = axpdbg_power_sply_open,
	.release     = axpdbg_power_sply_release,
};

 static s32 __init axp_debugfs_init(void)
{
	my_axpdbg_root = debugfs_create_dir("axpdbg", NULL);
	if(!debugfs_create_file("power_sply", 0644, my_axpdbg_root, NULL,&power_sply_ops))
		goto Fail;
	INIT_DELAYED_WORK(&supply_data.work, power_sply_work_func);

	return 0;

Fail:
	debugfs_remove_recursive(my_axpdbg_root);
	my_axpdbg_root = NULL;
	return -ENOENT;
}

late_initcall(axp_debugfs_init);

