/*
 * Regulators dependence driver for allwinnertech AXP
 *
 * Copyright (C) 2014 allwinnertech Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/async.h>
#include <linux/err.h>
#include <linux/ctype.h>
#include <linux/regulator/consumer.h>
#include <linux/power/axp_depend.h>

/*
 * function: add the pwr_dm specified by input id to sys_pwr_dm;
 *
 * */
s32 add_sys_pwr_dm(const char *id)
{
	s32 ret = 0, sys_id_conut = 0;
	char ldo_name[20] = {0};
	u32 sys_pwr_mask = 0;

	sys_id_conut = axp_check_sys_id(id);
	if (0 > sys_id_conut) {
		printk(KERN_ERR "%s: %s not sys id.\n", __func__, id);
		return -1;
	} else {
		sys_pwr_mask = get_sys_pwr_dm_mask();
		if (sys_pwr_mask & (0x1 << sys_id_conut)) {
			printk(KERN_INFO "%s: sys_pwr_mask = 0x%x, sys_mask already set.\n", __func__, sys_pwr_mask);
			return 1;
		}
	}

	ret = get_ldo_name(id, (char *)&ldo_name);
	if (ret < 0) {
		printk(KERN_ERR "%s: get ldo name  for id: %s failed\n", __func__, id);
		return -1;
	}

	ret = check_ldo_alwayson((const char *)&ldo_name);
	if (ret == 0) {
		if (set_ldo_alwayson((const char *)&ldo_name, 1)) {
			printk(KERN_ERR "%s: %s set_ldo_alwayson failed.\n", __func__, ldo_name);
			return -1;
		}
	} else if (ret == 1) {
		printk(KERN_ERR "%s: %s ldo already alwayson.\n", __func__, ldo_name);
	} else {
		printk(KERN_ERR "%s: %s set err.\n", __func__, ldo_name);
		return -1;
	}

	set_sys_pwr_dm_mask(sys_id_conut, 1);
	return 0;
}

s32 del_sys_pwr_dm(const char *id)
{
	s32 ret = 0, sys_id_conut = 0, i = 0;
	char ldo_name[20] = {0};
	char sys_ldo_name[20] = {0};
	u32 sys_pwr_mask = 0;
	char * sys_id;

	sys_id_conut = axp_check_sys_id(id);
	if (0 > sys_id_conut) {
		printk(KERN_ERR "%s: %s not sys id.\n", __func__, id);
		return -1;
	} else {

		sys_pwr_mask = get_sys_pwr_dm_mask();
		if (!(sys_pwr_mask & (0x1 << sys_id_conut)))
			return 1;
	}

	ret = get_ldo_name(id, (char *)&ldo_name);
	if (ret < 0) {
		printk(KERN_ERR "%s: get ldo name  for id: %s failed\n", __func__, id);
		return -1;
	}


	ret = check_ldo_alwayson((const char *)&ldo_name);
	if (ret == 0) {
		printk(KERN_ERR "%s: %s ldo is already not alwayson.\n", __func__, ldo_name);
	} else if (ret == 1) {
		for (i=0; i <VCC_MAX_INDEX; i++) {
			if (sys_id_conut == i)
				continue;
			if(is_sys_pwr_dm_active(i)) {
				sys_id = get_sys_pwr_dm_id(i);
				ret = get_ldo_name(sys_id, (char *)&sys_ldo_name);
				if (ret < 0) {
					printk(KERN_ERR "%s: get sys_ldo_name failed\n", __func__);
					return -1;
				}
				if (strcmp(sys_ldo_name, ldo_name) == 0) {
					set_sys_pwr_dm_mask(sys_id_conut, 0);
					return 0;
				}
			}
		}
		if (set_ldo_alwayson((const char *)&ldo_name, 0)) {
			printk(KERN_ERR "%s: %s set_ldo_alwayson failed.\n", __func__, ldo_name);
			return -1;
		}
	} else {
		printk(KERN_ERR "%s: %s set err.\n", __func__, ldo_name);
		return -1;
	}

	set_sys_pwr_dm_mask(sys_id_conut, 0);

	return 0;
}

/*
 *  function: judge whether pwr_dm is part of sys_pwr_domain.
 *  input: pwr_dm name, such as: "vdd_sys".
 *  return:
 *	nonnegative number: the sys_pwr_domain bitmap.
 *	-1: the input pwr_dm is not belong to sys_pwr_domain.
 *
 * */
s32 is_sys_pwr_dm_id(const char *id)
{
	s32 sys_id_conut = 0;

	sys_id_conut = axp_check_sys_id(id);
	if (0 <= sys_id_conut) {
		return sys_id_conut;
	} else {
		return -1;
	}
}

/*
 *  function: judge whether sys_pwr_domain is active.
 *  input: sys_pwr_domain bitmap.
 *  return:
 *	1: the input sys_pwr_domain is active.
 *	0: the input sys_pwr_domain is not active.
 *
 * */
s32 is_sys_pwr_dm_active(u32 bitmap)
{
	u32 sys_pwr_mask = 0;

	sys_pwr_mask = get_sys_pwr_dm_mask();
	if (sys_pwr_mask & (0x1 << bitmap))
			return 1;
	else
			return 0;
}
/*
 *  function:  get sys_pwr_dm_id name.
 *  input: sys_pwr_domain bitmap.
 *  return:
 *       failed:  NULL.
 *       success: sys_pwr_dm_id.
 * */
char *get_sys_pwr_dm_id(u32 bitmap)
{
	return axp_get_sys_id(bitmap);
}

static ssize_t add_sys_store(struct class *class,
        struct class_attribute *attr, const char *buf, size_t count)
{
	int name_len;
	const char *arg;
	char sys_name[20] = {0};

	/* Find length of lock name */
	arg = buf;
	while (*arg && !isspace(*arg))
		arg++;
	name_len = arg - buf;
	if ((!name_len)&&(name_len>20))
		goto bad_arg;

	strncpy(sys_name, buf, name_len);

	add_sys_pwr_dm(sys_name);

	return count;

bad_arg:
	printk(KERN_ERR "%s para error.\n", __func__);
	return count;

}

static ssize_t del_sys_store(struct class *class,
        struct class_attribute *attr, const char *buf, size_t count)
{
	int name_len;
	const char *arg;
	char sys_name[20] = {0};

	/* Find length of lock name */
	arg = buf;
	while (*arg && !isspace(*arg))
		arg++;
	name_len = arg - buf;
	if ((!name_len)&&(name_len>20))
		goto bad_arg;

	strncpy(sys_name, buf, name_len);

	del_sys_pwr_dm(sys_name);

	return count;

bad_arg:
	printk(KERN_ERR "%s para error.\n", __func__);
	return count;

}

static ssize_t get_sys_dump_show(struct class *class,
			struct class_attribute *attr,	char *buf)
{
	ssize_t count = 0;

	count += sprintf(buf+count, "0x%x", get_sys_pwr_dm_mask());

	return count;
}

char check_sys_name[20] = {0};
static ssize_t check_sys_store(struct class *class,
        struct class_attribute *attr, const char *buf, size_t count)
{
	int name_len;
	const char *arg;

	/* Find length of lock name */
	arg = buf;
	while (*arg && !isspace(*arg))
		arg++;
	name_len = arg - buf;
	if ((!name_len)&&(name_len>20))
		goto bad_arg;

	strncpy(check_sys_name, buf, name_len);

	return count;

bad_arg:
	printk(KERN_ERR "%s para error.\n", __func__);
	return count;

}


static ssize_t check_sys_show(struct class *class,
			struct class_attribute *attr,	char *buf)
{
	s32 sys_id_conut = 0,ret = 0;
	ssize_t count = 0;
	char ldo_name[20] = {0};

	sys_id_conut = axp_check_sys_id(check_sys_name);
	if (0 > sys_id_conut) {
		printk(KERN_ERR "%s: %s not sys id.\n", __func__, check_sys_name);
		return -1;
	}

	ret = get_ldo_name(check_sys_name, (char *)&ldo_name);
	if (ret < 0) {
		printk(KERN_ERR "%s: get ldo name  for id: %s failed\n", __func__, check_sys_name);
		return -1;
	}

	ret = is_sys_pwr_dm_active(sys_id_conut);

	count += sprintf(buf+count, "%d", ret);

	return count;
}

struct class_attribute axp_depend_class_attrs[] = {
	__ATTR(add_sys,S_IRUGO|S_IWUSR,NULL,add_sys_store),
	__ATTR(del_sys,S_IRUGO|S_IWUSR,NULL,del_sys_store),
	__ATTR(get_sys,S_IRUGO|S_IWUSR,get_sys_dump_show,NULL),
	__ATTR(check_sys,S_IRUGO|S_IWUSR,check_sys_show,check_sys_store),
	__ATTR_NULL,
};

static struct class depend_class = {
	.name		= "axp_depend",
	.owner		= THIS_MODULE,
	.class_attrs	= axp_depend_class_attrs,
};

static int __init depend_module_init(void)
{
	int status;

	status = class_register(&depend_class);
	if(status < 0)
		pr_err("%s,%d err, status:%d\n", __func__, __LINE__, status);

	return status;
}

static void __exit depend_module_exit(void)
{
	class_unregister(&depend_class);
}

module_init(depend_module_init);
module_exit(depend_module_exit);

MODULE_AUTHOR("Ming Li");
MODULE_DESCRIPTION("axp depend driver");
MODULE_LICENSE("GPL");

