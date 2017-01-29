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
#include <sparse_format.h>
#include "sparse.h"
#include "../sprite_verify.h"

#define   SPARSE_FORMAT_TYPE_TOTAL_HEAD       0xff00
#define   SPARSE_FORMAT_TYPE_CHUNK_HEAD       0xff01
#define   SPARSE_FORMAT_TYPE_CHUNK_DATA       0xff02


static uint  android_format_checksum;
static uint  sparse_format_type;
static uint  chunk_count;
static int  last_rest_size;
static int  chunk_length;
static uint  flash_start;
static sparse_header_t globl_header;
static uint  total_chunks;
/*
************************************************************************************************************
*
*                                             unsparse_probe
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int unsparse_probe(char *source, uint length, uint android_format_flash_start)
{
	sparse_header_t *header = (sparse_header_t*) source;

	if (header->magic != SPARSE_HEADER_MAGIC)
	{
		printf("sparse: bad magic\n");

		return ANDROID_FORMAT_BAD;
	}

	if ((header->major_version != SPARSE_HEADER_MAJOR_VER) ||
	    (header->file_hdr_sz != sizeof(sparse_header_t)) ||
	    (header->chunk_hdr_sz != sizeof(chunk_header_t)))
	{
		printf("sparse: incompatible format\n");

		return ANDROID_FORMAT_BAD;
	}
	android_format_checksum  = 0;
	last_rest_size = 0;
	chunk_count = 0;
	chunk_length = 0;
	sparse_format_type = SPARSE_FORMAT_TYPE_TOTAL_HEAD;
	flash_start = android_format_flash_start;
	total_chunks = header->total_chunks;

	return ANDROID_FORMAT_DETECT;
}
/*
************************************************************************************************************
*
*                                             DRAM_Write
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int  unsparse_direct_write(void *pbuf, uint length)
{
	int   unenough_length;
	int   this_rest_size;
	int   tmp_down_size;
	char *tmp_buf, *tmp_dest_buf;
	chunk_header_t   *chunk;
    //首先计算传进的数据的校验和
	android_format_checksum += add_sum(pbuf, length);

    this_rest_size = last_rest_size + length;
    tmp_buf = (char *)pbuf - last_rest_size;
	last_rest_size = 0;

    while(this_rest_size > 0)
    {
		switch(sparse_format_type)
		{
			case SPARSE_FORMAT_TYPE_TOTAL_HEAD:
			{
				memcpy(&globl_header, tmp_buf, sizeof(sparse_header_t));
            	this_rest_size -= sizeof(sparse_header_t);
            	tmp_buf += sizeof(sparse_header_t);

                sparse_format_type = SPARSE_FORMAT_TYPE_CHUNK_HEAD;

				break;
			}
			case SPARSE_FORMAT_TYPE_CHUNK_HEAD:
			{
				if(this_rest_size < sizeof(chunk_header_t))
				{
					printf("sparse: chunk head data is not enough\n");
					last_rest_size = this_rest_size;
					tmp_dest_buf = (char *)pbuf - this_rest_size;
		    		memcpy(tmp_dest_buf, tmp_buf, this_rest_size);
					this_rest_size = 0;

		    		break;
				}
				chunk = (chunk_header_t *)tmp_buf;
				/* move to next chunk */
				tmp_buf += sizeof(chunk_header_t);        //此时tmp_buf已经指向下一个chunk或者data起始地址
				this_rest_size -= sizeof(chunk_header_t); //剩余的数据长度
				chunk_length = chunk->chunk_sz * globl_header.blk_sz;   //当前数据块需要写入的数据长度
				printf("chunk %d(%d)\n", chunk_count ++, total_chunks);

				switch (chunk->chunk_type)
				{
					case CHUNK_TYPE_RAW:

						if (chunk->total_sz != (chunk_length + sizeof(chunk_header_t)))
						{
							printf("sparse: bad chunk size for chunk %d, type Raw\n", chunk_count);

							return -1;
						}
						//这里不处理数据部分，转到下一个状态
						sparse_format_type = SPARSE_FORMAT_TYPE_CHUNK_DATA;

		                            break;

                                        case CHUNK_TYPE_FILL:
                                            if(chunk->total_sz != sizeof(chunk_header_t) + sizeof(u32))
                                            {
                                                printf("spase : bad chunk size for chunk ,type FILL \n");
                                                return -1;
                                            }
                                            this_rest_size -= sizeof(u32);
                                            tmp_buf += sizeof(u32);
                                            flash_start += (chunk_length>>9);
                                            sparse_format_type = SPARSE_FORMAT_TYPE_CHUNK_HEAD;
                                            break;
					case CHUNK_TYPE_DONT_CARE:
						if (chunk->total_sz != sizeof(chunk_header_t))
						{
							printf("sparse: bogus DONT CARE chunk\n");

							return -1;
						}
						flash_start += (chunk_length>>9);
						sparse_format_type = SPARSE_FORMAT_TYPE_CHUNK_HEAD;

						break;

					default:
						printf("sparse: unknown chunk ID %x\n", chunk->chunk_type);

						return -1;
				}
				break;
			}
			case SPARSE_FORMAT_TYPE_CHUNK_DATA:
			{
				//首先判断数据是否足够当前chunk所需,如果不足，则计算出还需要的数据长度
				unenough_length = (chunk_length >= this_rest_size)? (chunk_length - this_rest_size):0;
				if(!unenough_length)
				{
					//数据足够，直接写入
					if(!sunxi_sprite_write(flash_start, chunk_length>>9, tmp_buf))
					{
						printf("sparse: flash write failed\n");

						return -1;
					}
					if(chunk_length & 511)
					{
						printf("data is not sector align 0\n");

						return -1;
					}
					flash_start += (chunk_length>>9);
					tmp_buf += chunk_length;
					this_rest_size -= chunk_length;
					chunk_length = 0;

					sparse_format_type = SPARSE_FORMAT_TYPE_CHUNK_HEAD;
				}
				else  //存在缺失数据的情况
				{
					if(this_rest_size < 8 * 1024) //先看已有数据是否不足8k
					{
						//当不足时，把这笔数据放到下一笔数据的前部，等待下一次处理
						tmp_dest_buf = (char *)pbuf - this_rest_size;
						memcpy(tmp_dest_buf, tmp_buf, this_rest_size);
                        last_rest_size = this_rest_size;
						this_rest_size = 0;

						break;
					}
					//当已有数据超过16k时
					//当缺失数据长度不足4k时,可能只缺几十个字节
					if(unenough_length < 4 * 1024)
					{
						//采用拼接方法，先烧写部分已有数据，然后在下一次把未烧写的已有数据和缺失数据一起烧录
						tmp_down_size = this_rest_size + unenough_length - 4 * 1024;
					}
					else //这里处理缺失数据超过8k(包含)的情况,同时已有数据也超过16k
					{
						//直接烧录当前全部数据;
						tmp_down_size = this_rest_size & (~(512 -1));  //扇区对齐
					}
					if(!sunxi_sprite_write(flash_start, tmp_down_size>>9, tmp_buf))
					{
						printf("sparse: flash write failed\n");

						return -1;
					}
					if(tmp_down_size & 511)
					{
						printf("data is not sector align 1\n");

						return -1;
					}
					tmp_buf += tmp_down_size;
					flash_start += (tmp_down_size>>9);
					chunk_length -= tmp_down_size;
					this_rest_size -= tmp_down_size;
					tmp_dest_buf = (char *)pbuf - this_rest_size;
					memcpy(tmp_dest_buf, tmp_buf, this_rest_size);
					last_rest_size = this_rest_size;
					this_rest_size = 0;

					sparse_format_type = SPARSE_FORMAT_TYPE_CHUNK_DATA;
				}

				break;
			}

			default:
			{
				printf("sparse: unknown status\n");

				return -1;
			}
		}
	}

	return 0;
}
/*
************************************************************************************************************
*
*                                             unsparse_checksum
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
uint unsparse_checksum(void)
{
	return android_format_checksum;
}


