#include "hdmi_core.h"

static __s32		hdmi_state = HDMI_State_Idle;
static __u32		video_on = 0;
static __u32		audio_on = 0;
static bool		video_enable = 0;
static bool		audio_enable = 0;
static __u32		cts_enable = 0;
static __u32		hdcp_enable = 0;
static __u8		isHDMI = 0;
static __u8		YCbCr444_Support = 0;
static __s32		HPD = 0;
static struct audio_para glb_audio_para;
static struct video_para glb_video_para;
//static __u8 video_need_update;
static __u8 audio_need_update;
static HDMI_AUDIO_INFO audio_info;

static __s32 audio_config_internal(void);

__u32		hdmi_print = 1;
__u32		hdmi_pll = 0;//0:video pll 0; 1:video pll 1
__u32		hdmi_clk = 297000000;

disp_video_timings video_timing[] =
{
	//VIC				   PCLK    AVI_PR  X      Y      HT      HBP   HFP   HST    VT     VBP  VFP  VST h_pol v_pol int vac   trd
	{HDMI1440_480I,     0, 13500000,  1,  720,   480,   858,   57,   19,   62,  525,   15,  4,  3,  0,   0,   1,   0,   0},
	{HDMI1440_576I,     0, 13500000,  1,  720,   576,   864,   69,   12,   63,  625,   19,  2,  3,  0,   0,   1,   0,   0},
	{HDMI480P,          0, 27000000,  0,  720,   480,   858,   60,   16,   62,  525,   30,  9,  6,  0,   0,   0,   0,   0},
	{HDMI576P,          0, 27000000,  0,  720,   576,   864,   68,   12,   64,  625,   39,  5,  5,  0,   0,   0,   0,   0},
	{HDMI720P_50,       0, 74250000,  0,  1280,  720,   1980,  220,  440,  40,  750,   20,  5,  5,  1,   1,   0,   0,   0},
	{HDMI720P_60,       0, 74250000,  0,  1280,  720,   1650,  220,  110,  40,  750,   20,  5,  5,  1,   1,   0,   0,   0},
	{HDMI1080I_50,      0, 74250000,  0,  1920,  1080,   2640,  148,  528,  44,  1125,  15,  2,  5,  1,   1,   1,   0,   0},
	{HDMI1080I_60,      0, 74250000,  0,  1920,  1080,   2200,  148,  88,   44,  1125,  15,  2,  5,  1,   1,   1,   0,   0},
	{HDMI1080P_50,      0, 148500000, 0,  1920,  1080,  2640,  148,  528,  44,  1125,  36,  4,  5,  1,   1,   0,   0,   0},
	{HDMI1080P_60,      0, 148500000, 0,  1920,  1080,  2200,  148,  88,   44,  1125,  36,  4,  5,  1,   1,   0,   0,   0},
	{HDMI1080P_24,      0, 74250000,  0,  1920,  1080,  2750,  148,  638,  44,  1125,  36,  4,  5,  1,   1,   0,   0,   0},
	{HDMI1080P_25,      0, 74250000,  0,  1920,  1080,  2640,  148,  528,  44,  1125,  36,  4,  5,  0,   0,   0,   0,   0},
	{HDMI1080P_30,      0, 74250000,  0,  1920,  1080,  2200,  148,  88,   44,  1125,  36,  4,  5,  0,   0,   0,   0,   0},
	{HDMI1080P_24_3D_FP,0, 148500000, 0,  1920,  2160,  2750,  148,  638,  44,  1125,  36,  4,  5,  1,   1,   0,   45,  1},
	{HDMI720P_50_3D_FP, 0, 148500000, 0,  1280,  1440,  1980,  220,  440,  40,  750,   20,  5,  5,  1,   1,   0,   30,  1},
	{HDMI720P_60_3D_FP, 0, 148500000, 0,  1280,  1440,  1650,  220,  110,  40,  750,   20,  5,  5,  1,   1,   0,   30,  1},
	{HDMI3840_2160P_30, 0, 297000000, 0,  3840,  2160,  4400,  296,  176,  88,  2250,  72,  8, 10,  1,   1,   0,    0,  0},
	{HDMI3840_2160P_25, 0, 297000000, 0,  3840,  2160,  5280,  296, 1056,  88,  2250,  72,  8, 10,  1,   1,   0,    0,  0},
};

static void hdmi_para_reset(void)
{
	hdmi_state	  = HDMI_State_Idle;
	video_on = 0;
	audio_on = 0;
	video_enable = 0;
	audio_enable = 0;
}

static void hdmi_para_init(void)
{
	glb_video_para.vic = HDMI720P_50;
	glb_video_para.csc = BT601;
	glb_video_para.is_hdmi = 1;
	glb_video_para.is_yuv = 0;
	glb_video_para.is_hcts = 0;
	glb_audio_para.type = 1; //default pcm
	glb_audio_para.sample_rate = 44100;
	glb_audio_para.sample_bit = 16;
	glb_audio_para.ch_num = 2;
	glb_audio_para.ca = 0;
}

__s32 hdmi_core_initial(void)
{
	memset(&audio_info,0,sizeof(HDMI_AUDIO_INFO));
	api_set_func(hdmi_delay_us);
	bsp_hdmi_init();
	//bsp_hdmi_set_addr(HDMI_BASE); //fixme
	//bsp_hdmi_standby();
	hdmi_para_init();
	return 0;
}

void hdmi_core_exit(void)
{

}

void hdmi_set_base_addr(__u32 base_addr)
{
	bsp_hdmi_set_addr(base_addr);
}

static __s32 main_Hpd_Check(void)
{
	__s32 i,times;
	times	= 0;

	for(i=0;i<3;i++) {
		if( bsp_hdmi_get_hpd())
		{
				times++;
		}
		else
		{
			//hdmi_state = HDMI_State_Idle;
		}
#if defined(__LINUX_PLAT__)
		if((cts_enable==1) && (hdcp_enable==1))
			hdmi_delay_ms(20); //200
		else
			hdmi_delay_ms(200); //200
#else
			hdmi_delay_ms(10);
#endif
	}

	if(times >= 3)
		return 1;
	else
		return 0;
}

__s32 hdmi_main_task_loop(void)
{
	static __u32 times = 0;

	HPD = main_Hpd_Check();
	if( 0 == HPD )
	{
		if(hdmi_state > HDMI_State_Wait_Hpd) {
			__inf("plugout\n");
			hdmi_state = HDMI_State_Idle;
			video_on = 0;
			audio_on = 0;
			//Hdmi_hpd_event();
		}

		if((times++) >= 10) {
			times = 0;
			__inf("unplug state !!\n");
		}
  }

	switch(hdmi_state) {

		case HDMI_State_Idle:
			__inf("HDMI_State_Idle\n");
			bsp_hdmi_hrst();
			bsp_hdmi_standby();

			hdmi_state = HDMI_State_Wait_Hpd;
		case HDMI_State_Wait_Hpd:
			__inf("HDMI_State_Wait_Hpd\n");
			if(HPD) {
				hdmi_state = HDMI_State_EDID_Parse;
				__inf("plugin\n");
			} else {
				return 0;
			}

		case HDMI_State_Rx_Sense:

		case HDMI_State_EDID_Parse:
			__inf("HDMI_State_EDID_Parse\n");
			ParseEDID();
			//Hdmi_hpd_event();
			if(video_enable)
				set_video_enable(true);
			hdmi_state = HDMI_State_HPD_Done;

		case HDMI_State_HPD_Done:
			//__inf("HDMI_State_HPD_Done\n");
			if(video_on && hdcp_enable)
				bsp_hdmi_hdl();
			return 0;
		default:
			__wrn(" unkonwn hdmi state, set to idle\n");
			hdmi_state = HDMI_State_Idle;
			return 0;
	}
}

__s32 Hpd_Check(void)
{
	if(hdmi_state >= HDMI_State_HPD_Done)
		return 1;
	else
		return 0;
}

__s32 get_video_info(__s32 vic)
{
	__s32 i,count;
	count = sizeof(video_timing)/sizeof(disp_video_timings);
	for(i=0;i<count;i++) {
		if(vic == video_timing[i].vic)
			return i;
	}
	__wrn("can't find the video timing parameters\n");
	return -1;
}

__s32 get_audio_info(__s32 sample_rate)
{
	//ACR_N 32000 44100 48000 88200 96000 176400 192000
	//		4096  6272  6144  12544 12288  25088  24576
	__inf("sample_rate:%d in get_audio_info\n", sample_rate);

	switch(sample_rate) {
		case 32000 :{	audio_info.ACR_N = 4096 ;
			audio_info.CH_STATUS0 = (3 <<24);
			audio_info.CH_STATUS1 = 0x0000000b;
			break;}
		case 44100 :{	audio_info.ACR_N = 6272 ;
			audio_info.CH_STATUS0 = (0 <<24);
			audio_info.CH_STATUS1 = 0x0000000b;
			break;}
		case 48000 :{	audio_info.ACR_N = 6144 ;
			audio_info.CH_STATUS0 = (2 <<24);
			audio_info.CH_STATUS1 = 0x0000000b;
			break;}
		case 88200 :{	audio_info.ACR_N = 12544;
			audio_info.CH_STATUS0 = (8 <<24);
			audio_info.CH_STATUS1 = 0x0000000b;
			break;}
		case 96000 :{	audio_info.ACR_N = 12288;
			audio_info.CH_STATUS0 = (10<<24);
			audio_info.CH_STATUS1 = 0x0000000b;
			break;}
		case 176400:{	audio_info.ACR_N = 25088;
			audio_info.CH_STATUS0 = (12<<24);
			audio_info.CH_STATUS1 = 0x0000000b;
			break;}
		case 192000:{	audio_info.ACR_N = 24576;
			audio_info.CH_STATUS0 = (14<<24);
			audio_info.CH_STATUS1 = 0x0000000b;
			break;}
		default:	{	__wrn("un-support sample_rate,value=%d\n",sample_rate);
				return -1;}
	}

	if((glb_video_para.vic == HDMI1440_480I) || (glb_video_para.vic == HDMI1440_576I) ||
		/*(glb_video_para.vic == HDMI480P)	 || */(glb_video_para.vic == HDMI576P)) {
		audio_info.CTS =   ((27000000/100) *(audio_info.ACR_N /128)) / (sample_rate/100);
	} else if( (glb_video_para.vic == HDMI720P_50 )||(glb_video_para.vic == HDMI720P_60 ) ||
				 (glb_video_para.vic == HDMI1080I_50)||(glb_video_para.vic == HDMI1080I_60) ||
				 (glb_video_para.vic == HDMI1080P_24)||(glb_video_para.vic == HDMI1080P_25) ||
				 (glb_video_para.vic == HDMI1080P_30)) {
		audio_info.CTS =   ((74250000/100) *(audio_info.ACR_N /128)) / (sample_rate/100);
	} else if( (glb_video_para.vic == HDMI1080P_50)||(glb_video_para.vic == HDMI1080P_60)	   ||
			(glb_video_para.vic == HDMI1080P_24_3D_FP)||(glb_video_para.vic == HDMI720P_50_3D_FP) ||
			(glb_video_para.vic == HDMI720P_60_3D_FP) ) {
		audio_info.CTS =   ((148500000/100) *(audio_info.ACR_N /128)) / (sample_rate/100);
	} else {
		__wrn("unkonwn video format when configure audio\n");
		return -1;
	}
	__inf("audio CTS calc:%d\n",audio_info.CTS);
	return 0;
}

void set_hdcp_enable(__u32 enable)
{
	hdcp_enable = enable;
}

__u32 get_hdcp_enable()
{
	return hdcp_enable;
}

void set_cts_enable(__u32 enable)
{
	cts_enable = enable;
}

__u32 get_cts_enable()
{
	return cts_enable;
}

__u32 get_csc_type(void)
{
	int csc = 1;

	if((get_cts_enable() == 1) &&(GetIsYUV() == 0))
		csc = 0;

	if( (glb_video_para.vic == HDMI1080P_24)
  	|| (glb_video_para.vic == HDMI1080P_24_3D_FP)
  	|| (glb_video_para.vic == HDMI3840_2160P_24)
  	|| (glb_video_para.vic == HDMI3840_2160P_30)
  	|| (glb_video_para.vic == HDMI3840_2160P_25)
  	) {
  		csc = 0;
  	}
  
	return csc;
}

__s32 set_audio_enable(bool enable)
{
	int ret = 0;

	__inf("set_audio_enable = %x!\n",enable);
	if(enable && audio_need_update)
	{
		audio_need_update = 0;
		if(audio_config_internal())
		{
			__wrn("set_audio_enable err!\n");
			ret = -1;
		}
		else
		{
			audio_on = 1;
		}
	}
	else
	{
		audio_on = 0;
	}
	audio_enable = enable;

	return ret;
}

bool get_audio_enable()
{
	bool ret;

	ret = audio_enable;
	return ret;
}

static __s32 audio_config_internal()
{
	__inf("audio_config_internal, type code:%d\n", glb_audio_para.type);
	__inf("audio_config_internal, sample_rate:%d\n", glb_audio_para.sample_rate);
	__inf("audio_config_internal, sample_bit:%d\n", glb_audio_para.sample_bit);
	__inf("audio_config_internal, channel_num:%d\n", glb_audio_para.ch_num);
	__inf("audio_config_internal, channel allocation:%d\n", glb_audio_para.ca);

	if(video_on)
	{
		__inf("audio_config_internal when video on");
		if((cts_enable==1) && (isHDMI == 0))
		{
			__inf("sink is not hdmi, not sending audio\n");
			return 0;
		}

		if(bsp_hdmi_audio(&glb_audio_para))
		{
	  	__wrn("set hdmi audio error!\n");
	  	return -1;
	  }

	  audio_on = 1;
	}
	else
	{
		__inf("audio_config_internal when video off");
	}

	return 0;
}

__s32 audio_config(struct audio_para *audio_param)
{
	int ret = 0;

	__inf("audio_config\n");

	glb_audio_para.type = audio_param->type;
	glb_audio_para.sample_rate = audio_param->sample_rate;
	glb_audio_para.sample_bit = audio_param->sample_bit;
	glb_audio_para.ch_num = audio_param->ch_num;
	glb_audio_para.ca = audio_param->ca;

	audio_need_update = 1;

	return ret;
}

__u32 get_video_mode()
{
	__u32 ret;

	if(video_enable == 0)
		ret = 0;
	else
		ret =  glb_video_para.vic;

	return ret;
}


__s32 set_video_mode(__u32 vic)
{
	__u32 ret = 0;
	glb_video_para.vic = vic;

	return ret;
}

__s32 set_video_enable(bool enable)
{
	int ret = 0;
	__inf("set_video_enable = %x!\n",enable);
	__inf("video_on @ set_video_enable = %d!\n",video_on);
	if((hdmi_state == HDMI_State_HPD_Done) && enable && (0 == video_on))
	{
		video_config(glb_video_para.vic);
		__inf("set_video_enable, vic:%d,is_hdmi:%d,is_yuv:%d,is_hcts:%d\n",
			glb_video_para.vic, glb_video_para.is_hdmi,glb_video_para.is_yuv, glb_video_para.is_hcts);
		if(bsp_hdmi_video(&glb_video_para))
	  {
	  	__wrn("set hdmi video error!\n");
	  	ret = -1;
	  	goto video_en_end;
	  }

	  bsp_hdmi_set_video_en(enable);
	  video_on = 1;
#if 0
	  if(audio_config_internal())
	  {
	  	__wrn("set audio_config_internal error!\n");
	  	ret = -1;
	  	goto video_en_end;
	  }
#endif
	}
	else
	{
		video_on = 0;
	}

	video_enable = enable;

video_en_end:
	return ret;
}

bool get_video_enable()
{
	bool ret;
	ret = video_enable;
	return ret;
}

__s32 hdmi_core_get_list_num(void)
{
	return sizeof(video_timing)/sizeof(disp_video_timings);
}

__s32 video_config(__u32 vic)
{
	int ret = 0;

//	cts_enable = 1;
  isHDMI = GetIsHdmi();
  YCbCr444_Support = GetIsYUV();
//  hdcp_enable = 0;

	__inf("video_config, vic:%d,cts_enable:%d,isHDMI:%d,YCbCr444_Support:%d,hdcp_enable:%d\n",
    vic,cts_enable,isHDMI,YCbCr444_Support,hdcp_enable);

	glb_video_para.vic = vic;
	if((cts_enable==1) && (isHDMI == 0))
  	glb_video_para.is_hdmi = 0;
  else
  	glb_video_para.is_hdmi = 1;

	glb_video_para.is_yuv = get_csc_type();

  if(hdcp_enable)
  {
			glb_video_para.is_hcts = 1;
			bsp_hdmi_hrst();
      __inf("hdmi full function\n");
  }
  else
  {
      glb_video_para.is_hcts = 0;
      __inf("hdmi video + audio\n");
  }
	__inf("video_on @ video_config = %d!\n",video_on);
	return ret;
}

__s32 video_enter_lp(void)
{
	__inf("video enter lp\n");
	hdmi_state = HDMI_State_Idle;
	bsp_hdmi_standby();
	hdmi_para_reset();

	return 0;
}

