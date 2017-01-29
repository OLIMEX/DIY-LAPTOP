/*
************************************************************************************************************************
*                                                         eGON
*                                         the Embedded GO-ON Bootloader System
*
*                             Copyright(C), 2006-2008, SoftWinners Microelectronic Co., Ltd.
*											       All Rights Reserved
*
* File Name : adv_NF_read.c
*
* Author : Gary.Wang
*
* Version : 1.1.0
*
* Date : 2008.09.23
*
* Description :
*
* Others : None at present.
*
*
* History :
*
*  <Author>        <time>       <version>      <description>
*
* Gary.Wang      2008.09.23       1.1.0        build the file
*
************************************************************************************************************************
*/
#include "common.h"
#include "asm/arch/nand_boot0.h"


//#pragma arm section  code="load_and_check_in_one_blk"
/*******************************************************************************
*函数名称: load_and_check_in_one_blk
*函数原型：int32 load_and_check_in_one_blk( __u32 blk_num, void *buf,
*                       __u32 size, __u32 blk_size, const char *magic )
*函数功能: 从nand flash的某一块中找到一个完好备份将其载入到RAM中。如果成功，返
*          回OK；否则，返回ERROR。
*入口参数: blk_num           待访问的nand flash块的块号
*          buf               缓冲区的起始地址
*          size              待读入数据的大小
*          blk_size          待访问的nand flash块的尺寸
*          magic             文件的magic
*返 回 值: ADV_NF_OK                载入成功
*          ADV_NF_OVERTIME_ERR      操作超时
*          ADV_NF_ERROR             载入失败
*备    注: 1. 本函数不检验当前块的好坏；块的好坏必须在调用本函数前检验
*          2. 各个备份应该从当前块的起始地址处依次、紧密排放。
*******************************************************************************/
__s32 load_and_check_in_one_blk( __u32 blk_num, void *buf, __u32 size, __u32 blk_size)
{
	__u32 copy_base;
	__u32 copy_end;
	__u32 blk_end;
	__u32 blk_base = blk_num * blk_size;
	__s32  status;


	for( copy_base = blk_base, copy_end = copy_base + size, blk_end = blk_base + blk_size;
         copy_end <= blk_end;
         copy_base += size, copy_end = copy_base + size )
    {
        status = NF_read( copy_base >> NF_SCT_SZ_WIDTH, (void *)buf, size >> NF_SCT_SZ_WIDTH ); // 读入一个备份
        if( status == NF_OVERTIME_ERR )
            return ADV_NF_OVERTIME_ERR;
        else if( status == NF_ECC_ERR )
        	continue;

        /* 校验备份是否完好，如果完好，则程序返回OK */
        if( verify_addsum( (__u32 *)buf, size ) == 0 )
        {
            //printf("The file stored in %X of block %u is perfect.\n", ( copy_base - blk_base ), blk_num );
			return ADV_NF_OK;
		}
	}

	return ADV_NF_ERROR;                              // 当前块中不存在完好的备份
}



//#pragma arm section  code="load_in_many_blks"
/*******************************************************************************
*函数名称: load_in_many_blks
*函数原型：int32 load_in_many_blks( __u32 start_blk, __u32 last_blk_num, void *buf,
*						            __u32 size, __u32 blk_size, __u32 *blks )
*函数功能: 从nand flash的某一块start_blk开始，载入file_length长度的内容到内存中。
*入口参数: start_blk         待访问的nand flash起始块号
*          last_blk_num      最后一个块的块号，用来限制访问范围
*          buf               内存缓冲区的起始地址
*          size              文件尺寸
*          blk_size          待访问的nand flash的块大小
*          blks              所占据的块数，包括坏块
*返 回 值: ADV_NF_OK                操作成功
*          ADV_NF_OVERTIME_ERR   操作超时
*          ADV_NF_LACK_BLKS      块数不足
*备    注: 1. 本函数只载入，不校验
*******************************************************************************/
__s32 load_in_many_blks( __u32 start_blk, __u32 last_blk_num, void *buf,
						 __u32 size, __u32 blk_size, __u32 *blks )
{
	__u32 buf_base;
	__u32 buf_off;
    __u32 size_loaded;
    __u32 cur_blk_base;
    __u32 rest_size;
    __u32 blk_num;
	__u32 blk_size_load;
	__u32 lsb_page_type;

	lsb_page_type = NAND_Getlsbpage_type();
	if(lsb_page_type!=0)
		blk_size_load = NAND_GetLsbblksize();
	else
		blk_size_load = blk_size;

	for( blk_num = start_blk, buf_base = (__u32)buf, buf_off = 0;
         blk_num <= last_blk_num && buf_off < size;
         blk_num++ )
    {
    	printf("current block is %d and last block is %d.\n", blk_num, last_blk_num);
    	if( NF_read_status( blk_num ) == NF_BAD_BLOCK )		// 如果当前块是坏块，则进入下一块
    		continue;

    	cur_blk_base = blk_num * blk_size;
    	rest_size = size - buf_off ;                        // 未载入部分的尺寸
    	size_loaded = ( rest_size < blk_size_load ) ?  rest_size : blk_size_load ;  // 确定此次待载入的尺寸

    	if( NF_read( cur_blk_base >> NF_SCT_SZ_WIDTH, (void *)buf_base, size_loaded >> NF_SCT_SZ_WIDTH )
    		== NF_OVERTIME_ERR )
       		return ADV_NF_OVERTIME_ERR;

    	buf_base += size_loaded;
    	buf_off  += size_loaded;
    }


    *blks = blk_num - start_blk;                            // 总共涉及的块数
    if( buf_off == size )
		return ADV_NF_OK;                                          // 成功，返回OK
	else
	{
		printf("lack blocks with start block %d and buf size %x.\n", start_blk, size);
		return ADV_NF_LACK_BLKS;                                // 失败，块数不足
	}
}


