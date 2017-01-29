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
#ifndef  __USB_BASE_H__
#define  __USB_BASE_H__

#include <common.h>
#include <malloc.h>
#include <asm/arch/usb.h>
#include <usbdevice.h>
#include <usb_defs.h>
#include "usb_module.h"
#include "usb_status.h"
#include <spare_head.h>


//#define SUNXI_USB_DEBUG
#undef SUNXI_USB_DEBUG

#ifdef SUNXI_USB_DEBUG
#   define sunxi_usb_dbg(fmt,args...)	printf(fmt ,##args)
#else
#   define sunxi_usb_dbg(fmt,args...)
#endif


//-----------------------------------------------------------------------------
//   数据结构
//-----------------------------------------------------------------------------
#if defined(SUNXI_USB_30)
typedef struct sunxi_udc
{
	u32 address;		/* device address, that host distribute */
	int speed;		    /* flag. is high speed? 				*/

	int ep0_control_step;
	int eps_bulk_step;

	struct usb_device_request *standard_reg;
}
sunxi_udc_t;

#else
//-----------------------------------------------------------------------------
//   usb3.0控制器 数据结构
//-----------------------------------------------------------------------------
typedef struct sunxi_udc
{
    __hdle usbc_hd;

	u32 address;		/* device address, that host distribute */
	int speed;		    /* flag. is high speed? 				*/

	u32 bulk_ep_max;	/* bulk ep max packet size 				*/
	u32 fifo_size;		/* fifo size 							*/
	u32 bulk_in_addr;
	u32 bulk_out_addr;
	u32 dma_send_channal;
	u32 dma_recv_channal;
	struct usb_device_request standard_reg;
}
sunxi_udc_t;
#endif

#define CBWCDBLENGTH	16

#define CBWSIGNATURE	(0x43425355)
#define CSWSIGNATURE	(0x53425355)
/* Command Block Wrapper */
struct umass_bbb_cbw_t
{
	__u32		dCBWSignature;
	__u32		dCBWTag;
	__u32		dCBWDataTransferLength;
	__u8		bCBWFlags;
	__u8		bCBWLUN;
	__u8		bCDBLength;
	__u8		CBWCDB[CBWCDBLENGTH];
}__attribute__ ((packed));


/* Command Status Wrapper */
struct umass_bbb_csw_t
{
	__u32		dCSWSignature;
	__u32		dCSWTag;
	__u32		dCSWDataResidue;
	__u8		bCSWStatus;
}__attribute__ ((packed));


#define  SUNXI_USB_RX_BUFFER_COUNT			 (1)



typedef struct sunxi_ubuf_s
{
	uchar *rx_base_buffer;
	uchar *rx_req_buffer;			//bulk传输的请求阶段buffer
	uint   rx_req_length;			//bulk传输的请求阶段数据长度

	uint   rx_ready_for_data;		//表示数据接收已经完成标志

	uint   request_size;			//需要发送的数据长度
}
sunxi_ubuf_t;


extern  void sunxi_udc_ep_reset(void);

extern  int sunxi_udc_start_recv_by_dma(void* mem_buf, uint length);

extern  void sunxi_udc_send_setup(uint bLength, void *buffer);
extern  int  sunxi_udc_send_data(void *buffer, unsigned int buffer_size);


extern  int sunxi_udc_set_address(uchar address);
extern  int sunxi_udc_set_configuration(int config_param);

extern	int sunxi_udc_get_ep_max(void);
extern  int sunxi_udc_get_ep_in_type(void);
extern  int sunxi_udc_get_ep_out_type(void);

#endif
