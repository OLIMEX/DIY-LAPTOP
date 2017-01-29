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
#include <boot_type.h>
#include <sunxi_board.h>
#include <sunxi_flash.h>
#include <sys_config.h>
#include <sys_partition.h>
#include <malloc.h>
#include "flash_interface.h"


int sunxi_flash_init_uboot(int verbose);
extern int sunxi_card_fill_boot0_magic(void);
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

static int
sunxi_null_op(unsigned int start_block, unsigned int nblock, void *buffer){
	return 0;
}

static int
sunxi_null_erase(int erase, void *mbr_buffer)
{
	return 0;
}

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

static int
sunxi_null_force_erase(void){
    return 0;
}

#ifdef CONFIG_SUNXI_SPINOR
static int
sunxi_null_datafinish(void){
	return 0;
}
#endif


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
int (* sunxi_flash_phywrite_pt)(unsigned int start_block, unsigned int nblock, void *buffer) = sunxi_null_op;

int (* sunxi_sprite_init_pt)(int stage) = sunxi_null_init;
int (* sunxi_sprite_read_pt) (uint start_block, uint nblock, void *buffer) = sunxi_null_op;
int (* sunxi_sprite_write_pt)(uint start_block, uint nblock, void *buffer) = sunxi_null_op;
int (* sunxi_sprite_erase_pt)(int erase, void *mbr_buffer) = sunxi_null_erase;
uint (* sunxi_sprite_size_pt)(void) = sunxi_null_size;
int (* sunxi_sprite_exit_pt) (int force) = sunxi_null_exit;
int (* sunxi_sprite_flush_pt)(void) = sunxi_null_flush;
int (* sunxi_sprite_force_erase_pt)(void)  = sunxi_null_force_erase;
int (* sunxi_sprite_phyread_pt) (unsigned int start_block, unsigned int nblock, void *buffer) = sunxi_null_op;
int (* sunxi_sprite_phywrite_pt)(unsigned int start_block, unsigned int nblock, void *buffer) = sunxi_null_op;
#ifdef CONFIG_SUNXI_SPINOR
int (* sunxi_sprite_datafinish_pt) (void) = sunxi_null_datafinish;
#endif



//-------------------------------------noraml interface start--------------------------------------------
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

int sunxi_flash_flush(void)
{
	return sunxi_flash_flush_pt();
}

int sunxi_flash_phyread (uint start_block, uint nblock, void *buffer)
{
	return sunxi_flash_phyread_pt(start_block, nblock, buffer);
}

int sunxi_flash_phywrite(uint start_block, uint nblock, void *buffer)
{
	return sunxi_flash_phywrite_pt(start_block, nblock, buffer);
}
//-----------------------------------noraml interface end---------------------------------------------------


//-------------------------------------sprite interface start-----------------------------------------------
int sunxi_sprite_init(int stage)
{
	return sunxi_sprite_init_pt(stage);
}

int sunxi_sprite_read (uint start_block, uint nblock, void *buffer)
{
	return sunxi_sprite_read_pt(start_block, nblock, buffer);
}

int sunxi_sprite_write(uint start_block, uint nblock, void *buffer)
{
	return sunxi_sprite_write_pt(start_block, nblock, buffer);
}

int sunxi_sprite_erase(int erase, void *mbr_buffer)
{
	return sunxi_sprite_erase_pt(erase, mbr_buffer);
}

uint sunxi_sprite_size(void)
{
	return sunxi_sprite_size_pt();
}

int sunxi_sprite_exit(int force)
{
    return sunxi_sprite_exit_pt(force);
}

int sunxi_sprite_flush(void)
{
	return sunxi_sprite_flush_pt();
}

int sunxi_sprite_phyread (uint start_block, uint nblock, void *buffer)
{
	return sunxi_sprite_phyread_pt(start_block, nblock, buffer);
}

int sunxi_sprite_phywrite(uint start_block, uint nblock, void *buffer)
{
	return sunxi_sprite_phywrite_pt(start_block, nblock, buffer);
}

int sunxi_sprite_force_erase(void)
{
    return sunxi_sprite_force_erase_pt();
}
//-------------------------------------sprite interface end-----------------------------------------------

//sunxi flash boot interface init 
int sunxi_flash_boot_handle(int storage_type,int workmode )
{
	int card_no;
	int state;
	switch(storage_type)
	{
		case STORAGE_NAND:
		{
			//nand handle init
			state = nand_init_for_boot(workmode);
		}
		break;
		
		case STORAGE_SD:
		case STORAGE_EMMC:
		{
			//sdmmc handle init
			card_no = (storage_type == 1)?0:2; 
			state = sdmmc_init_for_boot(workmode,card_no);
		}
		break;
		
		case STORAGE_NOR:
		default:
		{
			printf("not support\n");
			state = -1;
		}
		break;
				
	}
	
	if(state != 0)
	{
		return -1;
	}
	
	//script_parser_patch("target", "storage_type", &storage_type, 1);
	tick_printf("sunxi flash init ok\n");
	return  0;
}

int sunxi_flash_sprite_handle(int storage_type,int workmode)
{
	int state = 0;
	//try nand ,spi-nor ,mmc2
	state = nand_init_for_sprite(workmode);
	
	if(state !=0 )
	{
		printf("try nand fail\n");
		state = sdmmc_init_for_sprite(workmode);
		if(state != 0)
		{
			printf("try emmc fail\n");	
			return -1;
		}
	}

	
	//sdcard burn mode
	if((workmode == WORK_MODE_CARD_PRODUCT) || (workmode == 0x30))
	{
		state = sdmmc_init_card0_for_sprite();
		if(state != 0)
		{
			return -1;	
		}
	}
	return 0;
}

int sunxi_flash_handle_init(void)
{

	int workmode = 0;
	int storage_type = 0;
	int state = 0;
	
	workmode     = uboot_spare_head.boot_data.work_mode;
	storage_type = uboot_spare_head.boot_data.storage_type;

	printf("workmode = %d,storage type = %d\n", workmode,storage_type);

	if (workmode == WORK_MODE_BOOT || workmode == WORK_MODE_SPRITE_RECOVERY)
	{
		state = sunxi_flash_boot_handle(storage_type,workmode);
	}
	else if((workmode & WORK_MODE_PRODUCT) || (workmode == 0x30))
	{
		state = sunxi_flash_sprite_handle(storage_type,workmode);
	}
	else if(workmode & WORK_MODE_UPDATE)		/* Éý¼¶Ä£Ê½ */
	{
	}
	else   /* undefined mode */
	{
	}
	//init blk dev for fat
	sunxi_flash_init_uboot(0);

	return state;
}


//-------------------------------------------------------------------------------------------
//sunxi flash blk device for fat
//-------------------------------------------------------------------------------------------
static block_dev_desc_t 	sunxi_flash_blk_dev;

block_dev_desc_t *sunxi_flash_get_dev(int dev)
{
	sunxi_flash_blk_dev.dev = dev;
	sunxi_flash_blk_dev.lba = sunxi_partition_get_size(dev);

	return ((block_dev_desc_t *) & sunxi_flash_blk_dev);
}

unsigned long  sunxi_flash_part_read(int dev_num, unsigned long start, lbaint_t blkcnt, void *dst)
{
	uint offset;

	offset = sunxi_partition_get_offset(dev_num);
	if(!offset)
	{
		printf("sunxi flash error: cant get part %d offset\n", dev_num);

		return 0;
	}
	start += offset;
	debug("nand try to read from %x, length %x block\n", (int )start, (int )blkcnt);
	return sunxi_flash_read((uint)start, (uint )blkcnt, dst);

}

unsigned long  sunxi_flash_part_write(int dev_num, unsigned long start, lbaint_t blkcnt, const void *dst)
{
	uint offset;

	offset = sunxi_partition_get_offset(dev_num);
	if(!offset)
	{
		printf("sunxi flash error: cant get part %d offset\n", dev_num);

		return 0;
	}
	start += offset;
	debug("nand try to write from %x, length %x block\n", (int )start, (int )blkcnt);

	return sunxi_flash_write((uint)start, (uint )blkcnt, (void *)dst);

}


int sunxi_flash_init_uboot(int verbose)
{

	debug("sunxi flash init uboot\n");
	sunxi_flash_blk_dev.if_type = IF_TYPE_SUNXI_FLASH;
	sunxi_flash_blk_dev.part_type = PART_TYPE_DOS;
	sunxi_flash_blk_dev.dev = 0;
	sunxi_flash_blk_dev.lun = 0;
	sunxi_flash_blk_dev.type = 0;

	/* FIXME fill in the correct size (is set to 32MByte) */
	sunxi_flash_blk_dev.blksz = 512;
	sunxi_flash_blk_dev.lba = 0;
	sunxi_flash_blk_dev.removable = 0;
	sunxi_flash_blk_dev.block_read = sunxi_flash_part_read;
	sunxi_flash_blk_dev.block_write = sunxi_flash_part_write;

	return 0;
}


extern int sunxi_sprite_download_uboot(void *buffer, int production_media, int mode);
extern  int nand_read_uboot_data(unsigned char *buf,unsigned int len);

int sunxi_flash_update_fdt(void* fdt_buf, size_t size)
{
	int storage_type = 0;
	void* uboot_buf = NULL;
	int total_length = 0;
	int new_uboot_length = 0;
	struct spare_boot_head_t *uboot_head = NULL;
	int ret  = 0;
	int uboot_length =0;
	int dtb_offset  = 0;
	int fdt_size = size;

	total_length = uboot_spare_head.boot_head.length;
	uboot_length = uboot_spare_head.boot_head.uboot_length;
	dtb_offset   = uboot_spare_head.boot_data.dtb_offset;

	new_uboot_length =  ALIGN((total_length + fdt_size),uboot_spare_head.boot_head.align_size);
	uboot_buf = (char*)malloc(new_uboot_length);
	if(uboot_buf == NULL)
	{
		printf("%s: malloc fail\n", __func__);
		return -1;
	}

	memset(uboot_buf,0,new_uboot_length);
	uboot_head =  (struct spare_boot_head_t *)uboot_buf;

	printf("total_length = 0x%x, uboot_length = 0x%x, dtb_offset = 0x%x\n",
	total_length,uboot_length,dtb_offset);
	storage_type = get_boot_storage_type();
	if(storage_type == STORAGE_NAND)
	{
		ret = nand_read_uboot_data(uboot_buf, total_length);
	}
	else if(storage_type == STORAGE_EMMC || storage_type == STORAGE_SD)
	{
		ret = sunxi_sprite_phyread(UBOOT_START_SECTOR_IN_SDMMC, total_length/512, uboot_buf);
	}
	else if(storage_type == STORAGE_NOR)
	{
	}

	printf("read uboot from flash: ret %d\n",ret);
	if(strncmp((const char *)uboot_head->boot_head.magic, UBOOT_MAGIC, MAGIC_SIZE))
	{
		printf("%s: uboot magic is error\n",__func__);
		goto _UPDATE_END;
	}

	if(dtb_offset <  uboot_length)
	{
		printf("first time update fdt\n");
		//copy new fdt memory to the end fo uboot.fex
		memcpy(uboot_buf+total_length,fdt_buf, fdt_size);
		//update dtb_offset and uboot_len
		uboot_head->boot_data.dtb_offset = total_length;
		uboot_head->boot_head.length = new_uboot_length;
	}
	else
	{
		//just use the previous dtb_offset, and cover it
		memcpy(uboot_buf+dtb_offset,fdt_buf, fdt_size);
		//only need to update uboot total len
		//note:dtb at the end of uboot
		uboot_head->boot_head.length = ALIGN(dtb_offset+fdt_size,uboot_spare_head.boot_head.align_size);
	}

	ret = sunxi_sprite_download_uboot(uboot_buf,storage_type,1);

_UPDATE_END:
	free(uboot_buf);
	return ret;

}

