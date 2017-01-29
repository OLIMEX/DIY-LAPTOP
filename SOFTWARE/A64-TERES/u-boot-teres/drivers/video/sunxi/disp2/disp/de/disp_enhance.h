#ifndef _DISP_ENHANCE_H_
#define _DISP_ENHANCE_H_

#include "disp_private.h"

static s32 disp_enhance_shadow_protect(struct disp_enhance *enhance, bool protect);
s32 disp_init_enhance(disp_bsp_init_para * para);
#if 0
//for inner debug
typedef struct
{
	//basic adjust
	u32         bright;
	u32         contrast;
	u32         saturation;
	u32         hue;
	u32         mode;
	//ehnance
	u32         sharp;	//0-off; 1~3-on.
	u32         auto_contrast;	//0-off; 1~3-on.
	u32					auto_color;	//0-off; 1-on.
	u32         fancycolor_red; //0-Off; 1-2-on.
	u32         fancycolor_green;//0-Off; 1-2-on.
	u32         fancycolor_blue;//0-Off; 1-2-on.
	struct disp_rect   window;
	u32         enable;
}disp_enhance_para;
#endif

s32 disp_enhance_set_para(struct disp_enhance* enhance, disp_enhance_para *para);


#endif

