/* linux/drivers/video/sunxi/lcd/video_source_interface.c
 *
 * Copyright (c) 2013 Allwinnertech Co., Ltd.
 * Author: Tyle <tyle@allwinnertech.com>
 *
 * Video source interface for LCD driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include "panels.h"

extern struct sunxi_lcd_drv g_lcd_drv;

/**
 * sunxi_lcd_delay_ms.
 * @ms: Delay time, unit: millisecond.
 */
s32 sunxi_lcd_delay_ms(u32 ms);

/**
 * sunxi_lcd_delay_us.
 * @us: Delay time, unit: microsecond.
 */
s32 sunxi_lcd_delay_us(u32 us);

/**
 * sunxi_lcd_tcon_enable - enable timing controller.
 * @screen_id: The index of screen.
 */
void sunxi_lcd_tcon_enable(u32 screen_id);


/**
 * sunxi_lcd_tcon_disable - disable timing controller.
 * @screen_id: The index of screen.
 */
void sunxi_lcd_tcon_disable(u32 screen_id);

/**
 * sunxi_lcd_backlight_enable - enable the backlight of panel.
 * @screen_id: The index of screen.
 */
void sunxi_lcd_backlight_enable(u32 screen_id);

/**
 * sunxi_lcd_backlight_disable - disable the backlight of panel.
 * @screen_id: The index of screen.
 */
void sunxi_lcd_backlight_disable(u32 screen_id);

/**
 * sunxi_lcd_power_enable - enable the power of panel.
 * @screen_id: The index of screen.
 * @pwr_id:     The index of power
 */
void sunxi_lcd_power_enable(u32 screen_id, u32 pwr_id);

/**
 * sunxi_lcd_power_disable - disable the power of panel.
 * @screen_id: The index of screen.
 * @pwr_id:     The index of power
 */
void sunxi_lcd_power_disable(u32 screen_id, u32 pwr_id);

/**
 * sunxi_pwm_enable - enable pwm modules, start ouput pwm wave.
 * @pwm_channel: The index of pwm channel.
 *
 * need to conifg gpio for pwm function
 */
s32 sunxi_lcd_pwm_enable(u32 pwm_channel);

/**
 * sunxi_pwm_disable - disable pwm modules, stop ouput pwm wave.
 * @pwm_channel: The index of pwm channel.
 */
s32 sunxi_lcd_pwm_disable(u32 pwm_channel);

/**
 * sunxi_lcd_cpu_write - write command and para to cpu panel.
 * @scree_id: The index of screen.
 * @command: Command to be transfer.
 * @para: The pointer to para
 * @para_num: The number of para
 */
s32 sunxi_lcd_cpu_write(u32 scree_id, u32 command, u32 *para, u32 para_num);

/**
 * sunxi_lcd_cpu_write_index - write command to cpu panel.
 * @scree_id: The index of screen.
 * @index: Command or index to be transfer.
 */
s32 sunxi_lcd_cpu_write_index(u32 scree_id, u32 index);

/**
 * sunxi_lcd_cpu_write_data - write data to cpu panel.
 * @scree_id: The index of screen.
 * @data: Data to be transfer.
 */
s32 sunxi_lcd_cpu_write_data(u32 scree_id, u32 data);

/**
 * sunxi_lcd_dsi_dcs_write - write command and para to mipi panel(DCS type).
 * @scree_id: The index of screen.
 * @command: Command to be transfer.
 * @para: The pointer to para.
 * @para_num: The number of para
 */
s32 sunxi_lcd_dsi_dcs_write(u32 scree_id, u8 command, u8 *para, u32 para_num);

/**
 * sunxi_lcd_dsi_dcs_write - write command and para to mipi panel.
 * @scree_id: The index of screen.
 * @command: Command to be transfer.
 * @paran: Para to be transfer.
 */
s32 sunxi_lcd_dsi_dcs_write_0para(u32 scree_id, u8 command);
s32 sunxi_lcd_dsi_dcs_write_1para(u32 scree_id, u8 command, u8 para1);
s32 sunxi_lcd_dsi_dcs_write_2para(u32 scree_id, u8 command, u8 para1, u8 para2);
s32 sunxi_lcd_dsi_dcs_write_3para(u32 scree_id, u8 command, u8 para1, u8 para2, u8 para3);
s32 sunxi_lcd_dsi_dcs_write_4para(u32 scree_id, u8 command, u8 para1, u8 para2, u8 para3, u8 para4);
s32 sunxi_lcd_dsi_dcs_write_5para(u32 scree_id, u8 command, u8 para1, u8 para2, u8 para3, u8 para4, u8 para5);

/**
 * sunxi_lcd_dsi_gen_write - write command and para to mipi panel(GEN type).
 * @scree_id: The index of screen.
 * @command: Command to be transfer.
 * @para: The pointer to para.
 * @para_num: The number of para
 */
s32 sunxi_lcd_dsi_gen_write(u32 scree_id, u8 command, u8 *para, u32 para_num);

/**
 * sunxi_lcd_dsi_gen_write - write command and para to mipi panel.
 * @scree_id: The index of screen.
 * @command: Command to be transfer.
 * @paran: Para to be transfer.
 */
s32 sunxi_lcd_dsi_gen_write_0para(u32 scree_id, u8 command);
s32 sunxi_lcd_dsi_gen_write_1para(u32 scree_id, u8 command, u8 para1);
s32 sunxi_lcd_dsi_gen_write_2para(u32 scree_id, u8 command, u8 para1, u8 para2);
s32 sunxi_lcd_dsi_gen_write_3para(u32 scree_id, u8 command, u8 para1, u8 para2, u8 para3);
s32 sunxi_lcd_dsi_gen_write_4para(u32 scree_id, u8 command, u8 para1, u8 para2, u8 para3, u8 para4);
s32 sunxi_lcd_dsi_gen_write_5para(u32 scree_id, u8 command, u8 para1, u8 para2, u8 para3, u8 para4, u8 para5);
/**
 * sunxi_lcd_dsi_clk_enable - enable dsi clk.
 * @scree_id: The index of screen.
 */
s32 sunxi_lcd_dsi_clk_enable(u32 scree_id);

/**
 * sunxi_lcd_dsi_clk_disable - disable dsi clk.
 * @scree_id: The index of screen.
 */
s32 sunxi_lcd_dsi_clk_disable(u32 scree_id);

/**
 * sunxi_lcd_pin_cfg - config pin panel used
 * @screen_id: The index of screen.
 * @bon:     1: config pin according to sys_config, 0: set disable state
 */
s32 sunxi_lcd_pin_cfg(u32 screen_id, u32 bon);

/**
 * sunxi_lcd_set_panel_funs -  set panel functions.
 * @name: The panel driver name.
 * @lcd_cfg: The functions.
 */
s32 sunxi_lcd_set_panel_funs(char *name, disp_lcd_panel_fun * lcd_cfg);

/**
 * sunxi_lcd_gpio_set_value
 * @screen_id: The index of screen.
 * @io_index:  the index of gpio
 * @value: value of gpio to be set
 */
s32 sunxi_lcd_gpio_set_value(u32 screen_id, u32 io_index, u32 value);

/**
 * sunxi_lcd_gpio_set_direction
 * @screen_id: The index of screen.
 * @io_index:  the index of gpio
 * @direction: value of gpio to be set
 */
s32 sunxi_lcd_gpio_set_direction(u32 screen_id, u32 io_index, u32 direction);
