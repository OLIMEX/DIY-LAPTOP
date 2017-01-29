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
#include <linux/pm_runtime.h>
#if defined(CONFIG_DEVFREQ_DRAM_FREQ_WITH_SOFT_NOTIFY)
#include <linux/sunxi_dramfreq.h>
#endif

disp_drv_info g_disp_drv;

#define MY_BYTE_ALIGN(x) ( ( (x + (4*1024-1)) >> 12) << 12)             /* alloc based on 4K byte */

static u32 suspend_output_type[2] = {0,0};
static u32 suspend_status = 0;//0:normal; suspend_status&1 != 0:in early_suspend; suspend_status&2 != 0:in suspend;
static u32 suspend_prestep = 3; //0:after early suspend; 1:after suspend; 2:after resume; 3 :after late resume
static u32 power_status_init = 0;

//static unsigned int gbuffer[4096];
static struct info_mm  g_disp_mm[10];

static struct cdev *my_cdev;
static dev_t devid ;
static struct class *disp_class;
static struct device *display_dev;

static unsigned int g_disp = 0, g_enhance_mode = 0, g_cvbs_enhance_mode = 0;
static u32 DISP_print = 0xffff;   //print cmd which eq DISP_print
static bool g_pm_runtime_enable = 0; //when open the CONFIG_PM_RUNTIME,this bool can also control if use the PM_RUNTIME.
#ifndef CONFIG_OF
static struct sunxi_disp_mod disp_mod[] = {
	{DISP_MOD_DE      ,    "de"   },
	{DISP_MOD_LCD0    ,    "lcd0" },
	{DISP_MOD_DSI0    ,    "dsi0" },
#ifdef DISP_DEVICE_NUM
	#if DISP_DEVICE_NUM == 2
	{DISP_MOD_LCD1    ,    "lcd1" }
	#endif
#else
#	error "DEVICE_NUM undefined!"
#endif
};

static struct resource disp_resource[] =
{
};
#endif


static ssize_t disp_sys_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
	struct disp_manager *mgr = NULL;
	struct disp_device *dispdev = NULL;
  ssize_t count = 0;
	int num_screens, screen_id;
	int num_layers, layer_id;
	int num_chans, chan_id;
	//int hpd;

	num_screens = bsp_disp_feat_get_num_screens();
	for (screen_id=0; screen_id < num_screens; screen_id ++) {
		int width = 0,height = 0;
		int fps = 0;
		disp_health_info info;

		mgr = disp_get_layer_manager(screen_id);
		if (NULL == mgr)
			continue;
		dispdev = mgr->device;
		if (NULL == dispdev)
			continue;
		dispdev->get_resolution(dispdev, &width, &height);
		fps = bsp_disp_get_fps(screen_id);
		bsp_disp_get_health_info(screen_id, &info);

		if (!dispdev->is_enabled(dispdev))
			continue;
		count += sprintf(buf + count, "screen %d:\n", screen_id);
		/* output */
		if (dispdev->type == DISP_OUTPUT_TYPE_LCD) {
			count += sprintf(buf + count, "\tlcd output\tbacklight(%3d)\tfps:%d.%d", dispdev->get_bright(dispdev), fps/10, fps%10);
		} else if (dispdev->type == DISP_OUTPUT_TYPE_HDMI) {
			unsigned int mode = dispdev->get_mode(dispdev);
			count += sprintf(buf + count, "\thdmi output mode(%d)\tfps:%d.%d", mode, fps/10, fps%10);
		} else if (dispdev->type == DISP_OUTPUT_TYPE_TV) {
			unsigned int mode = dispdev->get_mode(dispdev);
			count += sprintf(buf + count, "\ttv output mode(%d)\tfps:%d.%d", mode, fps/10, fps%10);
		}
		if (dispdev->type != DISP_OUTPUT_TYPE_NONE) {
			count += sprintf(buf + count, "\t%4ux%4u\n", width, height);
			count += sprintf(buf + count, "\terr:%u\tskip:%u\tirq:%u\tvsync:%u\n", info.error_cnt, info.skip_cnt, info.irq_cnt, info.vsync_cnt);
		}

		num_chans = bsp_disp_feat_get_num_channels(screen_id);

		/* layer info */
		for (chan_id=0; chan_id<num_chans; chan_id++) {
			num_layers = bsp_disp_feat_get_num_layers_by_chn(screen_id, chan_id);
			for (layer_id=0; layer_id<num_layers; layer_id++) {
				struct disp_layer *lyr = NULL;
				struct disp_layer_config config;

				lyr = disp_get_layer(screen_id, chan_id, layer_id);
				config.channel = chan_id;
				config.layer_id = layer_id;
				mgr->get_layer_config(mgr, &config, 1);
				if (lyr && (true == config.enable) && lyr->dump)
					count += lyr->dump(lyr, buf + count);
			}
		}
	}

	count += composer_dump(buf+count);

	return count;
}

static ssize_t disp_sys_store(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
  return count;
}

static DEVICE_ATTR(sys, S_IRUGO|S_IWUSR|S_IWGRP,
    disp_sys_show, disp_sys_store);

static ssize_t disp_disp_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
  return sprintf(buf, "%d\n", g_disp);
}

static ssize_t disp_disp_store(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
  int err;
  unsigned long val;
  unsigned int num_screens;

  err = strict_strtoul(buf, 10, &val);
  if (err) {
    printk("Invalid size\n");
    return err;
  }

  num_screens = bsp_disp_feat_get_num_screens();
  if ((val>num_screens))
    printk("Invalid value, <%d is expected!\n", num_screens);
  else
    g_disp = val;

  return count;
}
static DEVICE_ATTR(disp, S_IRUGO|S_IWUSR|S_IWGRP,
    disp_disp_show, disp_disp_store);

static ssize_t disp_enhance_mode_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", g_enhance_mode);
}

static ssize_t disp_enhance_mode_store(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
	int err;
	unsigned long val;

	err = strict_strtoul(buf, 10, &val);
	if (err) {
		printk("Invalid size\n");
		return err;
	}

	if (val>2)
		printk("Invalid value, 0/1/2 is expected!\n");
	else {
		int num_screens = 2;
		struct disp_manager *mgr = NULL;
		struct disp_enhance *enhance = NULL;

		if (g_enhance_mode != val)
		{
			g_enhance_mode = val;

			num_screens = bsp_disp_feat_get_num_screens();

			if (g_disp < num_screens)
				mgr = g_disp_drv.mgr[g_disp];

			if (mgr) {
				enhance = mgr->enhance;
				if (enhance && enhance->set_mode)
					enhance->set_mode(enhance, g_enhance_mode);
			}
		}
	}

  return count;
}
static DEVICE_ATTR(enhance_mode, S_IRUGO|S_IWUSR|S_IWGRP,
    disp_enhance_mode_show, disp_enhance_mode_store);



static ssize_t disp_cvbs_enhance_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", g_cvbs_enhance_mode);
}

static ssize_t disp_cvbs_enhance_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count)
{
	int err;
	unsigned long val;

	err = strict_strtoul(buf, 10, &val);
	if (val>2)
		printk("Invalid value, 0/1/2 is expected!\n");
	else {
		int num_screens = 0;
		unsigned int disp;
		struct disp_device*  ptv = NULL;

		g_cvbs_enhance_mode = val;
		num_screens = bsp_disp_feat_get_num_screens();

		for (disp=0; disp<num_screens; disp++) {
			ptv = disp_device_find(disp, DISP_OUTPUT_TYPE_TV);
			if (ptv && ptv->set_enhance_mode)
				ptv->set_enhance_mode(ptv, g_cvbs_enhance_mode);
		}
	}

	return count;
}

static DEVICE_ATTR(cvbs_enhacne_mode, S_IRUGO|S_IWUGO,
    disp_cvbs_enhance_show, disp_cvbs_enhance_store);



static ssize_t disp_runtime_enable_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", g_pm_runtime_enable);
}

static ssize_t disp_runtime_enable_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count)
{
	int err;
	unsigned long val;

	err = strict_strtoul(buf, 10, &val);
	if (val>1)
		printk("Invalid value, 0/1 is expected!\n");
	else{
		g_pm_runtime_enable = val;
	}

	return count;
}


static DEVICE_ATTR(runtime_enable, S_IRUGO|S_IWUGO,
    disp_runtime_enable_show, disp_runtime_enable_store);

static struct attribute *disp_attributes[] = {
    &dev_attr_sys.attr,
    &dev_attr_disp.attr,
    &dev_attr_enhance_mode.attr,
    &dev_attr_cvbs_enhacne_mode.attr,
    &dev_attr_runtime_enable.attr,
    NULL
};

static struct attribute_group disp_attribute_group = {
  .name = "attr",
  .attrs = disp_attributes
};

unsigned int disp_boot_para_parse(const char *name)
{
	unsigned int value = 0;
	if (of_property_read_u32(g_disp_drv.dev->of_node, name, &value) < 0)
		__wrn("of_property_read disp.%s fail\n", name);

	pr_info("[DISP] %s:0x%x\n", name, value);
	return value;
}

EXPORT_SYMBOL(disp_boot_para_parse);

static s32 parser_disp_init_para(const struct device_node *np, disp_init_para * init_para)
{
	int  value;
	int  i;

	memset(init_para, 0, sizeof(disp_init_para));

	if (of_property_read_u32(np, "disp_init_enable", &value) < 0) {
		__wrn("of_property_read disp_init.disp_init_enable fail\n");
		return -1;
	}
	init_para->b_init = value;

	if (of_property_read_u32(np, "disp_mode", &value) < 0)	{
		__wrn("of_property_read disp_init.disp_mode fail\n");
		return -1;
	}
	init_para->disp_mode= value;

	//screen0
	if (of_property_read_u32(np, "screen0_output_type", &value) < 0)	{
		__wrn("of_property_read disp_init.screen0_output_type fail\n");
		return -1;
	}
	if (value == 0) {
		init_para->output_type[0] = DISP_OUTPUT_TYPE_NONE;
	}	else if (value == 1) {
		init_para->output_type[0] = DISP_OUTPUT_TYPE_LCD;
	}	else if (value == 2)	{
		init_para->output_type[0] = DISP_OUTPUT_TYPE_TV;
	}	else if (value == 3)	{
		init_para->output_type[0] = DISP_OUTPUT_TYPE_HDMI;
	}	else if (value == 4)	{
		init_para->output_type[0] = DISP_OUTPUT_TYPE_VGA;
	}	else {
		__wrn("invalid screen0_output_type %d\n", init_para->output_type[0]);
		return -1;
	}

	if (of_property_read_u32(np, "screen0_output_mode", &value) < 0)	{
		__wrn("of_property_read disp_init.screen0_output_mode fail\n");
		return -1;
	}
	if (init_para->output_type[0] == DISP_OUTPUT_TYPE_TV || init_para->output_type[0] == DISP_OUTPUT_TYPE_HDMI
	    || init_para->output_type[0] == DISP_OUTPUT_TYPE_VGA) {
		init_para->output_mode[0]= value;
	}

	//screen1
	if (of_property_read_u32(np, "screen1_output_type", &value) < 0)	{
		__wrn("of_property_read disp_init.screen1_output_type fail\n");
		return -1;
	}
	if (value == 0) {
		init_para->output_type[1] = DISP_OUTPUT_TYPE_NONE;
	}	else if (value == 1)	{
		init_para->output_type[1] = DISP_OUTPUT_TYPE_LCD;
	}	else if (value == 2)	{
		init_para->output_type[1] = DISP_OUTPUT_TYPE_TV;
	}	else if (value == 3)	{
		init_para->output_type[1] = DISP_OUTPUT_TYPE_HDMI;
	}	else if (value == 4)	{
		init_para->output_type[1] = DISP_OUTPUT_TYPE_VGA;
	}	else {
		__wrn("invalid screen1_output_type %d\n", init_para->output_type[1]);
		return -1;
	}

	if (of_property_read_u32(np, "screen1_output_mode", &value) < 0)	{
		__wrn("of_property_read disp_init.screen1_output_mode fail\n");
		return -1;
	}
	if (init_para->output_type[1] == DISP_OUTPUT_TYPE_TV || init_para->output_type[1] == DISP_OUTPUT_TYPE_HDMI
	    || init_para->output_type[1] == DISP_OUTPUT_TYPE_VGA) {
		init_para->output_mode[1]= value;
	}

	//screen2
	if (of_property_read_u32(np, "screen2_output_type", &value) < 0)	{
		__inf("of_property_read disp_init.screen2_output_type fail\n");
	}
	if (value == 0) {
		init_para->output_type[2] = DISP_OUTPUT_TYPE_NONE;
	}	else if (value == 1) {
		init_para->output_type[2] = DISP_OUTPUT_TYPE_LCD;
	}	else if (value == 2)	{
		init_para->output_type[2] = DISP_OUTPUT_TYPE_TV;
	}	else if (value == 3)	{
		init_para->output_type[2] = DISP_OUTPUT_TYPE_HDMI;
	}	else if (value == 4)	{
		init_para->output_type[2] = DISP_OUTPUT_TYPE_VGA;
	}	else {
		__inf("invalid screen0_output_type %d\n", init_para->output_type[2]);
	}

	if (of_property_read_u32(np, "screen2_output_mode", &value) < 0)	{
		__inf("of_property_read disp_init.screen2_output_mode fail\n");
	}
	if (init_para->output_type[2] == DISP_OUTPUT_TYPE_TV || init_para->output_type[2] == DISP_OUTPUT_TYPE_HDMI
	    || init_para->output_type[2] == DISP_OUTPUT_TYPE_VGA) {
		init_para->output_mode[2]= value;
	}

	//fb0
	init_para->buffer_num[0]= 2;

	if (of_property_read_u32(np, "fb0_format", &value) < 0) {
		__wrn("of_property_read disp_init.fb0_format fail\n");
		return -1;
	}
	init_para->format[0]= value;

	if (of_property_read_u32(np, "fb0_width", &value) < 0)	{
		__inf("of_property_read disp_init.fb0_width fail\n");
		return -1;
	}
	init_para->fb_width[0]= value;

	if (of_property_read_u32(np, "fb0_height", &value) < 0)	{
		__inf("of_property_read disp_init.fb0_height fail\n");
		return -1;
	}
	init_para->fb_height[0]= value;

	//fb1
	init_para->buffer_num[1]= 2;

	if (of_property_read_u32(np, "fb1_format", &value) < 0) {
		__wrn("of_property_read disp_init.fb1_format fail\n");
	}
	init_para->format[1]= value;

	if (of_property_read_u32(np, "fb1_width", &value) < 0) {
		__inf("of_property_read disp_init.fb1_width fail\n");
	}
	init_para->fb_width[1]= value;

	if (of_property_read_u32(np, "fb1_height", &value) < 0) {
		__inf("of_property_read disp_init.fb1_height fail\n");
	}
	init_para->fb_height[1]= value;

	//fb2
	init_para->buffer_num[2]= 2;

	if (of_property_read_u32(np, "fb2_format", &value) < 0) {
		__inf("of_property_read disp_init.fb2_format fail\n");
	}
	init_para->format[2]= value;

	if (of_property_read_u32(np, "fb2_width", &value) < 0) {
		__inf("of_property_read disp_init.fb2_width fail\n");
	}
	init_para->fb_width[2]= value;

	if (of_property_read_u32(np, "fb2_height", &value) < 0) {
		__inf("of_property_read disp_init.fb2_height fail\n");
	}
	init_para->fb_height[2]= value;

	__inf("====display init para begin====\n");
	__inf("b_init:%d\n", init_para->b_init);
	__inf("disp_mode:%d\n\n", init_para->disp_mode);
	for (i=0; i<3; i++) {
		__inf("output_type[%d]:%d\n", i, init_para->output_type[i]);
		__inf("output_mode[%d]:%d\n", i, init_para->output_mode[i]);
	}
	for (i=0; i<3; i++) {
		__inf("buffer_num[%d]:%d\n", i, init_para->buffer_num[i]);
		__inf("format[%d]:%d\n", i, init_para->format[i]);
		__inf("fb_width[%d]:%d\n", i, init_para->fb_width[i]);
		__inf("fb_height[%d]:%d\n", i, init_para->fb_height[i]);
	}
	__inf("====display init para end====\n");

	return 0;
}

void *disp_malloc(u32 num_bytes, void *phys_addr)
{
	u32 actual_bytes;
	void* address = NULL;

	if (num_bytes != 0) {
		actual_bytes = MY_BYTE_ALIGN(num_bytes);

		address = dma_alloc_coherent(g_disp_drv.dev, actual_bytes, (dma_addr_t*)phys_addr, GFP_KERNEL);
		if (address) {
			__inf("dma_alloc_coherent ok, address=0x%p, size=0x%x\n", (void*)(*(unsigned long*)phys_addr), num_bytes);
			return address;
		} else {
			__wrn("dma_alloc_coherent fail, size=0x%x\n", num_bytes);
			return NULL;
		}
	} else {
		__wrn("%s size is zero\n", __func__);
	}

	return NULL;
}

void  disp_free(void *virt_addr, void* phys_addr, u32 num_bytes)
{
	u32 actual_bytes;

	actual_bytes = MY_BYTE_ALIGN(num_bytes);
	if (phys_addr && virt_addr)
		dma_free_coherent(g_disp_drv.dev, actual_bytes, virt_addr, (dma_addr_t)phys_addr);

	return ;
}

s32 disp_set_hdmi_func(struct disp_device_func * func)
{
	return bsp_disp_set_hdmi_func(func);
}

s32 disp_set_hdmi_detect(bool hpd)
{
	return bsp_disp_hdmi_set_detect(hpd);
}
EXPORT_SYMBOL(disp_set_hdmi_detect);

s32 disp_tv_register(struct disp_tv_func * func)
{
	return bsp_disp_tv_register(func);
}
EXPORT_SYMBOL(disp_tv_register);

static void resume_proc(unsigned disp)
{
	struct disp_manager *mgr = NULL;

	mgr = g_disp_drv.mgr[disp];
	if (!mgr || !mgr->device)
		return ;

	if (DISP_OUTPUT_TYPE_LCD == mgr->device->type) {
		mgr->device->fake_enable(mgr->device);
	}
}

static void resume_work_0(struct work_struct *work)
{
	resume_proc(0);
}

static void resume_work_1(struct work_struct *work)
{
	resume_proc(1);
}

static void resume_work_2(struct work_struct *work)
{
	resume_proc(2);
}

static void start_work(struct work_struct *work)
{
	int num_screens;
	int screen_id;
	int count = 0;

	num_screens = bsp_disp_feat_get_num_screens();
	while ((g_disp_drv.inited == 0) && (count < 5)) {
		count ++;
		msleep(10);
	}
	if (count >= 5)
		pr_warn("%s, timeout\n", __func__);
	if (g_disp_drv.para.boot_info.sync == 0) {
		for (screen_id = 0; screen_id<num_screens; screen_id++) {
			int disp_mode = g_disp_drv.disp_init.disp_mode;
			int output_type = g_disp_drv.disp_init.output_type[screen_id];
			int output_mode = g_disp_drv.disp_init.output_mode[screen_id];
			int lcd_registered = bsp_disp_get_lcd_registered(screen_id);
			int hdmi_registered = bsp_disp_get_hdmi_registered();

			__inf("sel=%d, output_type=%d, lcd_reg=%d, hdmi_reg=%d\n",
				screen_id, output_type, lcd_registered, hdmi_registered);
			if (((disp_mode	== DISP_INIT_MODE_SCREEN0) && (screen_id == 0))
				|| ((disp_mode	== DISP_INIT_MODE_SCREEN1) && (screen_id == 1))) {
				if ((output_type == DISP_OUTPUT_TYPE_LCD)) {
					if (lcd_registered	&& bsp_disp_get_output_type(screen_id) != DISP_OUTPUT_TYPE_LCD) {
						bsp_disp_device_switch(screen_id, output_type, output_mode);
						suspend_output_type[screen_id] = output_type;
					}
				}
				else if (output_type == DISP_OUTPUT_TYPE_HDMI) {
					if (hdmi_registered	&& bsp_disp_get_output_type(screen_id) != DISP_OUTPUT_TYPE_HDMI) {
						msleep(600);
						bsp_disp_device_switch(screen_id, output_type, output_mode);
						suspend_output_type[screen_id] = output_type;
					}
				} else {
					bsp_disp_device_switch(screen_id, output_type, output_mode);
					suspend_output_type[screen_id] = output_type;
				}
			}
		}
	}else {
		if ((g_disp_drv.para.boot_info.type == DISP_OUTPUT_TYPE_HDMI) && !bsp_disp_get_hdmi_registered())
			return;
		if (bsp_disp_get_output_type(g_disp_drv.para.boot_info.disp) != g_disp_drv.para.boot_info.type) {
			bsp_disp_sync_with_hw(&g_disp_drv.para);
			suspend_output_type[g_disp_drv.para.boot_info.disp] = g_disp_drv.para.boot_info.type;
		}
	}
}

static s32 start_process(void)
{
	flush_work(&g_disp_drv.start_work);
	schedule_work(&g_disp_drv.start_work);

	return 0;
}

s32 disp_register_sync_proc(void (*proc)(u32))
{
	struct proc_list *new_proc;

	new_proc = (struct proc_list*)disp_sys_malloc(sizeof(struct proc_list));
	if (new_proc) {
		new_proc->proc = proc;
		list_add_tail(&(new_proc->list), &(g_disp_drv.sync_proc_list.list));
	} else {
		pr_warn("malloc fail in %s\n", __func__);
	}

	return 0;
}

s32 disp_unregister_sync_proc(void (*proc)(u32))
{
	struct proc_list *ptr;

	if ((NULL == proc)) {
		pr_warn("hdl is NULL in %s\n", __func__);
		return -1;
	}
	list_for_each_entry(ptr, &g_disp_drv.sync_proc_list.list, list) {
		if (ptr->proc == proc) {
			list_del(&ptr->list);
			disp_sys_free((void*)ptr);
			return 0;
		}
	}

	return -1;
}

s32 disp_register_sync_finish_proc(void (*proc)(u32))
{
	struct proc_list *new_proc;

	new_proc = (struct proc_list*)disp_sys_malloc(sizeof(struct proc_list));
	if (new_proc) {
		new_proc->proc = proc;
		list_add_tail(&(new_proc->list), &(g_disp_drv.sync_finish_proc_list.list));
	} else {
		pr_warn("malloc fail in %s\n", __func__);
	}

	return 0;
}

s32 disp_unregister_sync_finish_proc(void (*proc)(u32))
{
	struct proc_list *ptr;

	if ((NULL == proc)) {
		pr_warn("hdl is NULL in %s\n", __func__);
		return -1;
	}
	list_for_each_entry(ptr, &g_disp_drv.sync_finish_proc_list.list, list) {
		if (ptr->proc == proc) {
			list_del(&ptr->list);
			disp_sys_free((void*)ptr);
			return 0;
		}
	}

	return -1;
}

static s32 disp_sync_finish_process(u32 screen_id)
{
	struct proc_list *ptr;

	list_for_each_entry(ptr, &g_disp_drv.sync_finish_proc_list.list, list) {
		if (ptr->proc)
			ptr->proc(screen_id);
	}

	return 0;
}

s32 disp_register_ioctl_func(unsigned int cmd, int (*proc)(unsigned int cmd, unsigned long arg))
{
	struct ioctl_list *new_proc;

	new_proc = (struct ioctl_list*)disp_sys_malloc(sizeof(struct ioctl_list));
	if (new_proc) {
		new_proc->cmd = cmd;
		new_proc->func = proc;
		list_add_tail(&(new_proc->list), &(g_disp_drv.ioctl_extend_list.list));
	} else {
		pr_warn("malloc fail in %s\n", __func__);
	}

	return 0;
}

s32 disp_unregister_ioctl_func(unsigned int cmd)
{
	struct ioctl_list *ptr;

	list_for_each_entry(ptr, &g_disp_drv.ioctl_extend_list.list, list) {
		if (ptr->cmd == cmd) {
			list_del(&ptr->list);
			disp_sys_free((void*)ptr);
			return 0;
		}
	}

	pr_warn("no ioctl found(cmd:0x%x) in %s\n", cmd, __func__);
	return -1;
}

static s32 disp_ioctl_extend(unsigned int cmd, unsigned long arg)
{
	struct ioctl_list *ptr;

	list_for_each_entry(ptr, &g_disp_drv.ioctl_extend_list.list, list) {
		if (cmd == ptr->cmd)
			return ptr->func(cmd, arg);
	}

	return -1;
}

s32 disp_register_compat_ioctl_func(unsigned int cmd, int (*proc)(unsigned int cmd, unsigned long arg))
{
	struct ioctl_list *new_proc;

	new_proc = (struct ioctl_list*)disp_sys_malloc(sizeof(struct ioctl_list));
	if (new_proc) {
		new_proc->cmd = cmd;
		new_proc->func = proc;
		list_add_tail(&(new_proc->list), &(g_disp_drv.compat_ioctl_extend_list.list));
	} else {
		pr_warn("malloc fail in %s\n", __func__);
	}

	return 0;
}

s32 disp_unregister_compat_ioctl_func(unsigned int cmd)
{
	struct ioctl_list *ptr;

	list_for_each_entry(ptr, &g_disp_drv.compat_ioctl_extend_list.list, list) {
		if (ptr->cmd == cmd) {
			list_del(&ptr->list);
			disp_sys_free((void*)ptr);
			return 0;
		}
	}

	pr_warn("no ioctl found(cmd:0x%x) in %s\n", cmd, __func__);
	return -1;
}

static s32 disp_compat_ioctl_extend(unsigned int cmd, unsigned long arg)
{
	struct ioctl_list *ptr;

	list_for_each_entry(ptr, &g_disp_drv.compat_ioctl_extend_list.list, list) {
		if (cmd == ptr->cmd)
			return ptr->func(cmd, arg);
	}

	return -1;
}

s32 disp_register_standby_func(int (*suspend)(void), int (*resume)(void))
{
	struct standby_cb_list *new_proc;

	new_proc = (struct standby_cb_list*)disp_sys_malloc(sizeof(struct standby_cb_list));
	if (new_proc) {
		new_proc->suspend = suspend;
		new_proc->resume = resume;
		list_add_tail(&(new_proc->list), &(g_disp_drv.stb_cb_list.list));
	} else {
		pr_warn("malloc fail in %s\n", __func__);
	}

	return 0;
}

s32 disp_unregister_standby_func(int (*suspend)(void), int (*resume)(void))
{
	struct standby_cb_list *ptr;

	list_for_each_entry(ptr, &g_disp_drv.stb_cb_list.list, list) {
		if ((ptr->suspend == suspend) && (ptr->resume == resume)) {
			list_del(&ptr->list);
			disp_sys_free((void*)ptr);
			return 0;
		}
	}

	return -1;
}

static s32 disp_suspend_cb(void)
{
	struct standby_cb_list *ptr;

	list_for_each_entry(ptr, &g_disp_drv.stb_cb_list.list, list) {
		if (ptr->suspend)
			return ptr->suspend();
	}

	return -1;
}

static s32 disp_resume_cb(void)
{
	struct standby_cb_list *ptr;

	list_for_each_entry(ptr, &g_disp_drv.stb_cb_list.list, list) {
		if (ptr->resume)
			return ptr->resume();
	}

	return -1;
}

static s32 disp_init(struct platform_device *pdev)
{
	disp_bsp_init_para *para;
	int i, disp, num_screens;
	unsigned int value, output_type, output_mode;

	__inf("%s !\n", __func__);

	INIT_WORK(&g_disp_drv.resume_work[0], resume_work_0);
	INIT_WORK(&g_disp_drv.resume_work[1], resume_work_1);
	INIT_WORK(&g_disp_drv.resume_work[2], resume_work_2);
	INIT_WORK(&g_disp_drv.start_work, start_work);
	INIT_LIST_HEAD(&g_disp_drv.sync_proc_list.list);
	INIT_LIST_HEAD(&g_disp_drv.sync_finish_proc_list.list);
	INIT_LIST_HEAD(&g_disp_drv.ioctl_extend_list.list);
	INIT_LIST_HEAD(&g_disp_drv.compat_ioctl_extend_list.list);
	INIT_LIST_HEAD(&g_disp_drv.stb_cb_list.list);
	mutex_init(&g_disp_drv.mlock);
	parser_disp_init_para(pdev->dev.of_node, &g_disp_drv.disp_init);
	para = &g_disp_drv.para;

	memset(para, 0, sizeof(disp_bsp_init_para));
	for (i=0; i<DISP_MOD_NUM; i++)	{
		para->reg_base[i] = g_disp_drv.reg_base[i];
		para->irq_no[i]   = g_disp_drv.irq_no[i];
        para->mclk[i] = g_disp_drv.mclk[i];
		__inf("mod %d, base=0x%lx, irq=%d, mclk=0x%p\n", i, para->reg_base[i], para->irq_no[i], para->mclk[i]);
	}

	para->disp_int_process       = disp_sync_finish_process;
	para->vsync_event            = drv_disp_vsync_event;
	para->start_process          = start_process;

	value = disp_boot_para_parse("boot_disp");
	output_type = (value >> 8) & 0xff;
	output_mode = (value) & 0xff;
	if (output_type != (int)DISP_OUTPUT_TYPE_NONE) {
		para->boot_info.sync = 1;
		para->boot_info.disp = 0;//disp0
		para->boot_info.type = output_type;
		para->boot_info.mode = output_mode;
	} else {
		output_type = (value >> 24)& 0xff;
		output_mode = (value >> 16) & 0xff;
		if (output_type != (int)DISP_OUTPUT_TYPE_NONE) {
			para->boot_info.sync = 1;
			para->boot_info.disp = 1;//disp1
			para->boot_info.type = output_type;
			para->boot_info.mode = output_mode;
		}
	}

	if (1 == para->boot_info.sync) {
		g_disp_drv.disp_init.disp_mode = para->boot_info.disp;
		g_disp_drv.disp_init.output_type[para->boot_info.disp] = output_type;
		g_disp_drv.disp_init.output_mode[para->boot_info.disp] = output_mode;
	}

	bsp_disp_init(para);
	num_screens = bsp_disp_feat_get_num_screens();
	for (disp=0; disp<num_screens; disp++) {
		g_disp_drv.mgr[disp] = disp_get_layer_manager(disp);
	}
	lcd_init();
	bsp_disp_open();

	fb_init(pdev);
	composer_init(&g_disp_drv);
	g_disp_drv.inited = true;
	start_process();


	__inf("%s finish\n", __func__);
	return 0;
}

static s32 disp_exit(void)
{
	fb_exit();
	bsp_disp_close();
	bsp_disp_exit(g_disp_drv.exit_mode);
	return 0;
}

static int disp_mem_request(int sel,u32 size)
{
//#ifndef FB_RESERVED_MEM
#if 1
	unsigned map_size = 0;
	struct page *page;

	if (NULL != g_disp_mm[sel].info_base)
		return -EINVAL;

	g_disp_mm[sel].mem_len = size;
	map_size = PAGE_ALIGN(g_disp_mm[sel].mem_len);

	page = alloc_pages(GFP_KERNEL,get_order(map_size));
	if (page != NULL) {
		g_disp_mm[sel].info_base = page_address(page);
		if (NULL == g_disp_mm[sel].info_base)	{
			free_pages((unsigned long)(page),get_order(map_size));
			__wrn("page_address fail!\n");
			return -ENOMEM;
		}
		g_disp_mm[sel].mem_start = virt_to_phys(g_disp_mm[sel].info_base);
		memset(g_disp_mm[sel].info_base,0,size);

		__inf("pa=0x%p va=0x%p size:0x%x\n",(void*)g_disp_mm[sel].mem_start, g_disp_mm[sel].info_base, size);
		return 0;
	}	else {
		__wrn("alloc_pages fail!\n");
		return -ENOMEM;
	}
#else
	uintptr_t phy_addr;

	g_disp_mm[sel].info_base = disp_malloc(size, (void*)&phy_addr);
	if (g_disp_mm[sel].info_base) {
		g_disp_mm[sel].mem_start = phy_addr;
		g_disp_mm[sel].mem_len = size;
		memset(g_disp_mm[sel].info_base,0,size);
		__inf("pa=0x%p va=0x%p size:0x%x\n", (void*)g_disp_mm[sel].mem_start, g_disp_mm[sel].info_base, size);

		return 0;
	}	else {
		__wrn("disp_malloc fail!\n");
		return -ENOMEM;
	}
#endif
}

static int disp_mem_release(int sel)
{
//#ifndef FB_RESERVED_MEM
#if 1
	unsigned map_size = PAGE_ALIGN(g_disp_mm[sel].mem_len);
	unsigned page_size = map_size;

	if (NULL == g_disp_mm[sel].info_base)
		return -EINVAL;

	free_pages((unsigned long)(g_disp_mm[sel].info_base),get_order(page_size));
	memset(&g_disp_mm[sel],0,sizeof(struct info_mm));
#else
	if (g_disp_mm[sel].info_base == NULL)
		return -EINVAL;

	__inf("disp_mem_release, mem_id=%d, phy_addr=0x%p\n", sel, (void*)g_disp_mm[sel].mem_start);
	disp_free((void *)g_disp_mm[sel].info_base, (void*)g_disp_mm[sel].mem_start, g_disp_mm[sel].mem_len);
	memset(&g_disp_mm[sel],0,sizeof(struct info_mm));
#endif
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

int disp_mmap(struct file *file, struct vm_area_struct * vma)
{
	unsigned long mypfn = vma->vm_pgoff;
	unsigned long vmsize = vma->vm_end-vma->vm_start;
	vma->vm_pgoff = 0;

	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	if (remap_pfn_range(vma,vma->vm_start,mypfn,vmsize,vma->vm_page_prot))
		return -EAGAIN;

	return 0;
}

int disp_open(struct inode *inode, struct file *file)
{
	return 0;
}

int disp_release(struct inode *inode, struct file *file)
{
	return 0;
}
ssize_t disp_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	return 0;
}

ssize_t disp_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	return 0;
}

static int disp_probe(struct platform_device *pdev)
{
	int i;
	int ret;

	__inf("[DISP]disp_probe\n");
	memset(&g_disp_drv, 0, sizeof(disp_drv_info));

	g_disp_drv.dev = &pdev->dev;

	/* iomap */
	g_disp_drv.reg_base[DISP_MOD_DE] = (uintptr_t __force)of_iomap(pdev->dev.of_node, 0);
	if (!g_disp_drv.reg_base[DISP_MOD_DE]) {
		dev_err(&pdev->dev, "unable to map de registers\n");
		ret = -EINVAL;
		goto err_iomap1;
	}

	g_disp_drv.reg_base[DISP_MOD_LCD0] = (uintptr_t __force)of_iomap(pdev->dev.of_node, 1);
	if (!g_disp_drv.reg_base[DISP_MOD_LCD0]) {
		dev_err(&pdev->dev, "unable to map lcd registers\n");
		ret = -EINVAL;
		goto err_iomap2;
	}

	g_disp_drv.reg_base[DISP_MOD_DSI0] = (uintptr_t __force)of_iomap(pdev->dev.of_node, 2);
	if (!g_disp_drv.reg_base[DISP_MOD_DSI0]) {
		dev_err(&pdev->dev, "unable to map dsi registers\n");
		ret = -EINVAL;
		goto err_iomap2;
	}

	/* parse and map irq */
	for (i=0; i<DEVICE_NUM; i++) {
		g_disp_drv.irq_no[DISP_MOD_LCD0 + i] = irq_of_parse_and_map(pdev->dev.of_node, i);
		if (!g_disp_drv.irq_no[DISP_MOD_LCD0 + i]) {
			dev_err(&pdev->dev, "irq_of_parse_and_map irq %d fail for lcd%d\n", i, i);
		}
	}
	g_disp_drv.irq_no[DISP_MOD_DSI0] = irq_of_parse_and_map(pdev->dev.of_node, i);
	if (!g_disp_drv.irq_no[DISP_MOD_DSI0]) {
		dev_err(&pdev->dev, "irq_of_parse_and_map irq %d fail for dsi\n", i);
	}

	/* get clk */
	g_disp_drv.mclk[DISP_MOD_DE] = of_clk_get(pdev->dev.of_node, 0);
	if (IS_ERR(g_disp_drv.mclk[DISP_MOD_DE])) {
		dev_err(&pdev->dev, "fail to get clk for de\n");
	}
	g_disp_drv.mclk[DISP_MOD_LCD0] = of_clk_get(pdev->dev.of_node, 1);
	if (IS_ERR(g_disp_drv.mclk[DISP_MOD_LCD0])) {
		dev_err(&pdev->dev, "fail to get clk for lcd0\n");
	}
	g_disp_drv.mclk[DISP_MOD_LVDS] = of_clk_get(pdev->dev.of_node, 2);
	if (IS_ERR(g_disp_drv.mclk[DISP_MOD_LVDS])) {
		dev_err(&pdev->dev, "fail to get clk for lvds\n");
	}
	g_disp_drv.mclk[DISP_MOD_DSI0] = of_clk_get(pdev->dev.of_node, 3);
	if (IS_ERR(g_disp_drv.mclk[DISP_MOD_DSI0])) {
		dev_err(&pdev->dev, "fail to get clk for dsi\n");
	}

#ifdef DISP_DEVICE_NUM
	#if (DISP_DEVICE_NUM == 2)
	g_disp_drv.mclk[DISP_MOD_LCD1] = of_clk_get(pdev->dev.of_node, 4);
	if (IS_ERR(g_disp_drv.mclk[DISP_MOD_LCD1])) {
		dev_err(&pdev->dev, "fail to get clk for lcd1\n");
	}
	#endif
#else
#	error "DEVICE_NUM undefined!"
#endif

	disp_init(pdev);
	ret = sysfs_create_group(&display_dev->kobj,
                             &disp_attribute_group);
	if (ret)
		__wrn("sysfs_create_group fail!\n");

	power_status_init = 1;
#if defined(CONFIG_PM_RUNTIME)
	pm_runtime_set_active(&pdev->dev);
	pm_runtime_get_noresume(&pdev->dev);
	pm_runtime_set_autosuspend_delay(&pdev->dev, 5000);
	pm_runtime_use_autosuspend(&pdev->dev);
	pm_runtime_enable(&pdev->dev);
#endif
	__inf("[DISP]disp_probe finish\n");

	return ret;

err_iomap2:
	iounmap((char __iomem *)g_disp_drv.reg_base[DISP_MOD_DE]);
err_iomap1:

	return ret;
}

static int disp_remove(struct platform_device *pdev)
{
	pr_info("disp_remove call\n");

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static int disp_blank(bool blank)
{
	u32 screen_id = 0;
	int num_screens;
	struct disp_manager *mgr = NULL;

#if defined(CONFIG_DEVFREQ_DRAM_FREQ_WITH_SOFT_NOTIFY)
	/* notify dramfreq module that DE will access DRAM in a short time */
	if (!blank) {
		dramfreq_master_access(MASTER_DE, true);
	}
#endif
	num_screens = bsp_disp_feat_get_num_screens();

	for (screen_id=0; screen_id<num_screens; screen_id++) {
		mgr = g_disp_drv.mgr[screen_id];
		/* Currently remove !mgr->device condition,
		 * avoiding problem in the following case:
		 *
		 *   attach manager and device -> disp blank --> blank success
		 *   deattach manager and device -> disp unblank --> unblank fail (don't satisfy !mgr->device)
		 *   attach manager and device --> problem arises(manager is on unblank state)
		 *
		 * The real scenario is: hdmi plug in -> press power key to standy -> hdmi plug out
		 *  -> press power key to resume -> hdmi plug in -> display blank on hdmi screen
		 */
		if (!mgr)
			continue;

		if (mgr->blank)
			mgr->blank(mgr, blank);
	}

#if defined(CONFIG_DEVFREQ_DRAM_FREQ_WITH_SOFT_NOTIFY)
	/* notify dramfreq module that DE will not access DRAM any more */
	if (blank) {
		dramfreq_master_access(MASTER_DE, false);
	}
#endif

	return 0;
}

#if defined(CONFIG_PM_RUNTIME)
static int disp_runtime_suspend(struct device *dev)
{
	u32 screen_id = 0;
	int num_screens;
	struct disp_manager *mgr = NULL;
	struct disp_device* dispdev_suspend = NULL;
	struct list_head* disp_list= NULL;

	pr_info("%s\n", __func__);

	if (!g_pm_runtime_enable)
		return 0;

	num_screens = bsp_disp_feat_get_num_screens();

	disp_suspend_cb();
	for (screen_id=0; screen_id<num_screens; screen_id++) {
		mgr = g_disp_drv.mgr[screen_id];
		if (mgr && mgr->device) {
			struct disp_device *dispdev = mgr->device;

			if (suspend_output_type[screen_id] == DISP_OUTPUT_TYPE_LCD)
				flush_work(&g_disp_drv.resume_work[screen_id]);

			if (dispdev->is_enabled(dispdev))
				dispdev->disable(dispdev);
		}
	}

	disp_list = disp_device_get_list_head();
	list_for_each_entry(dispdev_suspend, disp_list, list) {
		if (dispdev_suspend->suspend) {
			dispdev_suspend->suspend(dispdev_suspend);
		}
	}

	suspend_status |= DISPLAY_LIGHT_SLEEP;
	suspend_prestep = 0;

	pr_info("%s finish\n", __func__);

	return 0;
}

static int disp_runtime_resume(struct device *dev)
{
	u32 screen_id = 0;
	int num_screens;
	struct disp_manager *mgr = NULL;
	struct disp_device* dispdev = NULL;
	struct list_head* disp_list= NULL;

	pr_info("%s\n", __func__);

	if (!g_pm_runtime_enable)
		return 0;

	num_screens = bsp_disp_feat_get_num_screens();

	disp_list = disp_device_get_list_head();
	list_for_each_entry(dispdev, disp_list, list) {
		if (dispdev->resume) {
			dispdev->resume(dispdev);
		}
	}

	for (screen_id=0; screen_id<num_screens; screen_id++) {
		mgr = g_disp_drv.mgr[screen_id];
		if (!mgr || !mgr->device)
			continue;

		if (suspend_output_type[screen_id] == DISP_OUTPUT_TYPE_LCD) {
			flush_work(&g_disp_drv.resume_work[screen_id]);
			if (!mgr->device->is_enabled(mgr->device)) {
				mgr->device->enable(mgr->device);
			} else {
				mgr->device->pwm_enable(mgr->device);
				mgr->device->backlight_enable(mgr->device);
			}
		} else if (suspend_output_type[screen_id] != DISP_OUTPUT_TYPE_NONE) {
			if (mgr->device->set_mode && mgr->device->get_mode) {
					u32 mode = mgr->device->get_mode(mgr->device);

					mgr->device->set_mode(mgr->device, mode);
			}
			if (!mgr->device->is_enabled(mgr->device))
				mgr->device->enable(mgr->device);
		}
	}

	suspend_status &= (~DISPLAY_LIGHT_SLEEP);
	suspend_prestep = 3;

	disp_resume_cb();

	pr_info("%s finish\n", __func__);

	return 0;
}

static int disp_runtime_idle(struct device *dev)
{
	pr_info("%s\n", __func__);

	if (g_disp_drv.dev) {
		pm_runtime_mark_last_busy(g_disp_drv.dev);
		pm_request_autosuspend(g_disp_drv.dev);
	} else {
		pr_warn("%s, display device is null\n", __func__);
	}

	/* return 0: for framework to request enter suspend.
		return non-zero: do susupend for myself;
	*/
	return -1;
}
#endif

static int disp_suspend(struct device *dev)
{
	u32 screen_id = 0;
	int num_screens;
	struct disp_manager *mgr = NULL;
	struct disp_device* dispdev_suspend = NULL;
	struct list_head* disp_list= NULL;

	pr_info("%s\n", __func__);

	if (!g_disp_drv.dev) {
		pr_warn("display device is null!\n");
		return 0;
	}
#if defined(CONFIG_PM_RUNTIME)
	if (!pm_runtime_status_suspended(g_disp_drv.dev))
#endif
	{
		num_screens = bsp_disp_feat_get_num_screens();
		disp_suspend_cb();
		if (g_pm_runtime_enable) {

			for (screen_id=0; screen_id<num_screens; screen_id++) {
				mgr = g_disp_drv.mgr[screen_id];
				if (!mgr || !mgr->device)
					continue;

				if (suspend_output_type[screen_id] == DISP_OUTPUT_TYPE_LCD)
					flush_work(&g_disp_drv.resume_work[screen_id]);
				if (suspend_output_type[screen_id] != DISP_OUTPUT_TYPE_NONE) {
						if (mgr->device->is_enabled(mgr->device))
							mgr->device->disable(mgr->device);
				}
			}
		}
		else {
			for (screen_id=0; screen_id<num_screens; screen_id++) {
				mgr = g_disp_drv.mgr[screen_id];
				if (!mgr || !mgr->device)
					continue;

				if (suspend_output_type[screen_id] != DISP_OUTPUT_TYPE_NONE) {
					if (mgr->device->is_enabled(mgr->device))
						mgr->device->disable(mgr->device);
				}
			}
		}

		/*suspend for all display device*/
		disp_list = disp_device_get_list_head();
		list_for_each_entry(dispdev_suspend, disp_list, list) {
			if (dispdev_suspend->suspend) {
				dispdev_suspend->suspend(dispdev_suspend);
			}
		}
	}

	//FIXME: hdmi suspend
	suspend_status |= DISPLAY_DEEP_SLEEP;
	suspend_prestep = 1;
#if defined(CONFIG_PM_RUNTIME)
	if (g_pm_runtime_enable) {
		pm_runtime_disable(g_disp_drv.dev);
		pm_runtime_set_suspended(g_disp_drv.dev);
		pm_runtime_enable(g_disp_drv.dev);
	}
#endif
	pr_info("%s finish\n", __func__);

	return 0;
}

static int disp_resume(struct device *dev)
{
	u32 screen_id = 0;
	int num_screens = bsp_disp_feat_get_num_screens();
	struct disp_manager *mgr = NULL;
#if defined(CONFIG_PM_RUNTIME)
	struct disp_device* dispdev = NULL;
	struct list_head* disp_list= NULL;

	disp_list = disp_device_get_list_head();
	list_for_each_entry(dispdev, disp_list, list) {
		if (dispdev->resume) {
			dispdev->resume(dispdev);
		}
	}
	if (g_pm_runtime_enable) {
		for (screen_id=0; screen_id<num_screens; screen_id++) {
			mgr = g_disp_drv.mgr[screen_id];
			if (!mgr || !mgr->device)
				continue;

			if (suspend_output_type[screen_id] == DISP_OUTPUT_TYPE_LCD) {
				schedule_work(&g_disp_drv.resume_work[screen_id]);
			}
		}
		if (g_pm_runtime_enable) {
			if (g_disp_drv.dev) {
				pm_runtime_disable(g_disp_drv.dev);
				pm_runtime_set_active(g_disp_drv.dev);
				pm_runtime_enable(g_disp_drv.dev);
			} else {
				pr_warn("%s, display device is null\n", __func__);
			}
		}
	}
	else {
		for (screen_id=0; screen_id<num_screens; screen_id++) {
			mgr = g_disp_drv.mgr[screen_id];
			if (!mgr || !mgr->device)
				continue;

			if (suspend_output_type[screen_id] != DISP_OUTPUT_TYPE_NONE) {
				if (mgr->device->set_mode && mgr->device->get_mode) {
						u32 mode = mgr->device->get_mode(mgr->device);

						mgr->device->set_mode(mgr->device, mode);
				}
				if (!mgr->device->is_enabled(mgr->device)) {
					mgr->device->enable(mgr->device);
				}
			}
		}
		disp_resume_cb();
	}
#else
	struct disp_device* dispdev = NULL;
	struct list_head* disp_list= NULL;

	disp_list = disp_device_get_list_head();
	list_for_each_entry(dispdev, disp_list, list) {
		if (dispdev->resume) {
			dispdev->resume(dispdev);
		}
	}

	for (screen_id=0; screen_id<num_screens; screen_id++) {
		mgr = g_disp_drv.mgr[screen_id];
		if (!mgr || !mgr->device)
			continue;

		if (suspend_output_type[screen_id] != DISP_OUTPUT_TYPE_NONE) {
			if (mgr->device->set_mode && mgr->device->get_mode) {
					u32 mode = mgr->device->get_mode(mgr->device);

					mgr->device->set_mode(mgr->device, mode);
			}
			mgr->device->enable(mgr->device);
		}
	}
	disp_resume_cb();
#endif

	suspend_status &= (~DISPLAY_DEEP_SLEEP);
	suspend_prestep = 2;

	pr_info("%s finish\n", __func__);

	return 0;
}

static const struct dev_pm_ops disp_runtime_pm_ops =
{
#ifdef CONFIG_PM_RUNTIME
	.runtime_suspend = disp_runtime_suspend,
	.runtime_resume  = disp_runtime_resume,
	.runtime_idle    = disp_runtime_idle,
#endif
	.suspend  = disp_suspend,
	.resume   = disp_resume,
};

static void disp_shutdown(struct platform_device *pdev)
{
	u32 screen_id = 0;
	int num_screens;

	num_screens = bsp_disp_feat_get_num_screens();

	for (screen_id=0; screen_id<num_screens; screen_id++) {
		struct disp_manager *mgr = g_disp_drv.mgr[screen_id];

		if (mgr && mgr->device && mgr->device->is_enabled && mgr->device->disable) {
			if (mgr->device->is_enabled(mgr->device)) {
				mgr->device->disable(mgr->device);
			}
		}
	}

	return ;
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

		if (!g_pm_runtime_enable)
			break;

		if (0 != ubuffer[0])
			break;

		if (ubuffer[1]) {
#if defined(CONFIG_PM_RUNTIME)
			if (g_disp_drv.dev)
				pm_runtime_put(g_disp_drv.dev);
			else
				pr_warn("%s, display device is null\n", __func__);
#endif
			suspend_status |= DISPLAY_BLANK;
			disp_blank(true);
		} else {
			if (power_status_init) {
				/* avoid first unblank, because device is ready when driver init */
				power_status_init = 0;
				break;
			}

			disp_blank(false);
			suspend_status &= ~DISPLAY_BLANK;
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
		/* if the whhole display device have already enter blank status,
		 * DISP_DEVICE_SWITCH request will not be responsed.
		 */
		if (!(suspend_status & DISPLAY_BLANK))
			ret = bsp_disp_device_switch(ubuffer[0], (enum disp_output_type)ubuffer[1], (enum disp_output_type)ubuffer[2]);
		suspend_output_type[ubuffer[0]] = ubuffer[1];
	#if defined(SUPPORT_TV) && defined(CONFIG_ARCH_SUN8IW7)
		bsp_disp_tv_set_hpd(1);
	#endif
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
	case DISP_HDMI_SUPPORT_MODE:
	{
		ret = bsp_disp_hdmi_check_support_mode(ubuffer[0], ubuffer[1]);
		break;
	}

	case DISP_SET_TV_HPD:
	{
		ret = bsp_disp_tv_set_hpd(ubuffer[0]);
		break;
	}

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

	//----for test----
	case DISP_MEM_REQUEST:
		ret =  disp_mem_request(ubuffer[0],ubuffer[1]);
		break;

	case DISP_MEM_RELEASE:
		ret =  disp_mem_release(ubuffer[0]);
		break;

	case DISP_MEM_GETADR:
		return g_disp_mm[ubuffer[0]].mem_start;

	default:
		ret = disp_ioctl_extend(cmd, (unsigned long)ubuffer);
		break;
	}

  return ret;
}

#ifdef CONFIG_COMPAT
static long disp_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	compat_uptr_t karg[4];
	unsigned long __user *ubuffer;

	if (copy_from_user((void*)karg,(void __user*)arg,4*sizeof(compat_uptr_t))) {
		__wrn("copy_from_user fail\n");
		return -EFAULT;
	}

	ubuffer = compat_alloc_user_space(4*sizeof(unsigned long));
	if (!access_ok(VERIFY_WRITE, ubuffer, 4*sizeof(unsigned long)))
		return -EFAULT;

	if (put_user(karg[0], &ubuffer[0]) ||
			put_user(karg[1], &ubuffer[1]) ||
			put_user(karg[2], &ubuffer[2]) ||
			put_user(karg[3], &ubuffer[3])) {
				__wrn("put_user fail\n");
		return -EFAULT;
	}

	if (DISP_HWC_COMMIT == cmd)
		return disp_compat_ioctl_extend(cmd, (unsigned long)ubuffer);

	return disp_ioctl(file, cmd, (unsigned long)ubuffer);
}
#endif

static const struct file_operations disp_fops = {
	.owner    = THIS_MODULE,
	.open     = disp_open,
	.release  = disp_release,
	.write    = disp_write,
	.read     = disp_read,
	.unlocked_ioctl = disp_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = disp_compat_ioctl,
#endif
	.mmap     = disp_mmap,
};

#ifndef CONFIG_OF
static struct platform_device disp_device = {
	.name           = "disp",
	.id             = -1,
	.num_resources  = ARRAY_SIZE(disp_resource),
	.resource       = disp_resource,
	.dev            =
	{
		.power        =
		{
			.async_suspend = 1,
		}
	}
};
#else
static const struct of_device_id sunxi_disp_match[] = {
	{ .compatible = "allwinner,sun50i-disp", },
	{},
}; 
#endif

static struct platform_driver disp_driver = {
	.probe    = disp_probe,
	.remove   = disp_remove,
	.shutdown = disp_shutdown,
	.driver   =
	{
		.name   = "disp",
		.owner  = THIS_MODULE,
		.pm	= &disp_runtime_pm_ops,
		.of_match_table = sunxi_disp_match,
	},
};

#ifdef CONFIG_DEVFREQ_DRAM_FREQ_IN_VSYNC
struct dramfreq_vb_time_ops
{
    int (*get_vb_time) (void);
    int (*get_next_vb_time) (void);
    int (*is_in_vb)(void);
};
extern s32 bsp_disp_get_vb_time(void);
extern s32 bsp_disp_get_next_vb_time(void);
extern int dramfreq_set_vb_time_ops(struct dramfreq_vb_time_ops *ops);
static struct dramfreq_vb_time_ops dramfreq_ops =
{
	.get_vb_time = bsp_disp_get_vb_time,
	.get_next_vb_time = bsp_disp_get_next_vb_time,
	.is_in_vb = bsp_disp_is_in_vb,
};
#endif

extern int disp_attr_node_init(void);
extern int capture_module_init(void);
extern void  capture_module_exit(void);
static int __init disp_module_init(void)
{
	int ret = 0, err;

	pr_info("[DISP]%s\n", __func__);

	alloc_chrdev_region(&devid, 0, 1, "disp");
	my_cdev = cdev_alloc();
	cdev_init(my_cdev, &disp_fops);
	my_cdev->owner = THIS_MODULE;
	err = cdev_add(my_cdev, devid, 1);
	if (err) {
		__wrn("cdev_add fail\n");
		return -1;
	}

	disp_class = class_create(THIS_MODULE, "disp");
	if (IS_ERR(disp_class))	{
		__wrn("class_create fail\n");
		return -1;
	}

	display_dev = device_create(disp_class, NULL, devid, NULL, "disp");

#ifndef CONFIG_OF
	ret = platform_device_register(&disp_device);
#endif
	if (ret == 0) {
		ret = platform_driver_register(&disp_driver);
	}
#ifdef CONFIG_DISP2_SUNXI_DEBUG
	dispdbg_init();
#endif

#ifdef CONFIG_DEVFREQ_DRAM_FREQ_IN_VSYNC
	dramfreq_set_vb_time_ops(&dramfreq_ops);
#endif

	pr_info("[DISP]%s finish\n", __func__);

	return ret;
}

static void __exit disp_module_exit(void)
{
	__inf("disp_module_exit\n");

#ifdef CONFIG_DISP2_SUNXI_DEBUG
	dispdbg_exit();
#endif

	disp_exit();

	platform_driver_unregister(&disp_driver);
#ifndef CONFIG_OF
	platform_device_unregister(&disp_device);
#endif

	device_destroy(disp_class,  devid);
	class_destroy(disp_class);

	cdev_del(my_cdev);
}

EXPORT_SYMBOL(disp_set_hdmi_func);
//EXPORT_SYMBOL(sunxi_disp_get_source_ops);

module_init(disp_module_init);
module_exit(disp_module_exit);

MODULE_AUTHOR("tyle");
MODULE_DESCRIPTION("display driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:disp");


