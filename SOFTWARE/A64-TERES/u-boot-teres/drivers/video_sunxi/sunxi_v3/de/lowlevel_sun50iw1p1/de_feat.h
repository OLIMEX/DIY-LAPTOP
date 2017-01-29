#ifndef _DE_FEAT_H_
#define _DE_FEAT_H_

#define DEVICE_NUM	2
#define CHN_NUM		4
#define VI_CHN_NUM	1
#define UI_CHN_NUM	(CHN_NUM - VI_CHN_NUM)
#define LAYER_NUM_PER_CHN_PER_VI_CHN	4
#define LAYER_NUM_PER_CHN_PER_UI_CHN	4
#define LAYER_MAX_NUM_PER_CHN 4
#define VEP_NUM  1

#define SUPPORT_DSI
//#define SUPPORT_SMBL
#define SUPPORT_HDMI

/* clk */
#define DE_LCD_CLK0 "lcd0"
#define DE_LCD_CLK1 "lcd1"
#define DE_LVDS_CLK "lvds"
#define DE_DSI_CLK0 "mipi_dsi0"
#define DE_DSI_CLK1 "mipi_dsi1"
#define DE_LCD_CLK_SRC "pll_video0"
#define DE_HDMI_CLK_SRC "pll_video1"

#define DE_CLK_SRC "pll_de"
#define DE_CORE_CLK "de"
#define DE_CORE_CLK_RATE 500000000

struct de_feat {
	const int num_devices;
	const int *num_chns;
	const int *num_vi_chns;
	const int *num_layers;
	const int *is_support_vep;
	const int *is_support_smbl;
	const int *is_support_wb;
	const int *supported_output_types;
	const int *is_support_scale;
};

int de_feat_init(void);
int de_feat_get_num_devices(void);
int de_feat_get_num_chns(unsigned int disp);
int de_feat_get_num_vi_chns(unsigned int disp);
int de_feat_get_num_ui_chns(unsigned int disp);
int de_feat_get_num_layers(unsigned int disp);
int de_feat_get_num_layers_by_chn(unsigned int disp, unsigned int chn);
int de_feat_is_support_vep(unsigned int disp);
int de_feat_is_support_vep_by_chn(unsigned int disp, unsigned int chn);
int de_feat_is_support_smbl(unsigned int disp);
int de_feat_is_supported_output_types(unsigned int  disp, unsigned int output_type);
int de_feat_is_support_wb(unsigned int disp);
int de_feat_is_support_scale(unsigned int disp);
int de_feat_is_support_scale_by_chn(unsigned int disp, unsigned int chn);

#endif

