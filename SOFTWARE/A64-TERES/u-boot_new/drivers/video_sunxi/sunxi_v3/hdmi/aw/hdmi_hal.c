#include "../hdmi_hal.h"
#include "hdmi_interface.h"
#include "hdmi_core.h"

#ifndef __UBOOT_PLAT__
static struct audio_para audio_parameter;
#endif

void Hdmi_set_reg_base(__u32 base)
{
	hdmi_set_base_addr(base);
}

__s32 Hdmi_hal_video_enable(bool enable)
{
	__inf("Hdmi_hal_video_enable = %x\n",enable);
	set_video_enable(enable);
	return 0;
}

__s32 Hdmi_hal_set_display_mode(__u32 hdmi_mode)
{
	__inf("Hdmi_hal_set_display_mode = %x\n",hdmi_mode);
	if(hdmi_mode != get_video_mode()) {
		set_video_mode(hdmi_mode);
	}
	return 0;
}

__s32 Hdmi_hal_audio_enable(__u8 mode, __u8 channel)
{
	__inf("Hdmi_hal_audio_enable = %x\n",channel);
	if(channel)
		set_audio_enable(1);
	else
		set_audio_enable(0);
	return 0;
}

#ifndef __UBOOT_PLAT__
__s32 Hdmi_hal_set_audio_para(hdmi_audio_t * audio_para)
{
	__u32 update_flag;

	__inf("Hdmi_hal_set_audio_para\n");
	if(!audio_para)
		return -1;

  __inf("sample_bit:%d in Hdmi_hal_set_audio_para, audio_para->sample_bit:%d\n", audio_parameter.sample_bit, audio_para->sample_bit);
  if(audio_para->sample_bit != audio_parameter.sample_bit) {
		audio_parameter.sample_bit   = audio_para->sample_bit;
		update_flag = 1;
		__inf("sample_bit:%d in Hdmi_hal_set_audio_para\n", audio_parameter.sample_bit);
  }

	if(audio_para->sample_rate != audio_parameter.sample_rate) {
		audio_parameter.sample_rate = audio_para->sample_rate;
		update_flag = 1;
		__inf("sample_rate:%d in Hdmi_hal_set_audio_para\n", audio_parameter.sample_rate);
	}

	if(audio_para->channel_num != audio_parameter.ch_num) {
		audio_parameter.ch_num = audio_para->channel_num;
		update_flag = 1;
		__inf("channel_num:%d in Hdmi_hal_set_audio_para\n", audio_parameter.ch_num);
	}
	if(audio_para->data_raw != audio_parameter.type) {
		audio_parameter.type = audio_para->data_raw;
		update_flag = 1;
		__inf("data_raw:%d in Hdmi_hal_set_audio_para\n", audio_parameter.type);
	}
	if(audio_para->ca != audio_parameter.ca) {
		audio_parameter.ca = audio_para->ca;
		update_flag = 1;
		__inf("ca:%d in Hdmi_hal_set_audio_para\n", audio_parameter.ca);
	}

	if(update_flag == 1)
		audio_config(&audio_parameter);
	return 0;
}
#endif

__s32 Hdmi_hal_cts_enable(__u32 mode)
{
	if(mode) {
		set_cts_enable(1);
	} else {
		set_cts_enable(0);
	}
	return 0;
}
__s32 Hdmi_hal_hdcp_enable(__u32 mode)
{
	if(mode) {
		set_hdcp_enable(1);
	} else {
		set_hdcp_enable(0);
	}
	return 0;
}

__s32 Hdmi_hal_get_hdcp_enable(void)
{
	if(get_hdcp_enable())
		return 1;
	else
		return 0;
}

__s32 Hdmi_hal_dvi_support(void)
{
	if((GetIsHdmi() == 0) && (get_cts_enable() == 1))
		return 1;
	else
		return 0;
}

__s32 Hdmi_hal_mode_support(__u32 mode)
{
	if(Hpd_Check() == 0)
		return 0;

	if(get_video_enable())
		return Device_Support_VIC[mode];	//fixme

	return 0;
}

//0:rgb, 1:yuv
__s32 Hmdi_hal_get_input_csc(void)
{
	return get_csc_type();
}

#ifndef __UBOOT_PLAT__
__s32 Hdmi_hal_get_HPD(void)
{
	return main_Hpd_Check();
}

#else
__s32 Hdmi_hal_get_HPD(void)
{
	return Hpd_Check();
}
#endif

__s32 Hdmi_hal_get_state(void)
{
//	return hdmi_state;	//fixme
	return 0;
}

__s32 Hdmi_hal_set_pll(__u32 pll, __u32 clk)
{
    hdmi_pll = pll;
    hdmi_clk = clk;
    return 0;
}

__s32 Hdmi_hal_main_task(void)
{
    hdmi_main_task_loop();
    return 0;
}

__s32 Hdmi_hal_get_video_info(__s32 vic)
{
	return get_video_info(vic);
}

__s32 Hdmi_hal_get_list_num(void)
{
	return hdmi_core_get_list_num();
}

__s32 Hdmi_hal_enter_lp(void)
{
	return video_enter_lp();
}

__s32 Hdmi_hal_init(void)
{
    //hdmi_audio_t audio_para;

	hdmi_core_initial();
//#ifdef __UBOOT_PLAT__
#if 1
{
    __u32 loop_count;

    loop_count = 3;
    while((loop_count--) && (!Hpd_Check()))
    {
        hdmi_main_task_loop();
    }
}
#endif
//for audio test
#if 0
    audio_para.ch0_en = 1;
    audio_para.sample_rate = 44100;
	Hdmi_hal_set_audio_para(&audio_para);

	Hdmi_hal_audio_enable(0, 1);
#endif

    return 0;
}

__s32 Hdmi_hal_exit(void)
{
	hdmi_core_exit();

    return 0;
}


__s32 Hdmi_hal_video_enable_sync(bool enable)
{
#if 0
    __u32 loop_count;

    if((video_enable != enable) && (hdmi_state >= HDMI_State_Video_config) )
	{
		hdmi_state 			= HDMI_State_Video_config;
	}
    video_enable = enable;

    loop_count = 3;
    while(loop_count--)
    {
        hdmi_main_task_loop();
        if(hdmi_state == HDMI_State_Playback)
        {
            return 0;
        }
        hdmi_delay_ms(1);
    }
#endif
    return -1;
}
