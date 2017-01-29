#include "disp_al.h"
#include "de_hal.h"

struct disp_al_private_data
{
	u32 output_type[DEVICE_NUM];
	u32 output_mode[DEVICE_NUM];//indicate mode for tv/hdmi, lcd_if for lcd
	u32 output_cs[DEVICE_NUM];//index according to device
};

static struct disp_al_private_data al_priv;

int disp_al_layer_apply(unsigned int disp, struct disp_layer_config_data *data, unsigned int layer_num)
{
	return de_al_lyr_apply(disp, data, layer_num);
}

int disp_al_manager_init(unsigned int disp)
{
	return de_clk_enable(DE_CLK_CORE0 + disp);
}

int disp_al_manager_exit(unsigned int disp)
{
	return de_clk_disable(DE_CLK_CORE0 + disp);
}

int disp_al_manager_apply(unsigned int disp, struct disp_manager_data *data)
{
	if(data->flag & MANAGER_ENABLE_DIRTY)
		al_priv.output_cs[data->config.disp_device] = data->config.cs;

	return de_al_mgr_apply(disp, data);
}

int disp_al_manager_sync(unsigned int disp)
{
	return de_al_mgr_sync(disp);
}

int disp_al_manager_update_regs(unsigned int disp)
{
	return de_al_mgr_update_regs(disp);
}

int disp_al_manager_query_irq(unsigned int disp)
{
	return de_al_query_irq(disp);
}

int disp_al_manager_enable_irq(unsigned int disp)
{
	return de_al_enable_irq(disp, 1);
}

int disp_al_manager_disable_irq(unsigned int disp)
{
	return de_al_enable_irq(disp, 0);
}

int disp_al_enhance_apply(unsigned int disp, struct disp_enhance_config *config)
{
	return de_enhance_apply(disp, config);
}

int disp_al_enhance_update_regs(unsigned int disp)
{
	return de_enhance_update_regs(disp);
}

int disp_al_enhance_sync(unsigned int disp)
{
	return de_enhance_sync(disp);
}

int disp_al_enhance_tasklet(unsigned int disp)
{
	return 0;
}

int disp_al_capture_init(unsigned int disp)
{
	return de_clk_enable(DE_CLK_WB);
}

int disp_al_capture_exit(unsigned int disp)
{
	return de_clk_disable(DE_CLK_WB);
}

int disp_al_capture_sync(u32 disp)
{
	WB_EBIOS_Update_Regs(disp);
	WB_EBIOS_Writeback_Enable(disp, 1);
	return 0;
}

int disp_al_capture_apply(unsigned int disp, struct disp_capture_config *cfg)
{
	return WB_EBIOS_Apply(disp, cfg);
}

int disp_al_capture_get_status(unsigned int disp)
{
	return WB_EBIOS_Get_Status(disp);
}

int disp_al_smbl_apply(unsigned int disp, struct disp_smbl_info *info)
{
	return de_smbl_apply(disp, info);
}

int disp_al_smbl_update_regs(unsigned int disp)
{
	return de_smbl_update_regs(disp);
}

int disp_al_smbl_sync(unsigned int disp)
{
	return 0;
}

int disp_al_smbl_tasklet(unsigned int disp)
{
	return 0;
}

int disp_al_smbl_get_status(unsigned int disp)
{
	return de_smbl_get_status(disp);
}

static struct lcd_clk_info clk_tbl[] = {
	{LCD_IF_HV,     6, 1, 1},
	{LCD_IF_CPU,   12, 1, 1},
	{LCD_IF_LVDS,   7, 1, 1},
	{LCD_IF_DSI,    4, 1, 4},
};
/* lcd */
/* lcd_dclk_freq * div -> lcd_clk_freq * div2 -> pll_freq */
/* lcd_dclk_freq * dsi_div -> lcd_dsi_freq */
int disp_al_lcd_get_clk_info(u32 screen_id, struct lcd_clk_info *info, disp_panel_para * panel)
{
	int tcon_div = 6;//tcon inner div
	int lcd_div = 1;//lcd clk div
	int dsi_div = 4;//dsi clk div
	int i;
	int find = 0;

	if(NULL == panel) {
		__wrn("panel is NULL\n");
		return 0;
	}

	for(i=0; i<sizeof(clk_tbl)/sizeof(struct lcd_clk_info); i++) {
		if(clk_tbl[i].lcd_if == panel->lcd_if) {
			tcon_div = clk_tbl[i].tcon_div;
			lcd_div = clk_tbl[i].lcd_div;
			dsi_div = clk_tbl[i].dsi_div;
			find = 1;
			break;
		}
	}

	if(0 == find)
		__wrn("cant find clk info for lcd_if %d\n", panel->lcd_if);

	info->tcon_div = tcon_div;
	info->lcd_div = lcd_div;
	info->dsi_div = dsi_div;

	return 0;
}

int disp_al_lcd_cfg(u32 screen_id, disp_panel_para * panel, panel_extend_para *extend_panel)
{
	struct lcd_clk_info info;

	al_priv.output_type[screen_id] = (u32)DISP_OUTPUT_TYPE_LCD;
	al_priv.output_mode[screen_id] = (u32)panel->lcd_if;

	tcon_init(screen_id);
	disp_al_lcd_get_clk_info(screen_id, &info, panel);
	tcon0_set_dclk_div(screen_id, info.tcon_div);

	if(0 != tcon0_cfg(screen_id, panel))
		DE_WRN("lcd cfg fail!\n");
	else
		DE_INF("lcd cfg ok!\n");

	tcon0_cfg_ext(screen_id, extend_panel);

	if(LCD_IF_DSI == panel->lcd_if)	{
#if defined(SUPPORT_DSI)
		if(0 != dsi_cfg(screen_id, panel)) {
			DE_WRN("dsi cfg fail!\n");
		}
#endif
	}

	return 0;
}

int disp_al_lcd_enable(u32 screen_id, disp_panel_para * panel)
{
	tcon0_open(screen_id, panel);
	if(LCD_IF_LVDS == panel->lcd_if) {
		lvds_open(screen_id, panel);
	} else if(LCD_IF_DSI == panel->lcd_if) {
#if defined(SUPPORT_DSI)
		dsi_open(screen_id, panel);
#endif
	}

	return 0;
}

int disp_al_lcd_disable(u32 screen_id, disp_panel_para * panel)
{
	if(LCD_IF_LVDS == panel->lcd_if) {
		lvds_close(screen_id);
	} else if(LCD_IF_DSI == panel->lcd_if) {
#if defined(SUPPORT_DSI)
		dsi_close(screen_id);
#endif
	}
	tcon0_close(screen_id);
	tcon_exit(screen_id);

	return 0;
}

/* query lcd irq, clear it when the irq queried exist
 */
int disp_al_lcd_query_irq(u32 screen_id, __lcd_irq_id_t irq_id, disp_panel_para * panel)
{
	int ret = 0;
	ret = tcon_irq_query(screen_id, irq_id);

	return ret;
}

int disp_al_lcd_enable_irq(u32 screen_id, __lcd_irq_id_t irq_id, disp_panel_para * panel)
{
	int ret = 0;
	ret = tcon_irq_enable(screen_id, irq_id);

	return ret;
}

int disp_al_lcd_disable_irq(u32 screen_id, __lcd_irq_id_t irq_id, disp_panel_para * panel)
{
	int ret = 0;
	ret = tcon_irq_disable(screen_id, irq_id);

	return ret;
}

int disp_al_lcd_tri_busy(u32 screen_id, disp_panel_para * panel)
{
	int busy = 0;
	int ret = 0;

	busy |= tcon0_tri_busy(screen_id);
#if defined(SUPPORT_DSI)
	busy |= dsi_inst_busy(screen_id);
#endif
	ret = (busy == 0)? 0:1;

	return (ret);
}
/* take dsi irq s32o account, todo? */
int disp_al_lcd_tri_start(u32 screen_id, disp_panel_para * panel)
{
#if defined(SUPPORT_DSI)
	if(LCD_IF_DSI == panel->lcd_if)
		dsi_tri_start(screen_id);
#endif
	return tcon0_tri_start(screen_id);
}

int disp_al_lcd_io_cfg(u32 screen_id, u32 enable, disp_panel_para * panel)
{
#if defined(SUPPORT_DSI)
	if(LCD_IF_DSI ==  panel->lcd_if) {
		if(enable) {
			dsi_io_open(screen_id, panel);
		} else {
			dsi_io_close(screen_id);
		}
	}
#endif

	return 0;
}

int disp_al_lcd_get_cur_line(u32 screen_id, disp_panel_para * panel)
{
	return tcon_get_cur_line(screen_id, 0);
}

int disp_al_lcd_get_start_delay(u32 screen_id, disp_panel_para * panel)
{

	return tcon_get_start_delay(screen_id, 0);
}

/* hdmi */
int disp_al_hdmi_enable(u32 screen_id)
{
	tcon1_open(screen_id);
	return 0;
}

int disp_al_hdmi_disable(u32 screen_id)
{
	tcon1_close(screen_id);
	tcon_exit(screen_id);

	return 0;
}

int disp_al_hdmi_cfg(u32 screen_id, disp_video_timings *video_info)
{
	al_priv.output_type[screen_id] = (u32)DISP_OUTPUT_TYPE_HDMI;
	al_priv.output_mode[screen_id] = (u32)video_info->vic;

	tcon_init(screen_id);
	tcon1_set_timming(screen_id, video_info);
	if(al_priv.output_cs[screen_id] != 0)//YUV output
		tcon1_hdmi_color_remap(screen_id,1);
	else
		tcon1_hdmi_color_remap(screen_id,0);

	return 0;
}

int disp_al_vdevice_cfg(u32 screen_id, disp_video_timings *video_info, disp_vdevice_interface_para *para)
{
	struct lcd_clk_info clk_info;
	disp_panel_para info;

	al_priv.output_type[screen_id] = (u32)DISP_OUTPUT_TYPE_LCD;
	al_priv.output_mode[screen_id] = (u32)para->intf;

	memset(&info, 0, sizeof(disp_panel_para));
	info.lcd_if = para->intf;
	info.lcd_x = video_info->x_res;
	info.lcd_y = video_info->y_res;
	info.lcd_hv_if = (disp_lcd_hv_if)para->sub_intf;
	info.lcd_dclk_freq = video_info->pixel_clk;
	info.lcd_ht = video_info->hor_total_time;
	info.lcd_hbp = video_info->hor_back_porch + video_info->hor_sync_time;
	info.lcd_hspw = video_info->hor_sync_time;
	info.lcd_vt = video_info->ver_total_time;
	info.lcd_vbp = video_info->ver_back_porch + video_info->ver_sync_time;
	info.lcd_vspw = video_info->ver_sync_time;
	info.lcd_hv_syuv_fdly = para->fdelay;
	if(LCD_HV_IF_CCIR656_2CYC == info.lcd_hv_if)
		info.lcd_hv_syuv_seq = para->sequence;
	else
		info.lcd_hv_srgb_seq = para->sequence;
	tcon_init(screen_id);
	disp_al_lcd_get_clk_info(screen_id, &clk_info, &info);
	clk_info.tcon_div = 11;//fixme
	tcon0_set_dclk_div(screen_id, clk_info.tcon_div);

	if(0 != tcon0_cfg(screen_id, &info))
		DE_WRN("lcd cfg fail!\n");
	else
		DE_INF("lcd cfg ok!\n");

	return 0;
}

int disp_al_vdevice_enable(u32 screen_id)
{
	disp_panel_para panel;

	memset(&panel, 0, sizeof(disp_panel_para));
	panel.lcd_if = LCD_IF_HV;
	tcon0_open(screen_id, &panel);

	return 0;
}

int disp_al_vdevice_disable(u32 screen_id)
{
	tcon0_close(screen_id);
	tcon_exit(screen_id);

	return 0;
}

/* screen_id: used for index of manager */
int disp_al_device_get_cur_line(u32 screen_id)
{
	u32 tcon_index = 0;

	tcon_index = (al_priv.output_type[screen_id] == (u32)DISP_OUTPUT_TYPE_LCD)?0:1;
	return tcon_get_cur_line(screen_id, tcon_index);
}

int disp_al_device_get_start_delay(u32 screen_id)
{
	u32 tcon_index = 0;

	tcon_index = (al_priv.output_type[screen_id] == (u32)DISP_OUTPUT_TYPE_LCD)?0:1;
	return tcon_get_start_delay(screen_id, tcon_index);
}

int disp_al_device_query_irq(u32 screen_id)
{
	int ret = 0;
	int irq_id = 0;

	irq_id = (al_priv.output_type[screen_id] == (u32)DISP_OUTPUT_TYPE_LCD)?\
		LCD_IRQ_TCON0_VBLK:LCD_IRQ_TCON1_VBLK;
	ret = tcon_irq_query(screen_id, irq_id);

	return ret;
}

int disp_al_device_enable_irq(u32 screen_id)
{
	int ret = 0;
	int irq_id = 0;

	irq_id = (al_priv.output_type[screen_id] == (u32)DISP_OUTPUT_TYPE_LCD)?\
		LCD_IRQ_TCON0_VBLK:LCD_IRQ_TCON1_VBLK;
	ret = tcon_irq_enable(screen_id, irq_id);

	return ret;
}

int disp_al_device_disable_irq(u32 screen_id)
{
	int ret = 0;
	int irq_id = 0;

	irq_id = (al_priv.output_type[screen_id] == (u32)DISP_OUTPUT_TYPE_LCD)?\
		LCD_IRQ_TCON0_VBLK:LCD_IRQ_TCON1_VBLK;
	ret = tcon_irq_disable(screen_id, irq_id);

	return ret;
}

int disp_init_al(disp_bsp_init_para * para)
{
	int i;

	memset(&al_priv, 0, sizeof(struct disp_al_private_data));
	de_al_init(para);
	de_enhance_init(para);
	de_ccsc_init(para);
	de_dcsc_init(para);
	WB_EBIOS_Init(para);
	de_clk_set_reg_base(para->reg_base[DISP_MOD_DE]);

	for(i=0; i<DEVICE_NUM; i++) {
		tcon_set_reg_base(i, para->reg_base[DISP_MOD_LCD0]);//calc lcd1 base
		de_smbl_init(i, para->reg_base[DISP_MOD_DE]);
		dsi_set_reg_base(i, para->reg_base[DISP_MOD_DSI0]);
	}

	return 0;
}

