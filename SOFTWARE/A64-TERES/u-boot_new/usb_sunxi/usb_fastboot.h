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

#ifndef  __USB_FASTBOOT_H__
#define  __USB_FASTBOOT_H__

#include <common.h>

#define  SUNXI_USB_FASTBOOT_DEV_MAX       (6)

char sunxi_fastboot_normal_LangID[8]     = {0x04, 0x03, 0x09, 0x04, '\0'};
#define SUNXI_FASTBOOT_DEVICE_MANUFACTURER				"USB Developer"  		/* 厂商信息 	*/
#define SUNXI_FASTBOOT_DEVICE_PRODUCT					"Android Fastboot"  	/* 产品信息 	*/
#define SUNXI_FASTBOOT_DEVICE_SERIAL_NUMBER				"20080411"  			/* 产品序列号 	*/
#define SUNXI_FASTBOOT_DEVICE_CONFIG        			"Android Fastboot"
#define SUNXI_FASTBOOT_DEVICE_INTERFACE     			"Android Bootloader Interface"

/* String 0 is the language id */
#define SUNXI_FASTBOOT_DEVICE_STRING_PRODUCT_INDEX       	1
#define SUNXI_FASTBOOT_DEVICE_STRING_SERIAL_NUMBER_INDEX 	2
#define SUNXI_FASTBOOT_DEVICE_STRING_CONFIG_INDEX        	3
#define SUNXI_FASTBOOT_DEVICE_STRING_INTERFACE_INDEX     	4
#define SUNXI_FASTBOOT_DEVICE_STRING_MANUFACTURER_INDEX  	5


char  *sunxi_usb_fastboot_dev[SUNXI_USB_FASTBOOT_DEV_MAX] = {	sunxi_fastboot_normal_LangID, 			\
																SUNXI_FASTBOOT_DEVICE_MANUFACTURER, 		\
																SUNXI_FASTBOOT_DEVICE_PRODUCT, 				\
																SUNXI_FASTBOOT_DEVICE_SERIAL_NUMBER, 		\
																SUNXI_FASTBOOT_DEVICE_CONFIG, 				\
																SUNXI_FASTBOOT_DEVICE_INTERFACE
															};

#define  SUNXI_USB_FASTBOOT_BUFFER_MAX               (32 * 1024 * 1024)


#define  SUNXI_USB_FASTBOOT_IDLE					 (0)
#define  SUNXI_USB_FASTBOOT_SETUP					 (1)
#define  SUNXI_USB_FASTBOOT_SEND_DATA				 (2)
#define  SUNXI_USB_FASTBOOT_RECEIVE_DATA			 (3)


typedef struct
{
	char *base_recv_buffer;		//存放接收到的数据
	char *act_recv_buffer;
	uint   try_to_recv;
	uint   act_recv;
	char *base_send_buffer;		//存放预发送数据
	char *act_send_buffer;
	uint   send_size;		        //需要发送数据的长度
}
fastboot_trans_set_t;


#define   SUNXI_FASTBOOT_SEND_MEM_SIZE   (64 * 1024)

#define	  DEVICE_VENDOR_ID  					0x1F3A
#define	  DEVICE_PRODUCT_ID 					0x1010
#define	  DEVICE_BCD        					0x0200

#endif

