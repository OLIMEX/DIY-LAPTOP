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

#ifndef  __USB_MASS_H__
#define  __USB_MASS_H__

#include <common.h>

#define  SUNXI_USB_MASS_DEV_MAX       (4)

unsigned char   normal_LangID[8]        = {0x04, 0x03, 0x09, 0x04, '\0'};

unsigned char  sunxi_usb_mass_iSerialNum0[32] = "20101201120001";

unsigned char  sunxi_usb_mass_iManufacturer[32] = "AllWinner Technology";

unsigned char  sunxi_usb_mass_iProduct[32] = "USB Mass Storage";

#define  SUNXI_USB_STRING_LANGIDS			 (0)
#define  SUNXI_USB_STRING_IMANUFACTURER	 	 (1)
#define  SUNXI_USB_STRING_IPRODUCT		 	 (2)
#define  SUNXI_USB_STRING_ISERIALNUMBER    	 (3)

unsigned char  *sunxi_usb_mass_dev[SUNXI_USB_MASS_DEV_MAX] = {	normal_LangID, 						\
																sunxi_usb_mass_iSerialNum0, 		\
																sunxi_usb_mass_iManufacturer, 		\
																sunxi_usb_mass_iProduct};


const unsigned char  InquiryData[40]  = {0x00, 0x80, 0x02, 0x02, 0x1f, 										\
										 0x00, 0x00, 0x00, 													\
										 'U',  'S',  'B',  '2',  '.',  '0',  0x00, 0x00, 					\
	                                     'U' , 'S',  'B',  ' ', 'S',  't',  'o' , 'r' , 'a' , 'g' , 'e',	\
	                                     0x00, 0x00, 0x00, 0x00, 0x00,
	                                     '0',  '1',  '0',  '0',  '\0' };

const unsigned char RequestSense[20] = {0x07,0x00,0x02,0x00,0x00,0x00,0x00,0x0a,0x00,0x00,0x00,0x00,0x3a,0x00,0x00,0x00,0x00,0x00};


#define  SUNXI_USB_MASS_IDLE					 (0)
#define  SUNXI_USB_MASS_SETUP					 (1)
#define  SUNXI_USB_MASS_SEND_DATA				 (2)
#define  SUNXI_USB_MASS_RECEIVE_DATA			 (3)
#define  SUNXI_USB_MASS_STATUS					 (4)



typedef struct
{
	uchar *base_recv_buffer;		//存放接收到的数据的首地址，必须足够大
	uchar *act_recv_buffer;//
	uint   recv_size;
	uint   to_be_recved_size;
	uchar *base_send_buffer;		//存放将要到的数据的首地址，必须足够大
	uchar *act_send_buffer;//
	uint   send_size;		//需要发送数据的长度
	uint   flash_start;			//起始位置，可能是内存，也可能是flash扇区
	uint   flash_sectors;
}
mass_trans_set_t;


#define   SUNXI_MASS_RECV_MEM_SIZE   (512 * 1024)
#define   SUNXI_MASS_SEND_MEM_SIZE   (512 * 1024)

#endif

