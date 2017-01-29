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
#include <malloc.h>
#include <sunxi_mbr.h>

#define  SUNXI_SPRITE_PROTECT_DATA_MAX    (16)
#define  SUNXI_SPRITE_PROTECT_PART        "private"

struct private_part_info
{
	uint part_sectors;
	char *part_buf;
	char part_name[32];
};

struct private_part_info  part_info[SUNXI_SPRITE_PROTECT_DATA_MAX];
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
int sunxi_sprite_store_part_data(void *buffer)
{
	int i, j;

	j = 0;
	sunxi_mbr_t  *mbr = (sunxi_mbr_t *)buffer;
	for(i=0;i<mbr->PartCount;i++)
	{
		printf("part name %s\n", mbr->array[i].name);
		printf("keydata = 0x%x\n", mbr->array[i].keydata);
		if( (!strcmp((const char *)mbr->array[i].name, SUNXI_SPRITE_PROTECT_PART)) || (mbr->array[i].keydata == 0x8000))
		{
			printf("find keypart %s\n", mbr->array[i].name);
			printf("keypart read start: 0x%x, sectors 0x%x\n", mbr->array[i].addrlo, mbr->array[i].lenlo);

			part_info[j].part_buf = (char *)malloc(mbr->array[i].lenlo * 512);
			if(!part_info[j].part_buf)
			{
				printf("sprite protect private data fail: cant malloc memory for part %s, sectors 0x%x\n", mbr->array[i].name, mbr->array[i].lenlo);

				goto __sunxi_sprite_store_part_data_fail;
			}
			if(!sunxi_sprite_read(mbr->array[i].addrlo, mbr->array[i].lenlo, (void *)part_info[j].part_buf))
			{
				printf("sunxi sprite error : read private data error\n");

				goto __sunxi_sprite_store_part_data_fail;
			}
			printf("keypart part %s read end: 0x%x, sectors 0x%x\n", mbr->array[i].name, mbr->array[i].addrlo, mbr->array[i].lenlo);

			part_info[j].part_sectors = mbr->array[i].lenlo;
			strcpy(part_info[j].part_name, (const char *)mbr->array[i].name);

			j ++;
		}
	}
	if(!j)
	{
		printf("there is no keypart part on local flash\n");
	}

	return 0;

__sunxi_sprite_store_part_data_fail:
	for(i=0;i<j;i++)
	{
		if(part_info[i].part_buf)
		{
			free(part_info[i].part_buf);
		}
		else
		{
			break;
		}
	}

	return -1;
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
int sunxi_sprite_restore_part_data(void *buffer)
{
	int i, j;
	int ret = -1;
	uint      down_sectors;
	sunxi_mbr_t  *mbr = (sunxi_mbr_t *)buffer;

	j = 0;
	while(part_info[j].part_buf)
	{
		for(i=0;i<mbr->PartCount;i++)
		{
			if(!strcmp(part_info[j].part_name, (const char*)mbr->array[i].name))
			{
				if(part_info[j].part_sectors > mbr->array[i].lenlo)
				{
					printf("origin sectors 0x%x, new part sectors 0x%x\n", part_info[j].part_sectors, mbr->array[i].lenlo);
					printf("fix it, only store less sectors\n");

					down_sectors = mbr->array[i].lenlo;
				}
				else
				{
					down_sectors = part_info[j].part_sectors;
				}

				printf("keypart write start: 0x%x, sectors 0x%x\n", mbr->array[i].addrlo, down_sectors);
				if(!sunxi_sprite_write(mbr->array[i].addrlo, down_sectors, (void *)part_info[j].part_buf))
				{
					printf("sunxi sprite error : write private data error\n");

					goto __sunxi_sprite_restore_part_data_fail;
				}

				printf("keypart write end: 0x%x, sectors 0x%x\n", mbr->array[i].addrlo, down_sectors);

				break;
			}
		}
		j ++;
	}
	if(!j)
	{
		printf("there is no private part need rewrite\n");
	}
	ret = 0;

__sunxi_sprite_restore_part_data_fail:
	for(i=0;i<j;i++)
	{
		if(part_info[i].part_buf)
		{
			free(part_info[i].part_buf);
		}
		else
		{
			break;
		}
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
int sunxi_sprite_probe_prvt(void  *buffer)
{
	int i;

	sunxi_mbr_t  *mbr = (sunxi_mbr_t *)buffer;
	for(i=0;i<mbr->PartCount;i++)
	{
		if( (!strcmp((const char *)mbr->array[i].name, SUNXI_SPRITE_PROTECT_PART)) || (mbr->array[i].keydata == 0x8000))
		{
			printf("private part exist\n");

			return 1;
		}
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
*    note          :
*
*
************************************************************************************************************
*/
int sunxi_sprite_erase_private_key(void *buffer)
{
	int count = 0;
	int flash_start = 0 , flash_sectors = 0;
	int i = 0 , len = 1024 * 1024;
	sunxi_mbr_t  *mbr = (sunxi_mbr_t *)buffer;
	char *fill_zero = NULL;
	
	for(i=0;i<mbr->PartCount;i++)
	{
		if( (!strcmp((const char *)mbr->array[i].name, SUNXI_SPRITE_PROTECT_PART)) || (mbr->array[i].keydata == 0x8000))
		{
			printf("private part exist\n");
			count = mbr->array[i].lenlo / 2048;
			flash_start = mbr->array[i].addrlo;
			break;
		}
	}
	
	if(i >= mbr->PartCount)
	{
		printf("private part is not exit \n");
		return -1;
	}
	
	fill_zero = (char *)malloc(len);
	if(fill_zero == NULL)
	{
		printf("no enough memory to malloc \n");
		return -1;
	}
	
	memset(fill_zero , 0x0, len);
	flash_sectors = len / 512;
	for(i = 0; i < count ; i++)
	{
		if(!sunxi_sprite_write(flash_start + i * flash_sectors, flash_sectors, (void *)fill_zero))
		{
			printf("sunxi_sprite_erase_private_key err: write flash from 0x%x, sectors 0x%x failed\n", flash_start + i * flash_sectors, flash_sectors);
			return -1;
		}

	}
	printf("erase key successed \n");
	return 0;
}
