
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
	struct tasklet_struct tasklet;
	disp_health_info      health_info;
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
	u32					  tv_registered;
}disp_dev_t;

extern disp_dev_t gdisp;
s32 disp_init_connections(disp_bsp_init_para * para);
s32 bsp_disp_shadow_protect(u32 disp, bool protect);
s32 bsp_disp_set_print_level(u32 print_level);
s32 bsp_disp_get_print_level(void);
void sync_event_proc(u32 disp, bool timeout);
s32 disp_device_attached(int disp_mgr, int disp_dev, enum disp_output_type output_type, enum disp_output_type mode);
s32 disp_device_attached_and_enable(int disp_mgr, int disp_dev, enum disp_output_type output_type, enum disp_output_type mode);
s32 disp_device_detach(int disp_mgr, int disp_dev, enum disp_output_type output_type);
void LCD_OPEN_FUNC(u32 screen_id, LCD_FUNC func, u32 delay);
void LCD_CLOSE_FUNC(u32 screen_id, LCD_FUNC func, u32 delay);
s32 bsp_disp_sync_with_hw(disp_bsp_init_para * para);
void disp_tasklet(unsigned long data);
s32 bsp_disp_get_fps(u32 disp);
s32 bsp_disp_get_health_info(u32 disp, disp_health_info *info);

s32 bsp_disp_init(disp_bsp_init_para * para);
s32 bsp_disp_exit(u32 mode);
s32 bsp_disp_open(void);
s32 bsp_disp_close(void);
s32 bsp_disp_feat_get_num_screens(void);
s32 bsp_disp_feat_get_num_channels(u32 disp);
s32 bsp_disp_feat_get_num_layers(u32 screen_id);
s32 bsp_disp_feat_get_num_layers_by_chn(u32 disp, u32 chn);
s32 bsp_disp_feat_is_supported_output_types(u32 screen_id, u32 output_type);
s32 bsp_disp_get_screen_physical_width(u32 disp);
s32 bsp_disp_get_screen_physical_height(u32 disp);
s32 bsp_disp_get_screen_width(u32 disp);
s32 bsp_disp_get_screen_height(u32 disp);
s32 bsp_disp_get_screen_width_from_output_type(u32 disp, u32 output_type, u32 output_mode);
s32 bsp_disp_get_screen_height_from_output_type(u32 disp, u32 output_type, u32 output_mode);
s32 bsp_disp_get_lcd_registered(u32 disp);
s32 bsp_disp_get_hdmi_registered(void);
s32 bsp_disp_get_tv_registered(void);

s32 bsp_disp_get_output_type(u32 disp);
s32 bsp_disp_device_switch(int disp, enum disp_output_type output_type, enum disp_output_type mode);
s32 bsp_disp_set_hdmi_func(struct disp_device_func * func);
s32 bsp_disp_hdmi_check_support_mode(u32 disp, enum disp_output_type mode);
s32 bsp_disp_hdmi_set_detect(bool hpd);
s32 bsp_disp_tv_register(struct disp_tv_func * func);
s32 bsp_disp_tv_set_hpd(u32 state);

//lcd
s32 bsp_disp_lcd_set_panel_funs(char *name, disp_lcd_panel_fun * lcd_cfg);
s32 bsp_disp_lcd_backlight_enable(u32 disp);
s32 bsp_disp_lcd_backlight_disable(u32 disp);
s32 bsp_disp_lcd_pwm_enable(u32 disp);
s32 bsp_disp_lcd_pwm_disable(u32 disp);
s32 bsp_disp_lcd_power_enable(u32 disp, u32 power_id);
s32 bsp_disp_lcd_power_disable(u32 disp, u32 power_id);
s32 bsp_disp_lcd_set_bright(u32 disp, u32 bright);
s32 bsp_disp_lcd_get_bright(u32 disp);
s32 bsp_disp_lcd_tcon_enable(u32 disp);
s32 bsp_disp_lcd_tcon_disable(u32 disp);
s32 bsp_disp_lcd_pin_cfg(u32 disp, u32 en);
s32 bsp_disp_lcd_gpio_set_value(u32 disp, u32 io_index, u32 value);
s32 bsp_disp_lcd_gpio_set_direction(u32 disp, unsigned int io_index, u32 direction);
disp_lcd_flow * bsp_disp_lcd_get_open_flow(u32 disp);
disp_lcd_flow * bsp_disp_lcd_get_close_flow(u32 disp);
s32 bsp_disp_get_panel_info(u32 disp, disp_panel_para *info);

s32 bsp_disp_vsync_event_enable(u32 disp, bool enable);
s32 bsp_disp_tv_suspend(void);
s32 bsp_disp_tv_resume(void);

int bsp_disp_get_fb_info(unsigned int disp, struct disp_layer_info *info);
int bsp_disp_get_display_size(u32 disp, unsigned int *width, unsigned int *height);

#ifdef CONFIG_DEVFREQ_DRAM_FREQ_IN_VSYNC
/* dramfreq interface */
s32 bsp_disp_get_vb_time(void);
s32 bsp_disp_get_next_vb_time(void);
s32 bsp_disp_is_in_vb(void);
#endif

#endif
