/*
************************************************************************************************************************
*                                                         eGON
*                                         the Embedded GO-ON Bootloader System
*
*                             Copyright(C), 2006-2008, SoftWinners Microelectronic Co., Ltd.
*											       All Rights Reserved
*
* File Name   : boot0.h
*
* Author      : Gary.Wang
*
* Version     : 1.1.0
*
* Date        : 2009.05.21
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
* Gary.Wang      2009.05.21       1.1.0        build the file
*
************************************************************************************************************************
*/
#ifndef  __toc_h
#define  __toc_h

#include "type_def.h"
#include "boot0_head.h"
#include "gpio.h"

#define  TOC0_MAGIC      "TOC0.GLH"
#define  TOC_MAIN_INFO_MAGIC   0x89119800
#define  TOC_MAIN_INFO_END     0x3b45494d
#define  TOC_ITEM_INFO_END     0x3b454949

#define  NAND0_GPIO_START     0
#define  NAND0_GPIO_MAX       24

#define  CARD0_GPIO_START     24
#define  CARD0_GPIO_MAX       16

#define  CARD2_GPIO_START     32
#define  CARD2_GPIO_MAX       16

#define  SPI0_GPIO_START      40
#define  SPI0_GPIO_MAX        10


typedef struct sbrom_toc0_config
{
	unsigned char    	config_vsn[4];
	unsigned int      	dram_para[32];  	// dram参数
	int				  	uart_port;      	// UART控制器编号
	normal_gpio_cfg   	uart_ctrl[2];    	// UART控制器GPIO
	int              	enable_jtag;    	// JTAG使能
	normal_gpio_cfg   	jtag_gpio[5];    	// JTAG控制器GPIO
	normal_gpio_cfg  	storage_gpio[50]; 	// 存储设备 GPIO信息
                							// 0-23放nand，24-31存放卡0，32-39放卡2
                							// 40-49存放spi
	char   				storage_data[384];  // 0-159,存储nand信息；160-255,存放卡信息
	unsigned int        secure_dram_mbytes; //
	unsigned int        drm_start_mbytes;   //
	unsigned int        drm_size_mbytes;    //
	unsigned int        boot_cpu;
	special_gpio_cfg	a15_power_gpio;		//the gpio config is to a15 extern power enable gpio
	unsigned int            next_exe_pa;
	unsigned int            secure_without_OS;      //1:secure boot without semelis 0: secure boot need semelis
 	unsigned char       debug_mode;         //1:turn on printf; 0 :turn off printf
	unsigned char       power_mode;          /* 0:axp , 1: dummy pmu  */
	unsigned char       reserver[2];
	unsigned int		card_work_mode;
	unsigned int      	res[2];   			// 总共1024字节
}
sbrom_toc0_config_t;

typedef struct sbrom_toc0_head_info
{
	char name[8];	 //user can modify
	u32  magic;	     //must equal TOC_U32_MAGIC
	u32  add_sum;

	u32  serial_num; //user can modify
	u32  status;	 //user can modify,such as TOC_MAIN_INFO_STATUS_ENCRYP_NOT_USED

	u32  items_nr;	 //total entry number
	u32  valid_len;
	u8   platform[4];
	u32  reserved[2];//reserved for future
	u32  end;
}
sbrom_toc0_head_info_t;

typedef struct sbrom_toc0_item_info
{
	u32 name;		  //名称，固定，TOC0部分已经确定
	u32 data_offset;  //文件数据起始位置
	u32 data_len;	  //文件数据长度
	u32 status;		  //状态，0：未使用；1：已使用
	u32 type;		  //类型，0：未使用；1：证书；2：代码
	u32 run_addr;	  //不确定，随意
	u32 reserved_1;	  //保留位
	u32 end;          //结束标志，值不确定
}
sbrom_toc0_item_info_t;

typedef struct sbrom_toc1_head_info
{
	char name[16]	;	//user can modify
	u32  magic	;	//must equal TOC_U32_MAGIC
	u32  add_sum	;

	u32  serial_num	;	//user can modify
	u32  status		;	//user can modify,such as TOC_MAIN_INFO_STATUS_ENCRYP_NOT_USED

	u32  items_nr;	//total entry number
	u32  valid_len;
	u32  main_version;
	u32  sub_version;
	u32  reserved[3];	//reserved for future
	u32  end;
}sbrom_toc1_head_info_t;


typedef struct sbrom_toc1_item_info
{
	char name[64];			//such as ITEM_NAME_SBROMSW_CERTIF
	u32  data_offset;
	u32  data_len;
	u32  encrypt;			//0: no aes   //1: aes
	u32  type;				//0: normal file, dont care  1: key certif  2: sign certif 3: bin file
	u32  run_addr;          //if it is a bin file, then run on this address; if not, it should be 0
	u32  index;             //if it is a bin file, this value shows the index to run; if not
	                       //if it is a certif file, it should equal to the bin file index
	                       //that they are in the same group
	                       //it should be 0 when it anyother data type
	u32  reserved[69];	   //reserved for future;
	u32  end;
}sbrom_toc1_item_info_t;



#endif     //  ifndef __boot0_h

/* end of boot0.h */
