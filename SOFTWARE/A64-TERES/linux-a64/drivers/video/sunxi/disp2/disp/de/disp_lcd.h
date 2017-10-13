
#ifndef __DISP_LCD_H__
#define __DISP_LCD_H__

#include "disp_private.h"

#define LCD_GPIO_NUM 6
#define LCD_POWER_NUM 4
#define LCD_POWER_STR_LEN 32
#define LCD_GPIO_REGU_NUM 3
#define LCD_GPIO_SCL (LCD_GPIO_NUM-2)
#define LCD_GPIO_SDA (LCD_GPIO_NUM-1)

typedef struct
{
	bool                  lcd_used;

	bool                  lcd_bl_en_used;
	disp_gpio_set_t       lcd_bl_en;
	char                  lcd_bl_en_power[LCD_POWER_STR_LEN];

	u32                   lcd_power_used[LCD_POWER_NUM];
	char                  lcd_power[LCD_POWER_NUM][LCD_POWER_STR_LEN];

	u32                   lcd_fix_power_used[LCD_POWER_NUM];/* 0: invalid, 1: gpio, 2: regulator */
	char                  lcd_fix_power[LCD_POWER_NUM][LCD_POWER_STR_LEN];

	bool                  lcd_gpio_used[LCD_GPIO_NUM];  //index4: scl;  index5: sda
	disp_gpio_set_t       lcd_gpio[LCD_GPIO_NUM];       //index4: scl; index5: sda
	u32                   gpio_hdl[LCD_GPIO_NUM];
	char                  lcd_gpio_power[LCD_GPIO_REGU_NUM][LCD_POWER_STR_LEN];

	char                  lcd_pin_power[LCD_GPIO_REGU_NUM][LCD_POWER_STR_LEN];

	u32                   backlight_bright;
	u32                   backlight_dimming;//IEP-drc backlight dimming rate: 0 -256 (256: no dimming; 0: the most dimming)
	u32                   backlight_curve_adjust[101];

	u32                   lcd_bright;
	u32                   lcd_contrast;
	u32                   lcd_saturation;
	u32                   lcd_hue;
}disp_lcd_cfg;

s32 disp_init_lcd(disp_bsp_init_para * para);
s32 disp_lcd_gpio_init(struct disp_device* lcd);
s32 disp_lcd_gpio_exit(struct disp_device* lcd);
s32 disp_lcd_gpio_set_direction(struct disp_device* lcd, u32 io_index, u32 direction);
s32 disp_lcd_gpio_get_value(struct disp_device* lcd,u32 io_index);
s32 disp_lcd_gpio_set_value(struct disp_device* lcd, u32 io_index, u32 data);
s32 disp_lcd_is_enabled(struct disp_device* lcd);

#endif
