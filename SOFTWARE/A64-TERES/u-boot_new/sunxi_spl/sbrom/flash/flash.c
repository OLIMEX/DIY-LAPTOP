/*
**********************************************************************************************************************
*
*						           the Embedded Secure Bootloader System
*
*
*						       Copyright(C), 2006-2014, Allwinnertech Co., Ltd.
*                                           All Rights Reserved
*
* File    :
*
* By      :
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/

#include "common.h"
#include "mmc.h"
#include "spare_head.h"
#include "private_toc.h"

extern int sunxi_mmc_init(int sdc_no, unsigned bus_width, normal_gpio_cfg *gpio_info, int offset);
extern int load_toc1_from_nand( void );

extern sbrom_toc0_config_t *toc0_config;
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int sunxi_flash_init(int boot_type)
{
	int ret;

	if((boot_type == BOOT_FROM_SD0) || (boot_type == BOOT_FROM_SD2))
	{
		u8  *tmp_buff = (u8 *)CONFIG_TOC1_STORE_IN_DRAM_BASE;
		uint head_size;
		sbrom_toc1_head_info_t  *toc1_head;
		int  sunxi_flash_mmc_card_no;

		if(boot_type == BOOT_FROM_SD0)
		{
			sunxi_flash_mmc_card_no = 0;
		}
		else
		{
			sunxi_flash_mmc_card_no = 2;
		}
		ret = sunxi_mmc_init(sunxi_flash_mmc_card_no, 4, toc0_config->storage_gpio + 24, 8);
		if(ret <= 0)
		{
			printf("sunxi_flash_init err: sunxi_mmc_init failed\n");

			return -1;
		}
		//一次读取64k数据
		ret = mmc_bread(sunxi_flash_mmc_card_no, UBOOT_START_SECTOR_IN_SDMMC, 64, tmp_buff);
		if(!ret)
		{
			printf("PANIC : sunxi_flash_init() error --1--\n");
			return -1;
		}
		toc1_head = (struct sbrom_toc1_head_info *)tmp_buff;
		if(toc1_head->magic != TOC_MAIN_INFO_MAGIC)
		{
			printf("PANIC : sunxi_flash_init() error --2--,toc1 magic error\n");
			return -1;
		}
		head_size = toc1_head->valid_len;
		if(head_size > 64 * 512)
		{
			tmp_buff += 64*512;
			ret = mmc_bread(sunxi_flash_mmc_card_no, UBOOT_START_SECTOR_IN_SDMMC + 64, (head_size - 64*512 + 511)/512, tmp_buff);
			if(!ret)
			{
				printf("PANIC : sunxi_flash_init() error --3--\n");

				return -1;
			}
		}

		return 0;
	}
	else if(boot_type == BOOT_FROM_NFC)
	{
		if(load_toc1_from_nand())
		{
			printf("sunxi_flash_init err: nand init failed\n");

			return -1;
		}

		return 0;
	}
	else if(boot_type == BOOT_FROM_SPI)
	{
		printf("PANIC:NVM_init() : spi not support now\n");
		return -1;
	}
	else
	{
		printf("PANIC:NVM_init() : nvm_id = %d not support now\n",boot_type);

		return -1;
	}
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int sunxi_flash_read(u32 start_sector, u32 blkcnt, void *buff)
{
	memcpy(buff, (void *)(CONFIG_TOC1_STORE_IN_DRAM_BASE + 512 * start_sector), 512 * blkcnt);

	return blkcnt;
}
