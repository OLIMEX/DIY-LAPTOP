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
#ifndef  __USB_DMA_CONTROL_H__
#define  __USB_DMA_CONTROL_H__

#include <common.h>

extern int usb_dma_init(uint husb);

extern uint usb_dma_request(void);

extern int usb_dma_release(uint dma_index);

extern int usb_dma_setting(uint dma_index, uint trans_dir, uint ep);

extern int usb_dma_set_pktlen(uint dma_index, uint pkt_len);

extern int usb_dma_start(uint dma_index, uint addr, uint bytes);

extern int usb_dma_stop(uint dma_index);

extern int usb_dma_int_query(void);

extern int usb_dma_int_clear(void);

#endif
