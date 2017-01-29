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

#include "imgdecode.h"
#include "imagefile_new.h"
#include "../sprite_card.h"

#define HEAD_ID				0		//头加密接口索引
#define TABLE_ID			1		//表加密接口索引
#define DATA_ID				2		//数据加密接口索引
#define IF_CNT				3		//加密接口个数	现在只有头加密，表加密，数据加密3种
#define	MAX_KEY_SIZE 		32		//密码长度

#pragma pack(push, 1)
typedef struct tag_IMAGE_HANDLE
{

//	HANDLE  fp;			//

	ImageHead_t  ImageHead;		//img头信息

	ImageItem_t *ItemTable;		//item信息表

//	RC_ENDECODE_IF_t rc_if_decode[IF_CNT];//解密接口

//	BOOL			bWithEncpy; // 是否加密
}IMAGE_HANDLE;

#define INVALID_INDEX		0xFFFFFFFF


typedef struct tag_ITEM_HANDLE{
	uint	index;					//在ItemTable中的索引
	uint    reserved[3];
//	long long pos;
}ITEM_HANDLE;

#define ITEM_PHOENIX_TOOLS 	  "PXTOOLS "

uint img_file_start;			//固件的起始位置
//------------------------------------------------------------------------------------------------------------
//image解析插件的接口
//------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
HIMAGE 	Img_Open	(char * ImageFile)
{
	IMAGE_HANDLE * pImage = NULL;
	uint ItemTableSize;					//固件索引表的大小

	img_file_start = sprite_card_firmware_start();
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


//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
long long Img_GetSize	(HIMAGE hImage)
{
	IMAGE_HANDLE* pImage = (IMAGE_HANDLE *)hImage;
	long long       size;

	if (NULL == hImage)
	{
		printf("sunxi sprite error : hImage is NULL\n");

		return 0;
	}

	size = pImage->ImageHead.lenHi;
	size <<= 32;
	size |= pImage->ImageHead.lenLo;

	return size;
}
//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
HIMAGEITEM 	Img_OpenItem	(HIMAGE hImage, char * MainType, char * subType)
{
	IMAGE_HANDLE* pImage = (IMAGE_HANDLE *)hImage;
	ITEM_HANDLE * pItem  = NULL;
	uint          i;

	if (NULL == pImage || NULL == MainType || NULL == subType)
	{
		return NULL;
	}

	pItem = (ITEM_HANDLE *) malloc(sizeof(ITEM_HANDLE));
	if (NULL == pItem)
	{
		printf("sunxi sprite error : cannot malloc memory for item\n");

		return NULL;
	}
	pItem->index = INVALID_INDEX;

	for (i = 0; i < pImage->ImageHead.itemcount ; i++)
	{
//		int nCmp = memcmp(ITEM_PHOENIX_TOOLS, MainType, MAINTYPE_LEN);
//
//		if (nCmp == 0)//
//		{
//			if (memcmp(MainType, pImage->ItemTable[i].mainType, MAINTYPE_LEN) == 0 )
//			{
//				pItem->index = i;
//
//				return pItem;
//			}
//		}
//		else
//		{
//			nCmp = memcmp(MainType, pImage->ItemTable[i].mainType, MAINTYPE_LEN);
//			if(nCmp == 0)
//			{
//				nCmp = memcmp(subType,  pImage->ItemTable[i].subType,  SUBTYPE_LEN);
//				if( nCmp == 0)
//				{
//					pItem->index = i;
//
//					return pItem;
//				}
//			}
//		}
		if(!memcmp(subType,  pImage->ItemTable[i].subType,  SUBTYPE_LEN))
		{
			pItem->index = i;
			//debug("try to malloc %x\n", (uint)pItem);

			return pItem;
		}
	}

	printf("sunxi sprite error : cannot find item %s %s\n", MainType, subType);

	free(pItem);
	pItem = NULL;

	return NULL;
}



//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
long long Img_GetItemSize	(HIMAGE hImage, HIMAGEITEM hItem)
{
	IMAGE_HANDLE* pImage = (IMAGE_HANDLE *)hImage;
	ITEM_HANDLE * pItem  = (ITEM_HANDLE  *)hItem;
	long long       size;

	if (NULL == pItem)
	{
		printf("sunxi sprite error : item is NULL\n");

		return 0;
	}

	size = pImage->ItemTable[pItem->index].filelenHi;
	size <<= 32;
	size |= pImage->ItemTable[pItem->index].filelenLo;

	return size;
}

//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
uint Img_GetItemStart	(HIMAGE hImage, HIMAGEITEM hItem)
{
	IMAGE_HANDLE* pImage = (IMAGE_HANDLE *)hImage;
	ITEM_HANDLE * pItem  = (ITEM_HANDLE  *)hItem;
	long long       start;
	long long		offset;

	if (NULL == pItem)
	{
		printf("sunxi sprite error : item is NULL\n");

		return 0;
	}
	offset = pImage->ItemTable[pItem->index].offsetHi;
	offset <<= 32;
	offset |= pImage->ItemTable[pItem->index].offsetLo;
	start = offset/512;

	return ((uint)start + img_file_start);
}
//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//     返回实际读取数据的长度
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
#if 0
uint Img_ReadItem(HIMAGE hImage, HIMAGEITEM hItem, void *buffer, uint buffer_size)
{
	IMAGE_HANDLE* pImage = (IMAGE_HANDLE *)hImage;
	ITEM_HANDLE * pItem  = (ITEM_HANDLE  *)hItem;
	long long     start;
	long long	  offset;
	uint	      file_size;
	void          *tmp;

	if (NULL == pItem)
	{
		printf("sunxi sprite error : item is NULL\n");

		return 0;
	}
	if(pImage->ItemTable[pItem->index].filelenHi)
	{
		printf("sunxi sprite error : the item too big\n");

		return 0;
	}
	file_size = pImage->ItemTable[pItem->index].filelenLo;
	debug("file size=%d, buffer size=%d\n", file_size, buffer_size);
	if(file_size > buffer_size)
	{
		printf("sunxi sprite error : buffer is smaller than data size\n");

		return 0;
	}
	if(file_size > 2 * 1024 * 1024)
	{
		printf("sunxi sprite error : this function cant be used to read data over 2M bytes\n");

		return 0;
	}
	file_size = (file_size + 1023) & (~(1024 - 1));
	offset = pImage->ItemTable[pItem->index].offsetHi;
	offset <<= 32;
	offset |= pImage->ItemTable[pItem->index].offsetLo;
	start = offset/512;

	debug("malloc size = %d\n", file_size);
	tmp = malloc(file_size);
	if(!tmp)
	{
		printf("sunxi sprite error : fail to get memory for temp data\n");

		return 0;
	}
	if(!sunxi_flash_read((uint)start + img_file_start, file_size/512, tmp))
	{
		printf("sunxi sprite error : read item data failed\n");
		free(tmp);

		return 0;
	}
	memcpy(buffer, tmp, buffer_size);
	free(tmp);

	return buffer_size;
}
#else
uint Img_ReadItem(HIMAGE hImage, HIMAGEITEM hItem, void *buffer, uint buffer_size)
{
	IMAGE_HANDLE* pImage = (IMAGE_HANDLE *)hImage;
	ITEM_HANDLE * pItem  = (ITEM_HANDLE  *)hItem;
	long long     start;
	long long	  offset;
	uint	      file_size;

	if (NULL == pItem)
	{
		printf("sunxi sprite error : item is NULL\n");

		return 0;
	}
	if(pImage->ItemTable[pItem->index].filelenHi)
	{
		printf("sunxi sprite error : the item too big\n");

		return 0;
	}
	file_size = pImage->ItemTable[pItem->index].filelenLo;
	file_size = (file_size + 1023) & (~(1024 - 1));
	debug("file size=%d, buffer size=%d\n", file_size, buffer_size);
	if(file_size > buffer_size)
	{
		printf("sunxi sprite error : buffer is smaller than data size\n");

		return 0;
	}
	offset = pImage->ItemTable[pItem->index].offsetHi;
	offset <<= 32;
	offset |= pImage->ItemTable[pItem->index].offsetLo;
	start = offset/512;

	if(!sunxi_flash_read((uint)start + img_file_start, file_size/512, buffer))
	{
		printf("sunxi sprite error : read item data failed\n");

		return 0;
	}

	return file_size;
}
#endif
//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//     返回实际读取数据的长度
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------

//static __int64 __Img_ReadItemData(HIMAGE hImage, HIMAGEITEM hItem, void * buffer, __int64 Length);
//
//// 根据分组进行加速处理的版本 scott 2009-06-22 10:37:17
////////////////////////////////////////////////////////////////////////////
////每次读取的大小扩展到10M
//
//__int64 Img_ReadItemData(HIMAGE hImage, HIMAGEITEM hItem, void * buffer, __int64 Length)
//{
//	__int64 readlen = 0;
//	__int64 nRet = 0;
//	IMAGE_HANDLE* pImage = (IMAGE_HANDLE *)hImage;
//	ITEM_HANDLE * pItem  = (ITEM_HANDLE  *)hItem;
//	DWORD dwLen;
//	//u8 buffer_encode[SIZE_32K];
//	u8* buffer_encode = (u8*)malloc(SIZE_10M);
//	__int64 this_read;
//	__int64 pos = 0;
//	pEnDecode pfDecode = pImage->rc_if_decode[DATA_ID].EnDecode;
//
//	//为了速度不进行参数的检测工作 scott 2009-06-22
//	//Msg("Img_ReadItemData:Length=%x datalen=%x pos=%x",
//	//	Length, pImage->ItemTable[pItem->index].datalen, pItem->pos); //debug
//
//	__int64 nLenTmp = Get64bitLen(pImage->ItemTable[pItem->index].filelenLo, pImage->ItemTable[pItem->index].filelenHi);
//	__int64 nFileLen = nLenTmp;
//	if (pItem->pos >= nLenTmp) //filelen <= datalen
//	{
//		Err("Img_ReadItemData", __FILE__, __LINE__,
//			"pos(%x) >= pItem->filelen(%x)",nLenTmp);
//		goto readEnd;
//	}
//	//------------------------------------------------
//	//约束数据不会超出加密数据的范围
//	//------------------------------------------------
//	nLenTmp = Get64bitLen(pImage->ItemTable[pItem->index].datalenLo, pImage->ItemTable[pItem->index].datalenHi);
//	Length = min(Length, nLenTmp - pItem->pos);
//	//Msg("Length_min=%x", Length);
//	//------------------------------------------------
//	//加密后的数据以16byte进行分组，需要处理跨边界的情况
//	//------------------------------------------------
//	if ((pItem->pos % ENCODE_LEN) == 0)	//pos正好是分组的边界的情况
//	{
//		nLenTmp = Get64bitLen(pImage->ItemTable[pItem->index].offsetLo , pImage->ItemTable[pItem->index].offsetHi);
//		pos = nLenTmp + pItem->pos;
//		//fseek(pImage->fp, pos, SEEK_SET);
//		FsSeek(pImage->fp, pos, SEEK_SET);
//		readlen = 0;
//		while(readlen < Length)
//		{
//			//------------------------------------------------
//			//每次读取n个分组
//			//------------------------------------------------
//			this_read = min(SIZE_10M, (Length - readlen));
//			u32 n = (this_read + ENCODE_LEN - 1) / ENCODE_LEN;	//
//			memset(buffer_encode, 0, n * ENCODE_LEN);
//			//fread(buffer_encode, this_read, 1, pImage->fp);		//一次读n个分组,速度更快 note has bug
//			//fread(buffer_encode, n * ENCODE_LEN, 1, pImage->fp);	//OK 测试通过，必须读取整个的分组
//			u32 nReadAlian =  n * ENCODE_LEN;
//			if(pImage->bWithEncpy == FALSE)
//			{
//				ReadFile(pImage->fp, buffer, nReadAlian/*n * ENCODE_LEN*/, &dwLen, NULL);
//			}
////			else
////			{
////				ReadFile(pImage->fp, buffer_encode, nReadAlian/*n * ENCODE_LEN*/, &dwLen, NULL);	//OK 测试通过，必须读取整个的分组
////
////				//fseek(pImage->fp, 0, SEEK_CUR);
////				//Msg("this_read=%x", this_read);
////				//------------------------------------------------
////				//分组数据解密
////				//------------------------------------------------
////				u8 * pin = buffer_encode;
////				u8 * pout= (u8 *)buffer;
////				pout     = pout + readlen;	//实际输出数据的偏移量
////
////				for (u32 i = 0; i < n; i++)	//逐个分组进行解密
////				{
////					//------------------------------------------------
////					//每次解密一个分组
////					//------------------------------------------------
////					if (OK !=  pfDecode(pImage->rc_if_decode[DATA_ID].handle, pin , pout))
////						return 0;
////					/*
////					//debug start
////					if (i == (n-1))
////					{
////						Msg("last:%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x ",
////						pout[0], pout[1], pout[2],  pout[3],  pout[4],  pout[5],  pout[6], pout[7],
////						pout[8], pout[9], pout[10], pout[11], pout[12], pout[13], pout[14],pout[15]);
////					}
////					//debug end
////					*/
////					pin += ENCODE_LEN;
////					pout+= ENCODE_LEN;
////				}
////
////			}
//			//------------------------------------------------
//			//计算实际有效数据长度
//			//------------------------------------------------
//			readlen += this_read;
//		}
//		if((pItem->pos + readlen) >= nFileLen )
//		{
//			readlen = nFileLen - pItem->pos;
//		}
//
//		pItem->pos += readlen;
//		//Msg("Img_ReadItemData: pos_new =%x", pItem->pos);
//	//	Msg("Read %d\ nFileLen = %x\n", readlen, nFileLen);
//		nRet = readlen;
//		goto readEnd;
//
//	}
//	else
//	{
//		//------------------------------------------------
//		//这里强制只处理分组对齐的情况，对于以前的一些固件包可能会引起不兼容的问题，
//		//那种情况下只好启用原始版本来处理了
//		//------------------------------------------------
//
//		//MessageBox(NULL, "请按照对齐规则来处理！", "警告", MB_OK);
//		//return 0;
//
//		Msg("请按照对齐规则来处理！");
//		nRet =   __Img_ReadItemData(hImage, hItem,  buffer, Length);
//	}
//
//
//
//readEnd:
//	free(buffer_encode);
//	return nRet;
//}
//
//
////原始的版本，可以运行，不过每次读img文件是16byte，速度不高，需要进行提速
//__int64 __Img_ReadItemData(HIMAGE hImage, HIMAGEITEM hItem, void * buffer, __int64 Length)
//{
//	__int64 readlen = 0;
//	IMAGE_HANDLE* pImage = (IMAGE_HANDLE *)hImage;
//	ITEM_HANDLE * pItem  = (ITEM_HANDLE  *)hItem;
//	u8 buffer_encode[ENCODE_LEN];
//	__int64 pos = 0;
//	DWORD dwLen;
//	pEnDecode pfDecode = pImage->rc_if_decode[DATA_ID].EnDecode;
//	if (NULL == pImage || NULL == pItem || NULL == buffer || 0 == Length)
//	{
//		return 0;
//	}
//
//
//	if (pItem->pos >= pImage->ItemTable[pItem->index].filelenLo) //filelen <= datalen
//	{
//		Err("Img_ReadItemData", __FILE__, __LINE__, "pos >= pItem->filelen");
//		return 0;
//	}
//	//------------------------------------------------
//	//约束数据不会超出加密数据的范围
//	//------------------------------------------------
//	__int64 nTmp = Get64bitLen(pImage->ItemTable[pItem->index].datalenLo, pImage->ItemTable[pItem->index].datalenHi);
//	Length = min(Length, nTmp - pItem->pos);
//
//	if(pImage->bWithEncpy == FALSE)
//	{
//		nTmp = Get64bitLen(pImage->ItemTable[pItem->index].offsetLo, pImage->ItemTable[pItem->index].offsetHi);
//		pos = nTmp + pItem->pos;
//		//fseek(pImage->fp, pos, SEEK_SET);
//		FsSeek(pImage->fp, pos, SEEK_SET);
//		ReadFile(pImage->fp, buffer, Length, &dwLen, NULL);
//		pItem->pos += Length;
//		return Length;
//	}
//
//	//------------------------------------------------
//	//加密后的数据以16byte进行分组，需要处理跨边界的情况
//	//------------------------------------------------
//	if ((pItem->pos % ENCODE_LEN) == 0)	//pos正好是分组的边界的情况
//	{
//		nTmp = Get64bitLen(pImage->ItemTable[pItem->index].offsetLo, pImage->ItemTable[pItem->index].offsetHi);
//		pos = nTmp + pItem->pos;
//		//fseek(pImage->fp, pos, SEEK_SET);
//		FsSeek(pImage->fp, pos, SEEK_SET);
//
//		while(readlen < Length)
//		{
//			//每次读取一个分组
//			memset(buffer_encode, 0, ENCODE_LEN);
//		//	fread(buffer_encode, ENCODE_LEN, 1, pImage->fp);
//		//	fseek(pImage->fp, 0, SEEK_CUR);
//			ReadFile(pImage->fp, buffer_encode, ENCODE_LEN, &dwLen, NULL);
//			//分组数据解密
//			u8 * pin = buffer_encode;
//			u8 * pout= (u8 *)buffer;
//			pout     = pout + readlen;
//			if (OK != pfDecode(pImage->rc_if_decode[DATA_ID].handle, pin , pout))
//				return 0;
//			//计算实际有效数据长度
//			readlen += min(Length- readlen, ENCODE_LEN);
//		}
//		pItem->pos += readlen;
//		return readlen;
//	}
//	else //pos不在边界
//	{
//		//pos不在边界，向头方向seek
//		pos = pImage->ItemTable[pItem->index].offsetLo +
//				  pItem->pos - (pItem->pos % ENCODE_LEN);
//		//fseek(pImage->fp, pos, SEEK_SET);
//		FsSeek(pImage->fp, pos, SEEK_SET);
//
//		//-----------------------------------
//		//**********************OOOOOOOOOOOOO     *表示已经读取得数据 O表示未读取得数据
//		//-----------------------------------
//		if ((0 < Length) && (Length < ENCODE_LEN)) //读取的数据不足一个分组长度
//		{
//			u32 read = ENCODE_LEN - (pItem->pos % ENCODE_LEN); //分组中未读取的数据长度
//			if (Length <= read)	//需要读取得数据小于等于分组中未读取的数据长度 只用读一个分组即可
//			{
//				//-----------------------------------
//				//**********************OOOOOOOOOOOOO     *表示已经读取得数据 O表示未读取得数据
//				//-----------------------------------
//				u32 read = ENCODE_LEN - pItem->pos % ENCODE_LEN;
//				memset(buffer_encode, 0, ENCODE_LEN);
//				//fread(buffer_encode, ENCODE_LEN, 1, pImage->fp);
//				ReadFile(pImage->fp, buffer_encode, ENCODE_LEN, &dwLen, NULL);
//
//				//分组数据解密
//				u8 * pin = buffer_encode;
//				u8 * pout= (u8 *)buffer;
//				pout     = pout + readlen;
//				if (OK != pfDecode(pImage->rc_if_decode[DATA_ID].handle, pin , pout))
//					return 0;
//
//				readlen = Length;
//				pItem->pos += readlen;
//				return readlen;
//			}
//			else //需要读两个分组的数据
//			{
//				//----------------------------------- //-----------------------------------
//				//**********************OOOOOOOOOOOOO //OOOOOOOOOO
//				//----------------------------------- //-----------------------------------
//				//第一个分组
//				u32 read = ENCODE_LEN - pItem->pos % ENCODE_LEN;
//				memset(buffer_encode, 0, ENCODE_LEN);
//				//fread(buffer_encode, ENCODE_LEN, 1, pImage->fp);
//				ReadFile(pImage->fp, buffer_encode, ENCODE_LEN, &dwLen, NULL);
//				//分组数据解密
//				u8 * pin = buffer_encode;
//				u8 * pout= (u8 *)buffer;
//				pout     = pout + readlen;
//				if (OK != pfDecode(pImage->rc_if_decode[DATA_ID].handle, pin , pout))
//					return 0;
//
//				readlen += read;
//
//				//第二个分组
//				__int64 Left_Length = Length - read;			//剩余的数据
//				memset(buffer_encode, 0, ENCODE_LEN);
//				//fread(buffer_encode, ENCODE_LEN, 1, pImage->fp);
//				//fseek(pImage->fp, 0, SEEK_CUR);
//				ReadFile(pImage->fp, buffer_encode, ENCODE_LEN, &dwLen, NULL);
//				//分组数据解密
//				pin = buffer_encode;
//				pout= (u8 *)buffer;
//				pout     = pout + readlen;
//				if (OK != pfDecode(pImage->rc_if_decode[DATA_ID].handle, pin , pout))
//					return 0;
//				readlen += Left_Length;
//
//				pItem->pos += readlen;
//				return readlen;
//			}
//		}
//		else if (Length >= ENCODE_LEN) //读取的数据不少于一个分组长度
//		{
//			//-----------------------------------
//			//**********************OOOOOOOOOOOOO     *表示已经读取得数据 O表示未读取得数据
//			//-----------------------------------
//			u32 read = ENCODE_LEN - pItem->pos % ENCODE_LEN;
//			memset(buffer_encode, 0, ENCODE_LEN);
//			//fread(buffer_encode, ENCODE_LEN, 1, pImage->fp);
//		//	fread(pImage->fp, buffer_encode, ENCODE_LEN, &dwLen,NULL);
//			ReadFile(pImage->fp, buffer_encode, ENCODE_LEN, &dwLen,NULL);
//
//			//分组数据解密
//			u8 * pin = buffer_encode;
//			u8 * pout= (u8 *)buffer;
//			pout     = pout + readlen;
//			if (OK != pfDecode(pImage->rc_if_decode[DATA_ID].handle, pin , pout))
//				return 0;
//
//			readlen += read;
//
//			//------------------------------------------------
//			//剩余的数据按照分组进行处理
//			//------------------------------------------------
//			u32 Left_Length = Length - read;
//			u32 Left_readlen= 0;
//			while(Left_readlen < Left_Length)
//			{
//				//每次读取一个分组
//				memset(buffer_encode, 0, ENCODE_LEN);
//			//	fread(buffer_encode, ENCODE_LEN, 1, pImage->fp);
//			//	fseek(pImage->fp, 0, SEEK_CUR);
//				ReadFile(pImage->fp, buffer_encode, ENCODE_LEN, &dwLen, NULL);
//				//分组数据解密
//				u8 * pin = buffer_encode;
//				u8 * pout= (u8 *)buffer;
//				pout     = pout + readlen;
//				if (OK != pfDecode(pImage->rc_if_decode[DATA_ID].handle, pin , pout))
//					return 0;
//				//计算实际有效数据长度
//				Left_readlen += min(Left_Length - Left_readlen, ENCODE_LEN);
//			}
//
//			readlen += Left_readlen;
//		}
//
//		pItem->pos += readlen;
//		return readlen;
//	}
//
//	return 0;
//}



//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
int Img_CloseItem	(HIMAGE hImage, HIMAGEITEM hItem)
{
	ITEM_HANDLE * pItem = (ITEM_HANDLE *)hItem;
	if (NULL == pItem)
	{
		printf("sunxi sprite error : item is null when close it\n");

		return -1;
	}
	//debug("try to free %x\n", (uint)pItem);
	free(pItem);
	pItem = NULL;

	return 0;
}



//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
void  Img_Close	(HIMAGE hImage)
{
	IMAGE_HANDLE * pImage = (IMAGE_HANDLE *)hImage;

	if (NULL == pImage)
	{
		printf("sunxi sprite error : imghead is null when close it\n");

		return ;
	}

	if (NULL != pImage->ItemTable)
	{
		free(pImage->ItemTable);
		pImage->ItemTable = NULL;
	}

	memset(pImage, 0, sizeof(IMAGE_HANDLE));
	free(pImage);
	pImage = NULL;

	return ;
}



