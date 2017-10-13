#ifndef __DE_CSC_H__
#define __DE_CSC_H__

#include "de_rtmx.h"
#include "../disp_private.h"

#ifndef put_wvalue
#define put_wvalue(addr, v)	(*((volatile unsigned long  *)(addr)) = (unsigned long)(v))
#endif
#ifndef get_wvalue
#define get_wvalue(addr)	(*((volatile unsigned long  *)(addr)))
#endif

int de_dcsc_apply(unsigned int sel, struct disp_csc_config *config);
int de_dcsc_init(disp_bsp_init_para *para);
int de_dcsc_update_regs(unsigned int sel);

int de_ccsc_apply(unsigned int sel, unsigned int ch_id, struct disp_csc_config *config);
int de_ccsc_update_regs(unsigned int sel);
int de_ccsc_init(disp_bsp_init_para *para);
int de_csc_coeff_calc(unsigned int infmt, unsigned int incscmod, unsigned int outfmt, unsigned int outcscmod,
								unsigned int brightness, unsigned int contrast, unsigned int saturation, unsigned int hue,
								unsigned int out_color_range, int *csc_coeff);

#endif

