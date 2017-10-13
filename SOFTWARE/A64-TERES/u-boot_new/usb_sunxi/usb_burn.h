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

#ifndef  __USB_PBURN_H__
#define  __USB_PBURN_H__

#include <common.h>

#define  SUNXI_USB_PBURN_DEV_MAX       (4)

unsigned char  sunxi_usb_pbur_normal_LangID[8]        = {0x04, 0x03, 0x09, 0x04, '\0'};

unsigned char  sunxi_usb_pburn_iSerialNum0[32] = "20101201120001";

unsigned char  sunxi_usb_pburn_iManufacturer[32] = "AllWinner Technology";

unsigned char  sunxi_usb_pburn_iProduct[32] = "USB Mass Storage";

#define  SUNXI_USB_STRING_LANGIDS			 (0)
#define  SUNXI_USB_STRING_IMANUFACTURER	 	 (1)
#define  SUNXI_USB_STRING_IPRODUCT		 	 (2)
#define  SUNXI_USB_STRING_ISERIALNUMBER    	 (3)

unsigned char  *sunxi_usb_pburn_dev[SUNXI_USB_PBURN_DEV_MAX] = {sunxi_usb_pbur_normal_LangID, 		\
																sunxi_usb_pburn_iSerialNum0, 		\
																sunxi_usb_pburn_iManufacturer, 		\
																sunxi_usb_pburn_iProduct};


const unsigned char  pburn_InquiryData[40]  = {0x00, 0x80, 0x02, 0x02, 0x1f, 										\
										 0x00, 0x00, 0x00, 													\
										 'U',  'S',  'B',  '2',  '.',  '0',  0x00, 0x00, 					\
	                                     'U' , 'S',  'B',  ' ', 'S',  't',  'o' , 'r' , 'a' , 'g' , 'e',	\
	                                     0x00, 0x00, 0x00, 0x00, 0x00,
	                                     '0',  '1',  '0',  '0',  '\0' };

const unsigned char pburn_RequestSense[20] = {0x07,0x00,0x02,0x00,0x00,0x00,0x00,0x0a,0x00,0x00,0x00,0x00,0x3a,0x00,0x00,0x00,0x00,0x00};


#define  SUNXI_USB_PBURN_IDLE					 (0)
#define  SUNXI_USB_PBURN_SETUP					 (1)
#define  SUNXI_USB_PBURN_SEND_DATA				 (2)
#define  SUNXI_USB_PBURN_RECEIVE_DATA			 (3)
#define  SUNXI_USB_PBURN_STATUS					 (4)
#define  SUNXI_USB_PBURN_EXIT                    (5)
#define  SUNXI_USB_PBURN_RECEIVE_NULL			 (6)


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
pburn_trans_set_t;


typedef struct
{
	char  magic[16];       	//特征字符串，固定是 "usbhandshake"，不足的填空
	int   sizelo;			//盘符的低32位，单位是扇区
	int   sizehi;			//盘符的高32位，单位是扇区
	int   res1;
	int   res2;
}
__usb_handshake_t;

typedef struct
{
	char  magic[32];       	//特征字符串，固定是 "usbhandshake"，不足的填空
}
__usb_handshake_sec_t;

typedef struct
{
	char  magic[64];   //特征字符串
	int   err_no;
}
__usb_handshake_ext_t;


typedef struct
{
    //以下信息重复，表示每个key的信息
    char  name[64];    //key的名称
    u32 len;           //key数据段的总长度
	u8  *key_data;   //这是一个数组，存放key的全部信息，数据长度由len指定
}
sunxi_efuse_key_info_t;


typedef struct
{
    //以下信息重复，表示每个key的信息
    char  name[64];      //key的名称
    u32 type;          //0:aes   1:rsa    其它：未知key
	u32 len;           //key数据段的总长度
	u32 if_burn;       //是否需要烧录，0：不需要，1：需要
	u32 if_replace;    //是否允许替换之前的key，0：不允许；1：允许
	u32 if_crypt;     //是否需要小机端加密存储
    u8  *key_data[];   //这是一个数组，存放key的全部信息，数据长度由len指定
}
sunxi_usb_burn_key_info_t;

typedef struct
{
	u8  magic[16];	//数据头标识符，必须是"key-group-db"
	u8  hash[256]; //hash值(采用rootkey的私钥对所有的key信息进行计算)
	u32 count;      //key的个数
	u32 res[3];     //保留

	sunxi_usb_burn_key_info_t  key_info;
}
sunxi_usb_burn_main_info_t;

#define   SUNXI_PBURN_RECV_MEM_SIZE   (512 * 1024)
#define   SUNXI_PBURN_SEND_MEM_SIZE   (512 * 1024)

#endif

