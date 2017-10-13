/* linux/drivers/video/sunxi/disp2/disp/dev_fb.c
 *
 * Copyright (c) 2013 Allwinnertech Co., Ltd.
 * Author: Tyle <tyle@allwinnertech.com>
 *
 * Framebuffer driver for sunxi platform
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include "dev_disp.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fb.h>
#include <linux/memblock.h>


typedef struct
{
	struct device           *dev;

	bool                    fb_enable[FB_MAX];
	enum disp_fb_mode            fb_mode[FB_MAX];
	u32                     layer_hdl[FB_MAX][2];//channel, layer_id
	struct fb_info *        fbinfo[FB_MAX];
	struct disp_fb_create_info     fb_para[FB_MAX];
	u32                     pseudo_palette [FB_MAX][16];
	wait_queue_head_t       wait[3];
	unsigned long           wait_count[3];
	struct task_struct      *vsync_task[3];
	ktime_t                 vsync_timestamp[3];

	int                     blank[3];
}fb_info_t;

static fb_info_t g_fbi;
static phys_addr_t bootlogo_addr = 0;
static int bootlogo_sz = 0;

extern disp_drv_info g_disp_drv;

static struct __fb_addr_para g_fb_addr;

s32 sunxi_get_fb_addr_para(struct __fb_addr_para *fb_addr_para)
{
	if (fb_addr_para){
		fb_addr_para->fb_paddr = g_fb_addr.fb_paddr;
		fb_addr_para->fb_size  = g_fb_addr.fb_size;
		return 0;
	}

	return -1;
}
EXPORT_SYMBOL(sunxi_get_fb_addr_para);


#define sys_put_wvalue(addr, data) writel(data, (void __iomem *)addr)
s32 fb_draw_colorbar(char * base, u32 width, u32 height, struct fb_var_screeninfo *var)
{
	u32 i=0, j=0;

	if (!base)
		return -1;

	for (i = 0; i<height; i++) {
		for (j = 0; j<width/4; j++) {
			u32 offset = 0;

			if (var->bits_per_pixel == 32)	{
					offset = width * i + j;
					sys_put_wvalue(base + offset*4, (((1<<var->transp.length)-1)<<var->transp.offset) | (((1<<var->red.length)-1)<<var->red.offset));

					offset = width * i + j + width/4;
					sys_put_wvalue(base + offset*4, (((1<<var->transp.length)-1)<<var->transp.offset) | (((1<<var->green.length)-1)<<var->green.offset));

					offset = width * i + j + width/4*2;
					sys_put_wvalue(base + offset*4, (((1<<var->transp.length)-1)<<var->transp.offset) | (((1<<var->blue.length)-1)<<var->blue.offset));

					offset = width * i + j + width/4*3;
					sys_put_wvalue(base + offset*4, (((1<<var->transp.length)-1)<<var->transp.offset) | (((1<<var->red.length)-1)<<var->red.offset) | (((1<<var->green.length)-1)<<var->green.offset));
				}
#if 0
				else if (var->bits_per_pixel == 16) {
					offset = width * i + j;
					sys_put_hvalue(base + offset*2, (((1<<var->transp.length)-1)<<var->transp.offset) | (((1<<var->red.length)-1)<<var->red.offset));

					offset = width * i + j + width/4;
					sys_put_hvalue(base + offset*2, (((1<<var->transp.length)-1)<<var->transp.offset) | (((1<<var->green.length)-1)<<var->green.offset));

					offset = width * i + j + width/4*2;
					sys_put_hvalue(base + offset*2, (((1<<var->transp.length)-1)<<var->transp.offset) | (((1<<var->blue.length)-1)<<var->blue.offset));

					offset = width * i + j + width/4*3;
					sys_put_hvalue(base + offset*2, (((1<<var->transp.length)-1)<<var->transp.offset) | (((1<<var->red.length)-1)<<var->red.offset) | (((1<<var->green.length)-1)<<var->green.offset));
			}
#endif
		}
	}

	return 0;
}

s32 fb_draw_gray_pictures(char * base, u32 width, u32 height, struct fb_var_screeninfo *var)
{
	u32 time = 0;

	for (time = 0; time<18; time++) {
		u32 i=0, j=0;

		for (i = 0; i<height; i++)	{
			for (j = 0; j<width; j++) {
				char *addr = base + (i*width+ j)*4;
				u32 value = (0xff<<24) | ((time*15)<<16) | ((time*15)<<8) | (time*15);

				sys_put_wvalue((void*)addr, value);
			}
		}
	}
	return 0;
}

static int Fb_map_video_memory(struct fb_info *info)
{
	info->screen_base = (char __iomem *)disp_malloc(info->fix.smem_len, (u32 *)(&info->fix.smem_start));
	if (info->screen_base)	{
		__inf("Fb_map_video_memory(reserve), pa=0x%p size:0x%x\n",(void*)info->fix.smem_start, (unsigned int)info->fix.smem_len);
		memset((void* __force)info->screen_base,0x0,info->fix.smem_len);

		g_fb_addr.fb_paddr = (uintptr_t)info->fix.smem_start;
		g_fb_addr.fb_size = info->fix.smem_len;

		return 0;
	} else {
		__wrn("disp_malloc fail!\n");
		return -ENOMEM;
	}

	return 0;
}


static inline void Fb_unmap_video_memory(struct fb_info *info)
{
	disp_free((void * __force)info->screen_base, (void*)info->fix.smem_start, info->fix.smem_len);
}

static void *Fb_map_kernel(unsigned long phys_addr, unsigned long size)
{
	int npages = PAGE_ALIGN(size) / PAGE_SIZE;
	struct page **pages = vmalloc(sizeof(struct page *) * npages);
	struct page **tmp = pages;
	struct page *cur_page = phys_to_page(phys_addr);
	pgprot_t pgprot;
	void *vaddr = NULL;
	int i;

	if (!pages)
		return NULL;

	for (i = 0; i < npages; i++)
		*(tmp++) = cur_page++;

	pgprot = pgprot_noncached(PAGE_KERNEL);
	vaddr = vmap(pages, npages, VM_MAP, pgprot);

	vfree(pages);
	return vaddr;
}

static void Fb_unmap_kernel(void *vaddr)
{
	vunmap(vaddr);
}

static s32 disp_fb_to_var(enum disp_pixel_format format, struct fb_var_screeninfo *var)
{
	switch(format) {
	case DISP_FORMAT_ARGB_8888:
		var->bits_per_pixel = 32;
		var->transp.length = 8;
		var->red.length = 8;
		var->green.length = 8;
		var->blue.length = 8;
		var->blue.offset = 0;
		var->green.offset = var->blue.offset + var->blue.length;
		var->red.offset = var->green.offset + var->green.length;
		var->transp.offset = var->red.offset + var->red.length;
		break;
	case DISP_FORMAT_ABGR_8888:
		var->bits_per_pixel = 32;
		var->transp.length = 8;
		var->red.length = 8;
		var->green.length = 8;
		var->blue.length = 8;
		var->red.offset = 0;
		var->green.offset = var->red.offset + var->red.length;
		var->blue.offset = var->green.offset + var->green.length;
		var->transp.offset = var->blue.offset + var->blue.length;
		break;
	case DISP_FORMAT_RGBA_8888:
		var->bits_per_pixel = 32;
		var->transp.length = 8;
		var->red.length = 8;
		var->green.length = 8;
		var->blue.length = 8;
		var->transp.offset = 0;
		var->blue.offset = var->transp.offset + var->transp.length;
		var->green.offset = var->blue.offset + var->blue.length;
		var->red.offset = var->green.offset + var->green.length;
		break;
	case DISP_FORMAT_BGRA_8888:
		var->bits_per_pixel = 32;
		var->transp.length = 8;
		var->red.length = 8;
		var->green.length = 8;
		var->blue.length = 8;
		var->transp.offset = 0;
		var->red.offset = var->transp.offset + var->transp.length;
		var->green.offset = var->red.offset + var->red.length;
		var->blue.offset = var->green.offset + var->green.length;
		break;
	case DISP_FORMAT_RGB_888:
		var->bits_per_pixel = 24;
		var->red.length = 8;
		var->green.length = 8;
		var->blue.length = 8;
		var->blue.offset = 0;
		var->green.offset = var->blue.offset + var->blue.length;
		var->red.offset = var->green.offset + var->green.length;

		break;
	case DISP_FORMAT_BGR_888:
		var->bits_per_pixel = 24;
		var->transp.length = 0;
		var->red.length = 8;
		var->green.length = 8;
		var->blue.length = 8;
		var->red.offset = 0;
		var->green.offset = var->red.offset + var->red.length;
		var->blue.offset = var->green.offset + var->green.length;

		break;
	case DISP_FORMAT_RGB_565:
		var->bits_per_pixel = 16;
		var->transp.length = 0;
		var->red.length = 5;
		var->green.length = 6;
		var->blue.length = 5;
		var->blue.offset = 0;
		var->green.offset = var->blue.offset + var->blue.length;
		var->red.offset = var->green.offset + var->green.length;

		break;
	case DISP_FORMAT_BGR_565:
		var->bits_per_pixel = 16;
		var->transp.length = 0;
		var->red.length = 5;
		var->green.length = 6;
		var->blue.length = 5;
		var->red.offset = 0;
		var->green.offset = var->red.offset + var->red.length;
		var->blue.offset = var->green.offset + var->green.length;

		break;
	case DISP_FORMAT_ARGB_4444:
		var->bits_per_pixel = 16;
		var->transp.length = 4;
		var->red.length = 4;
		var->green.length = 4;
		var->blue.length = 4;
		var->blue.offset = 0;
		var->green.offset = var->blue.offset + var->blue.length;
		var->red.offset = var->green.offset + var->green.length;
		var->transp.offset = var->red.offset + var->red.length;

		break;
	case DISP_FORMAT_ABGR_4444:
		var->bits_per_pixel = 16;
		var->transp.length = 4;
		var->red.length = 4;
		var->green.length = 4;
		var->blue.length = 4;
		var->red.offset = 0;
		var->green.offset = var->red.offset + var->red.length;
		var->blue.offset = var->green.offset + var->green.length;
		var->transp.offset = var->blue.offset + var->blue.length;

		break;
	case DISP_FORMAT_RGBA_4444:
		var->bits_per_pixel = 16;
		var->transp.length = 4;
		var->red.length = 4;
		var->green.length = 5;
		var->blue.length = 4;
		var->transp.offset = 0;
		var->blue.offset = var->transp.offset + var->transp.length;
		var->green.offset = var->blue.offset + var->blue.length;
		var->red.offset = var->green.offset + var->green.length;

		break;
	case DISP_FORMAT_BGRA_4444:
		var->bits_per_pixel = 16;
		var->transp.length = 4;
		var->red.length = 4;
		var->green.length = 4;
		var->blue.length = 4;
		var->transp.offset = 0;
		var->red.offset = var->transp.offset + var->transp.length;
		var->green.offset = var->red.offset + var->red.length;
		var->blue.offset = var->green.offset + var->green.length;

		break;
	case DISP_FORMAT_ARGB_1555:
		var->bits_per_pixel = 16;
		var->transp.length = 1;
		var->red.length = 5;
		var->green.length = 5;
		var->blue.length = 5;
		var->blue.offset = 0;
		var->green.offset = var->blue.offset + var->blue.length;
		var->red.offset = var->green.offset + var->green.length;
		var->transp.offset = var->red.offset + var->red.length;

		break;
	case DISP_FORMAT_ABGR_1555:
		var->bits_per_pixel = 16;
		var->transp.length = 1;
		var->red.length = 5;
		var->green.length = 5;
		var->blue.length = 5;
		var->red.offset = 0;
		var->green.offset = var->red.offset + var->red.length;
		var->blue.offset = var->green.offset + var->green.length;
		var->transp.offset = var->blue.offset + var->blue.length;

		break;
	case DISP_FORMAT_RGBA_5551:
		var->bits_per_pixel = 16;
		var->transp.length = 1;
		var->red.length = 5;
		var->green.length = 5;
		var->blue.length = 5;
		var->transp.offset = 0;
		var->blue.offset = var->transp.offset + var->transp.length;
		var->green.offset = var->blue.offset + var->blue.length;
		var->red.offset = var->green.offset + var->green.length;

		break;
	case DISP_FORMAT_BGRA_5551:
		var->bits_per_pixel = 16;
		var->transp.length = 1;
		var->red.length = 5;
		var->green.length = 5;
		var->blue.length = 5;
		var->transp.offset = 0;
		var->red.offset = var->transp.offset + var->transp.length;
		var->green.offset = var->red.offset + var->red.length;
		var->blue.offset = var->green.offset + var->green.length;

		break;
	default:
		__wrn("[FB]not support format %d\n", format);
	}

	__inf("disp_fb_to_var, format%d para: %dbpp, alpha(%d,%d),reg(%d,%d),green(%d,%d),blue(%d,%d)\n", (int)format, (int)var->bits_per_pixel,
	    (int)var->transp.offset, (int)var->transp.length, (int)var->red.offset, (int)var->red.length, (int)var->green.offset,
	    (int)var->green.length, (int)var->blue.offset, (int)var->blue.length);

	return 0;
}

static s32 var_to_disp_fb(struct disp_fb_info *fb, struct fb_var_screeninfo *var, struct fb_fix_screeninfo * fix)
{
	if (var->nonstd == 0)//argb
	{
		switch (var->bits_per_pixel)
		{
			case 32:
				if (var->red.offset == 16 && var->green.offset == 8 && var->blue.offset == 0)
					fb->format = DISP_FORMAT_ARGB_8888;
				else if (var->blue.offset == 24 && var->green.offset == 16 && var->red.offset == 8)
					fb->format = DISP_FORMAT_BGRA_8888;
				else if (var->blue.offset == 16 && var->green.offset == 8 && var->red.offset == 0)
					fb->format = DISP_FORMAT_ABGR_8888;
				else if (var->red.offset == 24 && var->green.offset == 16 && var->blue.offset == 8)
					fb->format = DISP_FORMAT_RGBA_8888;
				else
					__wrn("[FB]invalid argb format<transp.offset:%d,red.offset:%d,green.offset:%d,blue.offset:%d>\n",
							var->transp.offset,var->red.offset,var->green.offset,var->blue.offset);

				break;
			case 24:
				if (var->red.offset == 16 && var->green.offset == 8 && var->blue.offset == 0) { //rgb
					fb->format = DISP_FORMAT_RGB_888;
				} else if (var->blue.offset == 16 && var->green.offset == 8 && var->red.offset == 0) {//bgr
					fb->format = DISP_FORMAT_BGR_888;
				} else {
					__wrn("[FB]invalid format<transp.offset:%d,red.offset:%d,green.offset:%d,blue.offset:%d>\n",
							var->transp.offset,var->red.offset,var->green.offset,var->blue.offset);
				}

				break;
			case 16:
				if (var->red.offset == 11 && var->green.offset == 5 && var->blue.offset == 0) {
					fb->format = DISP_FORMAT_RGB_565;
				} else if (var->blue.offset == 11 && var->green.offset == 5 && var->red.offset == 0) {
					fb->format = DISP_FORMAT_BGR_565;
				} else if (var->transp.offset == 12 && var->red.offset == 8 &&
						var->green.offset == 4 && var->blue.offset == 0) {
					fb->format = DISP_FORMAT_ARGB_4444;
				} else if (var->transp.offset == 12 && var->blue.offset == 8 &&
						var->green.offset == 4 && var->red.offset == 0) {
					fb->format = DISP_FORMAT_ABGR_4444;
				} else if (var->red.offset == 12 && var->green.offset == 8 &&
						var->blue.offset == 4 && var->transp.offset == 0) {
					fb->format = DISP_FORMAT_RGBA_4444;
				} else if (var->blue.offset == 12 && var->green.offset == 8 &&
						var->red.offset == 4 && var->transp.offset == 0) {
					fb->format = DISP_FORMAT_BGRA_4444;
				} else if (var->transp.offset == 15 && var->red.offset == 10 &&
						var->green.offset == 5 && var->blue.offset == 0) {
					fb->format = DISP_FORMAT_ARGB_1555;
				} else if (var->transp.offset == 15 && var->blue.offset == 10 &&
						var->green.offset == 5 && var->red.offset == 0) {
					fb->format = DISP_FORMAT_ABGR_1555;
				} else if (var->red.offset == 11 && var->green.offset == 6 &&
						var->blue.offset == 1 && var->transp.offset == 0) {
					fb->format = DISP_FORMAT_RGBA_5551;
				} else if (var->blue.offset == 11 && var->green.offset == 6 &&
						var->red.offset == 1 && var->transp.offset == 0) {
					fb->format = DISP_FORMAT_BGRA_5551;
				} else {
					__wrn("[FB]invalid format<transp.offset:%d,red.offset:%d,green.offset:%d,blue.offset:%d>\n",
							var->transp.offset,var->red.offset,var->green.offset,var->blue.offset);
				}

				break;

			default:
				__wrn("invalid bits_per_pixel :%d\n", var->bits_per_pixel);
				return -EINVAL;
		}
	}
	__inf("var_to_disp_fb, format%d para: %dbpp, alpha(%d,%d),reg(%d,%d),green(%d,%d),blue(%d,%d)\n", (int)fb->format, (int)var->bits_per_pixel,
	    (int)var->transp.offset, (int)var->transp.length, (int)var->red.offset, (int)var->red.length, (int)var->green.offset,
	  (int)var->green.length, (int)var->blue.offset, (int)var->blue.length);

	fb->size[0].width = var->xres_virtual;
	fb->size[1].width = var->xres_virtual;
	fb->size[2].width = var->xres_virtual;

	fix->line_length = (var->xres_virtual * var->bits_per_pixel) / 8;

	return 0;
}


static int sunxi_fb_open(struct fb_info *info, int user)
{
	return 0;
}
static int sunxi_fb_release(struct fb_info *info, int user)
{
	return 0;
}


static int fb_wait_for_vsync(struct fb_info *info)
{
	unsigned long count;
	u32 sel = 0;
	int ret;
	int num_screens;

	num_screens = bsp_disp_feat_get_num_screens();

	for (sel = 0; sel < num_screens; sel++) {
		if (sel==g_fbi.fb_mode[info->node]) {
			struct disp_manager *mgr = g_disp_drv.mgr[sel];

			if (!mgr || !mgr->device || (NULL == mgr->device->is_enabled))
					return 0;

			if (0 == mgr->device->is_enabled(mgr->device))
				return 0;

			count = g_fbi.wait_count[sel];
			ret = wait_event_interruptible_timeout(g_fbi.wait[sel], count != g_fbi.wait_count[sel], msecs_to_jiffies(50));
			if (ret == 0)	{
				__inf("timeout\n");
				return -ETIMEDOUT;
			}
		}
	}

	return 0;
}

static int sunxi_fb_pan_display(struct fb_var_screeninfo *var,struct fb_info *info)
{
	u32 sel = 0;
	u32 num_screens;

	num_screens = bsp_disp_feat_get_num_screens();

	__inf("fb %d, pos=%d,%d\n", info->node, var->xoffset, var->yoffset);

	for (sel = 0; sel < num_screens; sel++) {
		if (sel==g_fbi.fb_mode[info->node]) {
			s32 chan = g_fbi.layer_hdl[info->node][0];
			s32 layer_id = g_fbi.layer_hdl[info->node][1];
			struct disp_layer_config config;
			struct disp_manager *mgr = g_disp_drv.mgr[sel];

			memset(&config, 0, sizeof(struct disp_layer_config));
			if (mgr && mgr->get_layer_config && mgr->set_layer_config) {
				config.channel = chan;
				config.layer_id = layer_id;
				if (0 != mgr->get_layer_config(mgr, &config, 1)) {
					__wrn("fb %d, get_layer_config(%d,%d,%d) fail\n", info->node, sel, chan, layer_id);
					return -1;
				}
				config.info.fb.crop.x = ((long long)(var->xoffset)) << 32;
				config.info.fb.crop.y = ((long long)(var->yoffset)) << 32;
				config.info.fb.crop.width = ((long long)(var->xres)) << 32;
				config.info.fb.crop.height = ((long long)(var->yres)) << 32;
				if (0 != mgr->set_layer_config(mgr, &config, 1)) {
					__wrn("fb %d, set_layer_config(%d,%d,%d) fail\n", info->node, sel, chan, layer_id);
					return -1;
				}
			}
		}
	}

	fb_wait_for_vsync(info);

	return 0;
}

static int sunxi_fb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	struct disp_fb_info disp_fb;
	struct fb_fix_screeninfo fix;

	if (var_to_disp_fb(&disp_fb, var, &fix) != 0) {
		switch (var->bits_per_pixel) {
			case 8:
			case 4:
			case 2:
			case 1:
				disp_fb_to_var(DISP_FORMAT_ARGB_8888, var);
				break;

			case 19:
			case 18:
			case 16:
				disp_fb_to_var(DISP_FORMAT_RGB_565, var);
			break;

			case 32:
			case 28:
			case 25:
			case 24:
				disp_fb_to_var(DISP_FORMAT_ARGB_8888, var);
				break;

		default:
			return -EINVAL;
		}
	}

	return 0;
}

static int sunxi_fb_set_par(struct fb_info *info)
{
	u32 sel = 0;
	u32 num_screens;

	num_screens = bsp_disp_feat_get_num_screens();

	__inf("sunxi_fb_set_par\n");

	for (sel = 0; sel < num_screens; sel++) {
		if (sel==g_fbi.fb_mode[info->node]) {
			struct fb_var_screeninfo *var = &info->var;
			struct fb_fix_screeninfo * fix = &info->fix;
			s32 chan = g_fbi.layer_hdl[info->node][0];
			s32 layer_id = g_fbi.layer_hdl[info->node][1];
			struct disp_layer_config config;
			struct disp_manager *mgr = g_disp_drv.mgr[sel];

			if (mgr && mgr->get_layer_config) {
				config.channel = chan;
				config.layer_id = layer_id;
				mgr->get_layer_config(mgr, &config, 1);
			}

			var_to_disp_fb(&(config.info.fb), var, fix);
			config.info.fb.crop.x = ((long long)(var->xoffset)) << 32;
			config.info.fb.crop.y = ((long long)(var->yoffset)) << 32;
			config.info.fb.crop.width = ((long long)(var->xres)) << 32;
			config.info.fb.crop.height = ((long long)(var->yres)) << 32;
			config.info.screen_win.width = var->xres;
			config.info.screen_win.height = var->yres;
			if (mgr && mgr->set_layer_config)
				mgr->set_layer_config(mgr, &config, 1);
		}
	}
	return 0;
}

static int sunxi_fb_blank(int blank_mode, struct fb_info *info)
{
	u32 sel = 0;
	u32 num_screens;

	num_screens = bsp_disp_feat_get_num_screens();

	__inf("sunxi_fb_blank,mode:%d\n",blank_mode);

	for (sel = 0; sel < num_screens; sel++) {
		if (sel == g_fbi.fb_mode[info->node]) {
			s32 chan = g_fbi.layer_hdl[info->node][0];
			s32 layer_id = g_fbi.layer_hdl[info->node][1];
			struct disp_layer_config config;
			struct disp_manager *mgr = g_disp_drv.mgr[sel];

			if (blank_mode == FB_BLANK_POWERDOWN)	{
				if (mgr && mgr->get_layer_config && mgr->set_layer_config) {
					config.channel = chan;
					config.layer_id = layer_id;
					mgr->get_layer_config(mgr, &config, 1);
					config.enable = 0;
					mgr->set_layer_config(mgr, &config, 1);
				}
			}	else {
				if (mgr && mgr->get_layer_config && mgr->set_layer_config) {
					config.channel = chan;
					config.layer_id = layer_id;
					mgr->get_layer_config(mgr, &config, 1);
					config.enable = 1;
					mgr->set_layer_config(mgr, &config, 1);
				}
			}
		}
	}
	return 0;
}

static int sunxi_fb_cursor(struct fb_info *info, struct fb_cursor *cursor)
{
	__inf("sunxi_fb_cursor\n");

	return -EINVAL;	/* just to force soft_cursor() call */
}

s32 drv_disp_vsync_event(u32 sel)
{
	g_fbi.vsync_timestamp[sel] = ktime_get();

	if (g_fbi.vsync_task[sel])
		wake_up_process(g_fbi.vsync_task[sel]);

	return 0;
}

static int vsync_proc(u32 disp)
{
	char buf[64];
	char *envp[2];

	snprintf(buf, sizeof(buf), "VSYNC%d=%llu",disp, ktime_to_ns(g_fbi.vsync_timestamp[disp]));
	envp[0] = buf;
	envp[1] = NULL;
	kobject_uevent_env(&g_fbi.dev->kobj, KOBJ_CHANGE, envp);

	return 0;
}

static int vsync_thread(void *parg)
{
	unsigned long disp = (unsigned long)parg;

	while (1) {

		vsync_proc(disp);
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
		if (kthread_should_stop())
			break;
		set_current_state(TASK_RUNNING);
	}

	return 0;
}

void DRV_disp_int_process(u32 sel)
{
	g_fbi.wait_count[sel]++;
	wake_up_interruptible(&g_fbi.wait[sel]);

	return ;
}

static inline u32 convert_bitfield(int val, struct fb_bitfield *bf)
{
	u32 mask = ((1 << bf->length) - 1) << bf->offset;
	return (val << bf->offset) & mask;
}

static int sunxi_fb_setcolreg(unsigned regno, unsigned red, unsigned green,
			    unsigned blue, unsigned transp, struct fb_info *info)
{
	u32 val;
	u32 ret = 0;

	switch (info->fix.visual) {
	case FB_VISUAL_PSEUDOCOLOR:
		ret = -EINVAL;
		break;
	case FB_VISUAL_TRUECOLOR:
		if (regno < 16) {
			val = convert_bitfield(transp, &info->var.transp) |
				convert_bitfield(red, &info->var.red) |
				convert_bitfield(green, &info->var.green) |
				convert_bitfield(blue, &info->var.blue);
			__inf("Fb_setcolreg,regno=%2d,a=%2X,r=%2X,g=%2X,b=%2X, "
					"result=%08X\n", regno, transp, red, green, blue,
					val);
			((u32 *) info->pseudo_palette)[regno] = val;
		} else {
			ret = 0;
		}
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int sunxi_fb_setcmap(struct fb_cmap *cmap, struct fb_info *info)
{
	unsigned int j, r = 0;
	unsigned char hred, hgreen, hblue, htransp = 0xff;
	unsigned short *red, *green, *blue, *transp;

	__inf("Fb_setcmap, cmap start:%d len:%d, %dbpp\n", cmap->start,
			cmap->len, info->var.bits_per_pixel);

	red = cmap->red;
	green = cmap->green;
	blue = cmap->blue;
	transp = cmap->transp;

	for (j = 0; j < cmap->len; j++) {
		hred = *red++;
		hgreen = *green++;
		hblue = *blue++;
		if (transp)
			htransp = (*transp++) & 0xff;
		else
			htransp = 0xff;

		r = sunxi_fb_setcolreg(cmap->start + j, hred, hgreen, hblue, htransp,
				info);
		if (r)
			return r;
	}

	return 0;
}


static int sunxi_fb_ioctl(struct fb_info *info, unsigned int cmd,unsigned long arg)
{
	long ret = 0;

	switch (cmd) {
#if 0
	case FBIOGET_VBLANK:
	{
		struct fb_vblank vblank;
		struct disp_video_timings tt;
		u32 line = 0;
		u32 sel;

		sel = (g_fbi.fb_mode[info->node] == FB_MODE_SCREEN1)?1:0;
		line = bsp_disp_get_cur_line(sel);
		bsp_disp_get_timming(sel, &tt);

		memset(&vblank, 0, sizeof(struct fb_vblank));
		vblank.flags |= FB_VBLANK_HAVE_VBLANK;
		vblank.flags |= FB_VBLANK_HAVE_VSYNC;
		if (line <= (tt.ver_total_time-tt.y_res))	{
			vblank.flags |= FB_VBLANK_VBLANKING;
		}
		if ((line > tt.ver_front_porch) && (line < (tt.ver_front_porch+tt.ver_sync_time)))	{
			vblank.flags |= FB_VBLANK_VSYNCING;
		}

		if (copy_to_user((void __user *)arg, &vblank, sizeof(struct fb_vblank)))
		ret = -EFAULT;

		break;
	}
#endif

	case FBIO_WAITFORVSYNC:
	{
		ret = fb_wait_for_vsync(info);
		break;
	}

	default:
		break;
	}
	return ret;
}

static struct fb_ops dispfb_ops =
{
	.owner		    = THIS_MODULE,
	.fb_open        = sunxi_fb_open,
	.fb_release     = sunxi_fb_release,
	.fb_pan_display	= sunxi_fb_pan_display,
	.fb_ioctl       = sunxi_fb_ioctl,
	.fb_check_var   = sunxi_fb_check_var,
	.fb_set_par     = sunxi_fb_set_par,
	.fb_blank       = sunxi_fb_blank,
	.fb_cursor      = sunxi_fb_cursor,
#if defined(CONFIG_FB_CONSOLE_SUNXI)
	.fb_fillrect    = cfb_fillrect,
	.fb_copyarea    = cfb_copyarea,
	.fb_imageblit   = cfb_imageblit,
#endif
	.fb_setcmap     = sunxi_fb_setcmap,
	.fb_setcolreg	= sunxi_fb_setcolreg,

};

static int Fb_map_kernel_logo(u32 sel, struct fb_info *info)
{
	void *vaddr = NULL;
	uintptr_t paddr =  0;
	void *screen_offset = NULL, *image_offset = NULL;
	char *tmp_buffer = NULL;
	char *bmp_data =NULL;
	sunxi_bmp_store_t s_bmp_info;
	sunxi_bmp_store_t *bmp_info = &s_bmp_info;
	bmp_pad_header_t bmp_pad_header;
	bmp_header_t *bmp_header;
	int zero_num = 0;
	unsigned int x, y, bmp_bpix, fb_width, fb_height;
	unsigned int effective_width, effective_height;
	uintptr_t offset;
	int i = 0;

	paddr = bootlogo_addr;
	if (0 == paddr) {
		__inf("Fb_map_kernel_logo failed!");
		return -1;
	}

	/* parser bmp header */
	offset = paddr & ~PAGE_MASK;
	vaddr = (void *)Fb_map_kernel(paddr, sizeof(bmp_header_t));
	if (NULL == vaddr) {
		__wrn("fb_map_kernel failed, paddr=0x%p,size=0x%x\n", (void*)paddr, (unsigned int)sizeof(bmp_header_t));
		return -1;
	}

	memcpy(&bmp_pad_header.signature[0], vaddr + offset, sizeof(bmp_header_t));
	bmp_header = (bmp_header_t *)&bmp_pad_header.signature[0];
	if ((bmp_header->signature[0]!='B') || (bmp_header->signature[1] !='M')) {
		__wrn("this is not a bmp picture.\n");
		return -1;
	}

	bmp_bpix = bmp_header->bit_count/8;

	if ((bmp_bpix != 3) && (bmp_bpix != 4)) {
		return -1;
	}

	if (bmp_bpix ==3) {
		zero_num = (4 - ((3*bmp_header->width) % 4))&3;
	}

	x = bmp_header->width;
	y = (bmp_header->height & 0x80000000) ? (-bmp_header->height):(bmp_header->height);
	fb_width = info->var.xres;
	fb_height = info->var.yres;
	if ((paddr <= 0) || x <= 1 || y <= 1) {
		__wrn("kernel logo para error!\n");
		return -EINVAL;
	}

	bmp_info->x = x;
	bmp_info->y = y;
	bmp_info->bit = bmp_header->bit_count;
	bmp_info->buffer = (void * __force)(info->screen_base);

	if (bmp_bpix == 3)
		info->var.bits_per_pixel = 24;
	else if (bmp_bpix == 4)
		info->var.bits_per_pixel = 32;
	else
		info->var.bits_per_pixel = 32;

	Fb_unmap_kernel(vaddr);

	/* map the total bmp buffer */
	vaddr = (void *)Fb_map_kernel(paddr, x * y * bmp_bpix + sizeof(bmp_header_t));
	if (NULL == vaddr) {
		__wrn("fb_map_kernel failed, paddr=0x%p,size=0x%x\n", (void*)paddr, (unsigned int)(x * y * bmp_bpix + sizeof(bmp_header_t)));
		return -1;
	}

	tmp_buffer = (char *)bmp_info->buffer;
	screen_offset = (void *)bmp_info->buffer;
	bmp_data = (char *)(vaddr + bmp_header->data_offset);
	image_offset = (void *)bmp_data;
	effective_width = (fb_width<x)?fb_width:x;
	effective_height = (fb_height<y)?fb_height:y;

	if (bmp_header->height & 0x80000000) {

		screen_offset = (void *)((void * __force)info->screen_base + (fb_width * (abs(fb_height - y) / 2)
				+ abs(fb_width - x) / 2) * (info->var.bits_per_pixel >> 3));

		for (i=0; i<effective_height; i++) {
			memcpy((void*)screen_offset, image_offset, effective_width*(info->var.bits_per_pixel >> 3));
			screen_offset = (void*)(screen_offset + fb_width*(info->var.bits_per_pixel >> 3));
			image_offset = (void *)image_offset + x *  (info->var.bits_per_pixel >> 3);
		}
	}
	else {

		screen_offset = (void *)((void * __force)info->screen_base + (fb_width * (abs(fb_height - y) / 2)
				+ abs(fb_width - x) / 2) * (info->var.bits_per_pixel >> 3));
		image_offset = (void *)(image_offset + (x * (abs(y - fb_height) / 2)
				+ abs(x - fb_width) / 2) * (info->var.bits_per_pixel >> 3));

		image_offset = (void *)bmp_data + (effective_height-1) * x *  (info->var.bits_per_pixel >> 3);
		for (i=effective_height-1; i>=0; i--) {
			memcpy((void*)screen_offset, image_offset, effective_width*(info->var.bits_per_pixel >> 3));
			screen_offset = (void*)(screen_offset + fb_width*(info->var.bits_per_pixel >> 3));
			image_offset = (void *)bmp_data + i * x *  (info->var.bits_per_pixel >> 3);
		}
    }

	Fb_unmap_kernel(vaddr);
	return 0;
}

static s32 display_fb_request(u32 fb_id, struct disp_fb_create_info *fb_para)
{
	struct fb_info *info = NULL;
	struct disp_layer_config config;
	u32 sel;
	u32 xres, yres;
	u32 num_screens;
	/* fb bound to layer(1,0)  */
	g_fbi.layer_hdl[fb_id][0] = 1;
	g_fbi.layer_hdl[fb_id][1] = 0;

	num_screens = bsp_disp_feat_get_num_screens();

	__inf("%s,fb_id:%d\n", __func__, fb_id);

	if (g_fbi.fb_enable[fb_id]) {
		__wrn("%s, fb%d is already requested!\n", __func__, fb_id);
		return -1;
	}
	info = g_fbi.fbinfo[fb_id];

	xres = fb_para->width;
	yres = fb_para->height;
	if ((0 == xres) || (0 == yres) || (0 == info->var.bits_per_pixel)) {
		__wrn("invalid paras xres(%d), yres(%d) bpp(%d) \n", xres, yres, info->var.bits_per_pixel);
		return -1;
	}

	info->var.xoffset       = 0;
	info->var.yoffset       = 0;
	info->var.xres          = xres;
	info->var.yres          = yres;
	info->var.xres_virtual  = xres;
	info->fix.line_length   = (fb_para->width * info->var.bits_per_pixel) >> 3;
	info->fix.smem_len      = info->fix.line_length * fb_para->height * fb_para->buffer_num;
	if (0 != info->fix.line_length)
		info->var.yres_virtual  = info->fix.smem_len / info->fix.line_length;
	Fb_map_video_memory(info);

	for (sel = 0; sel < num_screens; sel++) {
		if (sel == fb_para->fb_mode)	{
			u32 src_width = xres, src_height = yres;
			struct disp_video_timings tt;
			struct disp_manager *mgr = NULL;
			mgr = g_disp_drv.mgr[sel];
			if (mgr && mgr->device && mgr->device->get_timings) {
				mgr->device->get_timings(mgr->device, &tt);
				if (0 != tt.pixel_clk)
					g_fbi.fbinfo[fb_id]->var.pixclock = 1000000000 / tt.pixel_clk;
				g_fbi.fbinfo[fb_id]->var.left_margin = tt.hor_back_porch;
				g_fbi.fbinfo[fb_id]->var.right_margin = tt.hor_front_porch;
				g_fbi.fbinfo[fb_id]->var.upper_margin = tt.ver_back_porch;
				g_fbi.fbinfo[fb_id]->var.lower_margin = tt.ver_front_porch;
				g_fbi.fbinfo[fb_id]->var.hsync_len = tt.hor_sync_time;
				g_fbi.fbinfo[fb_id]->var.vsync_len = tt.ver_sync_time;
			}
			info->var.width = bsp_disp_get_screen_physical_width(sel);
			info->var.height = bsp_disp_get_screen_physical_height(sel);

			memset(&config, 0, sizeof(struct disp_layer_config));

			config.channel = g_fbi.layer_hdl[fb_id][0];
			config.layer_id = g_fbi.layer_hdl[fb_id][1];
			config.enable = 1;
			Fb_map_kernel_logo(sel, info);
			if (g_disp_drv.para.boot_info.sync == 1) {
				if ((sel == g_disp_drv.para.boot_info.disp) &&
				(g_disp_drv.para.boot_info.type != DISP_OUTPUT_TYPE_NONE)) {
					bsp_disp_get_display_size(sel, &fb_para->output_width, &fb_para->output_height);
				}
			}

			config.info.screen_win.width = (0 == fb_para->output_width)? src_width:fb_para->output_width;
			config.info.screen_win.height = (0 == fb_para->output_height)? src_height:fb_para->output_height;

			config.info.mode = LAYER_MODE_BUFFER;
			config.info.alpha_mode = 1;
			config.info.alpha_value = 0xff;
			config.info.fb.crop.x = 0LL;
			config.info.fb.crop.y = 0LL;
			config.info.fb.crop.width = ((long long)src_width) << 32;
			config.info.fb.crop.height = ((long long)src_height) << 32;
			config.info.screen_win.x = 0;
			config.info.screen_win.y = 0;
			var_to_disp_fb(&(config.info.fb), &(info->var), &(info->fix));
			config.info.fb.addr[0] = (unsigned long long)info->fix.smem_start;
			config.info.fb.addr[1] = 0;
			config.info.fb.addr[2] = 0;
			config.info.fb.flags = DISP_BF_NORMAL;
			config.info.fb.scan = DISP_SCAN_PROGRESSIVE;
			config.info.fb.size[0].width = fb_para->width;
			config.info.fb.size[0].height = fb_para->height;
			config.info.fb.size[1].width = fb_para->width;
			config.info.fb.size[1].height = fb_para->height;
			config.info.fb.size[2].width = fb_para->width;
			config.info.fb.size[2].height = fb_para->height;
			config.info.fb.color_space = DISP_BT601;

			if (mgr && mgr->set_layer_config)
				mgr->set_layer_config(mgr, &config, 1);
		}
	}

	g_fbi.fb_enable[fb_id] = 1;
	g_fbi.fb_mode[fb_id] = fb_para->fb_mode;
	memcpy(&g_fbi.fb_para[fb_id], fb_para, sizeof(struct disp_fb_create_info));

	return 0;
}

static s32 display_fb_release(u32 fb_id)
{
	u32 num_screens;

	num_screens = bsp_disp_feat_get_num_screens();

	__inf("%s, fb_id:%d\n", __func__, fb_id);

	if (g_fbi.fb_enable[fb_id]) {
		u32 sel = 0;
		struct fb_info * info = g_fbi.fbinfo[fb_id];

		for (sel = 0; sel < num_screens; sel++) {
			if (sel == g_fbi.fb_mode[fb_id])	{
				struct disp_manager *mgr = NULL;
				struct disp_layer_config config;

				mgr = g_disp_drv.mgr[sel];
				memset(&config, 0, sizeof(struct disp_layer_config));
				config.channel = g_fbi.layer_hdl[fb_id][0];
				config.layer_id = g_fbi.layer_hdl[fb_id][1];
				if (mgr && mgr->set_layer_config)
					mgr->set_layer_config(mgr, &config, 1);
			}
		}
		g_fbi.layer_hdl[fb_id][0] = 0;
		g_fbi.layer_hdl[fb_id][1] = 0;
		g_fbi.fb_mode[fb_id] = FB_MODE_SCREEN0;
		memset(&g_fbi.fb_para[fb_id], 0, sizeof(struct disp_fb_create_info));
		g_fbi.fb_enable[fb_id] = 0;
#if defined(CONFIG_FB_CONSOLE_SUNXI)
		fb_dealloc_cmap(&info->cmap);
#endif
		Fb_unmap_video_memory(info);

		return 0;
	}	else {
		__wrn("invalid paras fb_id:%d in %s\n", fb_id, __func__);
		return -1;
	}
}

s32 Display_set_fb_timming(u32 sel)
{
	u8 fb_id=0;

	for (fb_id=0; fb_id<FB_MAX; fb_id++) {
		if (g_fbi.fb_enable[fb_id]) {
			if (sel==g_fbi.fb_mode[fb_id])	{
				struct disp_video_timings tt;
				struct disp_manager *mgr = g_disp_drv.mgr[sel];

				if (mgr && mgr->device && mgr->device->get_timings) {
					mgr->device->get_timings(mgr->device, &tt);
					if (0 != tt.pixel_clk)
						g_fbi.fbinfo[fb_id]->var.pixclock = 1000000000 / tt.pixel_clk;
					g_fbi.fbinfo[fb_id]->var.left_margin = tt.hor_back_porch;
					g_fbi.fbinfo[fb_id]->var.right_margin = tt.hor_front_porch;
					g_fbi.fbinfo[fb_id]->var.upper_margin = tt.ver_back_porch;
					g_fbi.fbinfo[fb_id]->var.lower_margin = tt.ver_front_porch;
					g_fbi.fbinfo[fb_id]->var.hsync_len = tt.hor_sync_time;
					g_fbi.fbinfo[fb_id]->var.vsync_len = tt.ver_sync_time;
				}
			}
		}
	}

	return 0;
}

static s32 fb_parse_bootlogo_base(phys_addr_t *fb_base, int * fb_size)
{
	*fb_base = (phys_addr_t)disp_boot_para_parse("fb_base");

	return 0;
}

s32 fb_init(struct platform_device *pdev)
{
	struct disp_fb_create_info fb_para;
	unsigned long i;
	u32 num_screens;
	//struct sched_param param = { .sched_priority = MAX_RT_PRIO - 1 };

	g_fbi.dev = &pdev->dev;
	num_screens = bsp_disp_feat_get_num_screens();

	__inf("[DISP] %s\n", __func__);

	fb_parse_bootlogo_base(&bootlogo_addr, &bootlogo_sz);

	for (i=0; i<num_screens; i++) {
		char task_name[25];

		sprintf(task_name, "vsync proc %ld", i);
		g_fbi.vsync_task[i] = kthread_create(vsync_thread, (void*)i, task_name);
		if (IS_ERR(g_fbi.vsync_task[i])) {
			s32 err = 0;
			__wrn("Unable to start kernel thread %s.\n","hdmi proc");
			err = PTR_ERR(g_fbi.vsync_task[i]);
			g_fbi.vsync_task[i] = NULL;
		} else {
			//sched_setscheduler(g_fbi.vsync_task[i], SCHED_FIFO, &param);
			wake_up_process(g_fbi.vsync_task[i]);
		}
	}
	init_waitqueue_head(&g_fbi.wait[0]);
	init_waitqueue_head(&g_fbi.wait[1]);
	init_waitqueue_head(&g_fbi.wait[2]);
	disp_register_sync_finish_proc(DRV_disp_int_process);

	for (i=0; i<8; i++) {
		g_fbi.fbinfo[i] = framebuffer_alloc(0, g_fbi.dev);
		g_fbi.fbinfo[i]->fbops   = &dispfb_ops;
		g_fbi.fbinfo[i]->flags   = 0;
		g_fbi.fbinfo[i]->device  = g_fbi.dev;
		g_fbi.fbinfo[i]->par     = &g_fbi;
		g_fbi.fbinfo[i]->var.xoffset         = 0;
		g_fbi.fbinfo[i]->var.yoffset         = 0;
		g_fbi.fbinfo[i]->var.xres            = 800;
		g_fbi.fbinfo[i]->var.yres            = 480;
		g_fbi.fbinfo[i]->var.xres_virtual    = 800;
		g_fbi.fbinfo[i]->var.yres_virtual    = 480*2;
		g_fbi.fbinfo[i]->var.nonstd = 0;
		g_fbi.fbinfo[i]->var.bits_per_pixel = 32;
		g_fbi.fbinfo[i]->var.transp.length = 8;
		g_fbi.fbinfo[i]->var.red.length = 8;
		g_fbi.fbinfo[i]->var.green.length = 8;
		g_fbi.fbinfo[i]->var.blue.length = 8;
		g_fbi.fbinfo[i]->var.transp.offset = 24;
		g_fbi.fbinfo[i]->var.red.offset = 16;
		g_fbi.fbinfo[i]->var.green.offset = 8;
		g_fbi.fbinfo[i]->var.blue.offset = 0;
		g_fbi.fbinfo[i]->var.activate = FB_ACTIVATE_FORCE;
		g_fbi.fbinfo[i]->fix.type	    = FB_TYPE_PACKED_PIXELS;
		g_fbi.fbinfo[i]->fix.type_aux	= 0;
		g_fbi.fbinfo[i]->fix.visual 	= FB_VISUAL_TRUECOLOR;
		g_fbi.fbinfo[i]->fix.xpanstep	= 1;
		g_fbi.fbinfo[i]->fix.ypanstep	= 1;
		g_fbi.fbinfo[i]->fix.ywrapstep	= 0;
		g_fbi.fbinfo[i]->fix.accel	    = FB_ACCEL_NONE;
		g_fbi.fbinfo[i]->fix.line_length = g_fbi.fbinfo[i]->var.xres_virtual * 4;
		g_fbi.fbinfo[i]->fix.smem_len = g_fbi.fbinfo[i]->fix.line_length * g_fbi.fbinfo[i]->var.yres_virtual * 2;
		g_fbi.fbinfo[i]->screen_base = NULL;
		g_fbi.fbinfo[i]->pseudo_palette = g_fbi.pseudo_palette[i];
		g_fbi.fbinfo[i]->fix.smem_start = 0x0;
		g_fbi.fbinfo[i]->fix.mmio_start = 0;
		g_fbi.fbinfo[i]->fix.mmio_len = 0;

		if (fb_alloc_cmap(&g_fbi.fbinfo[i]->cmap, 256, 1) < 0) {
			return -ENOMEM;
		}
	}

	if (g_disp_drv.disp_init.b_init) {
		u32 fb_num = 0;

		fb_num = 1;
		for (i = 0; i<fb_num; i++)	{
			u32 screen_id = g_disp_drv.disp_init.disp_mode;

			if (g_disp_drv.para.boot_info.sync) {
				screen_id = g_disp_drv.para.boot_info.disp;
			}

			disp_fb_to_var(g_disp_drv.disp_init.format[i], &(g_fbi.fbinfo[i]->var));
			fb_para.buffer_num= g_disp_drv.disp_init.buffer_num[i];
			if ((g_disp_drv.disp_init.fb_width[i] == 0) || (g_disp_drv.disp_init.fb_height[i] == 0))	{
				fb_para.width = bsp_disp_get_screen_width_from_output_type(screen_id,
				    g_disp_drv.disp_init.output_type[screen_id], g_disp_drv.disp_init.output_mode[screen_id]);
				fb_para.height = bsp_disp_get_screen_height_from_output_type(screen_id,
				    g_disp_drv.disp_init.output_type[screen_id], g_disp_drv.disp_init.output_mode[screen_id]);
			}	else {
				fb_para.width = g_disp_drv.disp_init.fb_width[i];
				fb_para.height = g_disp_drv.disp_init.fb_height[i];
			}
			fb_para.output_width = bsp_disp_get_screen_width_from_output_type(screen_id,
				    g_disp_drv.disp_init.output_type[screen_id], g_disp_drv.disp_init.output_mode[screen_id]);
			fb_para.output_height = bsp_disp_get_screen_height_from_output_type(screen_id,
				    g_disp_drv.disp_init.output_type[screen_id], g_disp_drv.disp_init.output_mode[screen_id]);
			fb_para.fb_mode = screen_id;

			display_fb_request(i, &fb_para);
#if defined(CONFIG_DISP2_SUNXI_BOOT_COLORBAR)
			fb_draw_colorbar((char* __force)g_fbi.fbinfo[i]->screen_base, fb_para.width, fb_para.height*fb_para.buffer_num, &(g_fbi.fbinfo[i]->var));
#endif
		}
		for (i = 0; i < 8; i++){
			register_framebuffer(g_fbi.fbinfo[i]);
		}
	}

	return 0;
}

s32 fb_exit(void)
{
	u8 fb_id=0;

	for (fb_id=0; fb_id<FB_MAX; fb_id++) {
		if (g_fbi.fbinfo[fb_id] != NULL) {
			display_fb_release(fb_id);
		}
	}

	for (fb_id=0; fb_id<8; fb_id++) {
		unregister_framebuffer(g_fbi.fbinfo[fb_id]);
		framebuffer_release(g_fbi.fbinfo[fb_id]);
		g_fbi.fbinfo[fb_id] = NULL;
	}

	return 0;
}


