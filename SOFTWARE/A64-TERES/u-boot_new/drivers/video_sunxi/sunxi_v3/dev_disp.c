/* linux/drivers/video/sunxi/disp/dev_disp.c
 *
 * Copyright (c) 2013 Allwinnertech Co., Ltd.
 * Author: Tyle <tyle@allwinnertech.com>
 *
 * Display driver for sunxi platform
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include "dev_disp.h"

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

static disp_drv_info g_disp_drv;
static u32 init_flag;

#define MY_BYTE_ALIGN(x) ( ( (x + (4*1024-1)) >> 12) << 12)             /* alloc based on 4K byte */

//static u32 suspend_output_type[3] = {0,0,0};
static u32 suspend_status = 0;//0:normal; suspend_status&1 != 0:in early_suspend; suspend_status&2 != 0:in suspend;
//static u32 suspend_prestep = 0; //0:after early suspend; 1:after suspend; 2:after resume; 3 :after late resume

//uboot plat
static u32    lcd_flow_cnt[2] = {0};
static s8   lcd_op_finished[2] = {0};
static struct timer_list lcd_timer[2];
static s8   lcd_op_start[2] = {0};

#if defined (CONFIG_ARCH_SUN9IW1P1)
static unsigned int gbuffer[4096];
#endif
//static struct info_mm  g_disp_mm[10];
//static int g_disp_mm_sel = 0;

//static struct cdev *my_cdev;
//static dev_t devid ;
//static struct class *disp_class;
//struct device *display_dev;

//static u32 disp_print_cmd_level = 0;
static u32 disp_cmd_print = 0xffff;   //print cmd which eq disp_cmd_print

static u32 g_output_type = DISP_OUTPUT_TYPE_LCD;

static s32 copy_from_user(void *dest, void* src, u32 size)
{
    memcpy(dest, src, size);
	return 0;
}

static s32 copy_to_user(void *src, void* dest, u32 size)
{
    memcpy(dest, src, size);
	return 0;
}

static void drv_lcd_open_callback(void *parg)
{
    disp_lcd_flow *flow;
    u32 sel = (u32)parg;
    s32 i = lcd_flow_cnt[sel]++;

    flow = bsp_disp_lcd_get_open_flow(sel);

	if(i < flow->func_num)
    {
    	flow->func[i].func(sel);
        if(flow->func[i].delay == 0)
        {
            drv_lcd_open_callback((void*)sel);
        }
        else
        {
        	lcd_timer[sel].data = sel;
			lcd_timer[sel].expires = flow->func[i].delay;
			lcd_timer[sel].function = drv_lcd_open_callback;
			add_timer(&lcd_timer[sel]);
    	}
    }
    else if(i >= flow->func_num)
    {
        lcd_op_finished[sel] = 1;
    }
}


static s32 drv_lcd_enable(u32 sel)
{
	//if(bsp_disp_lcd_is_used(sel)) {
	//FIXME
	if(1) {
		lcd_flow_cnt[sel] = 0;
		lcd_op_finished[sel] = 0;
		lcd_op_start[sel] = 1;

		init_timer(&lcd_timer[sel]);

		drv_lcd_open_callback((void*)sel);
	}
    return 0;
}

static s8 drv_lcd_check_open_finished(u32 sel)
{
	//if(bsp_disp_lcd_is_used(sel) && (lcd_op_start[sel] == 1))
	//FIXME
	if((lcd_op_start[sel] == 1))
	{
	    if(lcd_op_finished[sel])
	    {
	        del_timer(&lcd_timer[sel]);
            lcd_op_start[sel] = 0;
	    }
		return lcd_op_finished[sel];
	}

	return 1;
}

static void drv_lcd_close_callback(void *parg)
{
    disp_lcd_flow *flow;
    u32 sel = (__u32)parg;
    s32 i = lcd_flow_cnt[sel]++;

    flow = bsp_disp_lcd_get_close_flow(sel);

    if(i < flow->func_num)
    {
    	flow->func[i].func(sel);
        if(flow->func[i].delay == 0)
        {
            drv_lcd_close_callback((void*)sel);
        }
        else
        {
            lcd_timer[sel].data = sel;
			lcd_timer[sel].expires = flow->func[i].delay;
			lcd_timer[sel].function = drv_lcd_close_callback;
			add_timer(&lcd_timer[sel]);
        }
    }
    else if(i == flow->func_num)
    {
        lcd_op_finished[sel] = 1;
    }
}

static s32 drv_lcd_disable(u32 sel)
{
    //if(bsp_disp_lcd_is_used(sel))
    //FIXME
    if(1)
    {
        lcd_flow_cnt[sel] = 0;
        lcd_op_finished[sel] = 0;
        lcd_op_start[sel] = 1;

        init_timer(&lcd_timer[sel]);

        drv_lcd_close_callback((void*)sel);
    }

    return 0;
}

static s8 drv_lcd_check_close_finished(u32 sel)
{
    //if(bsp_disp_lcd_is_used(sel) && (lcd_op_start[sel] == 1))
    //FIXME
    if((lcd_op_start[sel] == 1))
    {
        if(lcd_op_finished[sel])
        {
            del_timer(&lcd_timer[sel]);
            lcd_op_start[sel] = 0;
        }
        return lcd_op_finished[sel];
    }
    return 1;
}

#if defined(SUPPORT_HDMI)
s32 disp_set_hdmi_func(disp_hdmi_func * func)
{
	return bsp_disp_set_hdmi_func(func);
}
#endif

#if defined(SUPPORT_TV)
s32 disp_set_tv_func(disp_tv_func * func)
{
	return bsp_disp_set_tv_func(func);
}
#endif



extern s32 bsp_disp_delay_ms(u32 ms);

extern s32 bsp_disp_delay_us(u32 us);

extern __s32 Hdmi_init(void);
extern int sunxi_board_shutdown(void);

s32 drv_disp_check_spec(void)
{
	unsigned int lcd_used = 0;
	unsigned int lcd_x = 0, lcd_y = 0;
	int ret = 0;
	int value = 0;
	int limit_w = 0xffff, limit_h = 0xffff;

#if defined(CONFIG_ARCH_SUN8IW6)
	limit_w = 2048;
	limit_h = 1536;
#endif
	ret = disp_sys_script_get_item("lcd0_para", "lcd_used", &value, 1);
	if(ret == 1)
	{
	  lcd_used = value;
	}

	if(1 == lcd_used) {
		ret = disp_sys_script_get_item("lcd0_para", "lcd_x", &value, 1);
	  if(ret == 1)
	  {
	      lcd_x = value;
	  }

	  ret = disp_sys_script_get_item("lcd0_para", "lcd_y", &value, 1);
	  if(ret == 1)
	  {
	      lcd_y = value;
	  }

		if(((lcd_x > limit_w) && (lcd_y > limit_h))
			|| ((lcd_x > limit_h) && (lcd_y > limit_w))) {
			printf("fatal err: cannot support lcd with resolution(%d*%d) larger than %d*%d, the system will shut down!\n",
				lcd_x, lcd_y,limit_w,limit_h);
			sunxi_board_shutdown();
		}

	}

	return 0;
}

s32 drv_disp_init(void)
{
#ifdef CONFIG_FPGA
    return 0;
#else
  disp_bsp_init_para para;
  int disp, num_screens;

	drv_disp_check_spec();
	sunxi_pwm_init();
	disp_sys_clk_init();

	memset(&para, 0, sizeof(disp_bsp_init_para));

	para.reg_base[DISP_MOD_DE]    = DE_BASE;
	para.reg_size[DISP_MOD_DE]    = DE_SIZE;
	para.reg_base[DISP_MOD_LCD0]   = LCD0_BASE;
	para.reg_size[DISP_MOD_LCD0]   = 0x3fc;
#ifdef DISP_DEVICE_NUM
	#if DISP_DEVICE_NUM == 2
	para.reg_base[DISP_MOD_LCD1]   = LCD1_BASE;
	para.reg_size[DISP_MOD_LCD1]   = 0x3fc;
#endif
#else
#	error "DEVICE_NUM undefined!"
#endif

#ifdef SUPPORT_DSI
	para.reg_base[DISP_MOD_DSI0]   = MIPI_DSI0_BASE;
	para.reg_size[DISP_MOD_DSI0]   = 0x2fc;
#endif

	para.irq_no[DISP_MOD_DE]         = AW_IRQ_DEIRQ0;
	para.irq_no[DISP_MOD_LCD0]        = AW_IRQ_LCD0;
#if defined(DISP_DEVICE_NUM)
	#if DISP_DEVICE_NUM == 2
	para.irq_no[DISP_MOD_LCD1]        = AW_IRQ_LCD1;
	#endif
#else
#	error "DEVICE_NUM undefined!"
#endif
#if defined(SUPPORT_DSI)
	para.irq_no[DISP_MOD_DSI0]        = AW_IRQ_MIPIDSI;
#endif

	memset(&g_disp_drv, 0, sizeof(disp_drv_info));

	bsp_disp_init(&para);
	num_screens = bsp_disp_feat_get_num_screens();
	for(disp=0; disp<num_screens; disp++) {
		g_disp_drv.mgr[disp] = disp_get_layer_manager(disp);
	}
#if defined(SUPPORT_HDMI)
	Hdmi_init();
#endif
#if defined(SUPPORT_TV)
	tv_init();
#endif

	bsp_disp_open();

	lcd_init();
	//gm7121_module_init();

	init_flag = 1;

	printf("DRV_DISP_Init end\n");
	return 0;
#endif
}

s32 drv_disp_exit(void)
{
	printf("%s\n", __func__);
	if(init_flag == 1) {
		init_flag = 0;
		bsp_disp_close();
		bsp_disp_exit(g_disp_drv.exit_mode);
	}
	return 0;
}

int sunxi_disp_get_source_ops(struct sunxi_disp_source_ops *src_ops)
{
	src_ops->sunxi_lcd_set_panel_funs = bsp_disp_lcd_set_panel_funs;
	src_ops->sunxi_lcd_delay_ms = disp_delay_ms;
	src_ops->sunxi_lcd_delay_us = disp_delay_us;
	src_ops->sunxi_lcd_backlight_enable = bsp_disp_lcd_backlight_enable;
	src_ops->sunxi_lcd_backlight_disable = bsp_disp_lcd_backlight_disable;
	src_ops->sunxi_lcd_pwm_enable = bsp_disp_lcd_pwm_enable;
	src_ops->sunxi_lcd_pwm_disable = bsp_disp_lcd_pwm_disable;
	src_ops->sunxi_lcd_power_enable = bsp_disp_lcd_power_enable;
	src_ops->sunxi_lcd_power_disable = bsp_disp_lcd_power_disable;
	src_ops->sunxi_lcd_tcon_enable = bsp_disp_lcd_tcon_enable;
	src_ops->sunxi_lcd_tcon_disable = bsp_disp_lcd_tcon_disable;
	src_ops->sunxi_lcd_pin_cfg = bsp_disp_lcd_pin_cfg;
	src_ops->sunxi_lcd_gpio_set_value = bsp_disp_lcd_gpio_set_value;
	src_ops->sunxi_lcd_gpio_set_direction = bsp_disp_lcd_gpio_set_direction;
#ifdef SUPPORT_DSI
	src_ops->sunxi_lcd_dsi_dcs_write = dsi_dcs_wr;
	src_ops->sunxi_lcd_dsi_gen_write = dsi_gen_wr;
	src_ops->sunxi_lcd_dsi_clk_enable = dsi_clk_enable;
#endif
	return 0;
}

long disp_ioctl(void *hd, unsigned int cmd, void *arg)
{
	unsigned long karg[4];
	unsigned long ubuffer[4] = {0};
	s32 ret = 0;
	int num_screens = 2;
	struct disp_manager *mgr = NULL;
	struct disp_device *dispdev = NULL;
	struct disp_enhance *enhance = NULL;
	struct disp_smbl *smbl = NULL;
	struct disp_capture *cptr = NULL;

	num_screens = bsp_disp_feat_get_num_screens();

	if (copy_from_user((void*)karg,(void*)arg, 4*sizeof(unsigned long))) {
		__wrn("copy_from_user fail\n");
		return -1;
	}

	ubuffer[0] = *(unsigned long*)karg;
	ubuffer[1] = (*(unsigned long*)(karg+1));
	ubuffer[2] = (*(unsigned long*)(karg+2));
	ubuffer[3] = (*(unsigned long*)(karg+3));

	if(ubuffer[0] < num_screens)
		mgr = g_disp_drv.mgr[ubuffer[0]];
	if(mgr) {
		dispdev = mgr->device;
		enhance = mgr->enhance;
		smbl = mgr->smbl;
		cptr = mgr->cptr;
	}

	if(cmd < DISP_FB_REQUEST)	{
		if(ubuffer[0] >= num_screens) {
			__wrn("para err in disp_ioctl, cmd = 0x%x,screen id = %d\n", cmd, (int)ubuffer[0]);
			return -1;
		}
	}
	if(DISPLAY_DEEP_SLEEP == suspend_status) {
		__wrn("ioctl:%x fail when in suspend!\n", cmd);
		return -1;
	}

	if(cmd == disp_cmd_print) {
		//__wrn("cmd:0x%x,%ld,%ld\n",cmd, ubuffer[0], ubuffer[1]);
	}

	switch(cmd)	{
	//----disp global----
	case DISP_SET_BKCOLOR:
	{
		disp_color para;

		if(copy_from_user(&para, (void*)ubuffer[1],sizeof(disp_color)))	{
			__wrn("copy_from_user fail\n");
			return  -1;
		}
		if(mgr && (mgr->set_back_color != NULL))
			ret = mgr->set_back_color(mgr, &para);
		break;
	}

	case DISP_GET_OUTPUT_TYPE:
	{
		if(mgr && mgr->device)
			ret = mgr->device->type;
		break;
	}

	case DISP_GET_SCN_WIDTH:
	{
		unsigned int width = 0,height = 0;
		if(mgr && mgr->device && mgr->device->get_resolution) {
			mgr->device->get_resolution(mgr->device, &width, &height);
		}
		ret = width;
		break;
	}

	case DISP_GET_SCN_HEIGHT:
	{
		unsigned int width = 0,height = 0;
		if(mgr && mgr->device && mgr->device->get_resolution) {
			mgr->device->get_resolution(mgr->device, &width, &height);
		}
		ret = height;
		break;
	}

	case DISP_VSYNC_EVENT_EN:
	{
		ret = bsp_disp_vsync_event_enable(ubuffer[0], ubuffer[1]);
		break;
	}

	case DISP_SHADOW_PROTECT:
	{
		ret = bsp_disp_shadow_protect(ubuffer[0], ubuffer[1]);
		break;
	}

	case DISP_BLANK:
	{
		if(ubuffer[1]) {
			if(dispdev && dispdev->disable)
				ret = dispdev->disable(dispdev);
		} else {
			if(dispdev && dispdev->enable)
				ret = dispdev->enable(dispdev);
		}
		break;
	}

	case DISP_DEVICE_SWITCH:
	{
		if(ubuffer[1] == (unsigned long)DISP_OUTPUT_TYPE_LCD)
			ret = drv_lcd_enable(ubuffer[0]);
		else
			ret = bsp_disp_device_switch(ubuffer[0], (disp_output_type)ubuffer[1], (disp_tv_mode)ubuffer[2]);
		break;
	}

	//----layer----
	case DISP_LAYER_SET_CONFIG:
	{
		disp_layer_config para;

		if(copy_from_user(&para, (void *)ubuffer[1],sizeof(disp_layer_config)))	{
			__wrn("copy_from_user fail\n");
			return  -1;
		}
		if(mgr && mgr->set_layer_config)
			ret = mgr->set_layer_config(mgr, &para, ubuffer[2]);
		break;
	}

	case DISP_LAYER_GET_CONFIG:
	{
		disp_layer_config para;

		if(copy_from_user(&para, (void *)ubuffer[1],sizeof(disp_layer_config)))	{
			__wrn("copy_from_user fail\n");
			return  -1;
		}
		if(mgr && mgr->get_layer_config)
			ret = mgr->get_layer_config(mgr, &para, ubuffer[2]);
		if(copy_to_user(&para, (void *)ubuffer[1], sizeof(disp_layer_config)))	{
			__wrn("copy_to_user fail\n");
			return  -1;
		}
		break;
	}

	//----lcd----
	case DISP_LCD_SET_BRIGHTNESS:
	{
		if(dispdev && (DISP_OUTPUT_TYPE_LCD == dispdev->type)) {
			ret = dispdev->set_bright(dispdev, ubuffer[1]);
		}
		break;
	}

	case DISP_LCD_GET_BRIGHTNESS:
	{
		if(dispdev && (DISP_OUTPUT_TYPE_LCD == dispdev->type)) {
			ret = dispdev->get_bright(dispdev);
		}
		break;
	}


	case DISP_HDMI_GET_HPD_STATUS:
		if(DISPLAY_NORMAL == suspend_status) {
			ret = bsp_disp_hdmi_get_hpd_status(ubuffer[0]);
		}	else {
			ret = 0;
		}
		break;

	case DISP_HDMI_SUPPORT_MODE:
		ret = bsp_disp_hdmi_check_support_mode(ubuffer[0], ubuffer[1]);
		break;
#if defined (CONFIG_ARCH_SUN8IW7)
	case DISP_TV_GET_HPD_STATUS:
	if(DISPLAY_NORMAL == suspend_status) {
		ret = bsp_disp_tv_get_hpd_status(ubuffer[0]);
	}	else {
		ret = 0;
	}
	break;
#endif

#if 0
	case DISP_CMD_HDMI_SET_SRC:
		ret = bsp_disp_hdmi_set_src(ubuffer[0], (disp_lcd_src)ubuffer[1]);
		break;

	//----framebuffer----
	case DISP_CMD_FB_REQUEST:
	{
		disp_fb_create_info para;

		if(copy_from_user(&para, (void *)ubuffer[1],sizeof(disp_fb_create_info))) {
			__wrn("copy_from_user fail\n");
			return  -1;
		}
		ret = Display_Fb_Request(ubuffer[0], &para);
		break;
	}

	case DISP_CMD_FB_RELEASE:
	ret = Display_Fb_Release(ubuffer[0]);
	break;

	case DISP_CMD_FB_GET_PARA:
	{
		disp_fb_create_info para;

		ret = Display_Fb_get_para(ubuffer[0], &para);
		if(copy_to_user((void *)ubuffer[1],&para, sizeof(disp_fb_create_info))) {
			__wrn("copy_to_user fail\n");
			return  -1;
		}
		break;
	}

	case DISP_CMD_GET_DISP_INIT_PARA:
	{
		disp_init_para para;

		ret = Display_get_disp_init_para(&para);
		if(copy_to_user((void *)ubuffer[0],&para, sizeof(disp_init_para)))	{
			__wrn("copy_to_user fail\n");
			return  -1;
		}
		break;
	}

#endif
		//----enhance----
	case DISP_ENHANCE_ENABLE:
	{
		if(enhance && enhance->enable)
			ret = enhance->enable(enhance);
		break;
	}

	case DISP_ENHANCE_DISABLE:
	{
		if(enhance && enhance->disable)
			ret = enhance->disable(enhance);
		break;
	}

	//---smart backlight --
	case DISP_SMBL_ENABLE:
	{
		if(smbl && smbl->enable)
			ret = smbl->enable(smbl);
		break;
	}

	case DISP_SMBL_DISABLE:
	{
		if(smbl && smbl->disable)
			ret = smbl->disable(smbl);
		break;
	}

	case DISP_SMBL_SET_WINDOW:
	{
		disp_rect rect;

		if(copy_from_user(&rect, (void *)ubuffer[1],sizeof(disp_rect)))	{
			__wrn("copy_from_user fail\n");
			return  -1;
		}
		if(smbl && smbl->set_window)
			ret = smbl->set_window(smbl, &rect);
		break;
	}

	//---capture --
	case DISP_CAPTURE_START:
	{
		if(cptr && cptr->start)
			ret = cptr->start(cptr);
		break;
	}

	case DISP_CAPTURE_STOP:
	{
		if(cptr && cptr->stop)
			ret = cptr->stop(cptr);
		break;
	}

	case DISP_CAPTURE_COMMIT:
	{
		disp_capture_info info;

		if(copy_from_user(&info, (void *)ubuffer[1],sizeof(disp_capture_info)))	{
			__wrn("copy_from_user fail\n");
			return  -1;
		}
		if(cptr && cptr->commmit)
			ret = cptr->commmit(cptr, &info);
		break;
	}

#if defined(CONFIG_ARCH_SUN9IW1P1)

#if 0
	//----for test----
	case DISP_CMD_MEM_REQUEST:
		ret =  disp_mem_request(ubuffer[0],ubuffer[1]);
		break;

	case DISP_CMD_MEM_RELEASE:
		ret =  disp_mem_release(ubuffer[0]);
		break;

	case DISP_CMD_MEM_SELIDX:
		g_disp_mm_sel = ubuffer[0];
		break;

	case DISP_CMD_MEM_GETADR:
		ret = g_disp_mm[ubuffer[0]].mem_start;
		break;

//	case DISP_CMD_PRINT_REG:
//		ret = bsp_disp_print_reg(1, ubuffer[0], 0);
//		break;
#endif
#endif

	case DISP_SET_EXIT_MODE:
        ret = g_disp_drv.exit_mode = ubuffer[0];
		break;

	case DISP_LCD_CHECK_OPEN_FINISH:
		ret = drv_lcd_check_open_finished(ubuffer[0]);
		break;

	case DISP_LCD_CHECK_CLOSE_FINISH:
		ret = drv_lcd_check_close_finished(ubuffer[0]);
		break;

	default:
		break;
	}

  return ret;
}

#define  DELAY_ONCE_TIME   (50)

s32 drv_disp_standby(u32 cmd, void *pArg)
{
	s32 ret;
	s32 timedly = 5000;
	s32 check_time = timedly/DELAY_ONCE_TIME;

	if(cmd == BOOT_MOD_ENTER_STANDBY)
	{
	    if(g_output_type == DISP_OUTPUT_TYPE_HDMI)
	    {
		}
		else
        {
            drv_lcd_disable(0);
		}
		do
		{
			ret = drv_lcd_check_close_finished(0);
			if(ret == 1)
			{
				break;
			}
			else if(ret == -1)
			{
				return -1;
			}
			__msdelay(DELAY_ONCE_TIME);
			check_time --;
			if(check_time <= 0)
			{
				return -1;
			}
		}
		while(1);

		return 0;
	}
	else if(cmd == BOOT_MOD_EXIT_STANDBY)
	{
		if(g_output_type == DISP_OUTPUT_TYPE_HDMI)
		{
		}
		else
		{
			drv_lcd_enable(0);
        }

		do
		{
			ret = drv_lcd_check_open_finished(0);
			if(ret == 1)
			{
				break;
			}
			else if(ret == -1)
			{
				return -1;
			}
			__msdelay(DELAY_ONCE_TIME);
			check_time --;
			if(check_time <= 0)
			{
				return -1;
			}
		}
		while(1);

		return 0;
	}

	return -1;
}
