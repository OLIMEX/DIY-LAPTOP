#ifndef  __LT070ME05000_PANEL_H__
#define  __LT070ME05000_PANEL_H__

#include "panels.h"

extern __lcd_panel_t lt070me05000_panel;

extern s32 dsi_dcs_wr_0para(u32 sel,u8 cmd);
extern s32 dsi_dcs_wr_1para(u32 sel,u8 cmd,u8 para);
extern s32 dsi_dcs_wr_2para(u32 sel,u8 cmd,u8 para1,u8 para2);
extern s32 dsi_dcs_wr_3para(u32 sel,u8 cmd,u8 para1,u8 para2,u8 para3);
extern s32 dsi_dcs_wr_4para(u32 sel,u8 cmd,u8 para1,u8 para2,u8 para3,u8 para4);
extern s32 dsi_dcs_wr_5para(u32 sel,u8 cmd,u8 para1,u8 para2,u8 para3,u8 para4,u8 para5);
extern s32 dsi_gen_wr_0para(u32 sel,u8 cmd);
extern s32 dsi_gen_wr_1para(u32 sel,u8 cmd,u8 para);
extern s32 dsi_gen_wr_2para(u32 sel,u8 cmd,u8 para1,u8 para2);
extern s32 dsi_gen_wr_3para(u32 sel,u8 cmd,u8 para1,u8 para2,u8 para3);
extern s32 dsi_gen_wr_4para(u32 sel,u8 cmd,u8 para1,u8 para2,u8 para3,u8 para4);
extern s32 dsi_gen_wr_5para(u32 sel,u8 cmd,u8 para1,u8 para2,u8 para3,u8 para4,u8 para5);

#endif
