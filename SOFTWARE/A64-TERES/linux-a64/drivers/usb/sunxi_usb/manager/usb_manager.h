/*
 * drivers/usb/sunxi_usb/manager/usb_manager.h
 * (C) Copyright 2010-2015
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * javen, 2011-4-14, create this file
 *
 * usb manager programme.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#ifndef  __USB_MANAGER_H__
#define  __USB_MANAGER_H__

#if	0
#define DMSG_DBG_MANAGER     		pr_debug
#else
#define DMSG_DBG_MANAGER(...)
#endif
__s32 create_node_file(struct platform_device *pdev);
__s32 remove_node_file(struct platform_device *pdev);

#endif   //__USB_MANAGER_H__

