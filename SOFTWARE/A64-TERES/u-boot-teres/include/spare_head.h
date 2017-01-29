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

#ifndef  __spare_head_h__
#define  __spare_head_h__

/* work mode */
#define WORK_MODE_PRODUCT      (1<<4)
#define WORK_MODE_UPDATE       (1<<5)

#define WORK_MODE_BOOT			0x00	//正常启动
#define WORK_MODE_USB_PRODUCT	0x10	//用于USB量产
#define WORK_MODE_CARD_PRODUCT	0x11	//用于卡量产
#define WORK_MODE_USB_DEBUG	    0x12    //利用usb量产协议完成的测试
#define WORK_MODE_SPRITE_RECOVERY 0x13	//一键恢复
#define WORK_MODE_USB_UPDATE	0x20	//用于USB升级
#define WORK_MODE_OUTER_UPDATE	0x21	//用于外部盘升级

#define WORK_MODE_USB_TOOL_PRODUCT	0x04	//用于量产
#define WORK_MODE_USB_TOOL_UPDATE	0x08	//用于升级
#define WORK_MODE_ERASE_KEY			0x20	//用于擦除key

#define UBOOT_MAGIC				"uboot"
#define STAMP_VALUE             0x5F0A6C39
#define ALIGN_SIZE				16 * 1024
#define MAGIC_SIZE              8
#define STORAGE_BUFFER_SIZE     (256)

#define SUNXI_UPDATE_NEXT_ACTION_NORMAL			(1)
#define SUNXI_UPDATE_NEXT_ACTION_REBOOT			(2)
#define SUNXI_UPDATE_NEXT_ACTION_SHUTDOWN		(3)
#define SUNXI_UPDATE_NEXT_ACTION_REUPDATE		(4)
#define SUNXI_UPDATE_NEXT_ACTION_BOOT			(5)
#define SUNXI_UPDATA_NEXT_ACTION_SPRITE_TEST    (6)

#define SUNXI_VBUS_UNKNOWN                      (0)
#define SUNXI_VBUS_EXIST                        (1)
#define SUNXI_VBUS_NOT_EXIST                    (2)

#define BOOT0_SDMMC_START_ADDR                  (16)
#ifndef CONFIG_SUNXI_SECURE_SYSTEM
#define UBOOT_START_SECTOR_IN_SDMMC             (38192)
#else
#define UBOOT_START_SECTOR_IN_SDMMC             (32800)
#define UBOOT_START_SECTOR_PRE_IN_SDMMC         (38192)
#endif

#define SUNXI_NORMAL_MODE                            0
#define SUNXI_SECURE_MODE_WITH_SECUREOS              1
#define SUNXI_SECURE_MODE_NO_SECUREOS                2

#define   BOOT_FROM_SD0     0
#define   BOOT_FROM_SD2     2
#define   BOOT_FROM_NFC     1
#define   BOOT_FROM_SPI     3

//#define	TOC_MAIN_INFO_STATUS_ENCRYP_NOT_USED	0x00
//#define	TOC_MAIN_INFO_STATUS_ENCRYP_SSK			0x01
//#define	TOC_MAIN_INFO_STATUS_ENCRYP_BSSK		0x02
#define SUNXI_SECURE_MODE_USE_SEC_MONITOR             1

#define	TOC_ITEM_ENTRY_STATUS_ENCRYP_NOT_USED	0x00
#define	TOC_ITEM_ENTRY_STATUS_ENCRYP_USED		0x01

#define	TOC_ITEM_ENTRY_TYPE_NULL				0x00
#define	TOC_ITEM_ENTRY_TYPE_KEY_CERTIF			0x01
#define	TOC_ITEM_ENTRY_TYPE_BIN_CERTIF			0x02
#define	TOC_ITEM_ENTRY_TYPE_BIN     			0x03

typedef struct _normal_gpio_cfg
{
    char      port;                       //端口号
    char      port_num;                   //端口内编号
    char      mul_sel;                    //功能编号
    char      pull;                       //电阻状态
    char      drv_level;                  //驱动驱动能力
    char      data;                       //输出电平
    char      reserved[2];                //保留位，保证对齐
}
normal_gpio_cfg;

typedef struct _special_gpio_cfg
{
	unsigned char		port;				//端口号
	unsigned char		port_num;			//端口内编号
	char				mul_sel;			//功能编号
	char				data;				//输出电平
}special_gpio_cfg;
//SD卡相关数据结构
typedef struct sdcard_spare_info_t
{
	int 			card_no[4];                   //当前启动的卡控制器编号
	int 			speed_mode[4];                //卡的速度模式，0：低速，其它：高速
	int				line_sel[4];                  //卡的线制，0: 1线，其它，4线
	int				line_count[4];                //卡使用线的个数
}
sdcard_spare_info;

typedef enum
{
	STORAGE_NAND =0,
	STORAGE_SD,
	STORAGE_EMMC,
	STORAGE_NOR
}SUNXI_BOOT_STORAGE;

#endif


