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
#include <stdarg.h>
#include <common.h>
#include  "sprite_char_i.h"
#include  "sfte/FontEngine.h"

#include  "../sprite_cartoon_color.h"

_ui_char_info_t  ui_char_info;
/*
**********************************************************************************************************************
*                                               _change_to_new_line
*
* Description:
*
* Arguments  :
*
*
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/
int sprite_uichar_init(int char_size)
{
	char  font_name[] = "font24.sft";

	memset(&ui_char_info, 0, sizeof(_ui_char_info_t));

	ui_char_info.word_size = 24;
	if((char_size == 32) || (sprite_source.screen_width >= 400))
	{
		font_name[4] = '3';
    	font_name[5] = '2';
    	ui_char_info.word_size = 32;
	}
    debug("ui_char_info.word_size is %d\n",ui_char_info.word_size);
    if(open_font((const char *)font_name, ui_char_info.word_size, sprite_source.screen_width,  (uchar *)sprite_source.screen_buf))   //打开字库
    {
    	printf("ui_char_info.word_size is %d\n",ui_char_info.word_size);
		printf("boot_ui_char: open font failed\n");
    	return -1;
    }

    ui_char_info.crt_addr 	  = sprite_source.screen_buf;
    ui_char_info.total_height = ((sprite_source.screen_size / 4) / (sprite_source.screen_width)) / (ui_char_info.word_size);   //总的高度，可以显示的行数

    ui_char_info.rest_screen_height  = sprite_source.screen_height/(ui_char_info.word_size);  	//记录屏幕的剩余高度，行数, 剩余1行不用
    ui_char_info.rest_display_height = ui_char_info.total_height;    						//记录显示的剩余高度，行数，剩余1行不用
    ui_char_info.rest_screen_width   = sprite_source.screen_width;                        		//剩余宽度等于显示宽度, 像素单位
    ui_char_info.current_height 	 = 0;
    ui_char_info.x              	 = 0;
    ui_char_info.y              	 = 0;

    return 0;
}
/*
**********************************************************************************************************************
*                                               _change_to_new_line
*
* Description:
*
* Arguments  :
*
*
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/
static int uichar_change_newline(void)
{
	int ret = 0;

    if(ui_char_info.rest_display_height <= 0)               //剩余宽度不够了，需要切换回到第一个屏幕
    {
		printf("boot ui char: not enough space to printf\n");

		ret = -1;
	}
	else
	{
	    ui_char_info.rest_screen_width   = sprite_source.screen_width;   //作为新的一行的长度,像素单位

	    ui_char_info.rest_screen_height  -= 1;           //剩余的屏幕高度, 行数
	    ui_char_info.rest_display_height -= 1;           //剩余的显示高度，行数
	    ui_char_info.current_height      += 1;           //当前高度变多一行
	    ui_char_info.x                    = 0;           //
	    ui_char_info.y                    = ui_char_info.current_height * ui_char_info.word_size;
	}

    return ret;
}

/*
**********************************************************************************************************************
*                                               _debug_display_putchar
*
* Description:
*
* Arguments  :  ch         :  需要打印的字符
*               rest_width :  当前行剩余的宽度
*
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/
static int uichar_putchar(__u8 ch)
{
    __s32 ret, width;

    ret = check_change_line(ui_char_info.x, ch);
    if(ret == -1)             //访问失败，当前字符不处理
    {
        return 0;
    }
    else if(ret == 0)        //访问成功，当前字符处理，但是不需要换行
    {
        ;
    }
    else if(ret == 1)        //访问成功，当前字符处理，需要换行
    {
        if(uichar_change_newline( ))
        {
        	ret = -1;
        }
    }
    width = draw_bmp_ulc(ui_char_info.x, ui_char_info.y, sprite_source.color);    //显示字符,返回当前显示字符的宽度，像素单位
    ui_char_info.x += width;
    ui_char_info.rest_screen_width -= width;                        //记录当前行剩余的像素
    //调用打印函数

	return ret;
}
/*
**********************************************************************************************************************
*                                               debug_display_putstr
*
* Description:
*
* Arguments  :
*
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/
static int uichar_putstr(const char * str, int length)
{
	int count = 0;

	while( *str != '\0' )
	{
		if(*str == '\n')
		{
            //需要换行的时候，自动切换到下一行开始进行显示
            //调用换行函数
            debug("get new line char!\n");
			if(uichar_change_newline( ))
	        {
	        	return -1;
	        }
		}
		else
		{
			if(uichar_putchar(*str))
			{
				return -1;
			}
		}

		str ++;
		count ++;
		if(count > length)
		{
			debug("count %d  length! %d\n",count,length);
			if(uichar_change_newline( ))
	        {
	        	return -1;
	        }
		}
	}

	return 0;
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
void sprite_uichar_printf(const char * str, ...)
{
	char string[512];
	int  base_color;
	int field_width;
	va_list args;

	memset(string,0,512);
	base_color = sprite_cartoon_ui_get_color();
	//base_color = SPRITE_CARTOON_GUI_YELLOW;
	base_color &= 0xffffff;
	sprite_cartoon_ui_set_color(base_color);

	va_start(args, str);
	field_width = vsprintf(string,str,args);
	va_end(args);

	if(field_width <=0 || field_width >= 512){
		base_color |= 0xff000000;
		sprite_cartoon_ui_set_color(base_color);
	}

	uichar_putstr(string, field_width);
}



