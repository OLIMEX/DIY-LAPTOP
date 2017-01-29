#include "HE080IA_06B.h"
#include "panels.h"
#include <linux/sys_config.h>

static void LCD_power_on(u32 sel);
static void LCD_power_off(u32 sel);
static void LCD_bl_open(u32 sel);
static void LCD_bl_close(u32 sel);

static void LCD_panel_init(u32 sel);
static void LCD_panel_exit(u32 sel);

static void LCD_cfg_panel_info(panel_extend_para * info)
{
	u32 i = 0, j=0;
	u32 items;
	u8 lcd_gamma_tbl[][2] =
	{
		//{input value, corrected value}
		{0, 0},
		{15, 15},
		{30, 30},
		{45, 45},
		{60, 60},
		{75, 75},
		{90, 90},
		{105, 105},
		{120, 120},
		{135, 135},
		{150, 150},
		{165, 165},
		{180, 180},
		{195, 195},
		{210, 210},
		{225, 225},
		{240, 240},
		{255, 255},
	};

//	u8 lcd_bright_curve_tbl[][2] =
//	{
//		//{input value, corrected value}
//		{0    ,0  },//0
//		{15   ,3  },//0
//		{30   ,6  },//0
//		{45   ,9  },// 1
//		{60   ,12  },// 2
//		{75   ,16  },// 5
//		{90   ,22  },//9
//		{105   ,28 }, //15
//		{120  ,36 },//23
//		{135  ,44 },//33
//		{150  ,54 },
//		{165  ,67 },
//		{180  ,84 },
//		{195  ,108},
//		{210  ,137},
//		{225 ,171},
//		{240 ,210},
//		{255 ,255},
//	};

	u32 lcd_cmap_tbl[2][3][4] = {
	{
		{LCD_CMAP_G0,LCD_CMAP_B1,LCD_CMAP_G2,LCD_CMAP_B3},
		{LCD_CMAP_B0,LCD_CMAP_R1,LCD_CMAP_B2,LCD_CMAP_R3},
		{LCD_CMAP_R0,LCD_CMAP_G1,LCD_CMAP_R2,LCD_CMAP_G3},
		},
		{
		{LCD_CMAP_B3,LCD_CMAP_G2,LCD_CMAP_B1,LCD_CMAP_G0},
		{LCD_CMAP_R3,LCD_CMAP_B2,LCD_CMAP_R1,LCD_CMAP_B0},
		{LCD_CMAP_G3,LCD_CMAP_R2,LCD_CMAP_G1,LCD_CMAP_R0},
		},
	};

	//memset(info,0,sizeof(panel_extend_para));

	items = sizeof(lcd_gamma_tbl)/2;
	for(i=0; i<items-1; i++) {
		u32 num = lcd_gamma_tbl[i+1][0] - lcd_gamma_tbl[i][0];

		for(j=0; j<num; j++) {
			u32 value = 0;

			value = lcd_gamma_tbl[i][1] + ((lcd_gamma_tbl[i+1][1] - lcd_gamma_tbl[i][1]) * j)/num;
			info->lcd_gamma_tbl[lcd_gamma_tbl[i][0] + j] = (value<<16) + (value<<8) + value;
		}
	}
	info->lcd_gamma_tbl[255] = (lcd_gamma_tbl[items-1][1]<<16) + (lcd_gamma_tbl[items-1][1]<<8) + lcd_gamma_tbl[items-1][1];

//	items = sizeof(lcd_bright_curve_tbl)/2;
//	for(i=0; i<items-1; i++) {
//		u32 num = lcd_bright_curve_tbl[i+1][0] - lcd_bright_curve_tbl[i][0];
//
//		for(j=0; j<num; j++) {
//			u32 value = 0;
//
//			value = lcd_bright_curve_tbl[i][1] + ((lcd_bright_curve_tbl[i+1][1] - lcd_bright_curve_tbl[i][1]) * j)/num;
//			info->lcd_bright_curve_tbl[lcd_bright_curve_tbl[i][0] + j] = value;
//		}
//	}
//	info->lcd_bright_curve_tbl[255] = lcd_bright_curve_tbl[items-1][1];

	memcpy(info->lcd_cmap_tbl, lcd_cmap_tbl, sizeof(lcd_cmap_tbl));

}

static s32 LCD_open_flow(u32 sel)
{   
        printk(KERN_ERR"HE080IA_06B_panel++++LCD_open_flow+++++\n");
	LCD_OPEN_FUNC(sel, LCD_power_on, 30);   //open lcd power, and delay 50ms
	LCD_OPEN_FUNC(sel, LCD_panel_init, 10);   //open lcd power, than delay 200ms
	LCD_OPEN_FUNC(sel, sunxi_lcd_tcon_enable, 200);     //open lcd controller, and delay 100ms
	LCD_OPEN_FUNC(sel, LCD_bl_open, 0);     //open lcd backlight, and delay 0ms

	return 0;
}

static s32 LCD_close_flow(u32 sel)
{
        printk(KERN_ERR"++++LCD_close_flow+++++\n");
	LCD_CLOSE_FUNC(sel, LCD_bl_close, 0);       //close lcd backlight, and delay 0ms
	LCD_CLOSE_FUNC(sel, sunxi_lcd_tcon_disable, 0);         //close lcd controller, and delay 0ms
	LCD_CLOSE_FUNC(sel, LCD_panel_exit,	20);   //open lcd power, than delay 200ms
	LCD_CLOSE_FUNC(sel, LCD_power_off, 50);   //close lcd power, and delay 500ms

	return 0;
}

static void LCD_power_on(u32 sel)
{
	sunxi_lcd_pin_cfg(sel, 1);
	sunxi_lcd_power_enable(sel, 0);//config lcd_power pin to open lcd power0
	sunxi_lcd_power_enable(sel, 1);//config lcd_power pin to open lcd power1
	sunxi_lcd_power_enable(sel, 2);//config lcd_power pin to open lcd power2
}

static void LCD_power_off(u32 sel)
{
	sunxi_lcd_pin_cfg(sel, 0);
	sunxi_lcd_power_disable(sel, 0);//config lcd_power pin to close lcd power0
	sunxi_lcd_power_disable(sel, 1);//config lcd_power pin to close lcd power1
	sunxi_lcd_power_disable(sel, 2);//config lcd_power pin to close lcd power2
}

static void LCD_bl_open(u32 sel)
{
	sunxi_lcd_pwm_enable(sel);//open pwm module
	sunxi_lcd_backlight_enable(sel);//config lcd_bl_en pin to open lcd backlight
}

static void LCD_bl_close(u32 sel)
{
	sunxi_lcd_backlight_disable(sel);//config lcd_bl_en pin to close lcd backlight
	sunxi_lcd_pwm_disable(sel);//close pwm module
}

#define REGFLAG_DELAY             							0XFE 
#define REGFLAG_END_OF_TABLE      						0xFF   // END OF REGISTERS MARKER

struct LCM_setting_table {
    u8 cmd;
    u32 count;
    u8 para_list[64];
};


static struct LCM_setting_table LCM_HSX080IA_setting[] = {
	{0xFF,    4,     {0xAA,0x55,0xA5,0x80}},			
	{0x6F,    2,     {0x11,0x00}},			
	{0xF7,    2,     {0x20,0x00}},			
	{0x6F,    1,     {0x06}},			
	{0xF7,    1,     {0xA0}},			
	{0x6F,    1,     {0x19}},			
	{0xF7,    1,     {0x12}},	
	{0xF4,    1,     {0x03}},		
	{0x6F,    1,     {0x08}},			
	{0xFA,    1,     {0x40}},			
	{0x6F,    1,     {0x11}},			
	{0xF3,    1,     {0x01}},			
	{0xF0,    5,     {0x55,0xAA,0x52,0x08,0x00}},			
	{0xC8,    1,     {0x80}},			
	{0xB1,    2,     {0x6C,0x01}},//07			
	{0xB6,    1,     {0x08}},			
	{0x6F,    1,     {0x02}},			
	{0xB8,    1,     {0x08}},			
	{0xBB,    2,     {0x74,0x44}},			
	{0xBC,    2,     {0x00,0x00}},			
	{0xBD,    5,     {0x02,0xB0,0x0C,0x0A,0x00}},			
	{0xF0,    5,     {0x55,0xAA,0x52,0x08,0x01}},			
	{0xB0,    2,     {0x05,0x05}},			
	{0xB1,    2,     {0x05,0x05}},			
	{0xBC,    2,     {0x90,0x01}},			
	{0xBD,    2,     {0x90,0x01}},			
	{0xCA,    1,     {0x00}},			
	{0xC0,    1,     {0x04}},			
	//{0xB2,    2,     {0x00,0x00}},			
	{0xBE,    1,     {0x29}},			
	{0xB3,    2,     {0x37,0x37}},			
	{0xB4,    2,     {0x19,0x19}},			
	{0xB9,    2,     {0x44,0x44}},			
	{0xBA,    2,     {0x24,0x24}},			
	{0xF0,    5,     {0x55,0xAA,0x52,0x08,0x02}},			
	{0xEE,    1,     {0x01}},			
	{0xEF,    4,     {0x09,0x06,0x15,0x18}},			
	{0xB0,    6,     {0x00,0x00,0x00,0x25,0x00,0x43}},			
	{0x6F,    1,     {0x06}},			
	{0xB0,    6,     {0x00,0x54,0x00,0x68,0x00,0xA0}},			
	{0x6F,    1,     {0x0C}},		
	{0xB0,    4,     {0x00,0xC0,0x01,0x00}},                                                                                                                                                   			
	{0xB1,    6,     {0x01,0x30,0x01,0x78,0x01,0xAE}},			
	{0x6F,    1,     {0x06}},			
	{0xB1,    6,     {0x02,0x08,0x02,0x52,0x02,0x54}},			
	{0x6F,    1,     {0x0C}},			
	{0xB1,    4,     {0x02,0x99,0x02,0xF0}},			
	{0xB2,    6,     {0x03,0x20,0x03,0x56,0x03,0x76}},			
	{0x6F,    1,     {0x06}},			
	{0xB2,    6,     {0x03,0x93,0x03,0xA4,0x03,0xB9}},			
	{0x6F,    1,     {0x0C}},			
	{0xB2,    4,     {0x03,0xC9,0x03,0xE3}},			
	{0xB3,    4,     {0x03,0xFC,0x03,0xFF}},   			
	{0xF0,    5,     {0x55,0xAA,0x52,0x08,0x06}},			
	{0xB0,    2,     {0x00,0x10}},			
	{0xB1,    2,     {0x12,0x14}},			
	{0xB2,    2,     {0x16,0x18}},			
	{0xB3,    2,     {0x1A,0x29}},			
	{0xB4,    2,     {0x2A,0x08}},			
	{0xB5,    2,     {0x31,0x31}},			
	{0xB6,    2,     {0x31,0x31}},			
	{0xB7,    2,     {0x31,0x31}},			
	{0xB8,    2,     {0x31,0x0A}},			
	{0xB9,    2,     {0x31,0x31}},			
	{0xBA,    2,     {0x31,0x31}},			
	{0xBB,    2,     {0x0B,0x31}},			
	{0xBC,    2,     {0x31,0x31}},			
	{0xBD,    2,     {0x31,0x31}},			
	{0xBE,    2,     {0x31,0x31}},			
	{0xBF,    2,     {0x09,0x2A}},			
	{0xC0,    2,     {0x29,0x1B}},			
	{0xC1,    2,     {0x19,0x17}},			
	{0xC2,    2,     {0x15,0x13}},			
	{0xC3,    2,     {0x11,0x01}},			
	{0xE5,    2,     {0x31,0x31}},			
	{0xC4,    2,     {0x09,0x1B}},			
	{0xC5,    2,     {0x19,0x17}},			
	{0xC6,    2,     {0x15,0x13}},			
	{0xC7,    2,     {0x11,0x29}},			
	{0xC8,    2,     {0x2A,0x01}},			
	{0xC9,    2,     {0x31,0x31}},			
	{0xCA,    2,     {0x31,0x31}},			
	{0xCB,    2,     {0x31,0x31}},			
	{0xCC,    2,     {0x31,0x0B}},			
	{0xCD,    2,     {0x31,0x31}},			
	{0xCE,    2,     {0x31,0x31}},			
	{0xCF,    2,     {0x0A,0x31}},			
	{0xD0,    2,     {0x31,0x31}},			
	{0xD1,    2,     {0x31,0x31}},			
	{0xD2,    2,     {0x31,0x31}},			
	{0xD3,    2,     {0x00,0x2A}},			
	{0xD4,    2,     {0x29,0x10}},			
	{0xD5,    2,     {0x12,0x14}},			
	{0xD6,    2,     {0x16,0x18}},			
	{0xD7,    2,     {0x1A,0x08}},			
	{0xE6,    2,     {0x31,0x31}},			
	{0xD8,    5,     {0x00,0x00,0x00,0x54,0x00}},			
	{0xD9,    5,     {0x00,0x15,0x00,0x00,0x00}},			
	{0xE7,    1,     {0x00}},			
	{0xF0,    5,     {0x55,0xAA,0x52,0x08,0x03}},			
	{0xB0,    2,     {0x20,0x00}},			
	{0xB1,    2,     {0x20,0x00}},			
	{0xB2,    5,     {0x05,0x00,0x00,0x00,0x00}},			
	{0xB6,    5,     {0x05,0x00,0x00,0x00,0x00}},			
	{0xB7,    5,     {0x05,0x00,0x00,0x00,0x00}},			
	{0xBA,    5,     {0x57,0x00,0x00,0x00,0x00}},			
	{0xBB,    5,     {0x57,0x00,0x00,0x00,0x00}},			
	{0xC0,    4,     {0x00,0x00,0x00,0x00}},			
	{0xC1,    4,     {0x00,0x00,0x00,0x00}},			
	{0xC4,    1,     {0x60}},			
	{0xC5,    1,     {0x40}},			
	{0xF0,    5,     {0x55,0xAA,0x52,0x08,0x05}},			
	{0xBD,    5,     {0x03,0x01,0x03,0x03,0x03}},			
	{0xB0,    2,     {0x17,0x06}},			
	{0xB1,    2,     {0x17,0x06}},			
	{0xB2,    2,     {0x17,0x06}},			
	{0xB3,    2,     {0x17,0x06}},			
	{0xB4,    2,     {0x17,0x06}},			
	{0xB5,    2,     {0x17,0x06}},			
	{0xB8,    1,     {0x00}},			
	{0xB9,    1,     {0x00}},			
	{0xBA,    1,     {0x00}},			
	{0xBB,    1,     {0x02}},			
	{0xBC,    1,     {0x00}},			
	{0xC0,    1,     {0x07}},			
	{0xC4,    1,     {0x80}},			
	{0xC5,    1,     {0xA4}},			
	{0xC8,    2,     {0x05,0x30}},			
	{0xC9,    2,     {0x01,0x31}},			
	{0xCC,    3,     {0x00,0x00,0x3C}},			
	{0xCD,    3,     {0x00,0x00,0x3C}},			
	{0xD1,    5,     {0x00,0x04,0xFD,0x07,0x10}},			
	{0xD2,    5,     {0x00,0x05,0x02,0x07,0x10}},			
	{0xE5,    1,     {0x06}},			
	{0xE6,    1,     {0x06}},			
	{0xE7,    1,     {0x06}},			
	{0xE8,    1,     {0x06}},			
	{0xE9,    1,     {0x06}},			
	{0xEA,    1,     {0x06}},			
	{0xED,    1,     {0x30}},			
	{0x6F,    1,     {0x11}},			
	{0xF3,    1,     {0x01}},		
#if 0//selftest mode
  {0xF0,    5,     {0x55,0xAA,0x52,0x08,0x00}},	
  {0xEE,    4,     {0x87,0x78,0x02,0x40}},
  {0xEF,    3,     {0x00,0xFF,0x00}},
#else	
	{0x11,			0,     {0x00}},
	{REGFLAG_DELAY, REGFLAG_DELAY, {200}},
	{0x29,			0,     {0x00}},
	{REGFLAG_DELAY, REGFLAG_DELAY, {200}},
#endif
	{REGFLAG_END_OF_TABLE, REGFLAG_END_OF_TABLE, {}}
};


static void LCD_panel_init(u32 sel)
{
	u32 i;
	script_item_u   val;
  script_item_value_type_e	type;
  type = script_get_item("lcd0","lcd_model_name", &val);
  if (SCIRPT_ITEM_VALUE_TYPE_STR != type) {  	
	  printk("fetch lcd_model_name from sys_config failed\n");
	} else {
		printk("lcd_model_name = %s\n",val.str);
	}
	sunxi_lcd_pin_cfg(sel, 1);
	sunxi_lcd_delay_ms(10);
	panel_rst(0);
	sunxi_lcd_delay_ms(100);
	panel_rst(1);
	sunxi_lcd_delay_ms(100);
	printk(KERN_ERR"++LCD_panel_init+++val.str:%s++\n",val.str);
	for(i=0;;i++)
	{
		if(!strcmp("HSX080IA",val.str)) {
		     printk(KERN_ERR"HE080IA_06B_panel++HSX080IA++LCD_panel_init++normal -+++\n");
			if(LCM_HSX080IA_setting[i].count == REGFLAG_END_OF_TABLE)
				break;
			else if (LCM_HSX080IA_setting[i].count == REGFLAG_DELAY)
				sunxi_lcd_delay_ms(LCM_HSX080IA_setting[i].para_list[0]);
			else
				dsi_dcs_wr(0,LCM_HSX080IA_setting[i].cmd,LCM_HSX080IA_setting[i].para_list,LCM_HSX080IA_setting[i].count);
		}


		//break;
	}

	/*
	sunxi_lcd_dsi_write(sel,DSI_DCS_EXIT_SLEEP_MODE, 0, 0);
	sunxi_lcd_delay_ms(200);
	
	sunxi_lcd_dsi_write(sel,DSI_DCS_SET_DISPLAY_ON, 0, 0);
	sunxi_lcd_delay_ms(200);
	*/
	
	sunxi_lcd_dsi_clk_enable(sel);
	return;
}

static void LCD_panel_exit(u32 sel)
{
	//need to do？ 现在没有做exit处理，如果关屏时有状况，考虑是否要走正常的enter sleep的流程。。
	sunxi_lcd_dsi_clk_disable(sel);
	panel_rst(0);
	return ;
}

//sel: 0:lcd0; 1:lcd1
static s32 LCD_user_defined_func(u32 sel, u32 para1, u32 para2, u32 para3)
{
	return 0;
}

__lcd_panel_t HE080IA_06B_panel = {
	/* panel driver name, must mach the name of lcd_drv_name in sys_config.fex */
	.name = "HE080IA_06B_panel",
	.func = {
		.cfg_panel_info = LCD_cfg_panel_info,
		.cfg_open_flow = LCD_open_flow,
		.cfg_close_flow = LCD_close_flow,
		.lcd_user_defined_func = LCD_user_defined_func,
	},
};
