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
#ifndef __SUNXI_FLASH_H__
#define __SUNXI_FLASH_H__
#include <common.h>

//sprite 
extern int  sunxi_sprite_init(int stage);
extern int  sunxi_sprite_erase(int erase, void *mbr_buffer);
extern int  sunxi_sprite_exit(int force);
extern uint sunxi_sprite_size(void);

extern int  sunxi_sprite_read(uint start_block,uint nblock,void * buffer);
extern int  sunxi_sprite_write(uint start_block,uint nblock,void * buffer);
extern int  sunxi_sprite_flush(void);
extern int  sunxi_sprite_phyread(unsigned int start_block, unsigned int nblock, void *buffer);
extern int  sunxi_sprite_phywrite(unsigned int start_block, unsigned int nblock, void *buffer);
extern int sunxi_sprite_force_erase(void);
extern int sunxi_sprite_mmc_phywrite(unsigned int start_block, unsigned int nblock, void *buffer);
extern int sunxi_sprite_mmc_phyread(unsigned int start_block, unsigned int nblock, void *buffer);
extern int sunxi_sprite_mmc_phyerase(unsigned int start_block, unsigned int nblock, void *skip);
extern int sunxi_sprite_mmc_phywipe(unsigned int start_block, unsigned int nblock, void *skip);
extern void board_mmc_pre_init(int card_num);


//normal 
extern int  sunxi_flash_init (int type);
extern uint sunxi_flash_size (void);
extern int  sunxi_flash_exit (int force);
extern int  sunxi_flash_read (unsigned int start_block, unsigned int nblock, void *buffer);
extern int  sunxi_flash_write(unsigned int start_block, unsigned int nblock, void *buffer);
extern int  sunxi_flash_flush(void);
extern int  sunxi_flash_phyread(unsigned int start_block, unsigned int nblock, void *buffer);
extern int  sunxi_flash_phywrite(unsigned int start_block, unsigned int nblock, void *buffer);

//video
extern uint sprite_cartoon_create(void);
extern int  sprite_cartoon_upgrade(int rate);
extern int  sprite_cartoon_destroy(void);


//other
extern int  nand_get_mbr(char* buffer, uint len);
extern int  NAND_build_all_partition(void);
extern int card_erase(int erase, void *mbr_buffer);

#ifdef CONFIG_SUNXI_SPINOR
extern int sunxi_sprite_setdata_finish(void);
#endif

extern int nand_force_download_uboot(uint length,void *buffer);
extern uint add_sum(void *buffer, uint length);


#endif  /* __SUNXI_FLASH_H__ */
