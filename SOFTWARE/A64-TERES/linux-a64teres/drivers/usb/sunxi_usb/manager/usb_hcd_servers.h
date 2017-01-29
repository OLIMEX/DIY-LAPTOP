/*
 * drivers/usb/sunxi_usb/manager/usb_hcd_servers.h
 * (C) Copyright 2010-2015
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * javen, 2011-4-14, create this file
 *
 * usb host contoller driver. service function set.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#ifndef  __USB_HCD_SERVERS_H__
#define  __USB_HCD_SERVERS_H__

int sunxi_usb_disable_hcd(__u32 usbc_no);
int sunxi_usb_enable_hcd(__u32 usbc_no);

#endif  //__USB_HCD_SERVERS_H__

