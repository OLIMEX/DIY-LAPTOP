/*
 * Copyright 2008 Freescale Semiconductor, Inc.
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

#include <common.h>
#include <config.h>
#include <command.h>
#include <sunxi_mbr.h>
#include <boot_type.h>
#include <sprite.h>
//#include <sunxi_usb.h>
#include "asm/arch/timer.h"
#include <securestorage.h>
#include <asm/arch/key.h>
#include <sunxi_board.h>

extern int sunxi_usb_dev_register(uint dev_name);
extern  void sunxi_usb_main_loop(int delaytime);
extern  int sunxi_usb_extern_loop(void);
extern  int sunxi_usb_init(int delaytime);
extern  int sunxi_usb_exit(void);

DECLARE_GLOBAL_DATA_PTR;

volatile int sunxi_usb_burn_from_boot_handshake, sunxi_usb_burn_from_boot_init, sunxi_usb_burn_from_boot_setup;
volatile int sunxi_usb_burn_from_boot_overtime;
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
static void probe_usb_overtime(void *p)
{
	struct timer_list *timer_t;

	timer_t = (struct timer_list *)p;

	sunxi_usb_burn_from_boot_overtime = 1;
	tick_printf("timer occur\n");

	del_timer(timer_t);

	return;
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
int do_burn_from_boot(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct timer_list timer_t;
	int    ret;

	if(gd->vbus_status == SUNXI_VBUS_NOT_EXIST)
	{
		printf("out of usb burn from boot without usb\n");

		return 0;
	}
	tick_printf("usb burn from boot\n");

	if(sunxi_usb_dev_register(4) < 0)
	{
		printf("usb burn fail: not support burn private data\n");

		return -1;
	}

	sunxi_usb_burn_from_boot_overtime = 0;
	if(sunxi_usb_init(0))
	{
		printf("%s usb init fail\n", __func__);

		sunxi_usb_exit();

		return 0;
	}
	timer_t.data = (unsigned long)&timer_t;
	timer_t.expires = 800;
	timer_t.function = probe_usb_overtime;
	init_timer(&timer_t);

	tick_printf("usb prepare ok\n");
	add_timer(&timer_t);

	while(1)
	{
		if(sunxi_usb_burn_from_boot_init)		//当usb sof中断触发，跳出循环
		{
			printf("usb sof ok\n");
			del_timer(&timer_t);
			break;
		}
		if(sunxi_usb_burn_from_boot_overtime)	//当定时时间到，还没有中断，跳出循环
		{
			tick_printf("overtime\n");
			del_timer(&timer_t);
			sunxi_usb_exit();
			tick_printf("%s usb : no usb exist\n", __func__);

			return 0;
		}
	}
	sunxi_usb_burn_from_boot_overtime = 0;
	tick_printf("usb probe ok\n");				//开始等待加载驱动，这里不需要延时
	sunxi_usb_burn_from_boot_overtime = 0;
	tick_printf("usb setup ok\n");
	timer_t.expires = 3000;
	add_timer(&timer_t);

	while(1)
	{
		ret = sunxi_usb_extern_loop();			//执行usb主循环
		if(ret)									//当执行结果非0，表示到了最后一个步骤，需要往后执行
		{
			break;
		}
		if(sunxi_usb_burn_from_boot_handshake)	//当握手成功，停止检查定时器
		{
			del_timer(&timer_t);
		}
		if(sunxi_usb_burn_from_boot_overtime)   //当定时时间到，还没有握手成功，跳出循环
		{
			del_timer(&timer_t);
			sunxi_usb_exit();
			tick_printf("%s usb : have no handshake\n", __func__);

			return 0;
		}
		if(ctrlc())
		{
			del_timer(&timer_t);
			ret = SUNXI_UPDATE_NEXT_ACTION_NORMAL;
			break;
		}
		if(sunxi_key_read()>0)
		{
			del_timer(&timer_t);
			ret = SUNXI_UPDATE_NEXT_ACTION_NORMAL;
			break;
		}
	}
	tick_printf("exit usb burn from boot\n");
	sunxi_usb_exit();

	sunxi_update_subsequent_processing(ret);

	return 0;
}


U_BOOT_CMD(
	uburn, CONFIG_SYS_MAXARGS, 1, do_burn_from_boot,
	"do a burn from boot",
	"pburn [mode]"
	"NULL"
);


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
int do_read_from_boot(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#ifdef CONFIG_SUNXI_SECURE_STORAGE
	if(argc == 1)
	{
		return sunxi_secure_storage_list();
	}
	if(argc == 2)
	{
		char buffer[4096];
		int ret, data_len;

		memset(buffer, 0, 4096);
		ret = sunxi_secure_storage_init();
		if(ret < 0)
		{
			printf("%s secure storage init err\n", __func__);

			return -1;
		}
		ret = sunxi_secure_object_read(argv[1], buffer, 4096, &data_len);
		if(ret < 0)
		{
			printf("private data %s is not exist\n", argv[1]);

			return -1;
		}
		printf("private data:\n");
		sunxi_dump(buffer, strlen((const char *)buffer));

		return 0;
	}
#endif
	return -1;
}

U_BOOT_CMD(
	pbread, CONFIG_SYS_MAXARGS, 1, do_read_from_boot,
	"read data from private data",
	"pread [name]"
	"NULL"
);
