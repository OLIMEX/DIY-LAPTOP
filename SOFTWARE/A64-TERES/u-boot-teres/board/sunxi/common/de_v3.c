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
#include <common.h>
#include <malloc.h>
#include <sys_config.h>
#include <sunxi_display2.h>
#include <bmp_layout.h>
#include <fdt_support.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SUNXI_DISPLAY

static __u32 screen_id = 0;
static __u32 disp_para = 0;
extern __s32 disp_delay_ms(__u32 ms);
extern long disp_ioctl(void *hd, unsigned int cmd, void *arg);

static int board_display_update_para_for_kernel(char *name, int value)
{
	int node;
	int ret = -1;

	node = fdt_path_offset(working_fdt,"disp");
	if (node < 0) {
		printf("%s:disp_fdt_nodeoffset %s fail\n", __func__,"disp");
		goto exit;
	}
	ret = fdt_setprop_u32(working_fdt, node, name, (uint32_t)value);
	if ( ret < 0)
		printf("fdt_setprop_u32 %s.%s(0x%x) fail.err code:%s\n", "disp", name, value,fdt_strerror(ret));
	else
		ret = 0;

exit:
	return ret;
}

int board_display_layer_request(void)
{
	gd->layer_hd = 0;
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
int board_display_layer_release(void)
{
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
int board_display_wait_lcd_open(void)
{
	int ret;
	int timedly = 5000;
	int check_time = timedly/50;
	uint arg[4] = { 0 };
	uint cmd = 0;

	cmd = DISP_LCD_CHECK_OPEN_FINISH;

	do
	{
    	ret = disp_ioctl(NULL, cmd, (void*)arg);
		if(ret == 1)		//open already
		{
			break;
		}
		else if(ret == -1)  //open falied
		{
			return -1;
		}
		__msdelay(50);
		check_time --;
		if(check_time <= 0)
		{
			return -1;
		}
	}
	while(1);

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
int board_display_wait_lcd_close(void)
{
	int ret;
	int timedly = 5000;
	int check_time = timedly/50;
	uint arg[4] = { 0 };
	uint cmd = 0;

	cmd = DISP_LCD_CHECK_CLOSE_FINISH;

	do
	{
    	ret = disp_ioctl(NULL, cmd, (void*)arg);
		if(ret == 1)		//open already
		{
			break;
		}
		else if(ret == -1)  //open falied
		{
			return -1;
		}
		__msdelay(50);
		check_time --;
		if(check_time <= 0)
		{
			return -1;
		}
	}
	while(1);

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
int board_display_set_exit_mode(int lcd_off_only)
{
	uint arg[4] = { 0 };
	uint cmd = 0;
	cmd = DISP_SET_EXIT_MODE;

	if(lcd_off_only)
	{
		arg[0] = DISP_EXIT_MODE_CLEAN_PARTLY;
		disp_ioctl(NULL, cmd, (void *)arg);
	}
	else
	{
		cmd = DISP_LCD_DISABLE;
		disp_ioctl(NULL, cmd, (void *)arg);
		board_display_wait_lcd_close();
	}

	return 0;
}
/*
*******************************************************************************
*                     board_display_layer_open
*
* Description:
*    打开图层
*
* Parameters:
*    Layer_hd    :  input. 图层句柄
*
* Return value:
*    0  :  成功
*   !0  :  失败
*
* note:
*    void
*
*******************************************************************************
*/
int board_display_layer_open(void)
{
	uint arg[4];
	struct disp_layer_config *config = (struct disp_layer_config *)gd->layer_para;

	arg[0] = screen_id;
	arg[1] = (unsigned long)config;
	arg[2] = 1;
	arg[3] = 0;

	disp_ioctl(NULL,DISP_LAYER_GET_CONFIG,(void*)arg);
	config->enable = 1;
	disp_ioctl(NULL,DISP_LAYER_SET_CONFIG,(void*)arg);

	return 0;
}


/*
*******************************************************************************
*                     board_display_layer_close
*
* Description:
*    关闭图层
*
* Parameters:
*    Layer_hd    :  input. 图层句柄
*
* Return value:
*    0  :  成功
*   !0  :  失败
*
* note:
*    void
*
*******************************************************************************
*/
int board_display_layer_close(void)
{
	uint arg[4];
	struct disp_layer_config *config = (struct disp_layer_config *)gd->layer_para;

	arg[0] = screen_id;
	arg[1] = (unsigned long)config;
	arg[2] = 1;
	arg[3] = 0;

	disp_ioctl(NULL,DISP_LAYER_GET_CONFIG,(void*)arg);
	config->enable = 0;
	disp_ioctl(NULL,DISP_LAYER_SET_CONFIG,(void*)arg);



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
int board_display_layer_para_set(void)
{
	uint arg[4];

	arg[0] = screen_id;
	arg[1] = gd->layer_para;
	arg[2] = 1;
	arg[3] = 0;
	disp_ioctl(NULL,DISP_LAYER_SET_CONFIG,(void*)arg);

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
int board_display_show_until_lcd_open(int display_source)
{
	printf("%s\n", __func__);
	if(!display_source)
	{
		board_display_wait_lcd_open();
	}
	board_display_layer_para_set();
	board_display_layer_open();

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
int board_display_show(int display_source)
{
	board_display_layer_para_set();
	board_display_layer_open();

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
int board_display_framebuffer_set(int width, int height, int bitcount, void *buffer)
{

	struct disp_layer_config *layer_para;
	uint screen_width, screen_height;
	uint arg[4];
	uint32_t full = 0;
	int nodeoffset;

	if(!gd->layer_para)
	{
		layer_para = (struct disp_layer_config *)malloc(sizeof(struct disp_layer_config));
		if(!layer_para)
		{
			tick_printf("sunxi display error: unable to malloc memory for layer\n");

			return -1;
		}
	}
	else
	{
		layer_para = (struct disp_layer_config *)gd->layer_para;
	}

	nodeoffset = fdt_path_offset(working_fdt,"boot_disp");
	if (nodeoffset >= 0)
	{
		if(fdt_getprop_u32(working_fdt,nodeoffset,"output_full",&full) < 0)
		{
			printf("fetch script data boot_disp.output_full fail\n");
		}
	}

	arg[0] = screen_id;
	screen_width = disp_ioctl(NULL, DISP_GET_SCN_WIDTH, (void*)arg);
	screen_height = disp_ioctl(NULL, DISP_GET_SCN_HEIGHT, (void*)arg);
	printf("screen_id =%d, screen_width =%d, screen_height =%d\n", screen_id, screen_width, screen_height);
	memset((void *)layer_para, 0, sizeof(struct disp_layer_config));
	layer_para->info.fb.addr[0]		= (uint)buffer;
	printf("frame buffer address %x\n", (uint)buffer);
	layer_para->channel = 1;
	layer_para->layer_id = 0;
	layer_para->info.fb.format		= (bitcount == 24)? DISP_FORMAT_RGB_888:DISP_FORMAT_ARGB_8888;
	layer_para->info.fb.size[0].width	= width;
	layer_para->info.fb.size[0].height	= height;
	layer_para->info.fb.crop.x	= 0;
	layer_para->info.fb.crop.y	= 0;
	layer_para->info.fb.crop.width	= ((unsigned long long)width) << 32;
	layer_para->info.fb.crop.height	= ((unsigned long long)height) << 32;
	layer_para->info.fb.flags = DISP_BF_NORMAL;
	layer_para->info.fb.scan = DISP_SCAN_PROGRESSIVE;
	debug("bitcount = %d\n", bitcount);
	layer_para->info.mode     = LAYER_MODE_BUFFER;
	layer_para->info.alpha_mode    = 1;
	layer_para->info.alpha_value   = 0xff;
	if(full) {
		layer_para->info.screen_win.x	= 0;
		layer_para->info.screen_win.y	= 0;
		layer_para->info.screen_win.width	= screen_width;
		layer_para->info.screen_win.height	= screen_height;
	} else {
		layer_para->info.screen_win.x	= (screen_width - width) / 2;
		layer_para->info.screen_win.y	= (screen_height - height) / 2;
		layer_para->info.screen_win.width	= width;
		layer_para->info.screen_win.height	= height;
	}
	layer_para->info.b_trd_out		= 0;
	layer_para->info.out_trd_mode 	= 0;
	gd->layer_para = (uint)layer_para;

	board_display_update_para_for_kernel("fb_base", (uint)buffer - sizeof(bmp_header_t));

	return 0;
}

void board_display_set_alpha_mode(int mode)
{
	if(!gd->layer_para)
	{
		return;
	}

	struct disp_layer_config *layer_para;
	layer_para = (struct disp_layer_config *)gd->layer_para;
	layer_para->info.alpha_mode = mode;

}

int board_display_framebuffer_change(void *buffer)
{
	return 0;
}

int board_display_device_open(void)
{

	int  value = 1;
	int  ret = 0;
	__u32 output_type = 0;
	__u32 output_mode = 0;
	__u32 auto_hpd = 0;
	__u32 err_count = 0;
	unsigned long arg[4] = {0};
	int node;

	debug("De_OpenDevice\n");

	node = fdt_path_offset(working_fdt,"boot_disp");
	if (node >= 0) {
		/* getproc output_disp, indicate which disp channel will be using */
		if (fdt_getprop_u32(working_fdt, node, "output_disp", (uint32_t*)&screen_id) < 0) {
			printf("fetch script data boot_disp.output_disp fail\n");
			err_count ++;
		} else
			printf("boot_disp.output_disp=%d\n", screen_id);

		/* getproc output_type, indicate which kind of device will be using */
		if (fdt_getprop_u32(working_fdt, node, "output_type", (uint32_t*)&value) < 0) {
			printf("fetch script data boot_disp.output_type fail\n");
			err_count ++;
		} else
			printf("boot_disp.output_type=%d\n", value);

		if(value == 0)
		{
			output_type = DISP_OUTPUT_TYPE_NONE;
		}
		else if(value == 1)
		{
			output_type = DISP_OUTPUT_TYPE_LCD;
		}
		else if(value == 2)
		{
			output_type = DISP_OUTPUT_TYPE_TV;
		}
		else if(value == 3)
		{
			output_type = DISP_OUTPUT_TYPE_HDMI;
		}
		else if(value == 4)
		{
			output_type = DISP_OUTPUT_TYPE_VGA;
		}
		else
		{
			printf("invalid output_type %d\n", value);
			return -1;
		}
		
		/* getproc output_mode, indicate which kind of mode will be output */
		if (fdt_getprop_u32(working_fdt, node, "output_mode", (uint32_t*)&output_mode) < 0) {
			printf("fetch script data boot_disp.output_mode fail\n");
			err_count ++;
		} else
			printf("boot_disp.output_mode=%d\n", output_mode);

		/* getproc auto_hpd, indicate output device decided by the hot plug status of device */
		if (fdt_getprop_u32(working_fdt, node, "auto_hpd", (uint32_t*)&auto_hpd) < 0) {
			printf("fetch script data boot_disp.auto_hpd fail\n");
			err_count ++;
		 } else
			printf("boot_disp.auto_hpd=%d\n", auto_hpd);
	} else
		err_count = 4;

	if(err_count >= 4)//no boot_disp config
	{
		output_type = DISP_OUTPUT_TYPE_LCD;
	}
	else//has boot_disp config
	{

	}
	printf("disp%d device type(%d) enable\n", screen_id, output_type);

	arg[0] = screen_id;
	arg[1] = output_type;
	arg[2] = output_mode;
	disp_ioctl(NULL, DISP_DEVICE_SWITCH, (void *)arg);



	disp_para = ((output_type << 8) | (output_mode)) << (screen_id*16);
	board_display_update_para_for_kernel("boot_disp", disp_para);

	return ret;
}

void board_display_setenv(char *data)	
{
	if (!data)
		return;

	sprintf(data, "%x", disp_para);
}

int borad_display_get_screen_width(void)
{
	unsigned long arg[4] = {0};
	arg[0] = screen_id;

	return disp_ioctl(NULL, DISP_GET_SCN_WIDTH, (void*)arg);

}

int borad_display_get_screen_height(void)
{
	unsigned long arg[4] = {0};

	arg[0] = screen_id;
	return disp_ioctl(NULL, DISP_GET_SCN_HEIGHT, (void*)arg);

}

#else
int board_display_layer_request(void)
{
	return 0;
}

int board_display_layer_release(void)
{
	return 0;
}
int board_display_wait_lcd_open(void)
{
	return 0;
}
int board_display_wait_lcd_close(void)
{
	return 0;
}
int board_display_set_exit_mode(int lcd_off_only)
{
	return 0;
}
int board_display_layer_open(void)
{
	return 0;
}

int board_display_layer_close(void)
{
	return 0;
}

int board_display_layer_para_set(void)
{
	return 0;
}

int board_display_show_until_lcd_open(int display_source)
{
	return 0;
}

int board_display_show(int display_source)
{
	return 0;
}

int board_display_framebuffer_set(int width, int height, int bitcount, void *buffer)
{
	return 0;
}

void board_display_set_alpha_mode(int mode)
{
	return ;
}

int board_display_framebuffer_change(void *buffer)
{
	return 0;
}
int board_display_device_open(void)
{
	return 0;
}

int borad_display_get_screen_width(void)
{
	return 0;
}

int borad_display_get_screen_height(void)
{
	return 0;
}

void board_display_setenv(char *data)	
{
	return;
}

#endif
