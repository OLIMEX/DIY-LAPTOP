#include "de_smbl.h"

#if defined(SUPPORT_SMBL)
u16 pwrsv_lgc_tab[1408][256];




//u8 spatial_coeff[9]={228,241,228,241,255,241,228,241,228};

u8 smbl_filter_coeff[272];

u8 hist_thres_drc[8];
u8 hist_thres_pwrsv[8];
u8 drc_filter[IEP_LH_PWRSV_NUM];
u32 csc_bypass_coeff[12];
#endif

