//*********************************************************************************************************************
//  All Winner Tech, All Right Reserved. 2014-2015 Copyright (c)
//
//  File name   :	de_enhance.h
//
//  Description :	display engine 2.0 enhance basic function declaration
//
//  History     :	2014/04/02  vito cheng  v0.1  Initial version
//					2014/04/29  vito cheng  v0.2  Add disp_enhance_config_data struct delcaration
//												  Add updata_regs and init for every module
//
//*********************************************************************************************************************
#ifndef __DE_ENHANCE_H__
#define __DE_ENHANCE_H__

#include "../disp_private.h"
#include "de_fce_type.h"
#include "de_bws_type.h"
#include "de_peak_type.h"
#include "de_lti_type.h"
#include "de_ase_type.h"
#include "de_fcc_type.h"

typedef struct
{
	__fce_config_data fce_para;
	__bws_config_data bws_para;
	__lti_config_data lti_para;
	__peak_config_data peak_para;
	__ase_config_data ase_para;
	__fcc_config_data fcc_para;

}vep_config_data;

extern __hist_status_t *g_hist_status[DEVICE_NUM][CHN_NUM];
extern __ce_status_t *g_ce_status[DEVICE_NUM][CHN_NUM];
extern __bws_status_t *g_bws_status[DEVICE_NUM][CHN_NUM];
extern unsigned int *g_hist[DEVICE_NUM][CHN_NUM];
extern unsigned int*g_hist_p[DEVICE_NUM][CHN_NUM];
extern unsigned int g_sum[DEVICE_NUM][CHN_NUM];

//peak function declaration
int de_peak_set_reg_base(unsigned int sel, unsigned int chno, unsigned int base);
int de_peak_update_regs(unsigned int sel, unsigned int chno);
int de_peak_init(unsigned int sel, unsigned int chno, unsigned int reg_base);
int de_peak_enable(unsigned int sel, unsigned int chno, unsigned int en);
int de_peak_set_size(unsigned int sel, unsigned int chno, unsigned int width, unsigned int height);
int de_peak_set_window(unsigned int sel, unsigned int chno, unsigned int win_enable, de_rect window);
int de_peak_set_para(unsigned int sel, unsigned int chno, unsigned int gain);
int de_peak_info2para(unsigned int sharp, de_rect window, __peak_config_data *para);

//LTI function declaration
int de_lti_set_reg_base(unsigned int sel, unsigned int chno, unsigned int base);
int de_lti_update_regs(unsigned int sel, unsigned int chno);
int de_lti_init(unsigned int sel, unsigned int chno, unsigned int reg_base);
int de_lti_enable(unsigned int sel, unsigned int chno, unsigned int en);
int de_lti_set_size(unsigned int sel, unsigned int chno, unsigned int width, unsigned int height);
int de_lti_set_window(unsigned int sel, unsigned int chno, unsigned int win_enable, de_rect window);
int de_lti_set_para(unsigned int sel, unsigned int chno, unsigned int gain);
int de_lti_info2para(unsigned int gain, de_rect window, __lti_config_data *para);

//BWS function declaration
int de_bws_set_reg_base(unsigned int sel, unsigned int chno, unsigned int base);
int de_bws_update_regs(unsigned int sel, unsigned int chno);
int de_bws_init(unsigned int sel, unsigned int chno, unsigned int reg_base);
int de_bws_enable(unsigned int sel, unsigned int chno, unsigned int en);
int de_bws_set_size(unsigned int sel, unsigned int chno, unsigned int width, unsigned int height);
int de_bws_set_window(unsigned int sel, unsigned int chno, unsigned int win_enable, de_rect window);
int de_bws_info2para(unsigned int gain, de_rect window, __bws_config_data *para);
int de_bws_set_para(unsigned int sel, unsigned int chno,
						unsigned int min, unsigned int black, unsigned int white, unsigned int max,
						unsigned int slope0, unsigned int slope1, unsigned int slope2, unsigned int slope3);
int de_bws_sync(unsigned int screen_id, unsigned int chno, unsigned int frame_cnt);
int de_bws_apply(unsigned int screen_id, unsigned int chno, unsigned int bws_en, unsigned int auto_contrast_dirty);

//FCE function declaration
int de_fce_set_reg_base(unsigned int sel, unsigned int chno, unsigned int base);
int de_fce_update_regs(unsigned int sel, unsigned int chno);
int de_fce_init(unsigned int sel, unsigned int chno, unsigned int reg_base);
int de_fce_enable(unsigned int sel, unsigned int chno, unsigned int en);
int de_fce_set_size(unsigned int sel, unsigned int chno, unsigned int width, unsigned int height);
int de_fce_set_window(unsigned int sel, unsigned int chno, unsigned int win_enable, de_rect window);
int de_fce_set_para(unsigned int sel, unsigned int chno,
						unsigned int lce_en, unsigned int ftc_en, unsigned int ce_en, unsigned int hist_en);
int de_fce_csc_en(unsigned int sel, unsigned int chno, unsigned int csc_enable);
int de_fce_info2para(unsigned int detail, unsigned int auto_contrast, unsigned int auto_color, de_rect window, __fce_config_data *para);
int de_fce_get_hist(unsigned int sel, unsigned int chno, unsigned int hist[256], unsigned int *sum);
int de_fce_set_ce(unsigned int sel, unsigned int chno, unsigned char ce_lut[256]);
int de_fce_info2para(unsigned int sharp, unsigned int auto_contrast, unsigned int auto_color, de_rect window, __fce_config_data *para);
void auto_ce_model(unsigned int width, unsigned height, unsigned int sumcnt, unsigned int hist[256],
						unsigned int up_precent_thr, unsigned int down_precent_thr, unsigned char celut[256]);
int de_hist_apply(unsigned int screen_id, unsigned int chno, unsigned int hist_en, unsigned int auto_contrast_dirty);
int de_hist_sync(unsigned int screen_id, unsigned int chno, unsigned int frame_cnt);
int de_ce_apply(unsigned int screen_id, unsigned int chno, unsigned int ce_en, unsigned int auto_contrast_dirty);
int de_ce_sync(unsigned int screen_id, unsigned int chno, unsigned int frame_cnt);

//ASE function declaration
int de_ase_set_reg_base(unsigned int sel, unsigned int chno, unsigned int base);
int de_ase_update_regs(unsigned int sel, unsigned int chno);
int de_ase_init(unsigned int sel, unsigned int chno, unsigned int reg_base);
int de_ase_enable(unsigned int sel, unsigned int chno, unsigned int en);
int de_ase_set_size(unsigned int sel, unsigned int chno, unsigned int width, unsigned int height);
int de_ase_set_window(unsigned int sel, unsigned int chno, unsigned int win_enable, de_rect window);
int de_ase_set_para(unsigned int sel, unsigned int chno, unsigned int gain);
int de_ase_info2para(unsigned int gain, de_rect window, __ase_config_data *para);

//FCC function declaration
int de_fcc_set_reg_base(unsigned int sel, unsigned int chno, void *base);
int de_fcc_init(unsigned int sel, unsigned int chno, unsigned int reg_base);
int de_fcc_enable(unsigned int sel, unsigned int chno, unsigned int en);
int de_fcc_set_size(unsigned int sel, unsigned int chno, unsigned int width, unsigned int height);
int de_fcc_set_window(unsigned int sel, unsigned int chno, unsigned int win_en, de_rect window);
int de_fcc_set_para(unsigned int sel, unsigned int chno, unsigned int sgain[6]);
int de_fcc_csc_set(unsigned int sel, unsigned int chno, unsigned int en, unsigned int mode);
int de_fcc_info2para(unsigned int sgain0, unsigned int sgain1, unsigned int sgain2, unsigned int sgain3, unsigned int sgain4, unsigned int sgain5, de_rect window, __fcc_config_data *para);
int de_fcc_update_regs(unsigned int sel, unsigned int chno);

int de_enhance_apply(unsigned int screen_id, struct disp_enhance_config *config);
int de_enhance_sync(unsigned int screen_id);
int de_enhance_update_regs(unsigned int screen_id);
int de_enhance_init(disp_bsp_init_para *para);
int de_enhance_info2data(struct disp_enhance_config *config, vep_config_data *data);

int de_hist_apply(unsigned int screen_id, unsigned int chno, unsigned int hist_en, unsigned int auto_contrast_dirty);
int de_hist_sync(unsigned int screen_id, unsigned int chno, unsigned int frame_cnt);

int de_ce_apply(unsigned int screen_id, unsigned int chno, unsigned int ce_en, unsigned int auto_contrast_dirty);
int de_ce_sync(unsigned int screen_id, unsigned int chno, unsigned int frame_cnt);


#endif
