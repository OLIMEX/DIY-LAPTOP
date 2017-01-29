#ifndef _DISP_FEATURES_H_
#define _DISP_FEATURES_H_

//#include "include.h"

#if defined(CONFIG_ARCH_SUN8IW6)
#include "lowlevel_sun8iw6/de_feat.h"
#elif defined(CONFIG_ARCH_SUN8IW7)
#include "lowlevel_sun8iw7/de_feat.h"
#elif defined(CONFIG_ARCH_SUN8IW8)
#include "lowlevel_sun8iw8/de_feat.h"
#elif defined(CONFIG_ARCH_SUN8IW9)
#include "lowlevel_sun8iw9/de_feat.h"
#elif defined(CONFIG_ARCH_SUN50I)
#include "./lowlevel_sun50iw1/de_feat.h"
//#include "./lowlevel_sun50iw1/disp_al.h"
#else
//#error "undefined platform!!!"
#endif

#define DISP_DEVICE_NUM DEVICE_NUM

struct disp_features {
	const int num_screens;
	const int *num_channels;
	const int *num_layers;
	const int *is_support_capture;
	const int *supported_output_types;
};

int bsp_disp_feat_get_num_screens(void);
int bsp_disp_feat_get_num_channels(unsigned int disp);
int bsp_disp_feat_get_num_layers(unsigned int screen_id);
int bsp_disp_feat_get_num_layers_by_chn(unsigned int disp, unsigned int chn);
int bsp_disp_feat_is_supported_output_types(unsigned int screen_id, unsigned int output_type);
int bsp_disp_feat_is_support_capture(unsigned int disp);
int disp_feat_is_support_smbl(unsigned int disp);
int disp_init_feat(void);

#endif

