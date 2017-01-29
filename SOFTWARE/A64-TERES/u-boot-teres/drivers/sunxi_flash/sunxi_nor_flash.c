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
#include <common.h>
#include <mmc.h>
#include <sunxi_nand.h>
#include <boot_type.h>
#include "sunxi_flash.h"
#include "sys_config.h"
#include "sys_partition.h"
#ifdef CONFIG_SUNXI_SPINOR
#include <asm/arch/spinor.h>
#endif
static int
sunxi_null_op(unsigned int start_block, unsigned int nblock, void *buffer){
	return 0;
}

//static int
//sunxi_null_erase(int erase, void *mbr_buffer)
//{
//	return 0;
//}

static uint
sunxi_null_size(void){
	return 0;
}

static int
sunxi_null_init(int flag){
	return -1;
}

static int
sunxi_null_exit(int force){
	return -1;
}

static int
sunxi_null_flush(void){
	return 0;
}

//static int
//sunxi_null_datafinish(void){
//	return 0;
//}

/*
************************************************************************************************************
*
*                                             function
*
*
*
*
*
*
*
*
************************************************************************************************************
*/

int (* sunxi_flash_init_pt)(int stage) = sunxi_null_init;
int (* sunxi_flash_read_pt) (uint start_block, uint nblock, void *buffer) = sunxi_null_op;
//int (* sunxi_flash_read_sequence) (uint start_block, uint nblock, void *buffer) = sunxi_null_op;
int (* sunxi_flash_write_pt)(uint start_block, uint nblock, void *buffer) = sunxi_null_op;
uint (* sunxi_flash_size_pt)(void) = sunxi_null_size;
int (* sunxi_flash_exit_pt) (int force) = sunxi_null_exit;
int (* sunxi_flash_flush_pt) (void) = sunxi_null_flush;
int (* sunxi_flash_phyread_pt) (unsigned int start_block, unsigned int nblock, void *buffer) = sunxi_null_op;

/*
************************************************************************************************************
*
*                                             function
*
*
*
*
*
*
*
*
************************************************************************************************************
*/
int sunxi_flash_phyread (uint start_block, uint nblock, void *buffer)
{
	return sunxi_flash_phyread_pt(start_block, nblock, buffer);
}


static int
sunxi_flash_spinor_read(unsigned int start_block, unsigned int nblock, void *buffer)
{
	debug("spinor read: start 0x%x, sector 0x%x\n", start_block, nblock);

    return spinor_read(start_block + CONFIG_SPINOR_LOGICAL_OFFSET, nblock, buffer);
}

static int
sunxi_flash_spinor_write(unsigned int start_block, unsigned int nblock, void *buffer)
{
	debug("spinor write: start 0x%x, sector 0x%x\n", start_block, nblock);

	return spinor_write(start_block + CONFIG_SPINOR_LOGICAL_OFFSET, nblock, buffer);
}


static uint
sunxi_flash_spinor_size(void){

	return spinor_size();
}

//static int sunxi_flash_spinor_erase(int erase,void *img_buf)
//{
//	return spinor_erase_all_blocks(erase);
//}

//static int
//sunxi_flash_spinor_init(int stage)
//{
//	return spinor_init(stage);
//}

static int
sunxi_flash_spinor_exit(int force)
{
	return spinor_exit(force);
}

//static int
//sunxi_flash_spinor_datafinish(void)
//{
//	return spinor_datafinish();
//}

static int
sunxi_flash_spinor_flush(void)
{
	return spinor_flush_cache();
}

/*
************************************************************************************************************
*
*											  function
*
*
*
*
*
*
*
*
************************************************************************************************************
*/

int sunxi_flash_read (uint start_block, uint nblock, void *buffer)
{
	debug("sunxi flash read : start %d, sector %d\n", start_block, nblock);
	return sunxi_flash_read_pt(start_block, nblock, buffer);
}

int sunxi_flash_write(uint start_block, uint nblock, void *buffer)
{
	debug("sunxi flash write : start %d, sector %d\n", start_block, nblock);
	return sunxi_flash_write_pt(start_block, nblock, buffer);
}

uint sunxi_flash_size(void)
{
	return sunxi_flash_size_pt();
}

int sunxi_flash_exit(int force)
{
    return sunxi_flash_exit_pt(force);
}

int
sunxi_flash_flush(void)
{
	return sunxi_flash_flush_pt();
}

int sunxi_sprite_exit(int force)
{
    return 0;
}
/*
************************************************************************************************************
*
*											  function
*
*
*
*
*
*
*
*
************************************************************************************************************
*/
int sunxi_flash_handle_init(void)
{
    int workmode;

    workmode = uboot_spare_head.boot_data.work_mode;
#if 1	
    printf("workmode = %d\n", workmode);
    debug("storage type = %d\n", uboot_spare_head.boot_data.storage_type);
#endif

	if(workmode == WORK_MODE_BOOT)
	{
		//sunxi_flash_init_pt  = sunxi_flash_spinor_init;
        sunxi_flash_exit_pt  = sunxi_flash_spinor_exit;
		sunxi_flash_read_pt  = sunxi_flash_spinor_read;
		sunxi_flash_write_pt = sunxi_flash_spinor_write;
		sunxi_flash_size_pt  = sunxi_flash_spinor_size;
		sunxi_flash_flush_pt = sunxi_flash_spinor_flush;

		spinor_init(0);
		tick_printf("sunxi flash init ok\n");
	}
	else if(workmode & WORK_MODE_UPDATE)		/* Éý¼¶Ä£Ê½ */
	{
	}
	else   /* undefined mode */
	{
	}

	return 0;
}
