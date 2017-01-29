/*
 * (C) Copyright 2002
 * Detlev Zundel, DENX Software Engineering, dzu@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * BMP handling routines
 */

#include <common.h>
#include <bmp_layout.h>
#include <command.h>
#include <malloc.h>
#include <sunxi_bmp.h>
#include <sunxi_board.h>

static int sunxi_bmp_probe_info (uint addr);
static int sunxi_bmp_show(sunxi_bmp_store_t bmp_info);

DECLARE_GLOBAL_DATA_PTR;
/*
 * Allocate and decompress a BMP image using gunzip().
 *
 * Returns a pointer to the decompressed image data. Must be freed by
 * the caller after use.
 *
 * Returns NULL if decompression failed, or if the decompressed data
 * didn't contain a valid BMP signature.
 */

static int do_sunxi_bmp_info(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	uint addr;

	if(argc == 2)
	{
		/* use argument only*/
		addr = simple_strtoul(argv[1], NULL, 16);
		debug("bmp addr=%x\n", addr);
	}
	else if(argc == 3)
	{
		char  load_addr[8];
		char  filename[32];
		char *const bmp_argv[6] = { "fatload", "mcc", "0:1", load_addr, filename, NULL };

		addr = simple_strtoul(argv[1], NULL, 16);
		memcpy(load_addr, argv[1], 8);
		memset(filename, 0, 32);
		memcpy(filename, argv[2], strlen(argv[2]));
#ifdef DEBUG
	    int i;

		for(i=0;i<6;i++)
		{
	        printf("argv[%d] = %s\n", i, argv[i]);
		}
#endif
	    if(do_fat_fsload(0, 0, 5, bmp_argv))
		{
		   printf("sunxi bmp info error : unable to open bmp file %s\n", argv[2]);

		   return cmd_usage(cmdtp);
	    }
	}
	else
	{
		return cmd_usage(cmdtp);
	}

	return (sunxi_bmp_probe_info(addr));
}

U_BOOT_CMD(
	sunxi_bmp_info,	3,	1,	do_sunxi_bmp_info,
	"manipulate BMP image data",
	"only one para : the address where the bmp stored\n"
);


static int do_sunxi_bmp_display(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	uint addr;
	uint de_addr;
	sunxi_bmp_store_t bmp_info;

	if(argc == 2)
	{
		/* use argument only*/
		addr = simple_strtoul(argv[1], NULL, 16);
#if defined(CONFIG_SUNXI_LOGBUFFER)
		de_addr = CONFIG_SYS_SDRAM_BASE + gd->ram_size - SUNXI_DISPLAY_FRAME_BUFFER_SIZE;
#else
		de_addr = SUNXI_DISPLAY_FRAME_BUFFER_ADDR;
#endif
	}
	else if(argc == 3)
	{
		addr = simple_strtoul(argv[1], NULL, 16);
		de_addr = simple_strtoul(argv[2], NULL, 16);
	}
	else if(argc == 4)
	{
		char  load_addr[8];
		char  filename[32];
		char *const bmp_argv[6] = { "fatload", "mmc", "0:1", load_addr, filename, NULL };

		addr = simple_strtoul(argv[1], NULL, 16);
		memcpy(load_addr, argv[1], 8);
		memset(filename, 0, 32);
		memcpy(filename, argv[3], strlen(argv[3]));
		de_addr = simple_strtoul(argv[2], NULL, 16);
#ifdef DEBUG
	    int i;

		for(i=0;i<6;i++)
		{
	        printf("argv[%d] = %s\n", i, argv[i]);
		}
#endif
	    if(do_fat_fsload(0, 0, 5, bmp_argv))
		{
		   printf("sunxi bmp info error : unable to open bmp file %s\n", argv[2]);

		   return cmd_usage(cmdtp);
	    }
	}
	else
	{
		return cmd_usage(cmdtp);
	}
	if(de_addr < CONFIG_SYS_SDRAM_BASE)
	{
#if defined(CONFIG_SUNXI_LOGBUFFER)
		de_addr = CONFIG_SYS_SDRAM_BASE + gd->ram_size - SUNXI_DISPLAY_FRAME_BUFFER_SIZE;
#else
		de_addr = SUNXI_DISPLAY_FRAME_BUFFER_ADDR;
#endif
	}
	debug("bmp addr %x, display addr %x\n", addr, de_addr);
	bmp_info.buffer = (void *)de_addr;
	if(!sunxi_bmp_decode(addr, &bmp_info))
	{
		debug("decode bmp ok\n");

		return sunxi_bmp_show(bmp_info);
	}
	debug("decode bmp error\n");

	return -1;
}


U_BOOT_CMD(
	sunxi_bmp_show,	4,	1,	do_sunxi_bmp_display,
	"manipulate BMP image data",
	"sunxi_bmp_display addr [de addr]\n"
	"parameters 1 : the address where the bmp stored\n"
	"parameters 2 : option para, the address where the bmp display\n"
);

int sunxi_bmp_display(char *name)
{

        sunxi_bmp_store_t bmp_info;
	char  bmp_name[32];
	char  bmp_addr[32] = {0};
	char*  bmp_buff = NULL;
	int  ret = -1;
	//const size_t bmp_buff_len = 10<<20; //10M
	//size_t file_size = 0;
	char * bmp_argv[6] = { "fatload", "mmc", "0:1", "00000000", bmp_name, NULL };

	// free() function  will  take a long time,so not use malloc memory
	bmp_buff = (char*)CONFIG_SYS_SDRAM_BASE; 
	if(bmp_buff == NULL)
	{
		printf("sunxi bmp: alloc buffer for %s fail\n", name);
		return -1;
	}
	//set bmp decode addr is CONFIG_SYS_SDRAM_BASE
	sprintf(bmp_addr,"%lx", (ulong)bmp_buff);
	bmp_argv[3] = bmp_addr;

	memset(bmp_name, 0, 32);
	strcpy(bmp_name, name);
	if(do_fat_fsload(0, 0, 5, bmp_argv))
	{
		printf("sunxi bmp info error : unable to open logo file %s\n", bmp_argv[4]);
		return -1;
	}
	//file_size = simple_strtoul(getenv("filesize"), NULL, 16);

#if defined(CONFIG_SUNXI_LOGBUFFER)
	bmp_info.buffer = (void *)(CONFIG_SYS_SDRAM_BASE + gd->ram_size - SUNXI_DISPLAY_FRAME_BUFFER_SIZE);
#else
	bmp_info.buffer = (void *)(SUNXI_DISPLAY_FRAME_BUFFER_ADDR);
#endif
	printf("bmp file buffer: 0x%lx, bmp_info.buffer: %lx\n",(ulong)bmp_buff,(ulong)bmp_info.buffer);
	if(!sunxi_bmp_decode((ulong)bmp_buff, &bmp_info))
	{
		debug("decode bmp ok\n");
		ret = sunxi_bmp_show(bmp_info);
	}
	return ret;

}
/*
 * Subroutine:  bmp_info
 *
 * Description: Show information about bmp file in memory
 *
 * Inputs:	addr		address of the bmp file
 *
 * Return:      None
 *
 */
static int sunxi_bmp_probe_info(uint addr)
{
	bmp_image_t *bmp=(bmp_image_t *)addr;

	if((bmp->header.signature[0]!='B') || (bmp->header.signature[1]!='M'))
	{
		printf("this is not a bmp picture\n");

		return -1;
	}
	debug("bmp picture dectede\n");

	printf("Image size    : %d x %d\n", bmp->header.width, (bmp->header.height & 0x80000000) ? (-bmp->header.height):(bmp->header.height));
	printf("Bits per pixel: %d\n", bmp->header.bit_count);

	return(0);
}

/*
 * Subroutine:  bmp_display
 *
 * Description: Display bmp file located in memory
 *
 * Inputs:	addr		address of the bmp file
 *
 * Return:      None
 *
 */
int sunxi_bmp_decode(unsigned long addr, sunxi_bmp_store_t *bmp_info)
{
	char *tmp_buffer;
	char *bmp_data;
	int zero_num = 0;
	bmp_image_t *bmp = (bmp_image_t *)addr;
	int x, y, bmp_bpix;
	int tmp;

	if((bmp->header.signature[0]!='B') || (bmp->header.signature[1] !='M'))
	{
		printf("this is not a bmp picture\n");

		return -1;
	}
	debug("bmp dectece\n");

	bmp_bpix = bmp->header.bit_count/8;
	if((bmp_bpix != 3) && (bmp_bpix != 4))
	{
		printf("no support bmp picture without bpix 24 or 32\n");

		return -1;
	}
	if(bmp_bpix ==3)
	{		
		zero_num = (4 - ((3*bmp->header.width) % 4))&3;
	}
	debug("bmp bitcount %d\n", bmp->header.bit_count);
	x = bmp->header.width;
	y = (bmp->header.height & 0x80000000) ? (-bmp->header.height):(bmp->header.height);
	debug("bmp x = %x, bmp y = %x\n", x, y);

	tmp = bmp->header.height;
	if (0 == (bmp->header.height & 0x80000000))
		bmp->header.height = (-bmp->header.height);
	memcpy(bmp_info->buffer, bmp, sizeof(bmp_header_t));
	bmp_info->buffer += sizeof(bmp_header_t);
	bmp->header.height = tmp;

	tmp_buffer = (char *)bmp_info->buffer;
	bmp_data = (char *)(addr + bmp->header.data_offset);
	if(bmp->header.height & 0x80000000)
    {
	      if(zero_num == 0)
                {
                    memcpy(tmp_buffer,bmp_data,x*y*bmp_bpix);
                }
                else
                {
                    int i, line_bytes, real_line_byte;	
	            char *src;
	            line_bytes = (x * bmp_bpix) + zero_num;
		    real_line_byte = x * bmp_bpix;
		    for(i=0; i<y; i++)
                   {
             	    src = bmp_data + i*line_bytes;
                     memcpy(tmp_buffer, src, real_line_byte);
                    tmp_buffer += real_line_byte;
                    }
                }
    }
    else
    {
    	uint i, line_bytes, real_line_byte;
        char *src;

		line_bytes = (x * bmp_bpix) + zero_num;
		real_line_byte = x * bmp_bpix;
		for(i=0; i<y; i++)
        {
        	src = bmp_data + (y - i - 1) * line_bytes;
        	memcpy(tmp_buffer, src, real_line_byte);
            tmp_buffer += real_line_byte;
        }
    }
    bmp_info->x = x;
    bmp_info->y = y;
    bmp_info->bit = bmp->header.bit_count;
	//flush_cache((uint)bmp_info->buffer, x * y * bmp_bpix);
	flush_cache((uint)bmp_info->buffer-sizeof(bmp_header_t) , x * y * bmp_bpix+sizeof(bmp_header_t));

	return 0;
}

static int sunxi_bmp_show(sunxi_bmp_store_t bmp_info)
{
	debug("begin to set framebuffer\n");
	if(board_display_framebuffer_set(bmp_info.x, bmp_info.y, bmp_info.bit, (void *)bmp_info.buffer))
	{
		printf("sunxi bmp display error : set frame buffer error\n");

		return -2;
	}
	debug("begin to show layer\n");
	board_display_show(0);
	debug("bmp display finish\n");

	return 0;
}

int do_sunxi_logo(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	return sunxi_bmp_display("bootlogo.bmp");
}

U_BOOT_CMD(
	logo,	1,	0,	do_sunxi_logo,
	"show default logo",
	"no args\n"
);


