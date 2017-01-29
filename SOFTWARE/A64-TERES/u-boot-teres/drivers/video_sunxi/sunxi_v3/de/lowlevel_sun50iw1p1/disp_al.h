#ifndef _DISP_AL_H_
#define _DISP_AL_H_

#include "../include.h"
#include "de_feat.h"
#include "de_hal.h"
#include "de_enhance.h"
#include "de_wb.h"
#include "de_smbl.h"
#include "de_csc.h"
#include "de_lcd.h"
#include "de_dsi.h"
#include "de_clock.h"

struct lcd_clk_info {
	disp_lcd_if lcd_if;
	int tcon_div;
	int lcd_div;
	int dsi_div;
};

int disp_al_manager_init(unsigned int disp);
int disp_al_manager_exit(unsigned int disp);
extern int disp_al_manager_apply(unsigned int disp, struct disp_manager_data *data);
extern int disp_al_layer_apply(unsigned int disp, struct disp_layer_config_data *data, unsigned int layer_num);
extern int disp_init_al(disp_bsp_init_para * para);
extern int disp_al_manager_sync(unsigned int disp);
extern int disp_al_manager_update_regs(unsigned int disp);
int disp_al_manager_query_irq(unsigned int disp);
int disp_al_manager_enable_irq(unsigned int disp);
int disp_al_manager_disable_irq(unsigned int disp);

int disp_al_enhance_apply(unsigned int disp, struct disp_enhance_config *config);
int disp_al_enhance_update_regs(unsigned int disp);
int disp_al_enhance_sync(unsigned int disp);

int disp_al_smbl_apply(unsigned int disp, struct disp_smbl_info *info);
int disp_al_smbl_update_regs(unsigned int disp);
int disp_al_smbl_sync(unsigned int disp);
int disp_al_smbl_get_status(unsigned int disp);

int disp_al_capture_init(unsigned int disp);
int disp_al_capture_exit(unsigned int disp);
int disp_al_capture_sync(u32 disp);
int disp_al_capture_apply(unsigned int disp, struct disp_capture_config *cfg);
int disp_al_capture_get_status(unsigned int disp);

int disp_al_lcd_cfg(u32 screen_id, disp_panel_para * panel, panel_extend_para *extend_panel);
int disp_al_lcd_enable(u32 screen_id, disp_panel_para * panel);
int disp_al_lcd_disable(u32 screen_id, disp_panel_para * panel);
int disp_al_lcd_query_irq(u32 screen_id, __lcd_irq_id_t irq_id, disp_panel_para * panel);
int disp_al_lcd_tri_busy(u32 screen_id, disp_panel_para * panel);
int disp_al_lcd_tri_start(u32 screen_id, disp_panel_para * panel);
int disp_al_lcd_io_cfg(u32 screen_id, u32 enable, disp_panel_para * panel);
int disp_al_lcd_get_cur_line(u32 screen_id, disp_panel_para * panel);
int disp_al_lcd_get_start_delay(u32 screen_id, disp_panel_para * panel);
int disp_al_lcd_get_clk_info(u32 screen_id, struct lcd_clk_info *info, disp_panel_para * panel);
int disp_al_lcd_enable_irq(u32 screen_id, __lcd_irq_id_t irq_id, disp_panel_para * panel);
int disp_al_lcd_disable_irq(u32 screen_id, __lcd_irq_id_t irq_id, disp_panel_para * panel);

int disp_al_hdmi_enable(u32 screen_id);
int disp_al_hdmi_disable(u32 screen_id);
int disp_al_hdmi_cfg(u32 screen_id, disp_video_timings *video_info);

int disp_al_vdevice_cfg(u32 screen_id, disp_video_timings *video_info, disp_vdevice_interface_para *para);
int disp_al_vdevice_enable(u32 screen_id);
int disp_al_vdevice_disable(u32 screen_id);

int disp_al_device_get_cur_line(u32 screen_id);
int disp_al_device_get_start_delay(u32 screen_id);
int disp_al_device_query_irq(u32 screen_id);
int disp_al_device_enable_irq(u32 screen_id);
int disp_al_device_disable_irq(u32 screen_id);

#endif
