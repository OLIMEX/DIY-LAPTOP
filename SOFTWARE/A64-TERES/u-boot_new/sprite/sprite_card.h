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
#ifndef  __SUNXI_SPRITE_CARD_H__
#define  __SUNXI_SPRITE_CARD_H__

#include <common.h>
#include <sunxi_mbr.h>

extern uint sprite_card_firmware_start(void);

extern int sprite_card_firmware_probe(char *name);

extern int sprite_card_fetch_download_map(sunxi_download_info  *dl_map);

extern int sprite_card_fetch_mbr(void  *img_mbr);

extern int sunxi_sprite_deal_part(sunxi_download_info *dl_map);

extern int sunxi_sprite_deal_uboot(int production_media);

extern int sunxi_sprite_deal_boot0(int production_media);

extern int card_download_uboot(uint length, void *buffer);

extern int card_download_boot0(uint length, void *buffer);
extern int card_upload_boot0(uint length, void *buffer);

extern int card_download_standard_mbr(void *buffer);
#ifdef CONFIG_SUNXI_SPINOR
extern int sunxi_sprite_deal_fullimg(void);
#endif


#endif

