
#ifndef __DISP_DISPLAY_H__
#define __DISP_DISPLAY_H__

#include "disp_private.h"

typedef struct
{
	bool                  have_cfg_reg;
	u32                   cache_flag;
	u32                   cfg_cnt;
#ifdef __LINUX_PLAT__
	spinlock_t            flag_lock;
#endif
	bool                  vsync_event_en;
	bool                  dvi_enable;
}disp_screen_t;

typedef struct
{
	disp_bsp_init_para    init_para;//para from driver
	disp_screen_t         screen[3];
	u32                   print_level;
	u32                   lcd_registered[3];
	u32                   hdmi_registered;
	u32                   tv_registered;
}disp_dev_t;

extern disp_dev_t gdisp;
s32 disp_init_connections(void);
s32 bsp_disp_shadow_protect(u32 disp, bool protect);
s32 bsp_disp_set_print_level(u32 print_level);
void sync_event_proc(u32 disp);
s32 disp_device_attached(int disp_mgr, int disp_dev, disp_output_type output_type, disp_tv_mode mode);
s32 disp_device_attached_and_enable(int disp_mgr, int disp_dev, disp_output_type output_type, disp_tv_mode mode);
s32 disp_device_detach(int disp_mgr, int disp_dev, disp_output_type output_type);
void LCD_OPEN_FUNC(u32 screen_id, LCD_FUNC func, u32 delay);
void LCD_CLOSE_FUNC(u32 screen_id, LCD_FUNC func, u32 delay);
s32 bsp_disp_hdmi_get_hpd_status(u32 disp);
s32 bsp_disp_hdmi_check_support_mode(u32 disp, disp_tv_mode mode);
s32 bsp_disp_tv_get_hpd_status(u32 disp);


#endif
