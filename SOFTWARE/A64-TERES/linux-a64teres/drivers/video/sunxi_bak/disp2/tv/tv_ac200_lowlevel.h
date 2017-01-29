
#ifndef  _DRV_TV_AC200_LOWLEVEL_H_
#define  _DRV_TV_AC200_LOWLEVEL_H_

#include "tv_ac200.h"

s32 aw1683_tve_init(void);
s32 aw1683_tve_plug_status(void);
s32 aw1683_tve_set_mode(u32 mode);
s32 aw1683_tve_open(void);
s32 aw1683_tve_close(void);

#endif