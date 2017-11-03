#include <common.h>
#include <sys_config.h>
#include <sys_partition.h>
#include <boot_type.h>
#include <sunxi_board.h>
#include <mmc.h>
#include "flash_interface.h"

static struct mmc *mmc_boot,*mmc_sprite;

//-------------------------------------noraml interface--------------------------------------------
static int
sunxi_flash_mmc_read(unsigned int start_block, unsigned int nblock, void *buffer)
{
	debug("mmcboot read: start 0x%x, sector 0x%x\n", start_block, nblock);

	return mmc_boot->block_dev.block_read(mmc_boot->block_dev.dev, start_block + CONFIG_MMC_LOGICAL_OFFSET,
					nblock, buffer);
}


static int
sunxi_flash_mmc_write(unsigned int start_block, unsigned int nblock, void *buffer)
{
	debug("mmcboot write: start 0x%x, sector 0x%x\n", start_block, nblock);

	return mmc_boot->block_dev.block_write(mmc_boot->block_dev.dev, start_block + CONFIG_MMC_LOGICAL_OFFSET,
					nblock, buffer);
}

static uint
sunxi_flash_mmc_size(void){

	return mmc_boot->block_dev.lba;
}

static int
sunxi_flash_mmc_init(int stage){
	return 0;
}

static int
sunxi_flash_mmc_exit(int force){

	return mmc_exit();
	//return 0;
}

int sunxi_flash_mmc_phyread(unsigned int start_block, unsigned int nblock, void *buffer)
{
	return mmc_boot->block_dev.block_read_mass_pro(mmc_boot->block_dev.dev, start_block, nblock, buffer);
}

int sunxi_flash_mmc_phywrite(unsigned int start_block, unsigned int nblock, void *buffer)
{
	return mmc_boot->block_dev.block_write_mass_pro(mmc_boot->block_dev.dev, start_block, nblock, buffer);
}


//-------------------------------------sprite interface--------------------------------------------
static int
sunxi_sprite_mmc_read(unsigned int start_block, unsigned int nblock, void *buffer)
{
	debug("mmcsprite read: start 0x%x, sector 0x%x\n", start_block, nblock);

	return mmc_sprite->block_dev.block_read(mmc_sprite->block_dev.dev, start_block + CONFIG_MMC_LOGICAL_OFFSET,
					nblock, buffer);
}

static int
sunxi_sprite_mmc_write(unsigned int start_block, unsigned int nblock, void *buffer)
{
	debug("mmcsprite write: start 0x%x, sector 0x%x\n", start_block, nblock);

	return mmc_sprite->block_dev.block_write(mmc_sprite->block_dev.dev, start_block + CONFIG_MMC_LOGICAL_OFFSET,
					nblock, buffer);
}

static int
sunxi_sprite_mmc_erase(int erase, void *mbr_buffer)
{
	return card_erase(erase, mbr_buffer);
}

static uint
sunxi_sprite_mmc_size(void){

	return mmc_sprite->block_dev.lba;
}

static int
sunxi_sprite_mmc_init(int stage){
	return 0;
}

static int
sunxi_sprite_mmc_exit(int force){
	return 0;
}


int sunxi_sprite_mmc_phyread(unsigned int start_block, unsigned int nblock, void *buffer)
{
	return mmc_sprite->block_dev.block_read_mass_pro(mmc_sprite->block_dev.dev, start_block, nblock, buffer);
}

int sunxi_sprite_mmc_phywrite(unsigned int start_block, unsigned int nblock, void *buffer)
{
	return mmc_sprite->block_dev.block_write_mass_pro(mmc_sprite->block_dev.dev, start_block, nblock, buffer);
}

int sunxi_sprite_mmc_phyerase(unsigned int start_block, unsigned int nblock, void *skip)
{
	if (nblock == 0) {
		printf("%s: @nr is 0, erase from @from to end\n", __FUNCTION__);
		nblock = mmc_sprite->block_dev.lba - start_block - 1;
	}
	return mmc_sprite->block_dev.block_mmc_erase(mmc_sprite->block_dev.dev, start_block, nblock, skip);
}

int sunxi_sprite_mmc_phywipe(unsigned int start_block, unsigned int nblock, void *skip)
{
	if (nblock == 0) {
		printf("%s: @nr is 0, wipe from @from to end\n", __FUNCTION__);
		nblock = mmc_sprite->block_dev.lba - start_block - 1;
	}
	return mmc_sprite->block_dev.block_secure_wipe(mmc_sprite->block_dev.dev, start_block, nblock, skip);
}

int sunxi_sprite_mmc_force_erase(void)
{
    return 0;
}

//-----------------------------secure interface---------------------------------------
int sunxi_flash_mmc_secread( int item, unsigned char *buf, unsigned int nblock)
{
	return mmc_boot->block_dev.block_read_secure(2, item, (u8 *)buf, nblock);
}

int sunxi_flash_mmc_secwrite( int item, unsigned char *buf, unsigned int nblock)
{
	return mmc_boot->block_dev.block_write_secure(2, item, (u8 *)buf, nblock);
}

int sunxi_sprite_mmc_secwrite(int item ,unsigned char *buf,unsigned int nblock)
{
	if(mmc_sprite->block_dev.block_write_secure(2, item, (u8 *)buf, nblock) >=0)
        return 0;
    else
        return -1;
}

int sunxi_sprite_mmc_secread(int item ,unsigned char *buf,unsigned int nblock)
{
    if(mmc_sprite->block_dev.block_read_secure(2, item, (u8 *)buf, nblock) >=0)
        return 0;
    else
        return -1;
}
//-----------------------------------end ------------------------------------------------------

int sdmmc_init_for_boot(int workmode, int card_no)
{
	
	tick_printf("MMC:	 %d\n", card_no);
	
	board_mmc_set_num(card_no);
	debug("set card number\n");
	board_mmc_pre_init(card_no);
	debug("begin to find mmc\n");
	mmc_boot = find_mmc_device(card_no);
	if(!mmc_boot){
		printf("fail to find one useful mmc card\n");
		return -1;
	}
	debug("try to init mmc\n");
	if (mmc_init(mmc_boot)) {
		puts("MMC init failed\n");
		return  -1;
	}
	debug("mmc %d init ok\n", card_no);
	sunxi_flash_init_pt  = sunxi_flash_mmc_init;
	sunxi_flash_read_pt  = sunxi_flash_mmc_read;
	sunxi_flash_write_pt = sunxi_flash_mmc_write;
	sunxi_flash_size_pt  = sunxi_flash_mmc_size;
	sunxi_flash_exit_pt  = sunxi_flash_mmc_exit;
	sunxi_flash_phyread_pt  = sunxi_flash_mmc_phyread;
	sunxi_flash_phywrite_pt = sunxi_flash_mmc_phywrite;
	
	//for fastboot
	sunxi_sprite_phyread_pt  = sunxi_flash_mmc_phyread;
	sunxi_sprite_phywrite_pt = sunxi_flash_mmc_phywrite;
	sunxi_sprite_read_pt  = sunxi_flash_read_pt;
	sunxi_sprite_write_pt = sunxi_flash_write_pt;

	return 0;
	
}

int sdmmc_init_for_sprite(int workmode)
{
	printf("try nand fail\n");
	board_mmc_pre_init(2);
	mmc_sprite = find_mmc_device(2);
	if(!mmc_sprite){
		printf("fail to find one useful mmc card\n");
		return -1;
	}
	
	if (mmc_init(mmc_sprite)) {
		printf("MMC init failed\n");
		return  -1;
	}
	sunxi_sprite_init_pt  = sunxi_sprite_mmc_init;
	sunxi_sprite_exit_pt  = sunxi_sprite_mmc_exit;
	sunxi_sprite_read_pt  = sunxi_sprite_mmc_read;
	sunxi_sprite_write_pt = sunxi_sprite_mmc_write;
	sunxi_sprite_erase_pt = sunxi_sprite_mmc_erase;
	sunxi_sprite_size_pt  = sunxi_sprite_mmc_size;
	sunxi_sprite_phyread_pt  = sunxi_sprite_mmc_phyread;
	sunxi_sprite_phywrite_pt = sunxi_sprite_mmc_phywrite;
	sunxi_sprite_force_erase_pt = sunxi_sprite_mmc_force_erase;
	debug("sunxi sprite has installed sdcard2 function\n");
	uboot_spare_head.boot_data.storage_type = 2;
	
	return 0;
}

int sdmmc_init_card0_for_sprite(void)
{
	//init card0
	board_mmc_pre_init(0);
	mmc_boot = find_mmc_device(0);
	if(!mmc_boot)
	{
		printf("fail to find one useful mmc card\n");
		return -1;
	}
	
	if (mmc_init(mmc_boot))
	{
		printf("MMC sprite init failed\n");
		return  -1;
	}
	else
	{
		printf("mmc init ok\n");
	}
	
	sunxi_flash_init_pt  = sunxi_flash_mmc_init;
	sunxi_flash_read_pt  = sunxi_flash_mmc_read;
	sunxi_flash_write_pt = sunxi_flash_mmc_write;
	sunxi_flash_size_pt  = sunxi_flash_mmc_size;
	sunxi_flash_phyread_pt  = sunxi_flash_mmc_phyread;
	sunxi_flash_phywrite_pt = sunxi_flash_mmc_phywrite;
	sunxi_flash_exit_pt  = sunxi_flash_mmc_exit;
	
	return 0;
}