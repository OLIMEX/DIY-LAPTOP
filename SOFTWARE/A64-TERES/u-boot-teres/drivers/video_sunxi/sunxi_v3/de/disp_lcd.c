#include "disp_lcd.h"

struct disp_lcd_private_data
{
	disp_lcd_flow             open_flow;
	disp_lcd_flow             close_flow;
	disp_panel_para           panel_info;
	panel_extend_para         panel_extend_info;
	__disp_lcd_cfg_t          lcd_cfg;
	disp_lcd_panel_fun        lcd_panel_fun;
	bool                      enabling;
	bool                      disabling;
	u32                       irq_no;
	u32                       reg_base;
	u32                       irq_no_dsi;
	u32                       reg_base_dsi;
	u32                       irq_no_edp;
	u32                       enabled;
	u32                       power_enabled;
	u32                       bl_need_enabled;
	struct {
		s32                     dev;
		u32                     channel;
		u32                     polarity;
		u32                     period_ns;
		u32                     duty_ns;
		u32                     enabled;
	}pwm_info;
	char *clk;
	char *lvds_clk;
	char *dsi_clk0;
	char *dsi_clk1;
	char *edp_clk;
	char *clk_parent;
};
#if defined(__LINUX_PLAT__)
static spinlock_t lcd_data_lock;
#else
static int lcd_data_lock;
#endif

static struct disp_device *lcds = NULL;
static struct disp_lcd_private_data *lcd_private;

s32 disp_lcd_set_bright(struct disp_device *lcd, u32 bright);
s32 disp_lcd_get_bright(struct disp_device *lcd);

struct disp_device* disp_get_lcd(u32 disp)
{
	u32 num_screens;

	num_screens = bsp_disp_feat_get_num_screens();
	if(disp >= num_screens || !bsp_disp_feat_is_supported_output_types(disp, DISP_OUTPUT_TYPE_LCD)) {
		DE_INF("disp %d not support lcd output\n", disp);
		return NULL;
	}

	return &lcds[disp];
}
static struct disp_lcd_private_data *disp_lcd_get_priv(struct disp_device *lcd)
{
	if(NULL == lcd) {
		DE_WRN("param is NULL!\n");
		return NULL;
	}

	if(!bsp_disp_feat_is_supported_output_types(lcd->disp, DISP_OUTPUT_TYPE_LCD)) {
		DE_INF("disp %d not support lcd output\n", lcd->disp);
		return NULL;
	}

	return &lcd_private[lcd->disp];
}

static s32 disp_lcd_is_used(struct disp_device* lcd)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);
	s32 ret = 0;

	if((NULL == lcd) || (NULL == lcdp)) {
		ret = 0;
	} else {
		if(bsp_disp_feat_is_supported_output_types(lcd->disp, DISP_OUTPUT_TYPE_LCD))
			ret = (s32)lcdp->lcd_cfg.lcd_used;
	}
	return ret;
}

static s32 lcd_parse_panel_para(u32 disp, disp_panel_para * info)
{
    s32 ret = 0;
    char primary_key[25];
    s32 value = 0;

    sprintf(primary_key, "lcd%d_para", disp);
    memset(info, 0, sizeof(disp_panel_para));

    ret = disp_sys_script_get_item(primary_key, "lcd_x", &value, 1);
    if(ret == 1)
    {
        info->lcd_x = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_y", &value, 1);
    if(ret == 1)
    {
        info->lcd_y = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_width", &value, 1);
    if(ret == 1)
    {
        info->lcd_width = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_height", &value, 1);
    if(ret == 1)
    {
        info->lcd_height = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_dclk_freq", &value, 1);
    if(ret == 1)
    {
        info->lcd_dclk_freq = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_pwm_used", &value, 1);
    if(ret == 1)
    {
        info->lcd_pwm_used = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_pwm_ch", &value, 1);
    if(ret == 1)
    {
        info->lcd_pwm_ch = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_pwm_freq", &value, 1);
    if(ret == 1)
    {
        info->lcd_pwm_freq = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_pwm_pol", &value, 1);
    if(ret == 1)
    {
        info->lcd_pwm_pol = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_if", &value, 1);
    if(ret == 1)
    {
        info->lcd_if = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_hbp", &value, 1);
    if(ret == 1)
    {
        info->lcd_hbp = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_ht", &value, 1);
    if(ret == 1)
    {
        info->lcd_ht = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_vbp", &value, 1);
    if(ret == 1)
    {
        info->lcd_vbp = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_vt", &value, 1);
    if(ret == 1)
    {
        info->lcd_vt = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_hv_if", &value, 1);
    if(ret == 1)
    {
        info->lcd_hv_if = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_vspw", &value, 1);
    if(ret == 1)
    {
        info->lcd_vspw = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_hspw", &value, 1);
    if(ret == 1)
    {
        info->lcd_hspw = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_lvds_if", &value, 1);
    if(ret == 1)
    {
        info->lcd_lvds_if = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_lvds_mode", &value, 1);
    if(ret == 1)
    {
        info->lcd_lvds_mode = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_lvds_colordepth", &value, 1);
    if(ret == 1)
    {
        info->lcd_lvds_colordepth= value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_lvds_io_polarity", &value, 1);
    if(ret == 1)
    {
        info->lcd_lvds_io_polarity = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_cpu_if", &value, 1);
    if(ret == 1)
    {
        info->lcd_cpu_if = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_cpu_te", &value, 1);
    if(ret == 1)
    {
        info->lcd_cpu_te = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_frm", &value, 1);
    if(ret == 1)
    {
        info->lcd_frm = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_dsi_if", &value, 1);
    if(ret == 1)
    {
        info->lcd_dsi_if = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_dsi_lane", &value, 1);
    if(ret == 1)
    {
        info->lcd_dsi_lane = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_dsi_format", &value, 1);
    if(ret == 1)
    {
        info->lcd_dsi_format = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_dsi_eotp", &value, 1);
    if(ret == 1)
    {
        info->lcd_dsi_eotp = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_dsi_te", &value, 1);
    if(ret == 1)
    {
        info->lcd_dsi_te = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_edp_rate", &value, 1);
    if(ret == 1)
    {
        info->lcd_edp_rate = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_edp_lane", &value, 1);
    if(ret == 1)
    {
        info->lcd_edp_lane= value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_edp_colordepth", &value, 1);
    if(ret == 1)
    {
        info->lcd_edp_colordepth = value;
    }

	ret = disp_sys_script_get_item(primary_key, "lcd_edp_fps", &value, 1);
    if(ret == 1)
    {
        info->lcd_edp_fps = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_hv_clk_phase", &value, 1);
    if(ret == 1)
    {
        info->lcd_hv_clk_phase = value;
    }

	ret = disp_sys_script_get_item(primary_key, "lcd_hv_sync_polarity", &value, 1);
    if(ret == 1)
    {
        info->lcd_hv_sync_polarity = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_hv_syuv_seq", &value, 1);
    if(ret == 1)
    {
        info->lcd_hv_syuv_seq = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_hv_syuv_fdly", &value, 1);
    if(ret == 1)
    {
        info->lcd_hv_syuv_fdly = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_gamma_en", &value, 1);
    if(ret == 1)
    {
        info->lcd_gamma_en = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_cmap_en", &value, 1);
    if(ret == 1)
    {
        info->lcd_cmap_en = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_xtal_freq", &value, 1);
    if(ret == 1)
    {
        info->lcd_xtal_freq = value;
    }

    ret = disp_sys_script_get_item(primary_key, "lcd_size", (int*)info->lcd_size, 25/sizeof(int));
    ret = disp_sys_script_get_item(primary_key, "lcd_model_name", (int*)info->lcd_model_name, 25/sizeof(int));

    return 0;
}

#if 0
static void lcd_panel_parameter_check(u32 disp, struct disp_device* lcd)
{
	disp_panel_para* info;
	u32 cycle_num = 1;
	u32 Lcd_Panel_Err_Flag = 0;
	u32 Lcd_Panel_Wrn_Flag = 0;
	u32 Disp_Driver_Bug_Flag = 0;

	u32 lcd_fclk_frq;
	u32 lcd_clk_div;
	s32 ret = 0;

	char primary_key[20];
	s32 value = 0;

	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);
	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return ;
	}

	if(!disp_al_query_lcd_mod(lcd->disp))
		return;

	sprintf(primary_key, "lcd%d_para", lcd->disp);
	ret = disp_sys_script_get_item(primary_key, "lcd_used", &value, 1);

	if(ret != 0 ) {
		DE_WRN("get lcd%dpara lcd_used fail\n", lcd->disp);
		return;
	} else {
		if(value != 1) {
			DE_WRN("lcd%dpara is not used\n", lcd->disp);
			return;
		}
	}

	info = &(lcdp->panel_info);
	if(NULL == info) {
		DE_WRN("NULL hdl!\n");
		return;
	}

	if(info->lcd_if == 0 && info->lcd_hv_if == 8)
		cycle_num = 3;
	else if(info->lcd_if == 0 && info->lcd_hv_if == 10)
		cycle_num = 3;
	else if(info->lcd_if == 0 && info->lcd_hv_if == 11)
		cycle_num = 4;
	else if(info->lcd_if == 0 && info->lcd_hv_if == 12)
		cycle_num = 4;
	else if(info->lcd_if == 1 && info->lcd_cpu_if == 2)
		cycle_num = 3;
	else if(info->lcd_if == 1 && info->lcd_cpu_if == 4)
		cycle_num = 2;
	else if(info->lcd_if == 1 && info->lcd_cpu_if == 6)
		cycle_num = 2;
	else if(info->lcd_if == 1 && info->lcd_cpu_if == 10)
		cycle_num = 2;
	else if(info->lcd_if == 1 && info->lcd_cpu_if == 12)
		cycle_num = 3;
	else if(info->lcd_if == 1 && info->lcd_cpu_if == 14)
		cycle_num = 2;
	else
		cycle_num = 1;

	if(info->lcd_hbp > info->lcd_hspw)
	{
		;
	}
	else
	{
		Lcd_Panel_Err_Flag |= BIT0;
	}

	if(info->lcd_vbp > info->lcd_vspw)
	{
		;
	}
	else
	{
		Lcd_Panel_Err_Flag |= BIT1;
	}

	if(info->lcd_ht >= (info->lcd_hbp+info->lcd_x*cycle_num+4))
	{
		;
	}
	else
	{
		Lcd_Panel_Err_Flag |= BIT2;
	}

	if((info->lcd_vt) >= (info->lcd_vbp+info->lcd_y + 2))
	{
		;
	}
	else
	{
		Lcd_Panel_Err_Flag |= BIT3;
	}

	lcd_clk_div = disp_al_lcd_get_clk_div(disp);

	if(lcd_clk_div >= 6)
	{
		;
	}
	else if(lcd_clk_div >=2)
	{
		if((info->lcd_hv_clk_phase == 1) && (info->lcd_hv_clk_phase == 3))
		{
			Lcd_Panel_Err_Flag |= BIT10;
		}
	}
	else
	{
		Disp_Driver_Bug_Flag |= 1;
	}

	if((info->lcd_if == 1 && info->lcd_cpu_if == 0) ||	(info->lcd_if == 1 && info->lcd_cpu_if == 10)
		|| (info->lcd_if == 1 && info->lcd_cpu_if == 12) ||(info->lcd_if == 3 && info->lcd_lvds_colordepth == 1))
	{
		if(info->lcd_frm != 1)
			Lcd_Panel_Wrn_Flag |= BIT0;
	}
	else if(info->lcd_if == 1 && ((info->lcd_cpu_if == 2) || (info->lcd_cpu_if == 4) || (info->lcd_cpu_if == 6)
		|| (info->lcd_cpu_if == 8) || (info->lcd_cpu_if == 14)))
	{
		if(info->lcd_frm != 2)
			Lcd_Panel_Wrn_Flag |= BIT1;
	}

	lcd_fclk_frq = (info->lcd_dclk_freq * 1000 * 1000) / ((info->lcd_vt) * info->lcd_ht);
	if(lcd_fclk_frq < 50 || lcd_fclk_frq > 70)
	{
		Lcd_Panel_Wrn_Flag |= BIT2;
	}

	if(Lcd_Panel_Err_Flag != 0 || Lcd_Panel_Wrn_Flag != 0)
	{
		if(Lcd_Panel_Err_Flag != 0)
		{
			u32 i;
			for(i = 0; i < 200; i++)
			{
				//OSAL_PRINTF("*** Lcd in danger...\n");
			}
		}

		OSAL_PRINTF("*****************************************************************\n");
		OSAL_PRINTF("***\n");
		OSAL_PRINTF("*** LCD Panel Parameter Check\n");
		OSAL_PRINTF("***\n");
		OSAL_PRINTF("***             by guozhenjie\n");
		OSAL_PRINTF("***\n");
		OSAL_PRINTF("*****************************************************************\n");

		OSAL_PRINTF("*** \n");
		OSAL_PRINTF("*** Interface:");
		if(info->lcd_if == 0 && info->lcd_hv_if == 0)
			{OSAL_PRINTF("*** Parallel HV Panel\n");}
		else if(info->lcd_if == 0 && info->lcd_hv_if == 8)
			{OSAL_PRINTF("*** Serial HV Panel\n");}
		else if(info->lcd_if == 0 && info->lcd_hv_if == 10)
			{OSAL_PRINTF("*** Dummy RGB HV Panel\n");}
		else if(info->lcd_if == 0 && info->lcd_hv_if == 11)
			{OSAL_PRINTF("*** RGB Dummy HV Panel\n");}
		else if(info->lcd_if == 0 && info->lcd_hv_if == 12)
			{OSAL_PRINTF("*** Serial YUV Panel\n");}
		else if(info->lcd_if == 3 && info->lcd_lvds_colordepth== 0)
			{OSAL_PRINTF("*** 24Bit LVDS Panel\n");}
		else if(info->lcd_if == 3 && info->lcd_lvds_colordepth== 1)
			{OSAL_PRINTF("*** 18Bit LVDS Panel\n");}
		else if((info->lcd_if == 1) && (info->lcd_cpu_if == 0 || info->lcd_cpu_if == 10 || info->lcd_cpu_if == 12))
			{OSAL_PRINTF("*** 18Bit CPU Panel\n");}
		else if((info->lcd_if == 1) && (info->lcd_cpu_if == 2 || info->lcd_cpu_if == 4 ||
				info->lcd_cpu_if == 6 || info->lcd_cpu_if == 8 || info->lcd_cpu_if == 14))
			{OSAL_PRINTF("*** 16Bit CPU Panel\n");}
		else
		{
			OSAL_PRINTF("\n");
			OSAL_PRINTF("*** lcd_if:     %d\n",info->lcd_if);
			OSAL_PRINTF("*** lcd_hv_if:  %d\n",info->lcd_hv_if);
			OSAL_PRINTF("*** lcd_cpu_if: %d\n",info->lcd_cpu_if);
		}
		if(info->lcd_frm == 0)
			{OSAL_PRINTF("*** Lcd Frm Disable\n");}
		else if(info->lcd_frm == 1)
			{OSAL_PRINTF("*** Lcd Frm to RGB666\n");}
		else if(info->lcd_frm == 2)
			{OSAL_PRINTF("*** Lcd Frm to RGB565\n");}

		OSAL_PRINTF("*** \n");
		OSAL_PRINTF("*** Timing:\n");
		OSAL_PRINTF("*** lcd_x:      %d\n", info->lcd_x);
		OSAL_PRINTF("*** lcd_y:      %d\n", info->lcd_y);
		OSAL_PRINTF("*** lcd_ht:     %d\n", info->lcd_ht);
		OSAL_PRINTF("*** lcd_hbp:    %d\n", info->lcd_hbp);
		OSAL_PRINTF("*** lcd_vt:     %d\n", info->lcd_vt);
		OSAL_PRINTF("*** lcd_vbp:    %d\n", info->lcd_vbp);
		OSAL_PRINTF("*** lcd_hspw:   %d\n", info->lcd_hspw);
		OSAL_PRINTF("*** lcd_vspw:   %d\n", info->lcd_vspw);
		OSAL_PRINTF("*** lcd_frame_frq:  %dHz\n", lcd_fclk_frq);

		OSAL_PRINTF("*** \n");
		if(Lcd_Panel_Err_Flag & BIT0)
			{OSAL_PRINTF("*** Err01: Violate \"lcd_hbp > lcd_hspw\"\n");}
		if(Lcd_Panel_Err_Flag & BIT1)
			{OSAL_PRINTF("*** Err02: Violate \"lcd_vbp > lcd_vspw\"\n");}
		if(Lcd_Panel_Err_Flag & BIT2)
			{OSAL_PRINTF("*** Err03: Violate \"lcd_ht >= (lcd_hbp+lcd_x*%d+4)\"\n", cycle_num);}
		if(Lcd_Panel_Err_Flag & BIT3)
			{OSAL_PRINTF("*** Err04: Violate \"(lcd_vt) >= (lcd_vbp+lcd_y+2)\"\n");}
		if(Lcd_Panel_Err_Flag & BIT10)
			{OSAL_PRINTF("*** Err10: Violate \"lcd_hv_clk_phase\",use \"0\" or \"2\"");}
		if(Lcd_Panel_Wrn_Flag & BIT0)
			{OSAL_PRINTF("*** WRN01: Recommend \"lcd_frm = 1\"\n");}
		if(Lcd_Panel_Wrn_Flag & BIT1)
			{OSAL_PRINTF("*** WRN02: Recommend \"lcd_frm = 2\"\n");}
		if(Lcd_Panel_Wrn_Flag & BIT2)
			{OSAL_PRINTF("*** WRN03: Recommend \"lcd_dclk_frq = %d\"\n",
				((info->lcd_vt) * info->lcd_ht) * 60 / (1000 * 1000));}
		OSAL_PRINTF("*** \n");

		if(Lcd_Panel_Err_Flag != 0)
		{
			u32 image_base_addr;
			u32 reg_value = 0;

			image_base_addr = DE_Get_Reg_Base(disp);

			sys_put_wvalue(image_base_addr+0x804, 0xffff00ff);//set background color

			reg_value = sys_get_wvalue(image_base_addr + 0x800);
			sys_put_wvalue(image_base_addr+0x800, reg_value & 0xfffff0ff);//close all layer

			mdelay(2000);
			sys_put_wvalue(image_base_addr + 0x804, 0x00000000);//set background color
			sys_put_wvalue(image_base_addr + 0x800, reg_value);//open layer

			OSAL_PRINTF("*** Try new parameters,you can make it pass!\n");
		}
		OSAL_PRINTF("*** LCD Panel Parameter Check End\n");
		OSAL_PRINTF("*****************************************************************\n");
	}
}
#endif

static void lcd_get_sys_config(u32 disp, __disp_lcd_cfg_t *lcd_cfg)
{
    static char io_name[28][20] = {"lcdd0", "lcdd1", "lcdd2", "lcdd3", "lcdd4", "lcdd5", "lcdd6", "lcdd7", "lcdd8", "lcdd9", "lcdd10", "lcdd11",
                         "lcdd12", "lcdd13", "lcdd14", "lcdd15", "lcdd16", "lcdd17", "lcdd18", "lcdd19", "lcdd20", "lcdd21", "lcdd22",
                         "lcdd23", "lcdclk", "lcdde", "lcdhsync", "lcdvsync"};
    disp_gpio_set_t  *gpio_info;
    int  value = 1;
    char primary_key[20], sub_name[25];
    int i = 0;
    int  ret;

    sprintf(primary_key, "lcd%d_para", disp);

//lcd_used
    ret = disp_sys_script_get_item(primary_key, "lcd_used", &value, 1);
    if(ret == 1)
    {
        lcd_cfg->lcd_used = value;
    }

    if(lcd_cfg->lcd_used == 0) //no need to get lcd config if lcd_used eq 0
        return ;

//lcd_bl_en
    lcd_cfg->lcd_bl_en_used = 0;
    gpio_info = &(lcd_cfg->lcd_bl_en);
    ret = disp_sys_script_get_item(primary_key,"lcd_bl_en", (int *)gpio_info, sizeof(disp_gpio_set_t)/sizeof(int));
    if(ret == 3)
    {
        lcd_cfg->lcd_bl_en_used = 1;
    }

	sprintf(sub_name, "lcd_bl_regulator");
	ret = disp_sys_script_get_item(primary_key, sub_name, (int *)lcd_cfg->lcd_bl_regulator, 25/sizeof(int));

//lcd_power0
	for(i=0; i<LCD_POWER_NUM; i++)
	{
		if(i==0)
			sprintf(sub_name, "lcd_power");
		else
			sprintf(sub_name, "lcd_power%d", i);
		lcd_cfg->lcd_power_type[i] = 0; /* invalid */
		ret = disp_sys_script_get_item(primary_key,sub_name, (int *)(lcd_cfg->lcd_regu[i]), 25/sizeof(int));
		if(ret == 3) {
			/* gpio */
		  lcd_cfg->lcd_power_type[i] = 1; /* gpio */
		  memcpy(&(lcd_cfg->lcd_power[i]), lcd_cfg->lcd_regu[i], sizeof(disp_gpio_set_t));
		} else if(ret == 2) {
			/* str */
			lcd_cfg->lcd_power_type[i] = 2; /* regulator */
		}
	}

//lcd_gpio
    for(i=0; i<4; i++)
    {
        sprintf(sub_name, "lcd_gpio_%d", i);

        gpio_info = &(lcd_cfg->lcd_gpio[i]);
        ret = disp_sys_script_get_item(primary_key,sub_name, (int *)gpio_info, sizeof(disp_gpio_set_t)/sizeof(int));
        if(ret == 3)
        {
            lcd_cfg->lcd_gpio_used[i]= 1;
        }
    }

//lcd_gpio_scl,lcd_gpio_sda
    gpio_info = &(lcd_cfg->lcd_gpio[LCD_GPIO_SCL]);
    ret = disp_sys_script_get_item(primary_key,"lcd_gpio_scl", (int *)gpio_info, sizeof(disp_gpio_set_t)/sizeof(int));
    if(ret == 3)
    {
        lcd_cfg->lcd_gpio_used[LCD_GPIO_SCL]= 1;
    }
    gpio_info = &(lcd_cfg->lcd_gpio[LCD_GPIO_SDA]);
    ret = disp_sys_script_get_item(primary_key,"lcd_gpio_sda", (int *)gpio_info, sizeof(disp_gpio_set_t)/sizeof(int));
    if(ret == 3)
    {
        lcd_cfg->lcd_gpio_used[LCD_GPIO_SDA]= 1;
    }

	for(i = 0; i < LCD_GPIO_REGU_NUM; i++)
	{
		sprintf(sub_name, "lcd_gpio_regulator%d", i);

		ret = disp_sys_script_get_item(primary_key, sub_name, (int *)lcd_cfg->lcd_gpio_regulator[i], 25/sizeof(int));
	}

//lcd io
    for(i=0; i<28; i++)
    {
        gpio_info = &(lcd_cfg->lcd_io[i]);
        ret = disp_sys_script_get_item(primary_key,io_name[i], (int *)gpio_info, sizeof(disp_gpio_set_t)/sizeof(int));
        if(ret == 3)
        {
            lcd_cfg->lcd_io_used[i]= 1;
        }
    }

	for(i=0; i<LCD_GPIO_REGU_NUM; i++) {
		if(0==i)
			sprintf(sub_name, "lcd_io_regulator");
		else
			sprintf(sub_name, "lcd_io_regulator%d", i);
		ret = disp_sys_script_get_item(primary_key, sub_name, (int *)lcd_cfg->lcd_io_regulator[i], 25/sizeof(int));
	}

//backlight adjust
	for(i = 0; i < 101; i++) {
		sprintf(sub_name, "lcd_bl_%d_percent", i);
		lcd_cfg->backlight_curve_adjust[i] = 0;

		if(i == 100)
			lcd_cfg->backlight_curve_adjust[i] = 255;

		ret = disp_sys_script_get_item(primary_key, sub_name, &value, 1);
		if(ret == 1) {
			value = (value > 100)? 100:value;
			value = value * 255 / 100;
			lcd_cfg->backlight_curve_adjust[i] = value;
		}
	}


//init_bright
    sprintf(primary_key, "disp_init");
    sprintf(sub_name, "lcd%d_backlight", disp);

    ret = disp_sys_script_get_item(primary_key, sub_name, &value, 1);
    if(ret < 0)
    {
        lcd_cfg->backlight_bright = 197;
    }
    else
    {
        if(value > 256)
        {
            value = 256;
        }
        lcd_cfg->backlight_bright = value;
    }

//bright,constraction,saturation,hue
    sprintf(primary_key, "disp_init");
    sprintf(sub_name, "lcd%d_bright", disp);
    ret = disp_sys_script_get_item(primary_key, sub_name, &value, 1);
    if(ret < 0)
    {
        lcd_cfg->lcd_bright = 50;
    }
    else
    {
        if(value > 100)
        {
            value = 100;
        }
        lcd_cfg->lcd_bright = value;
    }

    sprintf(sub_name, "lcd%d_contrast", disp);
    ret = disp_sys_script_get_item(primary_key, sub_name, &value, 1);
    if(ret < 0)
    {
        lcd_cfg->lcd_contrast = 50;
    }
    else
    {
        if(value > 100)
        {
            value = 100;
        }
        lcd_cfg->lcd_contrast = value;
    }

    sprintf(sub_name, "lcd%d_saturation", disp);
    ret = disp_sys_script_get_item(primary_key, sub_name, &value, 1);
    if(ret < 0)
    {
        lcd_cfg->lcd_saturation = 50;
    }
    else
    {
        if(value > 100)
        {
            value = 100;
        }
        lcd_cfg->lcd_saturation = value;
    }

    sprintf(sub_name, "lcd%d_hue", disp);
    ret = disp_sys_script_get_item(primary_key, sub_name, &value, 1);
    if(ret < 0)
    {
        lcd_cfg->lcd_hue = 50;
    }
    else
    {
        if(value > 100)
        {
            value = 100;
        }
        lcd_cfg->lcd_hue = value;
    }
}

static s32 lcd_clk_init(struct disp_device* lcd)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return -1;
	}

	DE_INF("lcd %d clk init\n", lcd->disp);

	if(LCD_IF_LVDS == lcdp->panel_info.lcd_if) {
		//disp_sys_clk_set_parent(lcdp->lvds_clk, lcdp->clk_parent);
	} else if(LCD_IF_DSI == lcdp->panel_info.lcd_if) {
		disp_sys_clk_set_parent(lcdp->dsi_clk0, lcdp->clk_parent);
		//disp_sys_clk_set_parent(lcdp->dsi_clk1, lcdp->clk_parent);
	} else if(LCD_IF_EDP == lcdp->panel_info.lcd_if) {
		disp_sys_clk_set_parent(lcdp->edp_clk, lcdp->clk_parent);
	}

	disp_sys_clk_set_parent(lcdp->clk, lcdp->clk_parent);

	return DIS_SUCCESS;
}

static s32 lcd_clk_exit(struct disp_device* lcd)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return -1;
	}

	return DIS_SUCCESS;
}

static s32 lcd_clk_config(struct disp_device* lcd)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);
	struct lcd_clk_info clk_info;
	unsigned long pll_rate, lcd_rate, dclk_rate, dsi_rate = 0;//hz
	unsigned long pll_rate_set, lcd_rate_set, dclk_rate_set, dsi_rate_set = 0;//hz

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return -1;
	}
	memset(&clk_info, 0, sizeof(struct lcd_clk_info));
	disp_al_lcd_get_clk_info(lcd->disp, &clk_info, &lcdp->panel_info);
	dclk_rate = lcdp->panel_info.lcd_dclk_freq * 1000000;//Mhz -> hz
	lcd_rate = dclk_rate * clk_info.tcon_div;
	pll_rate = lcd_rate * clk_info.lcd_div;
	dsi_rate = pll_rate / clk_info.dsi_div;

	disp_sys_clk_set_rate(lcdp->clk_parent, pll_rate);
	pll_rate_set = disp_sys_clk_get_rate(lcdp->clk_parent);
	lcd_rate_set = pll_rate_set / clk_info.lcd_div;
	disp_sys_clk_set_rate(lcdp->clk, lcd_rate_set);
	lcd_rate_set = disp_sys_clk_get_rate(lcdp->clk);
	if(LCD_IF_DSI == lcdp->panel_info.lcd_if) {
		dsi_rate_set = pll_rate_set / clk_info.dsi_div;
		disp_sys_clk_set_rate(lcdp->dsi_clk0, dsi_rate_set);
		//disp_sys_clk_set_rate(lcdp->dsi_clk1, dsi_rate_set);//FIXME, dsi clk0 = dsi clk1(rate)
	}
	dclk_rate_set = lcd_rate_set / clk_info.tcon_div;
	if((pll_rate_set != pll_rate) || (lcd_rate_set != lcd_rate)
		|| (dclk_rate_set != dclk_rate)) {
			DE_WRN("disp %d, clk: pll(%ld),clk(%ld),dclk(%ld) dsi_rate(%ld)\n     clk real:pll(%ld),clk(%ld),dclk(%ld) dsi_rate(%ld)\n",
				lcd->disp, pll_rate, lcd_rate, dclk_rate, dsi_rate, pll_rate_set, lcd_rate_set, dclk_rate_set, dsi_rate_set);
	}

	return 0;
}

static s32 lcd_clk_enable(struct disp_device* lcd)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return -1;
	}
	lcd_clk_config(lcd);

	disp_sys_clk_enable(lcdp->clk);

	if(LCD_IF_LVDS == lcdp->panel_info.lcd_if) {
		disp_sys_clk_enable(lcdp->lvds_clk);
	} else if(LCD_IF_DSI == lcdp->panel_info.lcd_if) {
		disp_sys_clk_enable(lcdp->dsi_clk0);
		disp_sys_clk_enable(lcdp->dsi_clk1);
	} else if(LCD_IF_EDP == lcdp->panel_info.lcd_if) {
		disp_sys_clk_enable(lcdp->edp_clk);
	}

	return	DIS_SUCCESS;
}

static s32 lcd_clk_disable(struct disp_device* lcd)
{	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return -1;
	}

	if(LCD_IF_LVDS == lcdp->panel_info.lcd_if) {
		disp_sys_clk_disable(lcdp->lvds_clk);
	} else if(LCD_IF_DSI == lcdp->panel_info.lcd_if) {
		disp_sys_clk_disable(lcdp->dsi_clk1);
		disp_sys_clk_disable(lcdp->dsi_clk0);
	} else if(LCD_IF_EDP == lcdp->panel_info.lcd_if) {
		disp_sys_clk_disable(lcdp->edp_clk);
	}
	disp_sys_clk_disable(lcdp->clk);

	return	DIS_SUCCESS;
}

static s32 disp_lcd_tcon_enable(struct disp_device *lcd)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return -1;
	}

	return disp_al_lcd_enable(lcd->disp, &lcdp->panel_info);
}

static s32 disp_lcd_tcon_disable(struct disp_device *lcd)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return -1;
	}

	return disp_al_lcd_disable(lcd->disp, &lcdp->panel_info);
}

static s32 disp_lcd_pin_cfg(struct disp_device *lcd, u32 bon)
{
	int lcd_pin_hdl;
	int  i;
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}
	DE_INF("lcd %d pin config, state %s, %d\n", lcd->disp, (bon)? "on":"off", bon);

	//io-pad
	if(bon == 1) {
		for(i=0; i<LCD_GPIO_REGU_NUM; i++) {
			if(!((!strcmp(lcdp->lcd_cfg.lcd_io_regulator[i], "")) || (!strcmp(lcdp->lcd_cfg.lcd_io_regulator[i], "none"))))
				disp_sys_power_enable(lcdp->lcd_cfg.lcd_io_regulator[i]);
		}
	}

	for(i=0; i<28; i++)	{
		if(lcdp->lcd_cfg.lcd_io_used[i]) {
			disp_gpio_set_t  gpio_info[1];

			memcpy(gpio_info, &(lcdp->lcd_cfg.lcd_io[i]), sizeof(disp_gpio_set_t));
			if(!bon) {
				gpio_info->mul_sel = 7;
			}	else {
				if((lcdp->panel_info.lcd_if == 3) && (gpio_info->mul_sel==2))	{
					gpio_info->mul_sel = 3;
				}
			}
			lcd_pin_hdl = disp_sys_gpio_request(gpio_info, 1);
			disp_sys_gpio_release(lcd_pin_hdl, 2);
		}
	}
	disp_al_lcd_io_cfg(lcd->disp, bon, &lcdp->panel_info);

	if(bon == 0) {
		for(i=LCD_GPIO_REGU_NUM-1; i>=0; i--) {
			if(!((!strcmp(lcdp->lcd_cfg.lcd_io_regulator[i], "")) || (!strcmp(lcdp->lcd_cfg.lcd_io_regulator[i], "none"))))
				disp_sys_power_disable(lcdp->lcd_cfg.lcd_io_regulator[i]);
		}
	}

	return DIS_SUCCESS;
}

static s32 disp_lcd_pwm_enable(struct disp_device *lcd)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}

	if(disp_lcd_is_used(lcd) && lcdp->pwm_info.dev) {
		return disp_sys_pwm_enable(lcdp->pwm_info.dev);
	}
	DE_WRN("pwm device hdl is NULL\n");

	return DIS_FAIL;
}

static s32 disp_lcd_pwm_disable(struct disp_device *lcd)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}

	if(disp_lcd_is_used(lcd) && lcdp->pwm_info.dev) {
		return disp_sys_pwm_disable(lcdp->pwm_info.dev);
	}
	DE_WRN("pwm device hdl is NULL\n");

	return DIS_FAIL;
}

static s32 disp_lcd_backlight_enable(struct disp_device *lcd)
{
	disp_gpio_set_t  gpio_info[1];
	int hdl;
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);
	unsigned long flags;

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}

	disp_sys_irqlock((void*)&lcd_data_lock, &flags);
	lcdp->bl_need_enabled = 1;
	disp_sys_irqunlock((void*)&lcd_data_lock, &flags);

	if(disp_lcd_is_used(lcd) && (0 < lcdp->lcd_cfg.backlight_bright)) {
		if(lcdp->lcd_cfg.lcd_bl_en_used) {
			//io-pad
			if(!((!strcmp(lcdp->lcd_cfg.lcd_bl_regulator, "")) || (!strcmp(lcdp->lcd_cfg.lcd_bl_regulator, "none"))))
				disp_sys_power_enable(lcdp->lcd_cfg.lcd_bl_regulator);

			memcpy(gpio_info, &(lcdp->lcd_cfg.lcd_bl_en), sizeof(disp_gpio_set_t));

			hdl = disp_sys_gpio_request(gpio_info, 1);
			disp_sys_gpio_release(hdl, 2);
		}
	}

	return 0;
}

static s32 disp_lcd_backlight_disable(struct disp_device *lcd)
{
	disp_gpio_set_t  gpio_info[1];
	int hdl;
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}

	if(disp_lcd_is_used(lcd)) {
		if(lcdp->lcd_cfg.lcd_bl_en_used) {
			memcpy(gpio_info, &(lcdp->lcd_cfg.lcd_bl_en), sizeof(disp_gpio_set_t));
			gpio_info->data = (gpio_info->data==0)?1:0;
			gpio_info->mul_sel = 7;
			hdl = disp_sys_gpio_request(gpio_info, 1);
			disp_sys_gpio_release(hdl, 2);

			//io-pad
			if(!((!strcmp(lcdp->lcd_cfg.lcd_bl_regulator, "")) || (!strcmp(lcdp->lcd_cfg.lcd_bl_regulator, "none"))))
				disp_sys_power_disable(lcdp->lcd_cfg.lcd_bl_regulator);
		}
	}

	return 0;
}

static s32 disp_lcd_power_enable(struct disp_device *lcd, u32 power_id)
{
	disp_gpio_set_t  gpio_info[1];
	int hdl;
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}

	if(disp_lcd_is_used(lcd)) {
		if(lcdp->lcd_cfg.lcd_power_type[power_id] == 1) {
			/* gpio type */
			memcpy(gpio_info, &(lcdp->lcd_cfg.lcd_power[power_id]), sizeof(disp_gpio_set_t));

			hdl = disp_sys_gpio_request(gpio_info, 1);
			disp_sys_gpio_release(hdl, 2);
		} else if(lcdp->lcd_cfg.lcd_power_type[power_id] == 2) {
			/* regulator type */
			disp_sys_power_enable(lcdp->lcd_cfg.lcd_regu[power_id]);
		}
	}

	return 0;
}

static s32 disp_lcd_power_disable(struct disp_device *lcd, u32 power_id)
{
	disp_gpio_set_t  gpio_info[1];
	int hdl;
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}

	if(disp_lcd_is_used(lcd)) {
		if(lcdp->lcd_cfg.lcd_power_type[power_id] == 1) {
			memcpy(gpio_info, &(lcdp->lcd_cfg.lcd_power[power_id]), sizeof(disp_gpio_set_t));
			gpio_info->data = (gpio_info->data==0)?1:0;
			gpio_info->mul_sel = 7;
			hdl = disp_sys_gpio_request(gpio_info, 1);
			disp_sys_gpio_release(hdl, 2);
		} else if(lcdp->lcd_cfg.lcd_power_type[power_id] == 2) {
			/* regulator type */
			disp_sys_power_disable(lcdp->lcd_cfg.lcd_regu[power_id]);
		}
	}

	return 0;
}

static s32 disp_lcd_bright_get_adjust_value(struct disp_device *lcd, u32 bright)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}
	bright = (bright > 255)? 255:bright;
	return lcdp->panel_extend_info.lcd_bright_curve_tbl[bright];
}

static s32 disp_lcd_bright_curve_init(struct disp_device *lcd)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);
	u32 i = 0, j=0;
	u32 items = 0;
	u32 lcd_bright_curve_tbl[101][2];

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}

	for(i = 0; i < 101; i++) {
		if(lcdp->lcd_cfg.backlight_curve_adjust[i] == 0) {
			if(i == 0) {
				lcd_bright_curve_tbl[items][0] = 0;
				lcd_bright_curve_tbl[items][1] = 0;
				items++;
			}
		}	else {
			lcd_bright_curve_tbl[items][0] = 255 * i / 100;
			lcd_bright_curve_tbl[items][1] = lcdp->lcd_cfg.backlight_curve_adjust[i];
			items++;
		}
	}

	for(i=0; i<items-1; i++) {
		u32 num = lcd_bright_curve_tbl[i+1][0] - lcd_bright_curve_tbl[i][0];

		for(j=0; j<num; j++) {
			u32 value = 0;

			value = lcd_bright_curve_tbl[i][1] + ((lcd_bright_curve_tbl[i+1][1] - lcd_bright_curve_tbl[i][1]) * j)/num;
			lcdp->panel_extend_info.lcd_bright_curve_tbl[lcd_bright_curve_tbl[i][0] + j] = value;
		}
	}
	lcdp->panel_extend_info.lcd_bright_curve_tbl[255] = lcd_bright_curve_tbl[items-1][1];

	return 0;
}

s32 disp_lcd_set_bright(struct disp_device *lcd, u32 bright)
{
	u32 duty_ns;
	__u64 backlight_bright = bright;
	__u64 backlight_dimming;
	__u64 period_ns;
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);
	bool need_enable_bl = false, need_disable_bl = false;
	unsigned long flags;
	bool bright_update = false;
	struct disp_manager *mgr = NULL;
	struct disp_smbl *smbl = NULL;

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}
	mgr = lcd->manager;
	if(NULL == mgr) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}
	smbl = mgr->smbl;

	disp_sys_irqlock((void*)&lcd_data_lock, &flags);
	if((0 == lcdp->lcd_cfg.backlight_bright) && (0 != bright) && (lcdp->bl_need_enabled))
		need_enable_bl = true;
	if((0 != lcdp->lcd_cfg.backlight_bright) && (0 == bright))
		need_disable_bl = true;
	backlight_bright = (backlight_bright > 255)? 255:backlight_bright;
	if(lcdp->lcd_cfg.backlight_bright != backlight_bright) {
		bright_update = true;
		lcdp->lcd_cfg.backlight_bright = backlight_bright;
	}
	disp_sys_irqunlock((void*)&lcd_data_lock, &flags);
	if(bright_update && smbl)
		smbl->update_backlight(smbl, backlight_bright);

	if(lcdp->pwm_info.dev) {
		if(backlight_bright != 0)	{
			backlight_bright += 1;
		}
		backlight_bright = disp_lcd_bright_get_adjust_value(lcd, backlight_bright);

		lcdp->lcd_cfg.backlight_dimming = (0 == lcdp->lcd_cfg.backlight_dimming)? 256:lcdp->lcd_cfg.backlight_dimming;
		backlight_dimming = lcdp->lcd_cfg.backlight_dimming;
		period_ns = lcdp->pwm_info.period_ns;
		duty_ns = (backlight_bright * backlight_dimming *  period_ns/256 + 128) / 256;
		lcdp->pwm_info.duty_ns = duty_ns;

		disp_sys_pwm_config(lcdp->pwm_info.dev, duty_ns, period_ns);
	}

	if(lcdp->lcd_panel_fun.set_bright) {
		lcdp->lcd_panel_fun.set_bright(lcd->disp,disp_lcd_bright_get_adjust_value(lcd,bright));
	}

	if(need_enable_bl && (lcdp->enabled || lcdp->enabling))
		disp_lcd_backlight_enable(lcd);
	if(need_disable_bl)
		disp_lcd_backlight_disable(lcd);

	return DIS_SUCCESS;
}

s32 disp_lcd_get_bright(struct disp_device *lcd)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}

	return lcdp->lcd_cfg.backlight_bright;
}

static s32 disp_lcd_set_bright_dimming(struct disp_device *lcd, u32 dimming)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);
	u32 bl = 0;

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}

	dimming = dimming > 256?256:dimming;
	lcdp->lcd_cfg.backlight_dimming = dimming;
	bl = disp_lcd_get_bright(lcd);
	disp_lcd_set_bright(lcd, bl);

	return DIS_SUCCESS;
}

static s32 disp_lcd_get_panel_info(struct disp_device *lcd, disp_panel_para* info)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}

	memcpy(info, (disp_panel_para*)(&(lcdp->panel_info)), sizeof(disp_panel_para));
	return 0;
}

//FIXME
extern void sync_event_proc(u32 disp);
#if defined(__LINUX_PLAT__)
static s32 disp_lcd_event_proc(int irq, void *parg)
#else
static s32 disp_lcd_event_proc(void *parg)
#endif
{
	u32 disp = (u32)parg;
	struct disp_device *lcd = disp_get_lcd(disp);
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);
	if((NULL == lcd) || (NULL == lcdp)) {
		//FIXME: return is ok?
		return 0;
	}

	if(disp_al_lcd_query_irq(disp, LCD_IRQ_TCON0_VBLK, &lcdp->panel_info)) {
		int cur_line = disp_al_lcd_get_cur_line(disp, &lcdp->panel_info);
		int start_delay = disp_al_lcd_get_start_delay(disp, &lcdp->panel_info);

		if(cur_line <= (start_delay-4)) {
			sync_event_proc(disp);
		} else {
			/* skip a frame */
		}
	}

	return 0;
}

static s32 disp_lcd_pre_enable(struct disp_device* lcd)
{
	unsigned long flags;
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);
	struct disp_manager *mgr = NULL;

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}
	DE_INF("lcd %d\n", lcd->disp);
	mgr = lcd->manager;
	if((NULL == mgr)) {
		DE_WRN("mgr is NULL!\n");
		return DIS_FAIL;
	}
	if(1 == disp_lcd_is_enabled(lcd))
		return 0;

	if(mgr->enable)
		mgr->enable(mgr);

#if defined(__FPGA_DEBUG__)

#if defined(CONFIG_ARCH_SUN9IW1)
#define SUNXI_CCM_VBASE SUNXI_CCM_MOD_VBASE
#endif

#if !defined(CONFIG_ARCH_SUN9IW1)
	writel(readl(SUNXI_CCM_VBASE) | 0x10, SUNXI_CCM_VBASE + 0x2c4);//lcd0 reset
	writel(readl(SUNXI_CCM_VBASE) | 0x1000, SUNXI_CCM_VBASE + 0x2c4);//de reset
#endif
	//gpio,pd28--pwm, ph0--pwr,pd29--bl_en
	writel(0x22222222, SUNXI_PIO_VBASE + 0x74);
	writel(0x00122222, SUNXI_PIO_VBASE + 0x78);
	writel((readl(SUNXI_PIO_VBASE + 0xfc) & (~0x0000000f)) | 0x00000001, SUNXI_PIO_VBASE + 0xfc);
	writel((readl(SUNXI_PIO_VBASE + 0x10C) & (~0x0000000f)) | 0x00000001, SUNXI_PIO_VBASE + 0x10C);
#endif

	disp_sys_irqlock((void*)&lcd_data_lock, &flags);
	lcdp->enabling = 1;
	disp_sys_irqunlock((void*)&lcd_data_lock, &flags);
	if(lcdp->lcd_panel_fun.cfg_panel_info)
		lcdp->lcd_panel_fun.cfg_panel_info(&lcdp->panel_extend_info);
	else
		DE_WRN("lcd_panel_fun[%d].cfg_panel_info is NULL\n", lcd->disp);
	disp_lcd_gpio_init(lcd);
	lcd_clk_enable(lcd);
	disp_al_lcd_cfg(lcd->disp, &lcdp->panel_info, &lcdp->panel_extend_info);

	return 0;
}

static s32 disp_lcd_post_enable(struct disp_device* lcd)
{
	unsigned long flags;
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}
	DE_INF("lcd %d\n", lcd->disp);

	disp_sys_irqlock((void*)&lcd_data_lock, &flags);
	lcdp->enabled = 1;
	lcdp->enabling = 0;
	disp_sys_irqunlock((void*)&lcd_data_lock, &flags);

	return 0;
}

static void disp_lcd_pre_enable_ex(unsigned int disp)
{
	struct disp_device *lcd = NULL;
	lcd = disp_get_lcd(disp);
	if(lcd != NULL)
		disp_lcd_pre_enable(lcd);

	return ;
}

static void disp_lcd_post_enable_ex(unsigned int disp)
{
	struct disp_device *lcd = NULL;
	lcd = disp_get_lcd(disp);
	if(lcd != NULL)
		disp_lcd_post_enable(lcd);

	return ;
}

static s32 disp_lcd_enable(struct disp_device* lcd)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);
	int i;
	disp_lcd_flow *open_flow = NULL;

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}
	DE_INF("lcd %d\n", lcd->disp);
	if(lcd->get_open_flow)
		open_flow = lcd->get_open_flow(lcd);

	if(open_flow) {
		for(i=0; i<open_flow->func_num; i++) {
			if(open_flow->func[i].func) {
				open_flow->func[i].func(lcd->disp);
				if(0 != open_flow->func[i].delay)
					disp_delay_ms(open_flow->func[i].delay);
			}
		}
	}

	return 0;
}

static s32 disp_lcd_pre_disable(struct disp_device* lcd)
{
	unsigned long flags;
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}
	DE_INF("lcd %d\n", lcd->disp);

	disp_sys_irqlock((void*)&lcd_data_lock, &flags);
	lcdp->disabling = 1;
	disp_sys_irqunlock((void*)&lcd_data_lock, &flags);

	return 0;
}

static s32 disp_lcd_post_disable(struct disp_device* lcd)
{
	unsigned long flags;
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);
	struct disp_manager *mgr = NULL;

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}
	DE_INF("lcd %d\n", lcd->disp);
	mgr = lcd->manager;
	if((NULL == mgr)) {
		DE_WRN("mgr is NULL!\n");
		return DIS_FAIL;
	}

	disp_lcd_gpio_exit(lcd);

	disp_sys_irqlock((void*)&lcd_data_lock, &flags);
	lcdp->enabled = 0;
	disp_sys_irqunlock((void*)&lcd_data_lock, &flags);
	lcd_clk_disable(lcd);

	if(mgr->disable)
		mgr->disable(mgr);

	return 0;
}

static void disp_lcd_pre_disable_ex(unsigned int disp)
{
	struct disp_device *lcd = NULL;
	lcd = disp_get_lcd(disp);
	if(lcd != NULL)
		disp_lcd_pre_disable(lcd);

	return ;
}

static void disp_lcd_post_disable_ex(unsigned int disp)
{
	struct disp_device *lcd = NULL;
	lcd = disp_get_lcd(disp);
	if(lcd != NULL)
		disp_lcd_post_disable(lcd);

	return ;
}

static s32 disp_lcd_disable(struct disp_device* lcd)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);
	int i;
	disp_lcd_flow *close_flow = NULL;

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}
	DE_INF("lcd %d\n", lcd->disp);
	if(lcd->get_close_flow)
		close_flow = lcd->get_close_flow(lcd);

	if(close_flow) {
		for(i=0; i<close_flow->func_num; i++) {
			if(close_flow->func[i].func) {
				DE_INF("step %d, delay %d\n", i, close_flow->func[i].delay);
				close_flow->func[i].func(lcd->disp);
				if(0 != close_flow->func[i].delay)
					disp_delay_ms(close_flow->func[i].delay);
			}
		}
	}

	return 0;
}

s32 disp_lcd_is_enabled(struct disp_device* lcd)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}

	return (s32)lcdp->enabled;
}

static s32 disp_lcd_set_open_func(struct disp_device* lcd, LCD_FUNC func, u32 delay)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return -1;
	}

	if(func) {
		lcdp->open_flow.func[lcdp->open_flow.func_num].func = func;
		lcdp->open_flow.func[lcdp->open_flow.func_num].delay = delay;
		lcdp->open_flow.func_num ++;
	}

	return DIS_SUCCESS;
}

static s32 disp_lcd_set_close_func(struct disp_device* lcd, LCD_FUNC func, u32 delay)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return -1;
	}

	if(func) {
		lcdp->close_flow.func[lcdp->close_flow.func_num].func = func;
		lcdp->close_flow.func[lcdp->close_flow.func_num].delay = delay;
		lcdp->close_flow.func_num ++;
	}

	return DIS_SUCCESS;
}

static s32 disp_lcd_set_panel_funs(struct disp_device* lcd, char *name, disp_lcd_panel_fun * lcd_cfg)
{
	char primary_key[20], drv_name[32];
	s32 ret;
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}

	sprintf(primary_key, "lcd%d_para", lcd->disp);

	ret = disp_sys_script_get_item(primary_key, "lcd_driver_name",  (int*)drv_name, 32/sizeof(int));
	if((2==ret) && !strcmp(drv_name, name)) {
		memset(&lcdp->lcd_panel_fun, 0, sizeof(disp_lcd_panel_fun));
		lcdp->lcd_panel_fun.cfg_panel_info= lcd_cfg->cfg_panel_info;
		lcdp->lcd_panel_fun.cfg_open_flow = lcd_cfg->cfg_open_flow;
		lcdp->lcd_panel_fun.cfg_close_flow = lcd_cfg->cfg_close_flow;
		lcdp->lcd_panel_fun.lcd_user_defined_func = lcd_cfg->lcd_user_defined_func;
		lcdp->lcd_panel_fun.set_bright = lcd_cfg->set_bright;
		return 0;
	}

	return -1;
}

static disp_lcd_flow * disp_lcd_get_open_flow(struct disp_device* lcd)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return NULL;
	}

	lcdp->open_flow.func_num = 0;
	lcdp->open_flow.func[lcdp->open_flow.func_num].func = disp_lcd_pre_enable_ex;
	lcdp->open_flow.func[lcdp->open_flow.func_num].delay = 0;
	lcdp->open_flow.func_num ++;
	
	if(lcdp->lcd_panel_fun.cfg_open_flow)	{
		lcdp->lcd_panel_fun.cfg_open_flow(lcd->disp);
	}	else {
		DE_WRN("lcd_panel_fun[%d].cfg_open_flow is NULL\n", lcd->disp);
	}
	lcdp->open_flow.func[lcdp->open_flow.func_num].func = disp_lcd_post_enable_ex;
	lcdp->open_flow.func[lcdp->open_flow.func_num].delay = 0;
	lcdp->open_flow.func_num ++;

	return &(lcdp->open_flow);
}

static disp_lcd_flow * disp_lcd_get_close_flow(struct disp_device* lcd)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return NULL;
	}

	lcdp->close_flow.func_num = 0;
	lcdp->close_flow.func[lcdp->close_flow.func_num].func = disp_lcd_pre_disable_ex;
	lcdp->close_flow.func[lcdp->close_flow.func_num].delay = 0;
	lcdp->close_flow.func_num ++;
	
	if(lcdp->lcd_panel_fun.cfg_close_flow)	{
		lcdp->lcd_panel_fun.cfg_close_flow(lcd->disp);
	}	else {
		DE_WRN("lcd_panel_fun[%d].cfg_close_flow is NULL\n", lcd->disp);
	}
	lcdp->close_flow.func[lcdp->close_flow.func_num].func = disp_lcd_post_disable_ex;
	lcdp->close_flow.func[lcdp->close_flow.func_num].delay = 0;
	lcdp->close_flow.func_num ++;

	return &(lcdp->close_flow);
}

s32 disp_lcd_gpio_init(struct disp_device* lcd)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);
	u32 i = 0;

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}

	//io-pad
	for(i = 0; i < LCD_GPIO_REGU_NUM; i++)
	{
		if(!((!strcmp(lcdp->lcd_cfg.lcd_gpio_regulator[i], "")) || (!strcmp(lcdp->lcd_cfg.lcd_gpio_regulator[i], "none"))))
			disp_sys_power_enable(lcdp->lcd_cfg.lcd_gpio_regulator[i]);
	}

	for(i=0; i<LCD_GPIO_NUM; i++) {
		lcdp->lcd_cfg.gpio_hdl[i] = 0;

		if(lcdp->lcd_cfg.lcd_gpio_used[i]) {
			disp_gpio_set_t  gpio_info[1];

			memcpy(gpio_info, &(lcdp->lcd_cfg.lcd_gpio[i]), sizeof(disp_gpio_set_t));
			lcdp->lcd_cfg.gpio_hdl[i] = disp_sys_gpio_request(gpio_info, 1);
		}
	}

	return 0;
}

s32 disp_lcd_gpio_exit(struct disp_device* lcd)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);
	int i = 0;

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}

	for(i=0; i<LCD_GPIO_NUM; i++) {
		if(lcdp->lcd_cfg.gpio_hdl[i]) {
			disp_gpio_set_t  gpio_info[1];

			disp_sys_gpio_release(lcdp->lcd_cfg.gpio_hdl[i], 2);

			memcpy(gpio_info, &(lcdp->lcd_cfg.lcd_gpio[i]), sizeof(disp_gpio_set_t));
			gpio_info->mul_sel = 7;
			lcdp->lcd_cfg.gpio_hdl[i] = disp_sys_gpio_request(gpio_info, 1);
			disp_sys_gpio_release(lcdp->lcd_cfg.gpio_hdl[i], 2);
			lcdp->lcd_cfg.gpio_hdl[i] = 0;
		}
	}

	//io-pad
	for(i = LCD_GPIO_REGU_NUM-1; i>=0; i--)
	{
		if(!((!strcmp(lcdp->lcd_cfg.lcd_gpio_regulator[i], "")) || (!strcmp(lcdp->lcd_cfg.lcd_gpio_regulator[i], "none"))))
			disp_sys_power_disable(lcdp->lcd_cfg.lcd_gpio_regulator[i]);
	}

	return 0;
}

//direction: input(0), output(1)
s32 disp_lcd_gpio_set_direction(struct disp_device* lcd, u32 io_index, u32 direction)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);
	char gpio_name[20];

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}

	sprintf(gpio_name, "lcd_gpio_%d", io_index);
	return  disp_sys_gpio_set_direction(lcdp->lcd_cfg.gpio_hdl[io_index], direction, gpio_name);
}

s32 disp_lcd_gpio_get_value(struct disp_device* lcd,__u32 io_index)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);
	char gpio_name[20];

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}

	sprintf(gpio_name, "lcd_gpio_%d", io_index);
	return disp_sys_gpio_get_value(lcdp->lcd_cfg.gpio_hdl[io_index], gpio_name);
}

s32 disp_lcd_gpio_set_value(struct disp_device* lcd, u32 io_index, u32 data)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);
	char gpio_name[20];

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}

	sprintf(gpio_name, "lcd_gpio_%d", io_index);
	return disp_sys_gpio_set_value(lcdp->lcd_cfg.gpio_hdl[io_index], data, gpio_name);
}

static s32 disp_lcd_init(struct disp_device* lcd)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}
	DE_INF("lcd %d\n", lcd->disp);

	lcd_get_sys_config(lcd->disp, &lcdp->lcd_cfg);
	if(disp_lcd_is_used(lcd)) {
		disp_video_timings *timmings;
		disp_panel_para *panel_info;
		lcd_parse_panel_para(lcd->disp, &lcdp->panel_info);
		timmings = &lcd->timings;
		panel_info = &lcdp->panel_info;
		timmings->pixel_clk = panel_info->lcd_dclk_freq * 1000;
		timmings->x_res = panel_info->lcd_x;
		timmings->y_res = panel_info->lcd_y;
		timmings->hor_total_time= panel_info->lcd_ht;
		timmings->hor_sync_time= panel_info->lcd_hspw;
		timmings->hor_back_porch= panel_info->lcd_hbp-panel_info->lcd_hspw;
		timmings->hor_front_porch= panel_info->lcd_ht-panel_info->lcd_hbp - panel_info->lcd_x;
		timmings->ver_total_time= panel_info->lcd_vt;
		timmings->ver_sync_time= panel_info->lcd_vspw;
		timmings->ver_back_porch= panel_info->lcd_vbp-panel_info->lcd_vspw;
		timmings->ver_front_porch= panel_info->lcd_vt-panel_info->lcd_vbp -panel_info->lcd_y;
	}
	disp_lcd_bright_curve_init(lcd);

	if(disp_lcd_is_used(lcd)) {
		__u64 backlight_bright;
		__u64 period_ns, duty_ns;
		if(lcdp->panel_info.lcd_pwm_used) {
			lcdp->pwm_info.channel = lcdp->panel_info.lcd_pwm_ch;
			lcdp->pwm_info.polarity = lcdp->panel_info.lcd_pwm_pol;
			lcdp->pwm_info.dev = disp_sys_pwm_request(lcdp->panel_info.lcd_pwm_ch);

			if(lcdp->panel_info.lcd_pwm_freq != 0) {
				period_ns = 1000*1000*1000 / lcdp->panel_info.lcd_pwm_freq;
			} else {
				DE_WRN("lcd%d.lcd_pwm_freq is ZERO\n", lcd->disp);
				period_ns = 1000*1000*1000 / 1000;  //default 1khz
			}

			backlight_bright = lcdp->lcd_cfg.backlight_bright;

			duty_ns = (backlight_bright * period_ns) / 256;
			//DE_DBG("[PWM]backlight_bright=%d,period_ns=%d,duty_ns=%d\n",(u32)backlight_bright,(u32)period_ns, (u32)duty_ns);
			disp_sys_pwm_set_polarity(lcdp->pwm_info.dev, lcdp->pwm_info.polarity);
			disp_sys_pwm_config(lcdp->pwm_info.dev, duty_ns, period_ns);
			lcdp->pwm_info.duty_ns = duty_ns;
			lcdp->pwm_info.period_ns = period_ns;
		}
		lcd_clk_init(lcd);
	}

	//lcd_panel_parameter_check(lcd->disp, lcd);

	disp_sys_register_irq(lcdp->irq_no,0,disp_lcd_event_proc,(void*)lcd->disp,0,0);
	disp_sys_enable_irq(lcdp->irq_no);
	return 0;
}

static s32 disp_lcd_exit(struct disp_device* lcd)
{
	struct disp_lcd_private_data *lcdp = disp_lcd_get_priv(lcd);

	if((NULL == lcd) || (NULL == lcdp)) {
		DE_WRN("NULL hdl!\n");
		return DIS_FAIL;
	}

	lcd_clk_exit(lcd);

	disp_sys_disable_irq(lcdp->irq_no);
	disp_sys_unregister_irq(lcdp->irq_no, disp_lcd_event_proc,(void*)lcd->disp);

	return 0;
}

s32 disp_init_lcd(disp_bsp_init_para * para)
{
	u32 num_screens;
	u32 disp;
	struct disp_device *lcd;
	struct disp_lcd_private_data *lcdp;

	DE_INF("disp_init_lcd\n");

#if defined(__LINUX_PLAT__)
	spin_lock_init(&lcd_data_lock);
#endif
	num_screens = bsp_disp_feat_get_num_screens();
	lcds = (struct disp_device *)disp_sys_malloc(sizeof(struct disp_device) * num_screens);
	if(NULL == lcds) {
		DE_WRN("malloc memory(%d bytes) fail!\n", sizeof(struct disp_device) * num_screens);
		return DIS_FAIL;
	}
	lcd_private = (struct disp_lcd_private_data *)disp_sys_malloc(sizeof(struct disp_lcd_private_data) * num_screens);
	if(NULL == lcd_private) {
		DE_WRN("malloc memory(%d bytes) fail!\n", sizeof(struct disp_lcd_private_data) * num_screens);
		return DIS_FAIL;
	}

	for(disp=0; disp<num_screens; disp++) {
		lcd = &lcds[disp];
		DE_INF("lcd %d, 0x%x\n", disp, (u32)lcd);
		lcdp = &lcd_private[disp];

		sprintf(lcd->name, "lcd%d", disp);
		lcd->disp = disp;
		lcd->type = DISP_OUTPUT_TYPE_LCD;
		lcdp->irq_no = para->irq_no[DISP_MOD_LCD0 + disp];
		switch(disp) {
			case 0:
				lcdp->clk = DE_LCD_CLK0;
				break;
			case 1:
				lcdp->clk = DE_LCD_CLK1;
				break;
			default:
				lcdp->clk = DE_LCD_CLK0;
		}
		lcdp->clk_parent = DE_LCD_CLK_SRC;
		lcdp->lvds_clk = DE_LVDS_CLK;
#if defined(SUPPORT_DSI)
		lcdp->dsi_clk0 = DE_DSI_CLK0;
		lcdp->dsi_clk1 = DE_DSI_CLK1;
#endif
		DE_INF("lcd %d, reg_base=0x%x, irq_no=%d, reg_base_dsi=0x%x, irq_no_dsi=%d\n",
		    disp, lcdp->reg_base, lcdp->irq_no, lcdp->reg_base_dsi, lcdp->irq_no_dsi);

		lcd->set_manager = disp_device_set_manager;
		lcd->unset_manager = disp_device_unset_manager;
		lcd->get_resolution = disp_device_get_resolution;
		lcd->get_timings = disp_device_get_timings;
		lcd->enable = disp_lcd_enable;
		lcd->disable = disp_lcd_disable;
		lcd->is_enabled = disp_lcd_is_enabled;
		lcd->set_bright = disp_lcd_set_bright;
		lcd->get_bright = disp_lcd_get_bright;
		lcd->set_bright_dimming = disp_lcd_set_bright_dimming;
		lcd->get_panel_info = disp_lcd_get_panel_info;

		lcd->set_panel_func = disp_lcd_set_panel_funs;
		lcd->set_open_func = disp_lcd_set_open_func;
		lcd->set_close_func = disp_lcd_set_close_func;
		lcd->backlight_enable = disp_lcd_backlight_enable;
		lcd->backlight_disable = disp_lcd_backlight_disable;
		lcd->pwm_enable = disp_lcd_pwm_enable;
		lcd->pwm_disable = disp_lcd_pwm_disable;
		lcd->power_enable = disp_lcd_power_enable;
		lcd->power_disable = disp_lcd_power_disable;
		lcd->pin_cfg = disp_lcd_pin_cfg;
		lcd->tcon_enable = disp_lcd_tcon_enable;
		lcd->tcon_disable = disp_lcd_tcon_disable;
		lcd->get_open_flow = disp_lcd_get_open_flow;
		lcd->get_close_flow  = disp_lcd_get_close_flow;
		lcd->gpio_set_value = disp_lcd_gpio_set_value;
		lcd->gpio_set_direction = disp_lcd_gpio_set_direction;

		lcd->init = disp_lcd_init;
		lcd->exit = disp_lcd_exit;

		if(bsp_disp_feat_is_supported_output_types(disp, DISP_OUTPUT_TYPE_LCD)) {
			lcd->init(lcd);
			disp_device_register(lcd);
		}
	}

	return 0;
}

