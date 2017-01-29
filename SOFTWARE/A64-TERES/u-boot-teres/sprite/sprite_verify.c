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
#include "sparse/sparse.h"
#include <sunxi_mbr.h>
#include <sunxi_board.h>

#define  VERIFY_ONCE_BYTES    (16 * 1024 * 1024)
#define  VERIFY_ONCE_SECTORS  (VERIFY_ONCE_BYTES/512)
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
uint add_sum(void *buffer, uint length)
{
#ifndef CONFIG_USE_NEON_SIMD
	unsigned int *buf;
	unsigned int count;
	unsigned int sum;

	count = length >> 2;                         // 以 字（4bytes）为单位计数
	sum = 0;
	buf = (unsigned int *)buffer;
	while(count--)
	{
		sum += *buf++;                         // 依次累加，求得校验和
	};

	switch(length & 0x03)
	{
		case 0:
			return sum;
		case 1:
			sum += (*buf & 0x000000ff);
			break;
		case 2:
			sum += (*buf & 0x0000ffff);
			break;
		case 3:
			sum += (*buf & 0x00ffffff);
			break;
	}
#else
	uint sum;

	sum = add_sum_neon(buffer, length);
#endif

	return sum;
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
uint sunxi_sprite_part_rawdata_verify(uint base_start, long long base_bytes)
{
	uint checksum = 0;
	uint unaligned_bytes, last_time_bytes;
	uint rest_sectors;
	uint crt_start;
	char *tmp_buf = NULL;

	tmp_buf = (char *)malloc(VERIFY_ONCE_BYTES);
	if(!tmp_buf)
	{
		printf("sunxi sprite err: unable to malloc memory for verify\n");

		return 0;
	}
	crt_start       = base_start;
	rest_sectors    = (uint)((base_bytes + 511)>>9);
	unaligned_bytes = (uint)base_bytes & 0x1ff;

	debug("read total sectors %d\n", rest_sectors);
	debug("read part start %d\n", crt_start);
    while(rest_sectors >= VERIFY_ONCE_SECTORS)
    {
    	if(sunxi_sprite_read(crt_start, VERIFY_ONCE_SECTORS, tmp_buf) != VERIFY_ONCE_SECTORS)
    	{
    		printf("sunxi sprite: read flash error when verify\n");
			checksum = 0;

    		goto __rawdata_verify_err;
    	}
    	crt_start    += VERIFY_ONCE_SECTORS;
    	rest_sectors -= VERIFY_ONCE_SECTORS;
    	checksum     += add_sum(tmp_buf, VERIFY_ONCE_BYTES);
    	debug("check sum = 0x%x\n", checksum);
    }
    if(rest_sectors)
    {
    	if(sunxi_sprite_read(crt_start, rest_sectors, tmp_buf) != rest_sectors)
    	{
    		printf("sunxi sprite: read flash error when verify\n");
			checksum = 0;

    		goto __rawdata_verify_err;
    	}
    	if(unaligned_bytes)
    	{
    		last_time_bytes = (rest_sectors - 1) * 512 + unaligned_bytes;
    	}
    	else
    	{
    		last_time_bytes = rest_sectors * 512;
    	}
    	checksum += add_sum(tmp_buf, last_time_bytes);
		debug("check sum = 0x%x\n", checksum);
    }

__rawdata_verify_err:
	if(tmp_buf)
	{
		free(tmp_buf);
	}

	return checksum;

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
uint sunxi_sprite_part_sparsedata_verify(void)
{
	return unsparse_checksum();
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
uint sunxi_sprite_generate_checksum(void *buffer, uint length, uint src_sum)
{
#ifndef CONFIG_USE_NEON_SIMD
	uint *buf;
	uint count;
	uint sum;

	/* 生成校验和 */
	count = length >> 2;                       // 以 字（4bytes）为单位计数
	sum = 0;
	buf = (__u32 *)buffer;
	do
	{
		sum += *buf++;                         // 依次累加，求得校验和
		sum += *buf++;                         // 依次累加，求得校验和
		sum += *buf++;                         // 依次累加，求得校验和
		sum += *buf++;                         // 依次累加，求得校验和
	}while( ( count -= 4 ) > (4-1) );

	while( count-- > 0 )
		sum += *buf++;
#else
	uint sum;

	sum = add_sum_neon(buffer, length);
#endif
	sum = sum - src_sum + STAMP_VALUE;

    return sum;
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
int sunxi_sprite_verify_checksum(void *buffer, uint length, uint src_sum)
{
#ifndef CONFIG_USE_NEON_SIMD
	uint *buf;
	uint count;
	uint sum;

	/* 生成校验和 */
	count = length >> 2;                       // 以 字（4bytes）为单位计数
	sum = 0;
	buf = (__u32 *)buffer;
	do
	{
		sum += *buf++;                         // 依次累加，求得校验和
		sum += *buf++;                         // 依次累加，求得校验和
		sum += *buf++;                         // 依次累加，求得校验和
		sum += *buf++;                         // 依次累加，求得校验和
	}while( ( count -= 4 ) > (4-1) );

	while( count-- > 0 )
		sum += *buf++;
#else
	uint sum;

	sum = add_sum_neon(buffer, length);
#endif
	sum = sum - src_sum + STAMP_VALUE;

	debug("src sum=%x, check sum=%x\n", src_sum, sum);
	if( sum == src_sum )
		return 0;               // 校验成功
	else
		return -1;              // 校验失败
}
/*
***************************************************************
*                       __mbr_map_dump
*
* Description:
*		check the input para
* Parameters:
*
* Return value:
*
* History:
*
***************************************************************
*/
static void __mbr_map_dump(u8 *buf)
{
	sunxi_mbr_t     *mbr_info = (sunxi_mbr_t *)buf;
	sunxi_partition *part_info;
	u32 i;
	char buffer[32];

	printf("*************MBR DUMP***************\n");
	printf("total mbr part %d\n", mbr_info->PartCount);
	printf("\n");
	for(part_info = mbr_info->array, i=0;i<mbr_info->PartCount;i++, part_info++)
	{
		memset(buffer, 0, 32);
		memcpy(buffer, part_info->name, 16);
		printf("part[%d] name      :%s\n", i, buffer);
		memset(buffer, 0, 32);
		memcpy(buffer, part_info->classname, 16);
		printf("part[%d] classname :%s\n", i, buffer);
		printf("part[%d] addrlo    :0x%x\n", i, part_info->addrlo);
		printf("part[%d] lenlo     :0x%x\n", i, part_info->lenlo);
		printf("part[%d] user_type :%d\n", i, part_info->user_type);
		printf("part[%d] keydata   :%d\n", i, part_info->keydata);
		printf("part[%d] ro        :%d\n", i, part_info->ro);
		printf("\n");
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
int sunxi_sprite_verify_mbr(void *buffer)
{
	sunxi_mbr_t *local_mbr;
	char        *tmp_buf = (char *)buffer;
	int          i;

	tmp_buf = buffer;
	for(i=0;i<SUNXI_MBR_COPY_NUM;i++)
    {
    	local_mbr = (sunxi_mbr_t *)tmp_buf;
    	if(crc32(0, (const unsigned char *)(tmp_buf + 4), SUNXI_MBR_SIZE - 4) != local_mbr->crc32)
    	{
    		printf("the %d mbr table is bad\n", i);

    		return -1;
    	}
    	else
    	{
    		printf("the %d mbr table is ok\n", i);
    		tmp_buf += SUNXI_MBR_SIZE;
    	}
    }
#if 1
	__mbr_map_dump(buffer);
#endif

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
int sunxi_sprite_verify_dlmap(void *buffer)
{
	sunxi_download_info *local_dlmap;
	char        *tmp_buf = (char *)buffer;

	tmp_buf = buffer;
   	local_dlmap = (sunxi_download_info *)tmp_buf;
    if(crc32(0, (const unsigned char *)(tmp_buf + 4), SUNXI_MBR_SIZE - 4) != local_dlmap->crc32)
    {
    	printf("downlaod map is bad\n");

    	return -1;
    }

    return 0;
}

