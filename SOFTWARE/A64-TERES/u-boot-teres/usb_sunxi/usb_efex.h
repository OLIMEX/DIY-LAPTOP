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
#ifndef  __USB_EFEX_H__
#define  __USB_EFEX_H__

#include <common.h>

#define DRIVER_VENDOR_ID			0x1F3A
#define DRIVER_PRODUCT_ID			0xEfE8

#define CBW_MAGIC		0x43555741	//AWUC
#define CSW_MAGIC		0x53555741	//AWUS
#define CSW_STATUS_PASS	0x00
#define CSW_STATUS_FAIL	0x01

#define CBW_TOTAL_LEN		32	//
#define CBW_MAX_CMD_SIZE	16

#define	FES_PLATFORM_HW_ID		    0x00161000

#define AL_VERIFY_DEV_TAG_LEN			8
#define AL_VERIFY_DEV_TAG_DATA		"AWUSBFEX"

#define AL_VERIFY_DEV_MODE_NULL			0x00
#define AL_VERIFY_DEV_MODE_FEL			0x01
#define AL_VERIFY_DEV_MODE_SRV			0x02
#define AL_VERIFY_DEV_MODE_UPDATE_COOL	0x03
#define AL_VERIFY_DEV_MODE_UPDATE_HOT	0x04

#define	PHOENIX_PRIV_DATA_LEN_NR	8								//2的8次 = 256
#define PHOENIX_PRIV_DATA_ADDR	    (SRAM_AREA_A + 0x7e00)			//给phoenix保留的空间
#define PHOENIX_PRIV_DATA_LEN	    (1 << PHOENIX_PRIV_DATA_LEN_NR)	//空间大小

//--hgl--传输层的命令
typedef struct tag_TRANSFERDATA
{
	u8	direction;					///
	u8	resv;						///
	u32	dataLen;					///
	u8  resv2[10];					///
}__attribute__ ((packed)) tTransferData;

struct sunxi_efex_cbw_t
{
	u32	magic;				//必须为CBW_MAGIC
	u32	tag;
	u32	data_transfer_len;	//表示本次传输的数据阶段要传递的数据大小
	u16	reserved_1;
	u8	reserved_2;
	u8	cmd_len;			//cmd_package的实际有效长度
	//u8	cmd_package[CBW_MAX_CMD_SIZE];
	tTransferData  cmd_package;
}__attribute__ ((packed));


#define CSW_TOTAL_LEN	13
struct sunxi_efex_csw_t
{
	u32	magic;		//必须为CSW_MAGIC
	u32	tag;
	u32	residue;		//没有发送/接收的数据长度
	u8	status;		//为CSW_STATUS_PASS或	CSW_STATUS_FAIL
}__attribute__ ((packed));


#define TL_CMD_RESERVED	    0x00
#define TL_CMD_TRANSMIT	    0x11
#define TL_CMD_RECEIVE		0x12


#define TRANSPORT_INFO_STATUS_NULL	0x00
#define TRANSPORT_INFO_STATUS_CBW	0x01
#define TRANSPORT_INFO_STATUS_DATA	0x02
#define TRANSPORT_INFO_STATUS_CSW	0x03


#define TRANSPORT_INFO_IS_CONNECT_FAIL	0x00
#define TRANSPORT_INFO_IS_CONNECT_TRUE	0x01


//==app layer之公共命令
#define APP_LAYER_COMMEN_CMD_VERIFY_DEV			0x0001
#define APP_LAYER_COMMEN_CMD_SWITCH_ROLE		0x0002
#define APP_LAYER_COMMEN_CMD_IS_READY			0x0003
#define APP_LAYER_COMMEN_CMD_GET_CMD_SET_VER 	0x0004
#define APP_LAYER_COMMEN_CMD_DISCONNECT			0x0010

#define	FEX_CMD_fes_trans						0x0201
#define	FEX_CMD_fes_run 						0x0202
#define FEX_CMD_fes_down						0x0206
#define FEX_CMD_fes_up	    					0x0207
#define FEX_CMD_fes_verify    					0x0208
#define FEX_CMD_fes_query_storage				0x0209
#define FEX_CMD_fes_probe_hardware  			0x020A
#define FEX_CMD_fes_flash_set_on				0x020A
#define FEX_CMD_fes_flash_set_off				0x020B
#define FEX_CMD_fes_verify_value    			0x020C
#define FEX_CMD_fes_verify_status   			0x020D
#define FEX_CMD_fes_flash_size_probe			0x020E
#define FEX_CMD_fes_tool_mode					0x020F
#define FEX_CMD_fes_memset                      0x0210
#define FEX_CMD_fes_pmu                         0x0211
#define FEX_CMD_fes_unseqmem_read   			0x0212
#define FEX_CMD_fes_unseqmem_write  			0x0213
#define FEX_CMD_fes_force_erase                 0x0220
#define FEX_CMD_fes_force_erase_key             0x0221
#define FEX_CMD_fes_reset_cpu					0x0214
#define FEX_CMD_fes_low_power_manger 			0x0215
#define FEX_CMD_fes_query_secure                0x0230
//各个app命令的cmd,data部分，status部分是共用的

//====================verify_dev====================
struct global_cmd_s
{
	u16	app_cmd;					//
	u16	tag;
}__attribute__ ((packed));


struct verify_dev_cmd_s
{
	u16	app_cmd;					//必须为APP_LAYER_COMMEN_CMD_VERIFY_DEV
	u16	tag;
	u8	reserved[12];
}__attribute__ ((packed));


struct verify_dev_data_s
{
	u8 tag[AL_VERIFY_DEV_TAG_LEN];	//必须为AL_VERIFY_DEV_TAG_DATA，用来区分
	u32 platform_id_hw;
	u32	platform_id_fw;
	u16 mode;						//如AL_VERIFY_DEV_MODE_NULL
	u8 pho_data_flag;
	u8 pho_data_len;				//
	u32 pho_data_start_addr;		//phoenix data的 start addr
	u8 reserved_2[8];
}__attribute__ ((packed));

//====================switch_role====================
struct switch_role_cmd_s{
	u16 app_cmd;					//必须为APP_LAYER_COMMEN_CMD_SWITCH_ROLE
	u16	state;						//如AL_VERIFY_DEV_MODE_FEL
	u8	reserved[12];
};


//====================is_ready====================
struct is_ready_cmd_s{
	u16	app_cmd;					//必须为APP_LAYER_COMMEN_CMD_IS_READY
	u16 	state;						//目标state,如AL_VERIFY_DEV_MODE_FEL
	u8 reserved[12];
}__attribute__ ((packed));

#define AL_IS_READY_STATE_NULL		0x00	//
#define AL_IS_READY_STATE_BUSY		0x01	//忙，请等待
#define AL_IS_READY_STATE_READY		0x02	//就绪
#define AL_IS_READY_STATE_FAIL			0x03	//失败

struct is_ready_data_s
{
	u16	state;						//当前所处的状态，如AL_IS_READY_STATE_READY
	u16	interval_ms;					//下次发送is_ready命令的延时，单位为ms
									//推荐为500 ~2000 ，该字段只有当
									//state == AL_IS_READY_STATE_BUSY时候才有效
	u8	reserved[12];
}__attribute__ ((packed));


//====================get_cmd_set_ver====================
struct get_cmd_set_ver_cmd_s
{
	u16	app_cmd;			//必须为APP_LAYER_COMMEN_CMD_GET_CMD_SET_VER
	u16	tag;
	u16	state;				//要查询cmd_set的state
	u8	reserved[10];
}__attribute__ ((packed));


struct get_cmd_set_ver_data_s
{
	u16	ver_high;		//version的高word部分
	u16	ver_low;		//version的低word部分
	u8	reserved[12];

}__attribute__ ((packed));

//====================disconnect====================
struct disconnect_cmd_s
{
	u16	app_cmd;
	u16	tag;
	u8	reserved[12];
}__attribute__ ((packed));


typedef struct tag_STATUS
{
	short	mark;						///0xffff
	short	tag;						///
	char	state;						///STATUS_SUCCESS
	char	rev[3];						///
}Status_t;

typedef struct tag_fes_trans
{
	u16 	app_cmd;
	u16 	tag;
	u32 	addr;				///
	u32 	len;				///
	u32     type;
}fes_trans_t;

typedef struct tag_fes_trans_old
{
	u16 app_cmd;
	u16 tag;
	u32	addr;				///
	u32	len;				///
	struct
	{
		u8	logicunit_index : 4;	///低4比特
		u8	media_index     : 4;	///高4比特
	}u1;
	struct
	{
		u8	res		: 4;	///低4比特
		u8	DOU		: 2;	///中间2比特 标识 Download Or Upload
		u8	OOC     : 2;	///高2比特
	}u2;
	u8	reserved[2];		///
}fes_trans_old_t;


typedef struct tag_fes_cmd_verify_value_s
{
	u16     cmd;
	u16		tag;
    u32     start;
    s64		size;
}fes_cmd_verify_value_t;


typedef struct tag_fes_cmd_verify_status_s
{
	u16     cmd;
	u16		tag;
    u32     start;
    u32     size;
    u32     data_tag;
}fes_cmd_verify_status_t;

#define  EFEX_CRC32_VALID_FLAG   (0x6a617603)

typedef  struct  tag_efex_verify_s
{
    u32  flag;				//标志crc计算完成, 固定为0x6a617603

    s32  fes_crc;			//fes接收数据的crc
    s32  media_crc;         //media数据的crc
}fes_efex_verify_t;

typedef struct tag_FES_RUN
{
	u16  app_cmd;
	u16  tag;
	u32	 addr;				///
	int  max_para;
	int  *para_addr;
}fes_run_t;


typedef  struct  tag_efex_tool_s
{
	u16  cmd;
	u16	 tag;
    u32  tool_mode;

    s32  next_mode;
    s32  res0;
}fes_efex_tool_t;


typedef  struct  tag_efex_memset_s
{
	u16  cmd;
	u16	 tag;

    u32  start_addr;
    u32  length;
    s32  value;
}fes_efex_memset_t;


typedef  struct  tag_efex_pmu_s
{
	u16  cmd;
	u16	 tag;

    u32  size;		//
    s32  res1;      //
    s32  type;
}fes_efex_pmu_t;


struct pmu_config_t
{
	char pmu_type[16];
	char vol_name[16];
	u32  voltage;
	u32  gate;
};


typedef  struct  tag_efex_unseq_mem_s
{
	u16  cmd;
	u16	 tag;

	u32   size;				///
	u32   count;			///
	u32   type;
}tag_efex_unseq_mem_t;


struct unseq_mem_config
{
	u32  addr;
	u32  value;
};

struct multi_unseq_mem_s
{
	int count;

	struct unseq_mem_config  *unseq_mem;
};

#define SUNXI_EFEX_RECV_MEM_SIZE	(1024 * 1024)


typedef struct
{
	uint type;			//存储内型，见下面的宏定义
	uchar *base_recv_buffer;		//存放接收到的数据的首地址，必须足够大
	uchar *act_recv_buffer;//
	uint   recv_size;
	uint   to_be_recved_size;
	uchar *base_send_buffer;		//存放将要到的数据的首地址，必须足够大
	uchar *act_send_buffer;//
	uint   send_size;		//需要发送数据的长度
	uint   flash_start;			//起始位置，可能是内存，也可能是flash扇区
	uint   flash_sectors;
	uchar *dram_trans_buffer;
	int  last_err;
	int  app_next_status;
}
efex_trans_set_t;

#define  SUNXI_EFEX_DATA_TYPE_MASK		(0x7fff)
#define  SUNXI_EFEX_DRAM_MASK			(0x7f00)
#define  SUNXI_EFEX_DRAM_TAG			(0x7f00)
#define  SUNXI_EFEX_MBR_TAG				(0x7f01)
#define  SUNXI_EFEX_BOOT1_TAG			(0x7f02)
#define  SUNXI_EFEX_BOOT0_TAG			(0x7f03)
#define  SUNXI_EFEX_ERASE_TAG           (0x7f04)
#define  SUNXI_EFEX_PMU_SET             (0x7f05)
#define  SUNXI_EFEX_UNSEQ_MEM_FOR_READ  (0x7f06)
#define  SUNXI_EFEX_UNSEQ_MEM_FOR_WRITE (0x7f07)
#define  SUNXI_EFEX_FULLIMG_SIZE_TAG    (0x7f10)

#define  SUNXI_EFEX_FLASH_TAG           (0x8000)

#define  SUNXI_EFEX_TRANS_MASK			(0x30000)
#define  SUNXI_EFEX_TRANS_START_TAG		(0x20000)
#define  SUNXI_EFEX_TRANS_FINISH_TAG	(0x10000)

#define  SUNXI_EFEX_VERIFY_STATUS		(0)
#define  SUNXI_EFEX_VERIFY_ADDSUM		(1)
#define  SUNXI_EFEX_VERIFY_CRC32		(2)


#endif

