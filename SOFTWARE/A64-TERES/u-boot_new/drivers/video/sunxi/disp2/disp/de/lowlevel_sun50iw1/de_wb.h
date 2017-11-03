//*****************************************************************************
//  All Winner Micro, All Right Reserved. 2006-2013 Copyright (c)
//
//  File name   :        de2_wb.h
//
//  Description :  DE2.0 Write-Back Controller  interface functions
//  History     :
//				   2014/03/03	initial version
//				   2014/04/02 change the register operation from bits to word
//	Code by		:  wangxuan
//******************************************************************************
#ifndef __DE_WB_H__
#define __DE_WB_H__

#include "de_rtmx.h"

s32 WB_EBIOS_Set_Reg_Base(u32 sel, uintptr_t base);
uintptr_t WB_EBIOS_Get_Reg_Base(u32 sel);
s32 WB_EBIOS_Init(disp_bsp_init_para *para);
s32 WB_EBIOS_Writeback_Enable(u32 sel, bool en);
s32 WB_EBIOS_Set_Para(u32 sel, struct disp_capture_config *cfg);
s32 WB_EBIOS_Apply(u32 sel, struct disp_capture_config *cfg);
s32 WB_EBIOS_Update_Regs(u32 sel);
u32 WB_EBIOS_Get_Status(u32 sel);
s32 WB_EBIOS_EnableINT(u32 sel);
s32 WB_EBIOS_DisableINT(u32 sel);
u32 WB_EBIOS_QueryINT(u32 sel);
u32 WB_EBIOS_ClearINT(u32 sel);

#endif
