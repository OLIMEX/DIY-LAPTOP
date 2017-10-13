/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef  __USB_STATUS_H__
#define  __USB_STATUS_H__


#define  SUNXI_USB_REQ_SUCCESSED				(0)
#define  SUNXI_USB_REQ_DEVICE_NOT_SUPPORTED		(-1)
#define  SUNXI_USB_REQ_UNKNOWN_COMMAND			(-2)
#define  SUNXI_USB_REQ_UNMATCHED_COMMAND		(-3)

#define  SUNXI_USB_REQ_DATA_HUNGRY				(-4)

#define  SUNXI_USB_REQ_OP_ERR					(-5)

#endif
