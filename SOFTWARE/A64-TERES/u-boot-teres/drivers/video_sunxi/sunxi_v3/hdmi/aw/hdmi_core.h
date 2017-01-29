#ifndef __HDMI_CORE_H__
#define __HDMI_CORE_H__

#include "../hdmi_hal.h"
#include "hdmi_interface.h"
#include "hdmi_edid.h"
#include "hdmi_bsp.h"

#define HDMI_State_Idle 			 0x00
#define HDMI_State_Wait_Hpd			 0x02
#define HDMI_State_Rx_Sense			 0x03
#define HDMI_State_EDID_Parse		 0x04
#define HDMI_State_HPD_Done			 0x05
//#define HDMI_State_Wait_Video_config 0x05
//#define HDMI_State_Video_config		 0x06
//#define HDMI_State_Audio_config		 0x07
//#define HDMI_State_Playback			 0x09


/*
typedef struct video_timing
{
    __s32 VIC;
    __s32 PCLK;
    __s32 AVI_PR;
    __s32 INPUTX;
    __s32 INPUTY;
    __s32 HT;
    __s32 HBP;
    __s32 HFP;
    __s32 HPSW;
    __s32 VT;
    __s32 VBP;
    __s32 VFP;
    __s32 VPSW;
}HDMI_VIDE_INFO;
*/

typedef struct audio_timing
{
    __s32   CTS;
    __s32   ACR_N;
    __s32   CH_STATUS0;
    __s32   CH_STATUS1;
}HDMI_AUDIO_INFO;

extern __u32 hdmi_ishdcp(void);
extern __s32 hdmi_core_initial(void);
extern void hdmi_core_exit(void);
extern void hdmi_set_base_addr(__u32 base_addr);
extern __s32 hdmi_core_open(void);
extern __s32 hdmi_core_close(void);
extern __s32 hdmi_main_task_loop(void);
extern __s32 Hpd_Check(void);
extern __s32 get_video_info(__s32 vic);
extern __s32 get_audio_info(__s32 sample_rate);
extern __s32 set_video_mode(__u32 vic);
extern __u32 get_video_mode(void);
extern __s32 video_config(__u32 vic);
extern __s32 audio_config(struct audio_para *audio_param);
extern __s32 set_video_enable(bool enable);
extern bool get_video_enable(void);
extern __s32 set_audio_enable(bool enable);
extern bool get_audio_enable(void);
extern void set_hdcp_enable(__u32 mode);
extern __u32 get_hdcp_enable(void);
extern void set_cts_enable(__u32 enable);
extern __u32 get_cts_enable(void);
extern __s32 video_enter_lp(void);
extern __u32 get_csc_type(void);

__s32 hdmi_core_get_list_num(void);


extern __u32 hdmi_pll;//0:video pll 0; 1:video pll 1
extern __u32 hdmi_clk;
extern __u32 hdmi_print;
//extern disp_video_timing video_timing[];

#endif

