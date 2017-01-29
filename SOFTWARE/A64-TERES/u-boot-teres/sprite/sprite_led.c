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

#include <common.h>
#include <asm/arch/timer.h>
#include <sys_config.h>
#include <asm/arch/platform.h>
#include <fdt_support.h>

struct timer_list TIMER0;
static int   sprite_led_status;
static __u32 sprite_led_hd;

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
static void sprite_timer_func(void *p)
{
	gpio_write_one_pin_value(sprite_led_hd, sprite_led_status, "sprite_gpio0");
	sprite_led_status = (~sprite_led_status) & 0x01;
	
	//printf("sprite_time_func\n");
	del_timer(&TIMER0);
	add_timer(&TIMER0);
	return;
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
int sprite_led_init(void)
{
	user_gpio_set_t	gpio_init;
	int	ret = 0;
	int	delay = 0;
	int nodeoffset;
	
	sprite_led_status = 1;
	
	//正常工作时，灯闪烁的时间
	//ret = script_parser_fetch("card_boot", "sprite_work_delay", (void *)&delay, 1);

	delay = 0;
	nodeoffset =  fdt_path_offset(working_fdt,FDT_PATH_CARD_BOOT);
	if(nodeoffset >0)
	{
		fdt_getprop_u32(working_fdt, nodeoffset,"sprite_work_delay", (uint32_t*)&delay);
	}
	if(!delay)
	{
		delay = 500;
	}
	
	printf("try sprite_led_gpio config\n");
	memset(&gpio_init, 0, sizeof(user_gpio_set_t));

	//配置输出gpio口
	//ret = script_parser_fetch("card_boot", "sprite_gpio0", (void *)&gpio_init, sizeof(user_gpio_set_t)>>2);
	ret = fdt_get_one_gpio(FDT_PATH_CARD_BOOT, "sprite_gpio0",&gpio_init);
	if(!ret)
	{
		if(gpio_init.port)
		{
			sprite_led_hd = gpio_request(&gpio_init, 1);
			if(!sprite_led_hd)
			{
				printf("reuqest gpio for led failed\n");
				return 1;
			}
			
			TIMER0.data = (unsigned long)&TIMER0;
			TIMER0.expires = delay;
			TIMER0.function = sprite_timer_func;
			//init_timer(&TIMER0);
			add_timer(&TIMER0);
			
			printf("sprite_led_gpio start\n");
			return 0;
		}
	}
	return 0;
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
int sprite_led_exit(int status)
{
	//int ret;
	int delay;
	int nodeoffset;

	del_timer(&TIMER0);
	
	//出错的时候，led的闪烁加快
	if(status < 0)
	{
		delay = 0;
		nodeoffset =  fdt_path_offset(working_fdt,FDT_PATH_CARD_BOOT);
		if(nodeoffset >0)
		{
			fdt_getprop_u32(working_fdt, nodeoffset,"sprite_err_delay", (uint32_t*)&delay);
		}
		if(!delay)
		{
			delay = 100;
		}
		del_timer(&TIMER0);
		TIMER0.data = (unsigned long)&TIMER0;
		TIMER0.expires = delay;
		TIMER0.function = sprite_timer_func;
		//init_timer(&TIMER0);
		add_timer(&TIMER0);
	}

	return 0;
}