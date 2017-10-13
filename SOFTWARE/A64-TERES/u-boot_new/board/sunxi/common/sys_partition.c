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
#include <sunxi_mbr.h>
#include <boot_type.h>
#include <u-boot/crc.h>


static char mbr_buf[SUNXI_MBR_SIZE];
static int  mbr_status;

DECLARE_GLOBAL_DATA_PTR;

int sunxi_partition_get_total_num(void)
{
	sunxi_mbr_t        *mbr  = (sunxi_mbr_t*)mbr_buf;
	if(!mbr_status)
	{
		return 0;
	}

	return mbr->PartCount;
}

int sunxi_partition_get_name(int index, char *buf)
{
	sunxi_mbr_t        *mbr  = (sunxi_mbr_t*)mbr_buf;

	if(mbr_status)
	{
		strncpy(buf, (const char *)mbr->array[index].name, 16);
	}
	else
	{
		memset(buf, 0, 16);
	}

	return 0;
}

uint sunxi_partition_get_offset(int part_index)
{
	sunxi_mbr_t        *mbr  = (sunxi_mbr_t*)mbr_buf;

	if((!mbr_status) || (part_index >= mbr->PartCount))
	{
		return 0;
	}

	return mbr->array[part_index].addrlo;
}

uint sunxi_partition_get_size(int part_index)
{
	sunxi_mbr_t        *mbr  = (sunxi_mbr_t*)mbr_buf;

	if((!mbr_status) || (part_index >= mbr->PartCount))
	{
		return 0;
	}

	return mbr->array[part_index].lenlo;
}

uint sunxi_partition_get_offset_byname(const char *part_name)
{
	sunxi_mbr_t        *mbr  = (sunxi_mbr_t*)mbr_buf;
	int			i;

	if(!mbr_status)
	{
		return 0;
	}
	for(i=0;i<mbr->PartCount;i++)
	{
		if(!strcmp(part_name, (const char *)mbr->array[i].name))
		{
			return mbr->array[i].addrlo;
		}
	}

	return 0;
}

int sunxi_partition_get_partno_byname(const char *part_name)
{
	sunxi_mbr_t        *mbr  = (sunxi_mbr_t*)mbr_buf;
	int			i;

	if(!mbr_status)
	{
		return -1;
	}
	for(i=0;i<mbr->PartCount;i++)
	{
		if(!strcmp(part_name, (const char *)mbr->array[i].name))
		{
			return i;
		}
	}

	return -1;
}


uint sunxi_partition_get_size_byname(const char *part_name)
{
	sunxi_mbr_t        *mbr  = (sunxi_mbr_t*)mbr_buf;
	int			i;

	if(!mbr_status)
	{
		return 0;
	}
	for(i=0;i<mbr->PartCount;i++)
	{
		if(!strcmp(part_name, (const char *)mbr->array[i].name))
		{
			return mbr->array[i].lenlo;
		}
	}

	return 0;
}

/* get the partition info, offset and size
 * input: partition name
 * output: part_offset and part_size (in byte)
 */
int sunxi_partition_get_info_byname(const char *part_name, uint *part_offset, uint *part_size)
{
	sunxi_mbr_t        *mbr  = (sunxi_mbr_t*)mbr_buf;
	int			i;

	if(!mbr_status)
	{
		return -1;
	}

	for(i=0;i<mbr->PartCount;i++)
	{
		if(!strcmp(part_name, (const char *)mbr->array[i].name))
		{
			*part_offset = mbr->array[i].addrlo;
			*part_size = mbr->array[i].lenlo;
			return 0;
		}
	}

	return -1;
}

void *sunxi_partition_fetch_mbr(void)
{
	if(!mbr_status)
	{
		return NULL;
	}

	return mbr_buf;
}

int sunxi_partition_refresh(void *buf, uint bytes)
{
	if(!mbr_status)
	{
		return -1;
	}
	if(bytes != SUNXI_MBR_SIZE)
	{
		return -1;
	}

	memcpy(mbr_buf, buf, bytes);

	return 0;
}

int sunxi_partition_init(void)
{
    sunxi_mbr_t    *mbr;

    if(!sunxi_flash_read(0, SUNXI_MBR_SIZE >> 9, mbr_buf))
	{
		printf("read flash error\n");

		return 0;
	}
	mbr = (sunxi_mbr_t*)mbr_buf;
   	if(!strncmp((const char*)mbr->magic, SUNXI_MBR_MAGIC, 8))
   	{
        int crc = 0;

        crc = crc32(0, (const unsigned char *)&mbr->version, SUNXI_MBR_SIZE-4);
        if(crc == mbr->crc32)
        {
			debug("mbr part count = %d\n", mbr->PartCount);
			mbr_status = 1;
			gd->lockflag = mbr->lockflag;

            return mbr->PartCount;
        }
   	}
	mbr_status = 0;
	debug("mbr crc error\n");

	return 0;
}
