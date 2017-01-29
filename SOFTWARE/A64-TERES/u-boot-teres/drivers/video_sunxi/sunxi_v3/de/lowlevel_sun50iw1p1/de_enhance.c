//*********************************************************************************************************************
//  All Winner Tech, All Right Reserved. 2014-2015 Copyright (c)
//
//  File name   :	de_enhance.c
//
//  Description :	display engine 2.0 enhance basic function definition
//
//  History     :	2014/04/29  vito cheng  v0.1  Initial version
//
//*********************************************************************************************************************
#include "de_rtmx.h"
#include "de_enhance.h"
#include "de_feat.h"

#define ONE_SCREEN_ONE_PARA	//only ONE parameters for one screen, and all VEPs in this screen use SAME parameters
//DEBUG
#define ASE_EXIST
#define LTI_EXIST
#define FCC_EXIST
#define BWS_EXIST

static int device_num;
static int vep_num[DEVICE_NUM];
static unsigned int frame_cnt[DEVICE_NUM] = {0};

//global histogram, use for bws and ce
unsigned int *g_hist[DEVICE_NUM][CHN_NUM], *g_hist_p[DEVICE_NUM][CHN_NUM];
unsigned int g_sum[DEVICE_NUM][CHN_NUM];

extern __ce_status_t *g_ce_status[DEVICE_NUM][CHN_NUM];
extern __bws_status_t *g_bws_status[DEVICE_NUM][CHN_NUM];

int de_enhance_info2data(struct disp_enhance_config *config, vep_config_data *data)
{
	de_rect tmp_win;
	struct disp_enhance_config tmp_config;

	memcpy(&tmp_config.info, config, sizeof(struct disp_enhance_config));
	memset(data, 0, sizeof(vep_config_data));

	tmp_win.x = config->info.window.x;
	tmp_win.y = config->info.window.y;
	tmp_win.w = config->info.window.width;
	tmp_win.h = config->info.window.height;

	tmp_config.info.sharp = (tmp_config.info.enable==0)?0:tmp_config.info.sharp;
	tmp_config.info.auto_color = (tmp_config.info.enable==0)?0:tmp_config.info.auto_color;
	tmp_config.info.auto_contrast = (tmp_config.info.enable==0)?0:tmp_config.info.auto_contrast;
	tmp_config.info.fancycolor_red = (tmp_config.info.enable==0)?0:tmp_config.info.fancycolor_red;
	tmp_config.info.fancycolor_green = (tmp_config.info.enable==0)?0:tmp_config.info.fancycolor_green;
	tmp_config.info.fancycolor_blue = (tmp_config.info.enable==0)?0:tmp_config.info.fancycolor_blue;

	//fce
	de_fce_info2para(tmp_config.info.sharp, tmp_config.info.auto_contrast, tmp_config.info.auto_color, tmp_win, &data->fce_para);
#ifdef BWS_EXIST
	//bws
	de_bws_info2para(tmp_config.info.auto_contrast, tmp_win, &data->bws_para);
#endif
	//FIXME
	//peak
	de_peak_info2para(tmp_config.info.sharp, tmp_win, &data->peak_para);
#ifdef LTI_EXIST
	//lti
	de_lti_info2para(tmp_config.info.sharp, tmp_win, &data->lti_para);
#endif
#ifdef ASE_EXIST
	//ase
	de_ase_info2para(tmp_config.info.auto_color, tmp_win, &data->ase_para);
#endif
#ifdef FCC_EXIST
	//fcc
	de_fcc_info2para(tmp_config.info.fancycolor_red, tmp_config.info.fancycolor_green, tmp_config.info.fancycolor_blue,
	0,0,0,tmp_win, &data->fcc_para);
#endif

	return 0;
}

int de_enhance_apply(unsigned int screen_id, struct disp_enhance_config *config)
{
	int ch_id,chno;
	vep_config_data data;
	int auto_contrast_dirty;

	chno = vep_num[screen_id];

	__inf("disp %d, en=%d, sharp=%d\n", screen_id, config[0].info.enable, config[0].info.sharp);
	for(ch_id=0; ch_id<chno; ch_id++)
	{
#ifdef ONE_SCREEN_ONE_PARA
		auto_contrast_dirty = (config[0].flags & ENHANCE_ENABLE_DIRTY)?1:0;

		//disp_enhance_info -> vep_config_data
		de_enhance_info2data(&config[0], &data);
		//FIXME: Update according to dirty flag
		//fce
		de_hist_apply(screen_id, ch_id, data.fce_para.hist_en, auto_contrast_dirty);
		de_ce_apply(screen_id, ch_id, data.fce_para.ce_en, auto_contrast_dirty);
		de_fce_enable(screen_id, ch_id, data.fce_para.fce_en);
		de_fce_set_size(screen_id, ch_id, config[0].info.size.width, config[0].info.size.height);
		de_fce_set_para(screen_id, ch_id, data.fce_para.lce_en, data.fce_para.ftc_en, data.fce_para.ce_en, data.fce_para.hist_en);
		de_fce_set_window(screen_id, ch_id, data.fce_para.win_en, data.fce_para.win);
		g_ce_status[screen_id][ch_id]->width = config[0].info.size.width;
		g_ce_status[screen_id][ch_id]->height = config[0].info.size.height;
#ifdef BWS_EXIST
		//bws
		de_bws_apply(screen_id, ch_id, data.bws_para.bws_en, auto_contrast_dirty);
		de_bws_enable(screen_id, ch_id, data.bws_para.bws_en);
		de_bws_set_size(screen_id, ch_id, config[0].info.size.width, config[0].info.size.height);
		de_bws_set_window(screen_id, ch_id, data.bws_para.win_en, data.bws_para.win);
		g_bws_status[screen_id][ch_id]->width = config[0].info.size.width;
		g_bws_status[screen_id][ch_id]->height = config[0].info.size.height;
#endif
		//peak
		de_peak_enable(screen_id, ch_id, data.peak_para.peak_en);
		de_peak_set_size(screen_id, ch_id, config[0].info.size.width, config[0].info.size.height);
		de_peak_set_para(screen_id, ch_id, data.peak_para.gain);
		de_peak_set_window(screen_id, ch_id, data.peak_para.win_en, data.peak_para.win);
#ifdef LTI_EXIST
		//lti
		de_lti_enable(screen_id, ch_id, data.lti_para.lti_en);
		de_lti_set_size(screen_id, ch_id, config[0].info.size.width, config[0].info.size.height);
		de_lti_set_para(screen_id, ch_id, data.lti_para.lti_en);
		de_lti_set_window(screen_id, ch_id, data.lti_para.win_en, data.lti_para.win);
#endif
#ifdef ASE_EXIST
		//ase
		de_ase_enable(screen_id, ch_id, data.ase_para.ase_en);
		de_ase_set_size(screen_id, ch_id, config[0].info.size.width, config[0].info.size.height);
		de_ase_set_para(screen_id, ch_id, data.ase_para.gain);
		de_ase_set_window(screen_id, ch_id, data.ase_para.win_en, data.ase_para.win);
#endif
#ifdef FCC_EXIST
		//fcc
		de_fcc_enable(screen_id, ch_id, data.fcc_para.fcc_en);
		de_fcc_set_size(screen_id, ch_id, config[0].info.size.width, config[0].info.size.height);
		de_fcc_set_para(screen_id, ch_id, data.fcc_para.sgain);
		de_fcc_set_window(screen_id, ch_id, data.fcc_para.win_en, data.fcc_para.win);
#endif
#else
		auto_contrast_dirty = (config[ch_id].flags & ENHANCE_ENABLE_DIRTY)?1:0;
		//disp_enhance_info -> vep_config_data
		de_enhance_info2data(&config[ch_id], &data);
		//fce
		de_hist_apply(screen_id, ch_id, data.fce_para.hist_en, auto_contrast_dirty);
		de_ce_apply(screen_id, ch_id, data.fce_para.ce_en, auto_contrast_dirty);
		de_fce_enable(screen_id, ch_id, data.fce_para.fce_en);
		de_fce_set_size(screen_id, ch_id, config[ch_id].info.size.width, config[ch_id].info.size.height);
		de_fce_set_para(screen_id, ch_id, data.fce_para.lce_en, data.fce_para.ftc_en, data.fce_para.ce_en, data.fce_para.hist_en);
		de_fce_set_window(screen_id, ch_id, data.fce_para.win_en, data.fce_para.win);
		g_ce_status[screen_id][ch_id]->width = config[ch_id].info.size.width;
		g_ce_status[screen_id][ch_id]->height = config[ch_id].info.size.height;
		//bws
		de_bws_apply(screen_id, ch_id, data.bws_para.bws_en, auto_contrast_dirty);
		de_bws_enable(screen_id, ch_id, data.bws_para.bws_en);
		de_bws_set_size(screen_id, ch_id, config[ch_id].info.size.width, config[ch_id].info.size.height);
		de_bws_set_window(screen_id, ch_id, data.bws_para.win_en, data.bws_para.win);
		g_bws_status[screen_id][ch_id]->width = config[ch_id].info.size.width;
		g_bws_status[screen_id][ch_id]->height = config[ch_id].info.size.height;
		//peak
		de_peak_enable(screen_id, ch_id, data.peak_para.peak_en);
		de_peak_set_size(screen_id, ch_id, config[ch_id].info.size.width, config[ch_id].info.size.height);
		de_peak_set_para(screen_id, ch_id, data.peak_para.gain);
		de_peak_set_window(screen_id, ch_id, data.peak_para.win_en, data.peak_para.win);
		//lti
		de_lti_enable(screen_id, ch_id, data.lti_para.gain);
		de_lti_set_size(screen_id, ch_id, config[ch_id].info.size.width, config[ch_id].info.size.height);
		de_lti_set_para(screen_id, ch_id, data.lti_para.lti_en);
		de_lti_set_window(screen_id, ch_id, data.lti_para.win_en, data.lti_para.win);
		//ase
		de_ase_enable(screen_id, ch_id, data.ase_para.gain);
		de_ase_set_size(screen_id, ch_id, config[ch_id].info.size.width, config[ch_id].info.size.height);
		de_ase_set_para(screen_id, ch_id, data.ase_para.gain);
		de_ase_set_window(screen_id, ch_id, data.ase_para.win_en, data.ase_para.win);
		//fcc
		de_fcc_enable(screen_id, ch_id, data.fcc_para.fcc_en);
		de_fcc_set_size(screen_id, ch_id, config[ch_id].info.size.width, config[ch_id].info.size.height);
		de_fcc_set_para(screen_id, ch_id, data.fcc_para.sgain);
		de_fcc_set_window(screen_id, ch_id, data.fcc_para.win_en, data.fcc_para.win);

#endif
	}

	return 0;
}

int de_enhance_sync(unsigned int screen_id)
{
	int ch_id,chno;

	chno = vep_num[screen_id];

	for(ch_id=0; ch_id<chno; ch_id++)
	{
		//hist
		de_hist_sync(screen_id, ch_id, frame_cnt[screen_id]);

		//ce
		de_ce_sync(screen_id, ch_id, frame_cnt[screen_id]);
#ifdef BWS_EXIST
		//bws
		de_bws_sync(screen_id, ch_id, frame_cnt[screen_id]);
#endif
		frame_cnt[screen_id]++;
	}
	return 0;
}

int de_enhance_update_regs(unsigned int screen_id)
{
	int chno, ch_id;
	static u32 count = 0;
	if(count < 10) {
		count ++;
		__inf("disp %d\n", screen_id);
	}

	chno = vep_num[screen_id];
	for(ch_id=0; ch_id<chno; ch_id++)
	{
		de_fce_update_regs(screen_id, ch_id);
#ifdef BWS_EXIST
		de_bws_update_regs(screen_id, ch_id);
#endif
#ifdef LTI_EXIST
		de_lti_update_regs(screen_id, ch_id);
#endif
		de_peak_update_regs(screen_id, ch_id);
#ifdef ASE_EXIST
		de_ase_update_regs(screen_id, ch_id);
#endif
#ifdef FCC_EXIST
		de_fcc_update_regs(screen_id, ch_id);
#endif
	}
	return 0;
}

int de_enhance_init(disp_bsp_init_para *para)
{
	int screen_id, ch_id;

	device_num = de_feat_get_num_devices();

	for(screen_id=0; screen_id<device_num; screen_id++)
		vep_num[screen_id] = de_feat_is_support_vep(screen_id);

	for(screen_id=0; screen_id<device_num; screen_id++)
		for(ch_id=0; ch_id<vep_num[screen_id]; ch_id++)
		{
			de_fce_init(screen_id, ch_id, para->reg_base[DISP_MOD_DE]);
#ifdef BWS_EXIST
			de_bws_init(screen_id, ch_id, para->reg_base[DISP_MOD_DE]);
#endif
#ifdef LTI_EXIST
			de_lti_init(screen_id, ch_id, para->reg_base[DISP_MOD_DE]);
#endif
			de_peak_init(screen_id, ch_id, para->reg_base[DISP_MOD_DE]);
#ifdef ASE_EXIST
			de_ase_init(screen_id, ch_id, para->reg_base[DISP_MOD_DE]);
#endif
#ifdef FCC_EXIST
			de_fcc_init(screen_id, ch_id, para->reg_base[DISP_MOD_DE]);
#endif
		}

	//initial
	for(screen_id=0; screen_id<device_num; screen_id++)
		frame_cnt[screen_id] = 0;

	return 0;
}

