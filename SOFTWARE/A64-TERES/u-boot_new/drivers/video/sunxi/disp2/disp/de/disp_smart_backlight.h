#ifndef _DISP_SMBL_H_
#define _DISP_SMBL_H_

#include "disp_private.h"

struct disp_smbl* disp_get_smbl(u32 disp);
s32 disp_smbl_shadow_protect(struct disp_smbl *smbl, bool protect);
s32 disp_init_smbl(disp_bsp_init_para * para);

#endif

