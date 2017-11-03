/* linux/drivers/video/sunxi/disp2/disp/dev_debugfs.h
 *
 * Copyright (c) 2014 Allwinnertech Co., Ltd.
 * Author: Tyle <tyle@allwinnertech.com>
 *
 * Display debugfs header for sunxi platform
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <asm/uaccess.h>
#include "de/bsp_display.h"
#include "de/disp_tv.h"

int dispdbg_init(void);
int dispdbg_exit(void);
