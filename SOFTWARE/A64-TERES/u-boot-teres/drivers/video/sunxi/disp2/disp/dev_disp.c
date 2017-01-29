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

disp_drv_info g_disp_drv;

#define MY_BYTE_ALIGN(x) ( ( (x + (4*1024-1)) >> 12) << 12)             /* alloc based on 4K byte */

static u32 suspend_output_type[2] = {0,0};
static u32 suspend_status = 0;//0:normal; suspend_status&1 != 0:in early_suspend; suspend_status&2 != 0:in suspend;
//static u32 suspend_prestep = 3; //0:after early suspend; 1:after suspend; 2:after resume; 3 :after late resume
static u32 power_status_init = 0;

//uboot plat
static u32    lcd_flow_cnt[2] = {0};
static s8   lcd_op_finished[2] = {0};
static struct timer_list lcd_timer[2];
static s8   lcd_op_start[2] = {0};

static u32 DISP_print = 0xffff;   //print cmd which eq DISP_print
static u32 g_output_type = DISP_OUTPUT_TYPE_LCD;

uintptr_t disp_getprop_regbase(char *main_name, char *sub_name, u32 index)
{
	char compat[32];
	u32 len = 0;
	int node;
	int ret = -1;
	int value[32] = {0};
	uintptr_t reg_base = 0;

	len = sprintf(compat, "%s", main_name);
	if (len > 32)
		__wrn("size of mian_name is out of range\n");

	//node = fdt_path_offset(working_fdt,compat);
	node = disp_fdt_nodeoffset(compat);
	if (node < 0) {
		__wrn("fdt_path_offset %s fail\n", compat);
		goto exit;
	}

	ret = fdt_getprop_u32(working_fdt, node, sub_name, (uint32_t*)value);
	if (0 > ret)
		__wrn("fdt_getprop_u32 %s.%s fail\n", main_name, sub_name);
	else {
		reg_base = value[index * 4] + value[index * 4 + 1];
	}

exit:
	return reg_base;
}

u32 disp_getprop_irq(char *main_name, char *sub_name, u32 index)
{
	char compat[32];
	u32 len = 0;
	int node;
	int ret = -1;
	int value[32] = {0};
	u32 irq = 0;

	len = sprintf(compat, "%s", main_name);
	if (len > 32)
		__wrn("size of mian_name is out of range\n");

	//node = fdt_path_offset(working_fdt,compat);
	node = disp_fdt_nodeoffset(compat);
	if (node < 0) {
		__wrn("fdt_path_offset %s fail\n", compat);
		goto exit;
	}

	ret = fdt_getprop_u32(working_fdt, node, sub_name, (uint32_t*)value);
	if (0 > ret)
		__wrn("fdt_getprop_u32 %s.%s fail\n", main_name, sub_name);
	else {
		irq = value[index * 3 + 1];
		if (0 == value[index * 3])
			irq += 32;
	}

exit:
	return irq;
}

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

	if (i < flow->func_num)
	{
		//printf("%s, step=%d, number=%d, todo\n", __func__, i, flow->func_num);
		flow->func[i].func(sel);
		//printf("%s, step=%d, number=%d, finish\n", __func__, i, flow->func_num);
		if (flow->func[i].delay == 0) {
			drv_lcd_open_callback((void*)sel);
		}	else {
			//printf("%s, setnext step timer, delay=%d\n", __func__, flow->func[i].delay);
			lcd_timer[sel].data = sel;
			lcd_timer[sel].expires = flow->func[i].delay;
			lcd_timer[sel].function = drv_lcd_open_callback;
			add_timer(&lcd_timer[sel]);
		}
	}	else if (i >= flow->func_num) {
		lcd_op_finished[sel] = 1;
	}
}


static s32 drv_lcd_enable(u32 sel)
{
	lcd_flow_cnt[sel] = 0;
	lcd_op_finished[sel] = 0;
	lcd_op_start[sel] = 1;

	init_timer(&lcd_timer[sel]);

	drv_lcd_open_callback((void*)sel);

    return 0;
}

static s8 drv_lcd_check_open_finished(u32 sel)
{
	if ((lcd_op_start[sel] == 1)) {
		if (lcd_op_finished[sel]) {
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

	if (i < flow->func_num) {
		flow->func[i].func(sel);
		if (flow->func[i].delay == 0)
			drv_lcd_close_callback((void*)sel);
		else {
			lcd_timer[sel].data = sel;
			lcd_timer[sel].expires = flow->func[i].delay;
			lcd_timer[sel].function = drv_lcd_close_callback;
			add_timer(&lcd_timer[sel]);
		}
	}	else if (i == flow->func_num) {
		lcd_op_finished[sel] = 1;
	}
}

static s32 drv_lcd_disable(u32 sel)
{
	lcd_flow_cnt[sel] = 0;
	lcd_op_finished[sel] = 0;
	lcd_op_start[sel] = 1;

	init_timer(&lcd_timer[sel]);

	drv_lcd_close_callback((void*)sel);

	return 0;
}

static s8 drv_lcd_check_close_finished(u32 sel)
{
	if ((lcd_op_start[sel] == 1)) {
		if (lcd_op_finished[sel])	{
			del_timer(&lcd_timer[sel]);
			lcd_op_start[sel] = 0;
		}
		return lcd_op_finished[sel];
	}
	return 1;
}

#if defined(SUPPORT_HDMI)
s32 disp_set_hdmi_func(struct disp_device_func * func)
{
	return bsp_disp_set_hdmi_func(func);
}

s32 disp_set_hdmi_detect(bool hpd)
{
	return bsp_disp_hdmi_set_detect(hpd);
}
EXPORT_SYMBOL(disp_set_hdmi_detect);
#endif

#if defined(SUPPORT_TV)
s32 disp_tv_register(struct disp_tv_func * func)
{
	return bsp_disp_tv_register(func);
}
EXPORT_SYMBOL(disp_tv_register);
#endif

extern int sunxi_board_shutdown(void);
static s32 drv_disp_check_spec(void)
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
	ret = disp_sys_script_get_item(FDT_LCD0_PATH, "lcd_used", &value, 1);
	if (ret == 1)
	  lcd_used = value;

	if (1 == lcd_used) {
		ret = disp_sys_script_get_item(FDT_LCD0_PATH, "lcd_x", &value, 1);
	  if (ret == 1)
	      lcd_x = value;

	  ret = disp_sys_script_get_item(FDT_LCD0_PATH, "lcd_y", &value, 1);
	  if (ret == 1)
	      lcd_y = value;

		if (((lcd_x > limit_w) && (lcd_y > limit_h)) ||
				((lcd_x > limit_h) && (lcd_y > limit_w))) {
			printf("fatal err: cannot support lcd with resolution(%d*%d) larger than %d*%d, the system will shut down!\n",
				lcd_x, lcd_y,limit_w,limit_h);
			sunxi_board_shutdown();
		}

	}

	return 0;
}

extern __s32 hdmi_init(void);
extern int tv_ac200_init(void);
extern void disp_fdt_init(void);

s32 drv_disp_init(void)
{
	disp_bsp_init_para *para;
	int disp, num_screens;
	int i;
	int ret = 0;

	printf("%s\n", __func__);
	disp_fdt_init();
	//for_test
	clk_init();
	/* check if the resolution of lcd supported */
	drv_disp_check_spec();

	/* pwm init */
	pwm_init();

	memset(&g_disp_drv, 0, sizeof(disp_drv_info));
	para = &g_disp_drv.para;

	/* iomap */
	para->reg_base[DISP_MOD_DE] = disp_getprop_regbase(FDT_DISP_PATH, "reg", 0);
	if (!para->reg_base[DISP_MOD_DE]) {
		__wrn("unable to map de registers\n");
		ret = -EINVAL;
		goto exit;
	}

	para->reg_base[DISP_MOD_LCD0] = disp_getprop_regbase(FDT_DISP_PATH, "reg", 1);
	if (!para->reg_base[DISP_MOD_LCD0]) {
		__wrn("unable to map lcd registers\n");
		ret = -EINVAL;
		goto exit;
	}

#if defined(SUPPORT_DSI)
	para->reg_base[DISP_MOD_DSI0] = disp_getprop_regbase(FDT_DISP_PATH, "reg", 2);
	if (!para->reg_base[DISP_MOD_DSI0]) {
		__wrn("unable to map dsi registers\n");
		ret = -EINVAL;
		goto exit;
	}
#endif

	/* parse and map irq */
	for (i=0; i<DEVICE_NUM; i++) {
		para->irq_no[DISP_MOD_LCD0 + i] = disp_getprop_irq(FDT_DISP_PATH, "interrupts", i);
		if (!para->irq_no[DISP_MOD_LCD0 + i]) {
			__wrn("irq_of_parse_and_map irq %d fail for lcd%d\n", i, i);
		}
	}
#if defined(SUPPORT_DSI)
	para->irq_no[DISP_MOD_DSI0] = disp_getprop_irq(FDT_DISP_PATH, "interrupts", i);
	if (!para->irq_no[DISP_MOD_DSI0]) {
		__wrn("irq_of_parse_and_map irq %d fail for dsi\n", i);
	}
#endif

	/* get clk for display modes */
	para->mclk[DISP_MOD_DE] = clk_get(NULL, "de");
	if (IS_ERR(para->mclk[DISP_MOD_DE])) {
		__wrn("fail to get clk for de\n");
	}
	para->mclk[DISP_MOD_LCD0] = clk_get(NULL, "tcon0");
	if (IS_ERR(para->mclk[DISP_MOD_LCD0])) {
		__wrn("fail to get clk for lcd0\n");
	}
	para->mclk[DISP_MOD_LCD1] = clk_get(NULL, "tcon1");
	if (IS_ERR(para->mclk[DISP_MOD_LCD1])) {
		__wrn("fail to get clk for lcd1\n");
	}
	para->mclk[DISP_MOD_LVDS] = clk_get(NULL, "lvds");
	if (IS_ERR(para->mclk[DISP_MOD_LVDS])) {
		__wrn("fail to get clk for lvds\n");
	}
	para->mclk[DISP_MOD_DSI0] = clk_get(NULL, "mipidsi");;
	if (IS_ERR(para->mclk[DISP_MOD_DSI0])) {
		__wrn("fail to get clk for dsi\n");
	}

	/* FIXME: set clk parent for all clk of dipslay modes
		should remove when clock supoort dts config
	*/
	{
		struct clk* pll;

		pll = clk_get(NULL, "pll_mipi");
		if (pll) {
			ret = clk_set_parent(para->mclk[DISP_MOD_LCD0], pll);
			if (0 != ret) {
				printf("clk_set parent tcon0 fail\n");
			}
		} else
			printf("clk(pll_mipi) get fail\n");
		clk_put(pll);

		pll = clk_get(NULL, "pll_video0");
		if (pll) {
			ret = clk_set_parent(para->mclk[DISP_MOD_LCD1], pll);
			if (0 != ret) {
				printf("clk_set parent tcon1 fail\n");
			}
			ret = clk_set_parent(para->mclk[DISP_MOD_LCD0]->parent, pll);
			if (0 != ret) {
				printf("clk_set parent pll_mipi fail\n");
			}
		} else
			printf("clk(pll_video0) get fail\n");
		clk_put(pll);

		pll = clk_get(NULL, "pll_de");
		if (pll) {
			ret = clk_set_parent(para->mclk[DISP_MOD_DE], pll);
			if (0 != ret) {
				printf("clk_set parent de fail\n");
			}
		} else
			printf("clk(pll_de) get fail\n");
		clk_put(pll);
	}

	bsp_disp_init(para);
	num_screens = bsp_disp_feat_get_num_screens();
	for (disp=0; disp<num_screens; disp++) {
		g_disp_drv.mgr[disp] = disp_get_layer_manager(disp);
	}
	lcd_init();
#if defined(SUPPORT_HDMI)
	hdmi_init();
#endif
#if defined(SUPPORT_TV)
	tv_init();
#endif

#if defined(CONFIG_USE_AC200)
	tv_ac200_init();
#endif
	bsp_disp_open();

	g_disp_drv.inited = true;

	printf("%s finish\n", __func__);

exit:
	return ret;
}

s32 drv_disp_exit(void)
{
	printf("%s\n", __func__);
	if (g_disp_drv.inited == true) {
		g_disp_drv.inited = false;
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

static int disp_blank(bool blank)
{
	u32 screen_id = 0;
	int num_screens;
	struct disp_manager *mgr = NULL;

	num_screens = bsp_disp_feat_get_num_screens();

	for (screen_id=0; screen_id<num_screens; screen_id++) {
		mgr = g_disp_drv.mgr[screen_id];
		if (!mgr || !mgr->device)
			continue;

		if (mgr->blank)
			mgr->blank(mgr, blank);
	}

	return 0;
}

long disp_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
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

	if (false == g_disp_drv.inited) {
		printf("%s, display not init yet\n", __func__);
		return -1;
	}
	num_screens = bsp_disp_feat_get_num_screens();

	if (copy_from_user((void*)karg,(void __user*)arg,4*sizeof(unsigned long))) {
		__wrn("copy_from_user fail\n");
		return -EFAULT;
	}

	ubuffer[0] = *(unsigned long*)karg;
	ubuffer[1] = (*(unsigned long*)(karg+1));
	ubuffer[2] = (*(unsigned long*)(karg+2));
	ubuffer[3] = (*(unsigned long*)(karg+3));

	if (ubuffer[0] < num_screens)
		mgr = g_disp_drv.mgr[ubuffer[0]];
	if (mgr) {
		dispdev = mgr->device;
		enhance = mgr->enhance;
		smbl = mgr->smbl;
		cptr = mgr->cptr;
	}

	if (cmd < DISP_FB_REQUEST)	{
		if (ubuffer[0] >= num_screens) {
			__wrn("para err in disp_ioctl, cmd = 0x%x,screen id = %d\n", cmd, (int)ubuffer[0]);
			return -1;
		}
	}
	if (DISPLAY_DEEP_SLEEP & suspend_status) {
		__wrn("ioctl:%x fail when in suspend!\n", cmd);
		return -1;
	}

	if (cmd == DISP_print) {
		__wrn("cmd:0x%x,%ld,%ld\n",cmd, ubuffer[0], ubuffer[1]);
	}

	switch(cmd)	{
	//----disp global----
	case DISP_SET_BKCOLOR:
	{
		struct disp_color para;

		if (copy_from_user(&para, (void __user *)ubuffer[1],sizeof(struct disp_color)))	{
			__wrn("copy_from_user fail\n");
			return  -EFAULT;
		}
		if (mgr && (mgr->set_back_color != NULL))
			ret = mgr->set_back_color(mgr, &para);
		break;
	}

	case DISP_GET_OUTPUT_TYPE:
	{
		if (suspend_status != DISPLAY_NORMAL)
			ret = suspend_output_type[ubuffer[0]];
		else
			ret = bsp_disp_get_output_type(ubuffer[0]);
		break;
	}

	case DISP_GET_SCN_WIDTH:
	{
		unsigned int width = 0,height = 0;
		if (mgr && mgr->device && mgr->device->get_resolution)
			mgr->device->get_resolution(mgr->device, &width, &height);
		ret = width;
		break;
	}

	case DISP_GET_SCN_HEIGHT:
	{
		unsigned int width = 0,height = 0;
		if (mgr && mgr->device && mgr->device->get_resolution)
			mgr->device->get_resolution(mgr->device, &width, &height);
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
		/* only response main device' blank request */
		if (0 != ubuffer[0])
			break;

		if (ubuffer[1]) {
#if defined(CONFIG_PM_RUNTIME)
			if (g_disp_drv.dev)
				pm_runtime_put(g_disp_drv.dev);
			else
				pr_warn("%s, display device is null\n", __func__);
#endif
			disp_blank(true);
		} else {
			if (power_status_init) {
				/* avoid first unblank, because device is ready when driver init */
				power_status_init = 0;
				break;
			}

			disp_blank(false);
#if defined(CONFIG_PM_RUNTIME)
			if (g_disp_drv.dev) {
				/* recover the pm_runtime status */
				pm_runtime_disable(g_disp_drv.dev);
				pm_runtime_set_suspended(g_disp_drv.dev);
				pm_runtime_enable(g_disp_drv.dev);
				pm_runtime_get_sync(g_disp_drv.dev);
			}
			else
				pr_warn("%s, display device is null\n", __func__);
#endif
		}
		break;
	}

	case DISP_DEVICE_SWITCH:
	{
		if (ubuffer[1] == (unsigned long)DISP_OUTPUT_TYPE_LCD)
			ret = drv_lcd_enable(ubuffer[0]);
		else
			ret = bsp_disp_device_switch(ubuffer[0], (enum disp_output_type)ubuffer[1], (enum disp_tv_mode)ubuffer[2]);
		suspend_output_type[ubuffer[0]] = ubuffer[1];
		break;
	}

	case DISP_GET_OUTPUT:
	{
		struct disp_output para;

		memset(&para, 0, sizeof(struct disp_output));

		if (mgr && mgr->device) {
			para.type = bsp_disp_get_output_type(ubuffer[0]);
			if (mgr->device->get_mode)
				para.mode = mgr->device->get_mode(mgr->device);
		}

		if (copy_to_user((void __user *)ubuffer[1],&para, sizeof(struct disp_output))) {
			__wrn("copy_from_user fail\n");
			return  -EFAULT;
		}
		break;
	}

	case DISP_SET_COLOR_RANGE:
	{
		if (mgr && mgr->set_output_color_range) {
			ret = mgr->set_output_color_range(mgr, ubuffer[1]);
		}

		break;
	}

	case DISP_GET_COLOR_RANGE:
	{
		if (mgr && mgr->get_output_color_range) {
			ret = mgr->get_output_color_range(mgr);
		}

		break;
	}

	//----layer----
	case DISP_LAYER_SET_CONFIG:
	{
		struct disp_layer_config para;

		if (copy_from_user(&para, (void __user *)ubuffer[1],sizeof(struct disp_layer_config)*ubuffer[2]))	{
			__wrn("copy_from_user fail\n");
			return  -EFAULT;
		}
		if (mgr && mgr->set_layer_config)
			ret = mgr->set_layer_config(mgr, &para, ubuffer[2]);
		break;
	}

	case DISP_LAYER_GET_CONFIG:
	{
		struct disp_layer_config para;

		if (copy_from_user(&para, (void __user *)ubuffer[1],sizeof(struct disp_layer_config)*ubuffer[2]))	{
			__wrn("copy_from_user fail\n");
			return  -EFAULT;
		}
		if (mgr && mgr->get_layer_config)
			ret = mgr->get_layer_config(mgr, &para, ubuffer[2]);
		if (copy_to_user((void __user *)ubuffer[1], &para, sizeof(struct disp_layer_config)*ubuffer[2]))	{
			__wrn("copy_to_user fail\n");
			return  -EFAULT;
		}
		break;
	}

	//---- lcd ---
	case DISP_LCD_SET_BRIGHTNESS:
	{
		if (dispdev && (DISP_OUTPUT_TYPE_LCD == dispdev->type)) {
			ret = dispdev->set_bright(dispdev, ubuffer[1]);
		}
		break;
	}

	case DISP_LCD_GET_BRIGHTNESS:
	{
		if (dispdev && (DISP_OUTPUT_TYPE_LCD == dispdev->type)) {
			ret = dispdev->get_bright(dispdev);
		}
		break;
	}

	//---- hdmi ---
	case DISP_HDMI_GET_HPD_STATUS:
	if (DISPLAY_NORMAL == suspend_status)
		ret = bsp_disp_hdmi_get_hpd_status(ubuffer[0]);
	else
		ret = 0;

	break;
	case DISP_HDMI_SUPPORT_MODE:
	{
		ret = bsp_disp_hdmi_check_support_mode(ubuffer[0], ubuffer[1]);
		break;
	}

	//---- tv ---
	case DISP_SET_TV_HPD:
	{
		ret = bsp_disp_tv_set_hpd(ubuffer[0]);
		break;
	}
#if defined (SUPPORT_TV)
	case DISP_TV_GET_HPD_STATUS:
	if (DISPLAY_NORMAL == suspend_status)
		ret = bsp_disp_tv_get_hpd_status(ubuffer[0]);
	else
		ret = 0;
	}
	break;
#endif

	//----enhance----
	case DISP_ENHANCE_ENABLE:
	{
		if (enhance && enhance->enable)
			ret = enhance->enable(enhance);
		break;
	}

	case DISP_ENHANCE_DISABLE:
	{
		if (enhance && enhance->disable)
			ret = enhance->disable(enhance);
		break;
	}

	case DISP_ENHANCE_DEMO_ENABLE:
	{
		if (enhance && enhance->demo_enable)
			ret = enhance->demo_enable(enhance);
		break;
	}

	case DISP_ENHANCE_DEMO_DISABLE:
	{
		if (enhance && enhance->demo_disable)
			ret = enhance->demo_disable(enhance);
		break;
	}

	case DISP_ENHANCE_SET_MODE:
	{
		if (enhance && enhance->set_mode)
			ret = enhance->set_mode(enhance, ubuffer[1]);
		break;
	}

	case DISP_ENHANCE_GET_MODE:
	{
		if (enhance && enhance->get_mode)
			ret = enhance->get_mode(enhance);
		break;
	}

	//---smart backlight --
	case DISP_SMBL_ENABLE:
	{
		if (smbl && smbl->enable)
			ret = smbl->enable(smbl);
		break;
	}

	case DISP_SMBL_DISABLE:
	{
		if (smbl && smbl->disable)
			ret = smbl->disable(smbl);
		break;
	}

	case DISP_SMBL_SET_WINDOW:
	{
		struct disp_rect rect;

		if (copy_from_user(&rect, (void __user *)ubuffer[1],sizeof(struct disp_rect)))	{
			__wrn("copy_from_user fail\n");
			return  -EFAULT;
		}
		if (smbl && smbl->set_window)
			ret = smbl->set_window(smbl, &rect);
		break;
	}

	//---capture --
	case DISP_CAPTURE_START:
	{
		if (cptr && cptr->start)
			ret = cptr->start(cptr);
		break;
	}

	case DISP_CAPTURE_STOP:
	{
		if (cptr && cptr->stop)
			ret = cptr->stop(cptr);
		break;
	}

	case DISP_CAPTURE_COMMIT:
	{
		struct disp_capture_info info;

		if (copy_from_user(&info, (void __user *)ubuffer[1],sizeof(struct disp_capture_info)))	{
			__wrn("copy_from_user fail\n");
			return  -EFAULT;
		}
		if (cptr && cptr->commmit)
			ret = cptr->commmit(cptr, &info);
		break;
	}

#if defined(__LINUX_PLAT__)
	//----for test----
	case DISP_MEM_REQUEST:
		ret =  disp_mem_request(ubuffer[0],ubuffer[1]);
		break;

	case DISP_MEM_RELEASE:
		ret =  disp_mem_release(ubuffer[0]);
		break;

	case DISP_MEM_GETADR:
		return g_disp_mm[ubuffer[0]].mem_start;
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

	if (cmd == BOOT_MOD_ENTER_STANDBY)
	{
	    if (g_output_type == DISP_OUTPUT_TYPE_HDMI)
	    {
		}
		else
        {
            drv_lcd_disable(0);
		}
		do
		{
			ret = drv_lcd_check_close_finished(0);
			if (ret == 1)
			{
				break;
			}
			else if (ret == -1)
			{
				return -1;
			}
			__msdelay(DELAY_ONCE_TIME);
			check_time --;
			if (check_time <= 0)
			{
				return -1;
			}
		}
		while (1);

		return 0;
	}
	else if (cmd == BOOT_MOD_EXIT_STANDBY)
	{
		if (g_output_type == DISP_OUTPUT_TYPE_HDMI)
		{
		}
		else
		{
			drv_lcd_enable(0);
        }

		do
		{
			ret = drv_lcd_check_open_finished(0);
			if (ret == 1)
			{
				break;
			}
			else if (ret == -1)
			{
				return -1;
			}
			__msdelay(DELAY_ONCE_TIME);
			check_time --;
			if (check_time <= 0)
			{
				return -1;
			}
		}
		while (1);

		return 0;
	}

	return -1;
}
