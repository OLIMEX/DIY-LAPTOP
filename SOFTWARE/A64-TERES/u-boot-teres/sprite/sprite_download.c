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
#include <config.h>
#include <common.h>
#include <private_toc.h>
#include <private_boot0.h>
#include <private_uboot.h>
#include <sunxi_mbr.h>
#include "sprite_verify.h"
#include "sprite_card.h"
#include <sunxi_nand.h>
#include <sunxi_flash.h>

DECLARE_GLOBAL_DATA_PTR;
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
int sunxi_sprite_download_mbr(void *buffer, uint buffer_size)
{
	int ret;

	if(buffer_size != (SUNXI_MBR_SIZE * SUNXI_MBR_COPY_NUM))
	{
		printf("the mbr size is bad\n");

		return -1;
	}
	if(sunxi_sprite_init(0))
	{
		printf("sunxi sprite init fail when downlaod mbr\n");

		return -1;
	}
	if(sunxi_sprite_write(0, buffer_size/512, buffer) == (buffer_size/512))
	{
		debug("mbr write ok\n");

		ret = 0;
	}
	else
	{
		debug("mbr write fail\n");

		ret = -1;
	}
	if(uboot_spare_head.boot_data.storage_type == 2)
	{
		printf("begin to write standard mbr\n");
		if(card_download_standard_mbr(buffer))
		{
			printf("write standard mbr err\n");

			return -1;
		}
		printf("successed to write standard mbr\n");
	}

	if(sunxi_sprite_exit(0))
	{
		printf("sunxi sprite exit fail when downlaod mbr\n");

		return -1;
	}

	return ret;
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
int sunxi_sprite_download_uboot(void *buffer, int production_media, int mode)
{
	if(!gd->securemode)
    {
    	struct spare_boot_head_t    *uboot  = (struct spare_boot_head_t *)buffer;

		//校验特征字符是否正确
		debug("%s\n", uboot->boot_head.magic);
		if(strncmp((const char *)uboot->boot_head.magic, UBOOT_MAGIC, MAGIC_SIZE))
		{
			printf("sunxi sprite: uboot magic is error\n");

			return -1;
		}
		//校验数据是否正确
		if(!mode)
		{
			if(sunxi_sprite_verify_checksum(buffer, uboot->boot_head.length, uboot->boot_head.check_sum))
			{
				printf("sunxi sprite: uboot checksum is error\n");

				return -1;
			}
			//读出dram参数
			//填充FLASH信息
			if(!production_media)
			{
				nand_uboot_get_flash_info((void *)uboot->boot_data.nand_spare_data, STORAGE_BUFFER_SIZE);
			}
		}
		/* regenerate check sum */
		uboot->boot_head.check_sum = sunxi_sprite_generate_checksum(buffer, uboot->boot_head.length, uboot->boot_head.check_sum);
		//校验数据是否正确
		if(sunxi_sprite_verify_checksum(buffer, uboot->boot_head.length, uboot->boot_head.check_sum))
		{
			printf("sunxi sprite: uboot checksum is error\n");

			return -1;
		}

		printf("uboot size = 0x%x\n", uboot->boot_head.length);
		printf("storage type = %d\n", production_media);
		if(!production_media)
		{
			debug("nand down uboot\n");
	        if(uboot_spare_head.boot_data.work_mode == WORK_MODE_BOOT)
	        {
	            printf("work_mode_boot \n");
	            return nand_force_download_uboot(uboot->boot_head.length,buffer);
	        }
	        else
	        {
			    return nand_download_uboot(uboot->boot_head.length, buffer);
	        }
	    }
		else
		{
			printf("mmc down uboot\n");
			return card_download_uboot(uboot->boot_head.length, buffer);
		}
	}
	else
	{
		sbrom_toc1_head_info_t *toc1 = (sbrom_toc1_head_info_t *)buffer;

		if(!production_media)
		{
			debug("nand down uboot\n");
			if(uboot_spare_head.boot_data.work_mode == WORK_MODE_BOOT)
			{
			    printf("work_mode_boot\n");
			    return nand_force_download_uboot(toc1->valid_len,buffer);
			}
			else
			{
				return nand_download_uboot(toc1->valid_len, buffer);
			}
	    }
		else
		{
			printf("mmc down uboot\n");
			return card_download_uboot(toc1->valid_len, buffer);
		}
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
int sunxi_sprite_download_boot0(void *buffer, int production_media)
{
	if(!gd->securemode)
    {
    	boot0_file_head_t    *boot0  = (boot0_file_head_t *)buffer;

		//校验特征字符是否正确
		debug("%s\n", boot0->boot_head.magic);
		if(strncmp((const char *)boot0->boot_head.magic, BOOT0_MAGIC, MAGIC_SIZE))
		{
			printf("sunxi sprite: boot0 magic is error\n");

			return -1;
		}
		//校验数据是否正确
		if(sunxi_sprite_verify_checksum(buffer, boot0->boot_head.length, boot0->boot_head.check_sum))
		{
			printf("sunxi sprite: boot0 checksum is error\n");

			return -1;
		}
		//读出dram参数
		//填充FLASH信息
		if(!production_media)
		{
			nand_uboot_get_flash_info((void *)boot0->prvt_head.storage_data, STORAGE_BUFFER_SIZE);
		}else{
			extern int mmc_write_info(int dev_num,void *buffer,u32 buffer_size);
			mmc_write_info(2,(void *)boot0->prvt_head.storage_data,STORAGE_BUFFER_SIZE);
		}
		{
			int i;
			uint *addr = (uint *)DRAM_PARA_STORE_ADDR;

			for(i=0;i<32;i++)
			{
				printf("dram para[%d] = %x\n", i, addr[i]);
			}
		}
		memcpy((void *)&boot0->prvt_head.dram_para, (void *)DRAM_PARA_STORE_ADDR, 32 * 4);
		/* regenerate check sum */
		boot0->boot_head.check_sum = sunxi_sprite_generate_checksum(buffer, boot0->boot_head.length, boot0->boot_head.check_sum);
		//校验数据是否正确
		if(sunxi_sprite_verify_checksum(buffer, boot0->boot_head.length, boot0->boot_head.check_sum))
		{
			printf("sunxi sprite: boot0 checksum is error\n");

			return -1;
		}
		printf("storage type = %d\n", production_media);
		if(!production_media)
		{
			return nand_download_boot0(boot0->boot_head.length, buffer);
		}
		else
		{
			return card_download_boot0(boot0->boot_head.length, buffer);
		}
	}
	else
	{
		toc0_private_head_t  *toc0   = (toc0_private_head_t *)buffer;
		sbrom_toc0_config_t  *toc0_config = (sbrom_toc0_config_t *)(buffer + 0x80);
		//校验特征字符是否正确
		debug("%s\n", (char *)toc0->name);
		if(strncmp((const char *)toc0->name, TOC0_MAGIC, MAGIC_SIZE))
		{
			printf("sunxi sprite: toc0 magic is error\n");

			return -1;
		}
		//校验数据是否正确
		if(sunxi_sprite_verify_checksum(buffer, toc0->length, toc0->check_sum))
		{
			printf("sunxi sprite: toc0 checksum is error\n");

			return -1;
		}
		//读出dram参数
		//填充FLASH信息
		if(!production_media)
		{
			nand_uboot_get_flash_info((void *)toc0_config->storage_data, STORAGE_BUFFER_SIZE);
		}else{
			extern int mmc_write_info(int dev_num,void *buffer,u32 buffer_size);
			//storage_data[384];  // 0-159,存储nand信息；160-255,存放卡信息
			mmc_write_info(2,(void *)(toc0_config->storage_data+160),384-160);
		}
		{
			int i;
			uint *addr = (uint *)DRAM_PARA_STORE_ADDR;

			for(i=0;i<32;i++)
			{
				printf("dram para[%d] = 0x%x\n", i, addr[i]);
			}
		}
		memcpy((void *)toc0_config->dram_para, (void *)DRAM_PARA_STORE_ADDR, 32 * 4);
		/* regenerate check sum */
		toc0->check_sum = sunxi_sprite_generate_checksum(buffer, toc0->length, toc0->check_sum);
		//校验数据是否正确
		if(sunxi_sprite_verify_checksum(buffer, toc0->length, toc0->check_sum))
		{
			printf("sunxi sprite: boot0 checksum is error\n");

			return -1;
		}
		printf("storage type = %d\n", production_media);
		if(!production_media)
		{
			return nand_download_boot0(toc0->length, buffer);
		}
		else
		{
			return card_download_boot0(toc0->length, buffer);
		}
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
#ifdef CONFIG_BOOT_A15
int sunxi_sprite_download_boot0_simple(void)
{
	int production_media = uboot_spare_head.boot_data.storage_type;
	u8  buffer[200 * 1024];

	if(!(uboot_spare_head.boot_data.reserved[0] & 0xff))    //不需要保存boot0/toc0
		return 0;
	if(!production_media)
		nand_upload_boot0(200 * 1024, buffer);
	else
		card_upload_boot0(200 * 1024, buffer);

	if(!gd->securemode)
    {
    	boot0_file_head_t    *boot0  = (boot0_file_head_t *)buffer;

		//校验特征字符是否正确
		debug("%s\n", boot0->boot_head.magic);
		if(strncmp((const char *)boot0->boot_head.magic, BOOT0_MAGIC, MAGIC_SIZE))
		{
			printf("sunxi sprite: boot0 magic is error\n");

			return -1;
		}
		//校验数据是否正确
		if(sunxi_sprite_verify_checksum(buffer, boot0->boot_head.length, boot0->boot_head.check_sum))
		{
			printf("sunxi sprite: boot0 checksum is error\n");

			return -1;
		}
		boot0->boot_head.boot_cpu = uboot_spare_head.boot_data.reserved[0] & 0xff00;
		printf("next boot_cpu=0x%x\n", boot0->boot_head.boot_cpu);
		/* regenerate check sum */
		boot0->boot_head.check_sum = sunxi_sprite_generate_checksum(buffer, boot0->boot_head.length, boot0->boot_head.check_sum);
		//校验数据是否正确
		if(sunxi_sprite_verify_checksum(buffer, boot0->boot_head.length, boot0->boot_head.check_sum))
		{
			printf("sunxi sprite: boot0 checksum is error\n");

			return -1;
		}
		printf("storage type = %d\n", production_media);
		if(!production_media)
		{
			return nand_download_boot0_simple(boot0->boot_head.length, buffer);
		}
		else
		{
			return card_download_boot0(boot0->boot_head.length, buffer);
		}
	}
	else
	{
		toc0_private_head_t  *toc0   = (toc0_private_head_t *)buffer;
		sbrom_toc0_config_t  *toc0_config = (sbrom_toc0_config_t *)(buffer + 0x80);
		//校验特征字符是否正确
		debug("%s\n", (char *)toc0->name);
		if(strncmp((const char *)toc0->name, TOC0_MAGIC, MAGIC_SIZE))
		{
			printf("sunxi sprite: boot0 magic is error\n");

			return -1;
		}
		//校验数据是否正确
		if(sunxi_sprite_verify_checksum(buffer, toc0->length, toc0->check_sum))
		{
			printf("sunxi sprite: toc0 verify error\n");

			return -1;
		}
		toc0_config->boot_cpu = uboot_spare_head.boot_data.reserved[0] & 0xff00;
		printf("next boot_cpu=0x%x\n", toc0_config->boot_cpu);
		toc0->check_sum = sunxi_sprite_generate_checksum(buffer, toc0->length, toc0->check_sum);
		//校验数据是否正确
		if(sunxi_sprite_verify_checksum(buffer, toc0->length, toc0->check_sum))
		{
			printf("sunxi sprite: boot0 checksum is error\n");

			return -1;
		}
		printf("storage type = %d\n", production_media);
		if(!production_media)
		{
			return nand_download_boot0_simple(toc0->length, buffer);
		}
		else
		{
			return card_download_boot0(toc0->length, buffer);
		}
	}
}
#endif

