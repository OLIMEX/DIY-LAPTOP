/*
 * drivers/usb/sunxi_usb/udc/sunxi_udc_board.h
 * (C) Copyright 2010-2015
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * javen, 2010-12-20, create this file
 *
 * usb board config.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#ifndef  __SUNXI_UDC_BOARD_H__
#define  __SUNXI_UDC_BOARD_H__

u32 open_usb_clock(sunxi_udc_io_t *sunxi_udc_io);
u32 close_usb_clock(sunxi_udc_io_t *sunxi_udc_io);

__s32 sunxi_udc_io_init(__u32 usbc_no, sunxi_udc_io_t *sunxi_udc_io);
__s32 sunxi_udc_io_exit(sunxi_udc_io_t *sunxi_udc_io);
__s32 sunxi_udc_bsp_init(sunxi_udc_io_t *sunxi_udc_io);
__s32 sunxi_udc_bsp_exit(sunxi_udc_io_t *sunxi_udc_io);
#endif   //__SUNXI_UDC_BOARD_H__
