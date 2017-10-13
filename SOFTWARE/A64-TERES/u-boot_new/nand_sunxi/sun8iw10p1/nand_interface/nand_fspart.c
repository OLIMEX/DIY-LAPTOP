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
#include <sunxi_mbr.h>
#include <fastboot.h>
#include <sys_partition.h>

static block_dev_desc_t 	nand_blk_dev;

uint nand_uboot_read(uint start, uint sectors, void *buffer);
uint nand_uboot_write(uint start, uint sectors, void *buffer);

block_dev_desc_t *nand_get_dev(int dev)
{
	nand_blk_dev.dev = dev;
	nand_blk_dev.lba = sunxi_partition_get_size(dev);

	return ((block_dev_desc_t *) & nand_blk_dev);
}

unsigned long  nand_read_uboot(int dev_num, unsigned long start, lbaint_t blkcnt, void *dst)
{
	start += sunxi_partition_get_offset(dev_num);
#ifdef DEBUG
	printf("nand try to read from %lx, length %lx block\n", start, blkcnt);
#endif
	return nand_uboot_read((uint)start, (uint)blkcnt, dst);
}

unsigned long  nand_write_uboot(int dev_num, unsigned long start, lbaint_t blkcnt, const void *dst)
{
	start += sunxi_partition_get_offset(dev_num);
#ifdef DEBUG
	printf("nand try to write from %lx, length %lx block\n", start, blkcnt);
#endif
	return nand_uboot_write((uint)start, (uint)blkcnt, (void *)dst);
}

int nand_erase_uboot(char *dev_part)
{
#if 0
	int start;
	int size;
	char *tmp_buf;
	int total_size;
	int pre_ratio, this_ratio;
	int dev_num;
	char partname[12];

	if((*dev_part >= '0') && (*dev_part <= '9'))
	{
		dev_num = simple_strtoul(dev_part, 0, 10);
		start =  sunxi_partition_get_offset(dev_num);
		size  =  sunxi_partition_get_size(dev_num);
		sunxi_partition_get_name(dev_num, &partname[0]);
	}
	else
	{
		start =  sunxi_partition_get_offset_byname(dev_part);
		size  =  sunxi_partition_get_size_byname(dev_part);
		strncpy(partname, dev_part, 12);
	}

	if(start < 0)
	{
		printf("error: unknown part name\n");
		return -1;
	}
	if(!size)
	{
		printf("error: part size is zero\n");
		return -1;
	}
	printf("part name = %s\n", partname);
	printf("part size = %d\n", size);
	pre_ratio  = 0;
	this_ratio = 0;
	tmp_buf = malloc(32 * 1024);
	memset(tmp_buf, 0xff, 32 * 1024);
	printf("erase this part:    00%%");
	total_size = size;
	while(size >= ((32 * 1024)>>9))
	{
		NAND_LogicWrite(start, (32 * 1024)>>9, tmp_buf);
		start += (32 * 1024)>>9;
		size  -= (32 * 1024)>>9;
		this_ratio = 100 - (size * 100)/total_size;
		if(pre_ratio != this_ratio)
		{
			printf("\b\b\b%2d%%", this_ratio);
		}
		pre_ratio = this_ratio;
	}
	if(size)
	{
		NAND_LogicWrite(start, size, tmp_buf);
	}
	printf("\b\b\b\b%3d%%", 100);
	puts("\n");
	free(tmp_buf);
	LML_FlushPageCache();
#endif
	return 0;
}


int nand_init_uboot(int verbose)
{
	nand_blk_dev.if_type = IF_TYPE_NAND;
	nand_blk_dev.part_type = PART_TYPE_DOS;
	nand_blk_dev.dev = 0;
	nand_blk_dev.lun = 0;
	nand_blk_dev.type = 0;

	/* FIXME fill in the correct size (is set to 32MByte) */
	nand_blk_dev.blksz = 512;
	nand_blk_dev.lba = 0;
	nand_blk_dev.removable = 0;
	nand_blk_dev.block_read = nand_read_uboot;
	nand_blk_dev.block_write = nand_write_uboot;

	//fat_register_device(&nand_blk_dev, 1);
	return 0;
}



