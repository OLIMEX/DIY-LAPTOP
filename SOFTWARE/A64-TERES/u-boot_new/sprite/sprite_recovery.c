/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Char <yanjianbo@allwinnertech.com>
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
#include <sunxi_mbr.h>
#include <malloc.h>
#include <sys_config.h>
#include <sunxi_board.h>
#include "sprite_card.h"
#include "sprite_download.h"
#include "sprite_erase.h"
#include "./firmware/imgdecode.h"
#include "./firmware/imagefile_new.h"

extern uint img_file_start;            //固件的起始位置
extern int sunxi_sprite_deal_part_from_sysrevoery(sunxi_download_info *dl_map);
extern int __imagehd(HIMAGE tmp_himage);

typedef struct tag_IMAGE_HANDLE
{

//	HANDLE  fp;			//

	ImageHead_t  ImageHead;		//img头信息

	ImageItem_t *ItemTable;		//item信息表

//	RC_ENDECODE_IF_t rc_if_decode[IF_CNT];//解密接口

//	BOOL			bWithEncpy; // 是否加密
}IMAGE_HANDLE;

/*
************************************************************************************************************
*
*                                             function
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
HIMAGE 	Img_Open_from_sysrecovery(__u32 start)
{
	IMAGE_HANDLE * pImage = NULL;
	uint ItemTableSize;					//固件索引表的大小

	img_file_start = start;
	if(!img_file_start)
	{
		printf("sunxi sprite error: unable to get firmware start position\n");

		return NULL;
	}
	debug("img start = 0x%x\n", img_file_start);
	pImage = (IMAGE_HANDLE *)malloc(sizeof(IMAGE_HANDLE));
	if (NULL == pImage)
	{
		printf("sunxi sprite error: fail to malloc memory for img head\n");

		return NULL;
	}
	memset(pImage, 0, sizeof(IMAGE_HANDLE));
	//------------------------------------------------
	//读img头
	//------------------------------------------------
	//debug("try to read mmc start %d\n", img_file_start);
	if(!sunxi_flash_read(img_file_start, IMAGE_HEAD_SIZE/512, &pImage->ImageHead))
	{
		printf("sunxi sprite error: read iamge head fail\n");

		goto _img_open_fail_;
	}
	debug("read mmc ok\n");
	//------------------------------------------------
	//比较magic
	//------------------------------------------------
	if (memcmp(pImage->ImageHead.magic, IMAGE_MAGIC, 8) != 0)
	{
		printf("sunxi sprite error: iamge magic is bad\n");

		goto _img_open_fail_;
	}
	//------------------------------------------------
	//为索引表开辟空间
	//------------------------------------------------
	ItemTableSize = pImage->ImageHead.itemcount * sizeof(ImageItem_t);
	pImage->ItemTable = (ImageItem_t*)malloc(ItemTableSize);
	if (NULL == pImage->ItemTable)
	{
		printf("sunxi sprite error: fail to malloc memory for item table\n");

		goto _img_open_fail_;
	}
	//------------------------------------------------
	//读出索引表
	//------------------------------------------------
	if(!sunxi_flash_read(img_file_start + (IMAGE_HEAD_SIZE/512), ItemTableSize/512, pImage->ItemTable))
	{
		printf("sunxi sprite error: read iamge item table fail\n");

		goto _img_open_fail_;
	}

	return pImage;

_img_open_fail_:
	if(pImage->ItemTable)
	{
		free(pImage->ItemTable);
	}
	if(pImage)
	{
		free(pImage);
	}

	return NULL;
}

/*
************************************************************************************************************
*
*                                             function
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
int  card_part_info(__u32 *part_start, __u32 *part_size, const char *str)
{
    char   buffer[SUNXI_MBR_SIZE];
    sunxi_mbr_t    *mbr;
    int    i;

    if(!sunxi_flash_read(0, SUNXI_MBR_SIZE/512, buffer))
    {
    	printf("read mbr failed\n");

    	return -1;
    }
    mbr = (sunxi_mbr_t *)buffer;

    for(i=0;i<mbr->PartCount;i++)
    {
    	printf("part name  = %s\n", mbr->array[i].name);
    	printf("part start = %d\n", mbr->array[i].addrlo);
    	printf("part size  = %d\n", mbr->array[i].lenlo);
    }

    for(i=0;i<mbr->PartCount;i++)
    {
        if(!strcmp(str, (char *)mbr->array[i].name))
        {
            *part_start = mbr->array[i].addrlo;
            *part_size  = mbr->array[i].lenlo;

            return 0;
        }
    }

    return -1;
}

/*
************************************************************************************************************
*
*                                             function
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
int sprite_form_sysrecovery(void)
{
    HIMAGEITEM  imghd = 0;
    __u32       part_size;
    __u32		img_start;
    sunxi_download_info   *dl_info  = NULL;
    char        *src_buf = NULL;
    int         ret = -1;
    
	printf("sunxi sprite begin\n");
	
	//启动动画显示
	sprite_cartoon_create();
	
	src_buf = (char *)malloc(1024 * 1024);
	if (!src_buf)
	{
		printf("sprite update error: fail to get memory for tmpdata\n");
		goto _update_error_;
	}
			
	/*开始对压缩包进行操作*/
//*************************************************************************************
//*************************************************************************************
	ret = card_part_info(&img_start, &part_size, "sysrecovery");
	if (ret)
	{
		printf("sprite update error: read image start error\n");
    	goto _update_error_;
	}
	printf("part start = %d\n", img_start);
	imghd = Img_Open_from_sysrecovery(img_start);
	if (!imghd)
	{
		printf("sprite update error: fail to open img\n");
		goto _update_error_;
	}
	__imagehd(imghd);		//这个函数其实没用,为了利用sprite提供的接口,兼容一下

	sprite_cartoon_upgrade(10);

	/*实现擦除data分区*/ 
//*************************************************************************************
//*************************************************************************************
	ret = card_part_info(&img_start, &part_size, "data");
	if (ret)
	{
		printf("sprite update error: no data part found\n");
	}
	else
	{
		__u32 tmp_size;
		__u32 tmp_start;

		tmp_start = img_start;
		tmp_size = part_size;
		printf("data part size=%d\n", tmp_size);
		printf("begin erase part data\n");
		memset(src_buf, 0xff, 1024 * 1024);
#if 0
		while (tmp_size >= 1024 * 1024)
		{
			sunxi_flash_write(tmp_start, 1024 * 1024/512, src_buf);
			tmp_start += 1024 * 1024/512;
			tmp_size  -= 1024 * 1024/512;
		}
		if (tmp_size)
		{
			sunxi_flash_write(tmp_start, tmp_size/512, src_buf);
		}
#else
		sunxi_flash_write(tmp_start, 1024 * 1024/512, src_buf);
#endif
		printf("finish erase part data\n");
	}
	
	/* dl info 获取内存空间 */
//*************************************************************************************
//*************************************************************************************
	dl_info = (sunxi_download_info  *)malloc(sizeof(sunxi_download_info ));
	if (!dl_info) 
	{
		printf("sprite update error: fail to get memory for download map\n");
		goto _update_error_;
	}
	memset(dl_info, 0, sizeof(sunxi_download_info ));
	
	/*获取 DOWNLOAD MAP	*/
	ret = sprite_card_fetch_download_map(dl_info);
	if (ret) {
		printf("sunxi sprite error: donn't download dl_map\n");
		goto _update_error_;
	}
	sprite_cartoon_upgrade(20);

	/*开始烧写分区*/
//*************************************************************************************
//*************************************************************************************
	if (sunxi_sprite_deal_part_from_sysrevoery(dl_info))
	{
		printf("sunxi sprite error : download part error\n");
		return -1;
	}
	
	__msdelay(3000);
	
	
/*****************************************************************************
*
*   关闭imghd和nand句柄
*
*
*****************************************************************************/
	Img_Close(imghd);
	if (dl_info)
    {
    	free(dl_info);
    }
	if (src_buf)
	{
		free(src_buf);
	}
	//处理烧写完成后重启
	sunxi_board_restart(0);
	return 0;
	
_update_error_:
	if (dl_info)
    {
    	free(dl_info);
    }
    if (src_buf)
	{
		free(src_buf);
	}
	printf("sprite update error: current card sprite failed\n");
	printf("now hold the machine\n");
    return -1;
}
