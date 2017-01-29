/*
 * nand_interface.c
 * (C) Copyright 2007-2011
 * Reuuimlla Technology Co., Ltd. <www.reuuimllatech.com>
 * huangxin <huangxin@Reuuimllatech.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */
#include <linux/init.h>
#include <linux/module.h>
#include "include/nand_include.h"

static int __init init_nand_libmodule(void)
{
	//printk("hello:%s,%d\n", __func__, __LINE__);
	//printk("hello:%s,%d\n", __func__, __LINE__);
	nand_init();
	return 0;
}
module_init(init_nand_libmodule);

static void __exit exit_nand_libmodule(void)
{
	nand_exit();
}
module_exit(exit_nand_libmodule);

MODULE_AUTHOR("Soft-Reuuimlla");
MODULE_DESCRIPTION("User mode CEDAR device interface");
MODULE_LICENSE("GPL");
