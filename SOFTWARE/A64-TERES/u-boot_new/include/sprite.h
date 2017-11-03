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

#ifndef __SPRITE_SYS_H
#define __SPRITE_SYS_H

extern int sunxi_card_probe_mmc0_boot(void);
extern int sunxi_card_sprite_main(int workmode, char *name);

extern int sunxi_sprite_download_mbr(void *buffer, uint buffer_size);
extern int sunxi_sprite_download_uboot(void *buffer, int production_media, int mode);
extern int sunxi_sprite_download_boot0(void *buffer, int production_media);

extern int sunxi_sprite_erase_flash(void  *img_mbr_buffer);
extern int sunxi_sprite_force_erase_key(void);

extern uint add_sum(void *buffer, uint length);
extern int sunxi_sprite_verify_mbr(void *buffer);
extern uint sunxi_sprite_part_rawdata_verify(uint base_start, long long base_bytes);

extern int sprite_form_sysrecovery(void);
extern int sprite_led_init(void);
extern int sprite_led_exit(int status);

#endif /* __SPRITE_SYS_H */
