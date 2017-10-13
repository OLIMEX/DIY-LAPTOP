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

#define  SUNXI_SPRITE_USERDATA_ADDRESS          (0x41000000)

struct  userdata_info
{
	char  *addr;
	uint   length;
};

struct  userdata_info  userdata[16];
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
int sunxi_sprite_erase_flash()
{
	int need_erase_flash;
    char buf[1024 * 1024];
    char *tmp_buf;
    sunxi_mbr  *local_mbr;
    int i;

    ret = script_parser_fetch("nand_para", "erase_flash", &need_erase_flash, 1);
    if((!ret) && (need_erase_flash))
    {
    	//check if need to protect part data
    	if(sprite_flash_init())
    	{
    		sprite_flash_erase();
    		return 0;
    	}
    	if(sprite_flash_read(0, 1024 * 1024/512, buf) == 0)
    	{
    		printf("read local mbr fail\n");
    	}
    	tmp_buf = buf + 4;
    	local_mbr = (sunxi_mbr *)(tmp_buf - 4);
    	for(i=0;i<SUNXI_MBR_MAX_COUNT;i++)
        {
        	if(crc32(0, tmp_buf, SUNXI_MBR_SIZE - 4) != local_mbr->crc32)
        	{
        		printf("bad mbr table\n");
        		tmp_buf += SUNXI_MBR_SIZE;
        	}
	        else
	        {
	        	sprite_store_part_data(local_mbr);
	            break;
	        }
	    }
	    sprite_flash_erase();
	    if(i != SUNXI_MBR_MAX_COUNT)   //this means there is no correct sunxi mbr copy, then consider the flash has no need to protect data
	    {
	    	sprite_restore_part_data();
	    }
        sprite_flash_exit();
    }

    return 0;
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
*    note          :  存储用户数据，包括所有的用户数据
*
*
************************************************************************************************************
*/
int sprite_store_part_data(local_mbr);
{
	int i, j;

	j = 0;
	memset(userdata, 0, 16 * sizoef(struct  userdata_info));
	userdata[0].addr = (uint *)SUNXI_SPRITE_USERDATA_ADDRESS;
	for(i=0;i<mbr->PartCount;i++)
	{
		if(mbr->array[i].private == 1)
		{
			//表示此分区是用户数据，需要保护起来
			sunxi_flash_read(mbr->array[i].addlo, mbr->array[i].lenlo, userdata[j].addr);
			userdata[j + 1].addr  = userdata[j].addr + userdata[j].length;
			userdata[j + 1].lenlo = mbr->array[i].lenlo;
		}
	}
}

