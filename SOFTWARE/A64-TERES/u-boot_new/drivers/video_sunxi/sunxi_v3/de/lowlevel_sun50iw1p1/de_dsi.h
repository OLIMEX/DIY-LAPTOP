#ifndef __DE_DSI_H_
#define __DE_DSI_H_

#include "../include.h"

extern u32  dsi_bits_per_pixel[4];

typedef enum
{
    //Processor to Peripheral Direction (Processor-Sourced) Packet Data Types
    DSI_DT_VSS                  = 0x01,
    DSI_DT_VSE                  = 0x11,
    DSI_DT_HSS                  = 0x21,
    DSI_DT_HSE                  = 0x31,
    DSI_DT_EOT                  = 0x08,
    DSI_DT_CM_OFF               = 0x02,
    DSI_DT_CM_ON                = 0x12,
    DSI_DT_SHUT_DOWN            = 0x22,
    DSI_DT_TURN_ON              = 0x32,
    DSI_DT_GEN_WR_P0            = 0x03,
    DSI_DT_GEN_WR_P1            = 0x13,
    DSI_DT_GEN_WR_P2            = 0x23,
    DSI_DT_GEN_RD_P0            = 0x04,
    DSI_DT_GEN_RD_P1            = 0x14,
    DSI_DT_GEN_RD_P2            = 0x24,
    DSI_DT_DCS_WR_P0            = 0x05,
    DSI_DT_DCS_WR_P1            = 0x15,
    DSI_DT_DCS_RD_P0            = 0x06,
    DSI_DT_MAX_RET_SIZE         = 0x37,
    DSI_DT_NULL                 = 0x09,
    DSI_DT_BLK                  = 0x19,
    DSI_DT_GEN_LONG_WR          = 0x29,
    DSI_DT_DCS_LONG_WR          = 0x39,
    DSI_DT_PIXEL_RGB565         = 0x0E,
    DSI_DT_PIXEL_RGB666P        = 0x1E,
    DSI_DT_PIXEL_RGB666         = 0x2E,
    DSI_DT_PIXEL_RGB888         = 0x3E,

    //Data Types for Peripheral-sourced Packets
    DSI_DT_ACK_ERR              = 0x02,
    DSI_DT_EOT_PERI             = 0x08,
    DSI_DT_GEN_RD_R1            = 0x11,
    DSI_DT_GEN_RD_R2            = 0x12,
    DSI_DT_GEN_LONG_RD_R        = 0x1A,
    DSI_DT_DCS_LONG_RD_R        = 0x1C,
    DSI_DT_DCS_RD_R1            = 0x21,
    DSI_DT_DCS_RD_R2            = 0x22,
}__dsi_dt_t;

typedef enum
{
	DSI_DCS_ENTER_IDLE_MODE				=	0x39 ,			//01
	DSI_DCS_ENTER_INVERT_MODE			=	0x21 ,			//02
	DSI_DCS_ENTER_NORMAL_MODE			=	0x13 ,			//03
	DSI_DCS_ENTER_PARTIAL_MODE			=	0x12 ,			//04
	DSI_DCS_ENTER_SLEEP_MODE			=	0x10 ,			//05
	DSI_DCS_EXIT_IDLE_MODE				=	0x38 ,			//06
	DSI_DCS_EXIT_INVERT_MODE			=	0x20 ,			//07
	DSI_DCS_EXIT_SLEEP_MODE				=	0x11 ,			//08
	DSI_DCS_GET_ADDRESS_MODE			=	0x0b ,			//09
	DSI_DCS_GET_BLUE_CHANNEL			=	0x08 ,			//10
	DSI_DCS_GET_DIAGNOSTIC_RESULT		=	0x0f ,			//11
	DSI_DCS_GET_DISPLAY_MODE			=	0x0d ,			//12
	DSI_DCS_GET_GREEN_CHANNEL			=	0x07 ,			//13
	DSI_DCS_GET_PIXEL_FORMAT			=	0x0c ,			//14
	DSI_DCS_GET_POWER_MODE				=	0x0a ,			//15
	DSI_DCS_GET_RED_CHANNEL				=	0x06 ,			//16
	DSI_DCS_GET_SCANLINE				=	0x45 ,			//17
	DSI_DCS_GET_SIGNAL_MODE				=	0x0e ,			//18
	DSI_DCS_NOP							=	0x00 ,			//19
	DSI_DCS_READ_DDB_CONTINUE			=	0xa8 ,			//20
	DSI_DCS_READ_DDB_START				=	0xa1 ,			//21
	DSI_DCS_READ_MEMORY_CONTINUE		=	0x3e ,			//22
	DSI_DCS_READ_MEMORY_START			=	0x2e ,			//23
	DSI_DCS_SET_ADDRESS_MODE			=	0x36 ,			//24
	DSI_DCS_SET_COLUMN_ADDRESS			=	0x2a ,			//25
	DSI_DCS_SET_DISPLAY_OFF				=	0x28 ,			//26
	DSI_DCS_SET_DISPLAY_ON				=	0x29 ,			//27
	DSI_DCS_SET_GAMMA_CURVE				=	0x26 ,			//28
	DSI_DCS_SET_PAGE_ADDRESS			=	0x2b ,			//29
	DSI_DCS_SET_PARTIAL_AREA			=	0x30 ,			//30
	DSI_DCS_SET_PIXEL_FORMAT			=	0x3a ,			//31
	DSI_DCS_SET_SCROLL_AREA				=	0x33 ,			//32
	DSI_DCS_SET_SCROLL_START			=	0x37 ,			//33
	DSI_DCS_SET_TEAR_OFF				=	0x34 ,			//34
	DSI_DCS_SET_TEAR_ON					=	0x35 ,			//35
	DSI_DCS_SET_TEAR_SCANLINE			=	0x44 ,			//36
	DSI_DCS_SOFT_RESET					=	0x01 ,			//37
	DSI_DCS_WRITE_LUT					=	0x2d ,			//38
	DSI_DCS_WRITE_MEMORY_CONTINUE		=	0x3c ,			//39
	DSI_DCS_WRITE_MEMORY_START			=	0x2c ,			//40
}__dsi_dcs_t;

typedef enum
{
	DSI_START_LP11				= 0,
	DSI_START_TBA				= 1,
	DSI_START_HSTX				= 2,
	DSI_START_LPTX				= 3,
	DSI_START_LPRX				= 4,
	DSI_START_HSC         = 5,
	DSI_START_HSD         = 6,
}__dsi_start_t;

typedef enum
{
	DSI_INST_ID_LP11			= 0,
	DSI_INST_ID_TBA				= 1,
	DSI_INST_ID_HSC				= 2,
	DSI_INST_ID_HSD				= 3,
	DSI_INST_ID_LPDT			= 4,
	DSI_INST_ID_HSCEXIT			= 5,
	DSI_INST_ID_NOP				= 6,
	DSI_INST_ID_DLY				= 7,
	DSI_INST_ID_END				= 15,
}__dsi_inst_id_t;

typedef enum
{
	DSI_INST_MODE_STOP			= 0,
	DSI_INST_MODE_TBA			= 1,
	DSI_INST_MODE_HS			= 2,
	DSI_INST_MODE_ESCAPE		= 3,
	DSI_INST_MODE_HSCEXIT		= 4,
	DSI_INST_MODE_NOP			= 5,
}__dsi_inst_mode_t;

typedef enum
{
	DSI_INST_ESCA_LPDT			= 0,
	DSI_INST_ESCA_ULPS			= 1,
	DSI_INST_ESCA_UN1			= 2,
	DSI_INST_ESCA_UN2			= 3,
	DSI_INST_ESCA_RESET			= 4,
	DSI_INST_ESCA_UN3			= 5,
	DSI_INST_ESCA_UN4			= 6,
	DSI_INST_ESCA_UN5			= 7,
}__dsi_inst_escape_t;

typedef enum
{
	DSI_INST_PACK_PIXEL			= 0,
	DSI_INST_PACK_COMMAND		= 1,
}__dsi_inst_packet_t;

s32 dsi_delay_ms(u32 ms);
s32 dsi_delay_us(u32 us);
__s32   dsi_set_reg_base(__u32 sel, __u32 base);
__u32   dsi_get_reg_base(__u32 sel);
__u32 	dsi_irq_query(__u32 sel,__dsi_irq_id_t id);
__s32   dsi_cfg(__u32 sel,disp_panel_para * panel);
__s32   dsi_exit(__u32 sel);
__s32   dsi_open(__u32 sel,disp_panel_para * panel);
__s32   dsi_close(__u32 sel);
__s32   dsi_inst_busy(__u32 sel);
__s32   dsi_tri_start(__u32 sel);
__s32   dsi_dcs_wr(__u32 sel,__u8 cmd,__u8* para_p,__u32 para_num);
__s32   dsi_dcs_wr_index(__u32 sel,__u8 index);
__s32   dsi_dcs_wr_data(__u32 sel,__u8 data);
__u32 	dsi_get_start_delay(__u32 sel);
__u32 	dsi_get_cur_line(__u32 sel);
__u32   dsi_io_open(__u32 sel,disp_panel_para * panel);
__u32   dsi_io_close(__u32 sel);
__s32   dsi_clk_enable(__u32 sel, __u32 en);
__u32 dsi_dphy_get_reg_base(__u32 sel);
__s32 dsi_irq_enable(__u32 sel, __dsi_irq_id_t id);
__s32 dsi_irq_disable(__u32 sel, __dsi_irq_id_t id);
__s32 dsi_dcs_wr_0para(__u32 sel,__u8 cmd);
__s32 dsi_dcs_wr_1para(__u32 sel,__u8 cmd,__u8 para);
__s32 dsi_dcs_wr_2para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2);
__s32 dsi_dcs_wr_3para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2,__u8 para3);
__s32 dsi_dcs_wr_4para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2,__u8 para3,__u8 para4);
__s32 dsi_dcs_wr_5para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2,__u8 para3,__u8 para4,__u8 para5);
__s32 dsi_dcs_wr_memory(__u32 sel,__u32* p_data,__u32 length);
__s32 dsi_dcs_rd(__u32 sel,__u8 cmd,__u8* para_p,__u32* num_p);
__s32 dsi_dcs_rd_memory(__u32 sel,__u32* p_data,__u32 length);
__s32 dsi_start(__u32 sel,__dsi_start_t func);
__s32 dsi_command_enable(__u32 sel,__u32 cmd_en,__u32 is_lp);
__s32 dsi_set_max_ret_size(__u32 sel,__u32 size);
__s32 dsi_gen_wr(__u32 sel,__u8 cmd,__u8* para_p,__u32 para_num);
__s32 dsi_gen_wr_0para(__u32 sel,__u8 cmd);
__s32 dsi_gen_wr_1para(__u32 sel,__u8 cmd,__u8 para);
__s32 dsi_gen_wr_2para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2);
__s32 dsi_gen_wr_3para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2,__u8 para3);
__s32 dsi_gen_wr_4para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2,__u8 para3,__u8 para4);
__s32 dsi_gen_wr_5para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2,__u8 para3,__u8 para4,__u8 para5);

__s32 dsi_dphy_cfg_0data(__u32 sel,__u32 code);
__s32 dsi_dphy_cfg_1data(__u32 sel,__u32 code,__u32 data);
__s32 dsi_dphy_cfg_2data(__u32 sel,__u32 code,__u32 data0,__u32 data1);

#endif
