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
#ifndef  __USB_MODULE_H__
#define  __USB_MODULE_H__


#include "usb_base.h"



#define  SUNXI_USB_DEVICE_MASS   	 1
#define  SUNXI_USB_DEVICE_EFEX   	 2
#define  SUNXI_USB_DEVICE_FASTBOOT   3
#define  SUNXI_USB_DEVICE_BURN       4
//#define  SUNXI_USB_DEVICE_EFEX_TEST  5

typedef struct sunxi_usb_setup_req_s
{
	int  (* state_init		  )(void);
	int  (* state_exit		  )(void);
	void (* state_reset       )(void);
	int  (* standard_req_op   )(uint cmd, struct usb_device_request *req, uchar *buffer);
	int  (* nonstandard_req_op)(uint cmd, struct usb_device_request *req, uchar *buffer, uint data_status);
	int  (* state_loop		  )(void *sunxi_udc);
	void (* dma_rx_isr		  )(void *p_arg);
	void (* dma_tx_isr		  )(void *p_arg);
}
sunxi_usb_setup_req_t;



#define  __sunxi_usb_module_init(name, state_init, state_exit, state_reset, standard_req_op, nonstandard_req_op, state_loop, dma_rx_isr, dma_tx_isr)					\
			sunxi_usb_setup_req_t setup_req_##name = {state_init, state_exit, state_reset, standard_req_op, nonstandard_req_op, state_loop, dma_rx_isr, dma_tx_isr };


#define  sunxi_usb_module_init(name, state_init, state_exit, state_reset, standard_req_op, nonstandard_req_op, state_loop, dma_rx_isr, dma_tx_isr)			\
			__sunxi_usb_module_init(name, state_init, state_exit, state_reset, standard_req_op, nonstandard_req_op, state_loop, dma_rx_isr, dma_tx_isr)


#define  __sunxi_usb_module_reg(name)						\
			sunxi_udev_active = &setup_req_##name


#define  sunxi_usb_module_reg(name)							\
			__sunxi_usb_module_reg(name)

#define  __sunxi_usb_module_ext(name)						\
			extern sunxi_usb_setup_req_t setup_req_##name

#define  sunxi_usb_module_ext(name)							\
			__sunxi_usb_module_ext(name)




#ifdef  SUNXI_USB_DEVICE_MASS
		sunxi_usb_module_ext(SUNXI_USB_DEVICE_MASS);
#endif
#ifdef  SUNXI_USB_DEVICE_EFEX
		sunxi_usb_module_ext(SUNXI_USB_DEVICE_EFEX);
#endif
#ifdef  SUNXI_USB_DEVICE_FASTBOOT
		sunxi_usb_module_ext(SUNXI_USB_DEVICE_FASTBOOT);
#endif
#ifdef  SUNXI_USB_DEVICE_BURN
		sunxi_usb_module_ext(SUNXI_USB_DEVICE_BURN);
#endif
#ifdef SUNXI_USB_DEVICE_EFEX_TEST
        sunxi_usb_module_ext(SUNXI_USB_DEVICE_EFEX_TEST);
#endif

#endif
