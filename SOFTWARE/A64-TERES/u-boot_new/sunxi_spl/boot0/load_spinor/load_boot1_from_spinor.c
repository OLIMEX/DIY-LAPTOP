/*
 * (C) Copyright 2012
 *     wangflord@allwinnertech.com
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 */
#include "common.h"
#include "spare_head.h"
#include "private_boot0.h"
#include "private_uboot.h"
extern const boot0_file_head_t  BT0_head;

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
int load_boot1_from_spinor(void)
{
	__u32 length;
	struct spare_boot_head_t  *bfh;

	if(spinor_init())
	{
		printf("spinor init fail\n");

		return -1;
	}
	/* 载入当前块最前面512字节的数据到SRAM中，目的是获取文件头 */
	if(spinor_read(UBOOT_START_SECTOR_IN_SPINOR, 1, (void *)CONFIG_SYS_TEXT_BASE ) )
	{
		printf("the first data is error\n");

		goto __load_boot1_from_spinor_fail;
	}
	printf("Succeed in reading Boot1 file head.\n");

	/* 察看是否是文件头 */
	if( check_magic( (__u32 *)CONFIG_SYS_TEXT_BASE, UBOOT_MAGIC ) != 0 )
	{
		printf("ERROR! Add %u doesn't store head of Boot1 copy.\n", UBOOT_START_SECTOR_IN_SPINOR );

		goto __load_boot1_from_spinor_fail;
	}

	bfh = (struct spare_boot_head_t *)CONFIG_SYS_TEXT_BASE;
	length =  bfh->boot_head.length;
	printf("The size of uboot is %x.\n", length );
	if( ( length & ( 512 - 1 ) ) != 0 ) 	// length必须是NF_SECTOR_SIZE对齐的
	{
		printf("the boot1 is not aligned by %x\n", bfh->boot_head.align_size);

		goto __load_boot1_from_spinor_fail;
	}

	if(spinor_read(UBOOT_START_SECTOR_IN_SPINOR, length/512, (void *)CONFIG_SYS_TEXT_BASE ))
	{
		printf("spinor read data	error\n");

		goto __load_boot1_from_spinor_fail;
	}
	bfh->boot_data.storage_type = 3;

	return 0;

__load_boot1_from_spinor_fail:

	return -1;
}

int load_boot1(void)
{
	//memcpy((void *)DRAM_PARA_STORE_ADDR, (void *)BT0_head.prvt_head.dram_para, SUNXI_DRAM_PARA_MAX * 4);

	return load_boot1_from_spinor();
}


