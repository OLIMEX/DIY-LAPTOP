
#include <common.h>
#include <sys_config.h>
#include <sys_partition.h>
#include <boot_type.h>
#include <sunxi_board.h>
#include <sunxi_nand.h>
#include "flash_interface.h"

static int
sunxi_flash_nand_read(uint start_block, uint nblock, void *buffer)
{
	return nand_uboot_read(start_block, nblock, buffer);

}

static int
sunxi_flash_nand_write(uint start_block, uint nblock, void *buffer)
{

	return nand_uboot_write(start_block, nblock, buffer);

}

static int
sunxi_flash_nand_erase(int erase, void *mbr_buffer)
{
	return nand_uboot_erase(erase);
}

static uint
sunxi_flash_nand_size(void)
{
	return nand_uboot_get_flash_size();
}

static int
sunxi_flash_nand_init(int stage)
{
	return nand_uboot_init(stage);
}

static int
sunxi_flash_nand_exit(int force)
{
	return nand_uboot_exit(force);
}

static int
sunxi_flash_nand_flush(void)
{
	return nand_uboot_flush();
}

static int
sunxi_flash_nand_force_erase(void)
{
    return NAND_Uboot_Force_Erase();
}


int  nand_init_for_boot(int workmode)
{
	int bootmode=0;
	
	tick_printf("NAND: ");
	bootmode = workmode == WORK_MODE_SPRITE_RECOVERY? 2:1;
	if(nand_uboot_init(bootmode))
	{
		tick_printf("nand init fail\n");
		return -1;
	}
	sunxi_flash_read_pt  = sunxi_flash_nand_read;
	sunxi_flash_write_pt = sunxi_flash_nand_write;
	sunxi_flash_size_pt  = sunxi_flash_nand_size;
	sunxi_flash_exit_pt  = sunxi_flash_nand_exit;
	sunxi_flash_flush_pt = sunxi_flash_nand_flush;
	
	sunxi_sprite_read_pt  = sunxi_flash_read_pt;
	sunxi_sprite_write_pt = sunxi_flash_write_pt;

	return 0;
}

int nand_init_for_sprite(int workmode)
{
	if(nand_uboot_probe())
	{
		return -1;
	}
	printf("nand found\n");
	sunxi_sprite_init_pt  = sunxi_flash_nand_init;
	sunxi_sprite_exit_pt  = sunxi_flash_nand_exit;
	sunxi_sprite_read_pt  = sunxi_flash_nand_read;
	sunxi_sprite_write_pt = sunxi_flash_nand_write;
	sunxi_sprite_erase_pt = sunxi_flash_nand_erase;
	sunxi_sprite_size_pt  = sunxi_flash_nand_size;
	sunxi_sprite_flush_pt = sunxi_flash_nand_flush;
	sunxi_sprite_force_erase_pt = sunxi_flash_nand_force_erase;
	debug("sunxi sprite has installed nand function\n");
	uboot_spare_head.boot_data.storage_type = 0;
	if(workmode == 0x30)
	{
		if(sunxi_sprite_init(1))
		{
			printf("nand init fail\n");
			return -1;
		}
	}
	uboot_spare_head.boot_data.storage_type = 0;
	return 0;
}

