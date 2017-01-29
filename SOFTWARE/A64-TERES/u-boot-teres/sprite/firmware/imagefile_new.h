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
#ifndef __IMAGE_FORMAT__H__
#define __IMAGE_FORMAT__H__	1

#include <config.h>
#include <common.h>
//#define IMAGE_VER	100
//------------------------------------------------------------------------------------------------------------
#define IMAGE_MAGIC			"IMAGEWTY"
#define	IMAGE_HEAD_VERSION	0x00000300

#define IMAGE_HEAD_SIZE     	  1024
#define IMAGE_ITEM_TABLE_SIZE     1024
//------------------------------------------------------------------------------------------------------------
///Image文件头数据结构
//------------------------------------------------------------------------------------------------------------
#pragma pack(push, 1)
typedef struct tag_ImageHead
{
	u8	magic[8];		//IMAGE_MAGIC
	u32 version;		//本结构的版本号，IMAGE_HEAD_VERSION
	u32	size;			//本结构的长度
	u32 attr;			//本结构的属性，格式按照version来确定，加密，压缩等
	u32 imagever;		//image的版本，由脚本指定
	u32 lenLo;			//image文件的总长度 低位
	u32 lenHi;			//image文件的总长度 高位
	u32	align;			//数据的对齐边界，缺省1024
	u32 pid;			//PID信息
	u32 vid;			//VID信息
	u32 hardwareid; 	//硬件平台ID
	u32 firmwareid; 	//软件平台ID
	u32 itemattr;		//item表的属性,"加密"
	u32	itemsize;		//item数据项的大小
	u32	itemcount;		//item数据项的个数
	u32	itemoffset;		//item表偏移量
	u32	imageattr;		//image文件属性
	u32 appendsize;		//附加数据的长度
	u32 appendoffsetLo;	//附加数据的偏移量
	u32 appendoffsetHi;	//附加数据的偏移量
	u8  reserve[980];	//预留
}ImageHead_t;
#pragma pack(pop)


//------------------------------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------------------------------
#define IMAGE_ALIGN_SIZE				0x400
#define HEAD_ATTR_NO_COMPRESS 	0x4d //1001101

//------------------------------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------------------------------
#pragma pack(push, 1)
typedef struct tagImageHeadAttr{
	u32	res		: 12;
	u32 len		: 8;
	u32 encode	: 7;		///HEAD_ATTR_NO_COMPRESS
	u32 compress: 5;
}ImageHeadAttr_t;
#pragma pack(pop)

//------------------------------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------------------------------
#define	IMAGE_ITEM_VERSION	0x00000100
#define MAINTYPE_LEN		8
#define SUBTYPE_LEN			16
#define FILE_PATH			256
#define IMAGE_ITEM_RCSIZE   640 // 数据项预留大小


//------------------------------------------------------------------------------------------------------------
///数据项数据结构
//------------------------------------------------------------------------------------------------------------
#pragma pack(push, 1)
typedef struct tag_ImageItem
{
	u32 version;				//本结构的版本号IMAGE_ITEM_VERSION
	u32	size;					//本结构的长度
	u8	mainType[MAINTYPE_LEN];	//描述的文件的类型
	u8	subType[SUBTYPE_LEN];	//描述子类型，默认由image配置脚本指定
	u32	attr;					//描述的文件的属性,格式按照version来确定，加密，压缩等
	u8	name[FILE_PATH];		//文件名称 260
	u32	datalenLo;				//文件数据在image文件中的长度
	u32	datalenHi;				//高位 文件数据在image文件中的长度
	u32 filelenLo;				//文件的实际长度
	u32 filelenHi;				//高位 文件的实际长度
	u32 offsetLo;				//文件起始位置偏移量
	u32 offsetHi;				//高位 文件起始位置偏移量
	u8	encryptID[64];			//加密插件ID，如果该文件不加密，该字段"\0"表示不加密
	u32 checksum;				//描述的文件的校验和
	u8	res[IMAGE_ITEM_RCSIZE];	//保留
}ImageItem_t;

#pragma pack(pop)

//------------------------------------------------------------------------------------------------------------
// THE END !
//------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------
// THE END !
//------------------------------------------------------------------------------------------------------------

#endif //__IMAGE_FORMAT__H__

