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
#include <sunxi_board.h>

extern int nand_secure_storage_read( int item, unsigned char *buf, unsigned int len);
extern int nand_secure_storage_write(int item, unsigned char *buf, unsigned int len);

extern int sunxi_flash_mmc_secread( int item, unsigned char *buf, unsigned int len);
extern int sunxi_flash_mmc_secwrite( int item, unsigned char *buf, unsigned int len);

extern int sunxi_sprite_mmc_secwrite(int item ,unsigned char *buf,unsigned int nblock);
extern int sunxi_sprite_mmc_secread(int item ,unsigned char *buf,unsigned int nblock);


static unsigned char secure_storage_map[4096] = {0};
static unsigned int  secure_storage_inited = 0;
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
static unsigned char _inner_buffer[4096+64]; /*align temp buffer*/
int sunxi_secstorage_read(int item, unsigned char *buf, unsigned int len)
{
	unsigned char * align ; 
	unsigned int blkcnt;
	int ret ;

	if(!uboot_spare_head.boot_data.storage_type)
		return nand_secure_storage_read(item, buf, len);
	else{			
		if(((unsigned int)buf%32)){
			align = (unsigned char *)(((unsigned int)_inner_buffer + 0x20)&(~0x1f)) ;
			memset(align,0,4096);
		}else
			align = buf ;

		blkcnt = (len+511)/512 ;
		ret = (sunxi_flash_mmc_secread(item, align, blkcnt) == blkcnt) ? 0 : -1;
		if(ret< 0){
			printf("sunxi_secstorage_read fail\n");
			return -1;
		}
		if(((unsigned int)buf%32))
			memcpy(buf,align,len);
		return 0 ;
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
int sunxi_secstorage_write(int item, unsigned char *buf, unsigned int len)
{
	unsigned char * align ;
	unsigned int blkcnt;

	if(!uboot_spare_head.boot_data.storage_type)
		return nand_secure_storage_write(item, buf, len);
	else{
		if(((unsigned int)buf%32)){ // input buf not align
			align = (unsigned char *)(((unsigned int)_inner_buffer + 0x20)&(~0x1f)) ;
			memcpy(align, buf, len);
		}else
			align=buf;

		blkcnt = (len+511)/512 ;
		return (sunxi_flash_mmc_secwrite(item, align, blkcnt) == blkcnt) ? 0 : -1;
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
static int __probe_name_in_map(unsigned char *buffer, const char *item_name, int *len)
{
	unsigned char *buf_start = buffer;
	int   index = 1;
	char  name[64], length[32];
	int   i,j;

	while(*buf_start != '\0')
	{
		memset(name, 0, 64);
		memset(length, 0, 32);
		i=0;
		while(buf_start[i] != ':')
		{
			name[i] = buf_start[i];
			i ++;
		}
		i ++;j=0;
		while( (buf_start[i] != ' ') && (buf_start[i] != '\0') )
		{
			length[j] = buf_start[i];
			i ++;j++;
		}

		printf("name in map %s\n", name);
		if(!strcmp(item_name, (const char *)name))
		{
			buf_start += strlen(item_name) + 1;
			*len = simple_strtoul((const char *)length, NULL, 10);
			return index;
		}
		index ++;
		buf_start += strlen((const char *)buf_start) + 1;
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
static int __fill_name_in_map(unsigned char *buffer, const char *item_name, int length)
{
	unsigned char *buf_start = buffer;
	int   index = 1;
	int   name_len;

	while(*buf_start != '\0')
	{
		printf("name in map %s\n", buf_start);

		name_len = 0;
		while(buf_start[name_len] != ':')
			name_len ++;
		if(!memcmp((const char *)buf_start, item_name, name_len))
		{
			return index;
		}
		index ++;
		buf_start += strlen((const char *)buf_start) + 1;
	}
	if(index >= 32)
		return -1;

	sprintf((char *)buf_start, "%s:%d", item_name, length);

	return index;
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
static int __discard_name_in_map(unsigned char *buffer, const char *item_name)
{
	unsigned char *buf_start = buffer, *last_start;
	int   index = 1;
	int   name_len;

	while(*buf_start != '\0')
	{
		printf("name in map %s\n", buf_start);

		name_len = 0;
		while(buf_start[name_len] != ':')
			name_len ++;
		if(!memcmp((const char *)buf_start, item_name, name_len))
		{
			last_start = buffer + strlen((const char *)buf_start) + 1;
			if(*last_start == '\0')
			{
				memset(buf_start, 0, strlen((const char *)buf_start));
			}
			else
			{
				memcpy(buf_start, last_start, 4096 - (last_start - buffer));
			}

			return index;
		}
		index ++;
		buf_start += strlen((const char *)buf_start) + 1;
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
int sunxi_secure_storage_init(void)
{
	int ret;

	if(!secure_storage_inited)
	{
		ret = sunxi_secstorage_read(0, secure_storage_map, 4096);
		if(ret < 0)
		{
			printf("get secure storage map err\n");

			return -1;
		}
		else
		{
			if((secure_storage_map[0] == 0xff) || (secure_storage_map[0] == 0x0))
			{
				printf("the secure storage map is empty\n");
				memset(secure_storage_map, 0, 4096);
			}
		}
	}
	secure_storage_inited = 1;

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
int sunxi_secure_storage_exit(void)
{
	int ret;

	if(!secure_storage_inited)
	{
		printf("%s err: secure storage has not been inited\n", __func__);

		return -1;
	}
	ret = sunxi_secstorage_write(0, secure_storage_map, 4096);
	if(ret<0)
	{
		printf("write secure storage map\n");

		return -1;
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
int sunxi_secure_storage_list(void)
{
	int ret, index = 1;
	unsigned char *buf_start = secure_storage_map;
	unsigned char  buffer[4096];

	if(sunxi_secure_storage_init())
	{
		printf("%s secure storage init err\n", __func__);

		return -1;
	}

	char  name[64], length[32];
	int   i,j, len;

	while(*buf_start != '\0')
	{
		memset(name, 0, 64);
		memset(length, 0, 32);
		i=0;
		while(buf_start[i] != ':')
		{
			name[i] = buf_start[i];
			i ++;
		}
		i ++;j=0;
		while( (buf_start[i] != ' ') && (buf_start[i] != '\0') )
		{
			length[j] = buf_start[i];
			i ++;j++;
		}

		printf("name in map %s\n", name);
		len = simple_strtoul((const char *)length, NULL, 10);

		ret = sunxi_secstorage_read(index, buffer, 4096);
		if(ret < 0)
		{
			printf("get secure storage index %d err\n", index);

			return -1;
		}
		else if(ret > 0)
		{
			printf("the secure storage index %d is empty\n", index);

			return -1;
		}
		else
		{
			printf("%d data:\n", index);
			sunxi_dump(buffer, len);
		}
		index ++;
		buf_start += strlen((const char *)buf_start) + 1;
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
int sunxi_secure_storage_probe(const char *item_name)
{
	int ret;
	int len;

	if(!secure_storage_inited)
	{
		printf("%s err: secure storage has not been inited\n", __func__);

		return -1;
	}
	ret = __probe_name_in_map(secure_storage_map, item_name, &len);
	if(ret < 0)
	{
		printf("no item name %s in the map\n", item_name);

		return -1;
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
int sunxi_secure_storage_read(const char *item_name, char *buffer, int buffer_len, int *data_len)
{
	int ret, index;
	int len_in_store;
	unsigned char buffer_to_sec[4096];

	if(!secure_storage_inited)
	{
		printf("%s err: secure storage has not been inited\n", __func__);

		return -1;
	}
	index = __probe_name_in_map(secure_storage_map, item_name, &len_in_store);
	if(index < 0)
	{
		printf("no item name %s in the map\n", item_name);

		return -2;
	}
	memset(buffer_to_sec, 0, 4096);
	ret = sunxi_secstorage_read(index, buffer_to_sec, 4096);
	if(ret<0)
	{
		printf("read secure storage block %d name %s err\n", index, item_name);

		return -3;
	}
	if(len_in_store > buffer_len)
	{
		memcpy(buffer, buffer_to_sec, buffer_len);
	}
	else
	{
		memcpy(buffer, buffer_to_sec, len_in_store);
	}
	*data_len = len_in_store;

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

int sunxi_secure_storage_write(const char *item_name, char *buffer, int length)
{
	int ret, index;
	char tmp_buf[4096];

	if(!secure_storage_inited)
	{
		printf("%s err: secure storage has not been inited\n", __func__);

		return -1;
	}
	index = __fill_name_in_map(secure_storage_map, item_name, length);
	if(index < 0)
	{
		printf("write secure storage block %d name %s overrage\n", index, item_name);

		return -1;
	}
	memset(tmp_buf, 0x0, 4096);
	memcpy(tmp_buf, buffer, length);
	ret = sunxi_secstorage_write(index, (unsigned char *)tmp_buf, 4096);
	if(ret<0)
	{
		printf("write secure storage block %d name %s err\n", index, item_name);

		return -1;
	}
	printf("write secure storage: %d ok\n", index);

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

int sunxi_secure_storage_erase(const char *item_name)
{
	int ret, index;
	unsigned char  buffer[4096];

	if(!secure_storage_inited)
	{
		printf("%s err: secure storage has not been inited\n", __func__);

		return -1;
	}
	index = __discard_name_in_map(secure_storage_map, item_name);
	if(index < 0)
	{
		printf("no item name %s in the map\n", item_name);

		return -2;
	}
	memset(buffer, 0xff, 4096);
	ret = sunxi_secstorage_write(index, buffer, 4096);
	if(ret<0)
	{
		printf("erase secure storage block %d name %s err\n", index, item_name);

		return -1;
	}
	printf("erase secure storage: %d ok\n", index);

	return 0;
}

