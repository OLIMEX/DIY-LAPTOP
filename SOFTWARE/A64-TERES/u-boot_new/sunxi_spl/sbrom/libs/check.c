/*
************************************************************************************************************************
*                                          Boot rom
*                                         Seucre Boot
*
*                             Copyright(C), 2006-2013, AllWinners Microelectronic Co., Ltd.
*											       All Rights Reserved
*
* File Name   : Base.h
*
* Author      : glhuang
*
* Version     : 0.0.1
*
* Date        : 2013.09.05
*
* Description :
*
* Others      : None at present.
*
*
* History     :
*
*  <Author>        <time>       <version>      <description>
*
* glhuang       2013.09.05       0.0.1        build the file
*
************************************************************************************************************************
*/
#include "common.h"
#include "private_toc.h"

extern sbrom_toc0_config_t *toc0_config;

int verify_addsum( void *mem_base, __u32 size )
{
	__u32 *buf;
	__u32 count;
	__u32 src_sum;
	__u32 sum;
	sbrom_toc1_head_info_t  *bfh;

	bfh = (sbrom_toc1_head_info_t *)mem_base;

	/* 生成校验和 */
	src_sum = bfh->add_sum;                  // 从Boot_file_head中的“check_sum”字段取出校验和
	bfh->add_sum = STAMP_VALUE;              // 将STAMP_VALUE写入Boot_file_head中的“check_sum”字段

	count = size >> 2;                         // 以 字（4bytes）为单位计数
	sum = 0;
	buf = (__u32 *)mem_base;
	do
	{
		sum += *buf++;                         // 依次累加，求得校验和
		sum += *buf++;                         // 依次累加，求得校验和
		sum += *buf++;                         // 依次累加，求得校验和
		sum += *buf++;                         // 依次累加，求得校验和
	}while( ( count -= 4 ) > (4-1) );

	while( count-- > 0 )
		sum += *buf++;

	bfh->add_sum = src_sum;                  // 恢复Boot_file_head中的“check_sum”字段的值

	if( sum == src_sum )
		return 0;                           // 校验成功
	else
		return -1;                          // 校验失败
}


