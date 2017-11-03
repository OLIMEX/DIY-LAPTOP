#include "hdmi_core.h"
#include "drv_hdmi_i.h"

static s32		hdmi_state = HDMI_State_Idle;
static u32		video_on = 0;
static u32		audio_on = 0;
static bool		video_enable = 0;
static bool		audio_enable = false;
static u32		cts_enable = 0;
static u32		hdcp_enable = 0;
static u8		isHDMI = 0;
static u8		YCbCr444_Support = 0;
static s32		HPD = 0;
static struct audio_para glb_audio_para;
static struct video_para glb_video_para;
static bool audio_cfged = false;
static HDMI_AUDIO_INFO audio_info;
static struct mutex hdmi_lock;
static s32 audio_config_internal(void);
extern u32 is_exp;
u32	hdmi_print = 0;
u32 hdmi_hpd_mask = 0x00;//0x10: force unplug; 0x11: force plug; 0x1xx: unreport hpd state
static u32 hdmi_detect_time = 200;//ms

static s32 video_config(u32 vic);

uintptr_t hdmi_base_addr;
EXPORT_SYMBOL(hdmi_base_addr);
EXPORT_SYMBOL(hdmi_delay_ms);

struct disp_video_timings video_timing[] =
{
	//VIC				   PCLK    AVI_PR  X      Y      HT      HBP   HFP   HST    VT     VBP  VFP  VST h_pol v_pol int vac   trd
	{HDMI1440_480I,      0,13500000,  1,  720,   480,   858,   57,   19,   62,  525,   15,  4,  3,  0,   0,   1,   0,   0},
	{HDMI1440_576I,      0,13500000,  1,  720,   576,   864,   69,   12,   63,  625,   19,  2,  3,  0,   0,   1,   0,   0},
	{HDMI480P,           0,27000000,  0,  720,   480,   858,   60,   16,   62,  525,   30,  9,  6,  0,   0,   0,   0,   0},
	{HDMI576P,           0,27000000,  0,  720,   576,   864,   68,   12,   64,  625,   39,  5,  5,  0,   0,   0,   0,   0},
	{HDMI720P_50,        0,74250000,  0,  1280,  720,   1980,  220,  440,  40,  750,   20,  5,  5,  1,   1,   0,   0,   0},
	{HDMI720P_60,        0,74250000,  0,  1280,  720,   1650,  220,  110,  40,  750,   20,  5,  5,  1,   1,   0,   0,   0},
	{HDMI1080I_50,       0,74250000,  0,  1920,  1080,   2640,  148,  528,  44,  1125,  15,  2,  5,  1,   1,   1,   0,   0},
	{HDMI1080I_60,       0,74250000,  0,  1920,  1080,   2200,  148,  88,   44,  1125,  15,  2,  5,  1,   1,   1,   0,   0},
	{HDMI1080P_50,       0,148500000, 0,  1920,  1080,  2640,  148,  528,  44,  1125,  36,  4,  5,  1,   1,   0,   0,   0},
	{HDMI1080P_60,       0,148500000, 0,  1920,  1080,  2200,  148,  88,   44,  1125,  36,  4,  5,  1,   1,   0,   0,   0},
	{HDMI1080P_24,       0,74250000,  0,  1920,  1080,  2750,  148,  638,  44,  1125,  36,  4,  5,  1,   1,   0,   0,   0},
	{HDMI1080P_25,       0,74250000,  0,  1920,  1080,  2640,  148,  528,  44,  1125,  36,  4,  5,  0,   0,   0,   0,   0},
	{HDMI1080P_30,       0,74250000,  0,  1920,  1080,  2200,  148,  88,   44,  1125,  36,  4,  5,  0,   0,   0,   0,   0},
	{HDMI1080P_24_3D_FP, 0,148500000, 0,  1920,  2160,  2750,  148,  638,  44,  1125,  36,  4,  5,  1,   1,   0,   45,  1},
	{HDMI720P_50_3D_FP,  0,148500000, 0,  1280,  1440,  1980,  220,  440,  40,  750,   20,  5,  5,  1,   1,   0,   30,  1},
	{HDMI720P_60_3D_FP,  0,148500000, 0,  1280,  1440,  1650,  220,  110,  40,  750,   20,  5,  5,  1,   1,   0,   30,  1},
	{HDMI3840_2160P_30,  0,297000000, 0,  3840,  2160,  4400,  296,  176,  88,  2250,  72,  8, 10,  1,   1,   0,    0,  0},
	{HDMI3840_2160P_25,  0,297000000, 0,  3840,  2160,  5280,  296, 1056,  88,  2250,  72,  8, 10,  1,   1,   0,    0,  0},
	{HDMI_EDID,},
};

static void hdmi_para_reset(void)
{
	hdmi_state	  = HDMI_State_Idle;
	video_on = 0;
	audio_on = 0;
	video_enable = 0;
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
	audio_enable = 0;
	audio_cfged = false;
}

s32 hdmi_core_initial(bool sw_only)
{
	hdmi_bsp_func func;

	func.delay_us = hdmi_delay_us;
	func.delay_ms = hdmi_delay_ms;
	memset(&audio_info,0,sizeof(HDMI_AUDIO_INFO));
	mutex_init(&hdmi_lock);
	bsp_hdmi_set_version(hdmi_get_soc_version());
	bsp_hdmi_set_func(&func);
	hdmi_para_init();
	if (sw_only) {
		video_enable = 1;
		hdmi_state = HDMI_State_HPD_Done;
		if (bsp_hdmi_get_hpd()) {
			hdmi_edid_parse();
			video_on = 1;
			if (0 == (hdmi_hpd_mask & 0x100))
				hdmi_hpd_event();
		}
	} else {
		bsp_hdmi_init();
	}

	return 0;
}

void hdmi_core_exit(void)
{
	mutex_destroy(&hdmi_lock);
}

void hdmi_core_set_base_addr(uintptr_t base_addr)
{
    printk(KERN_INFO "HDMI base address: %p\n", base_addr);
	bsp_hdmi_set_addr(base_addr);
	hdmi_base_addr = base_addr;
}

static s32 main_Hpd_Check(void)
{
	s32 i,times;
	times	= 0;

	for (i=0;i<3;i++) {
		mutex_lock(&hdmi_lock);
		if (hdmi_hpd_mask & 0x10)
			times += (hdmi_hpd_mask & 0x1);//for debug
		else if ( bsp_hdmi_get_hpd())
		{
				times++;
		}
		mutex_unlock(&hdmi_lock);
		if ((cts_enable==1) && (hdcp_enable==1))
			hdmi_delay_ms(20);
		else {
			if (0 != hdmi_detect_time)
				hdmi_delay_ms(hdmi_detect_time);
		}
	}

	if (times >= 3)
		return 1;
	else
		return 0;
}

s32 hdmi_core_loop(void)
{
	static u32 times = 0;

	HPD = main_Hpd_Check();
	if ( 0 == HPD )
	{
		if ((hdmi_state > HDMI_State_Wait_Hpd)  || (hdmi_state == HDMI_State_Idle)) {
			__inf("plugout\n");
			hdmi_state = HDMI_State_Idle;
			if (1 == video_on)
				hdmi_clk_disable_prepare();
			video_on = 0;
			audio_on = 0;
			if (0 == (hdmi_hpd_mask & 0x100))
				hdmi_hpd_event();
			sunxi_hdmi_notifier_call_chain(hdmi_state);
		}

		if ((times++) >= 10) {
			times = 0;
			__inf("unplug state !!\n");
		}
  }

	switch (hdmi_state) {

		case HDMI_State_Idle:
			__inf("HDMI_State_Idle\n");
			bsp_hdmi_hrst();
			bsp_hdmi_standby();

			hdmi_state = HDMI_State_Wait_Hpd;
		case HDMI_State_Wait_Hpd:
			__inf("HDMI_State_Wait_Hpd\n");
			if (HPD) {
				hdmi_state = HDMI_State_EDID_Parse;
				__inf("plugin\n");
				sunxi_hdmi_notifier_call_chain(hdmi_state);

			} else {
				return 0;
			}
			msleep(200);
		case HDMI_State_Rx_Sense:

		case HDMI_State_EDID_Parse:
			__inf("HDMI_State_EDID_Parse\n");
			mutex_lock(&hdmi_lock);
			hdmi_edid_parse();
			mutex_unlock(&hdmi_lock);
			hdmi_state = HDMI_State_HPD_Done;
			if (0 == (hdmi_hpd_mask & 0x100))
				hdmi_hpd_event();

			if (video_enable)
				hdmi_core_set_video_enable(true);
			sunxi_hdmi_notifier_call_chain(hdmi_state);
		case HDMI_State_HPD_Done:
			if (video_on && hdcp_enable)
				bsp_hdmi_hdl();
			return 0;
		default:
			__wrn(" unkonwn hdmi state, set to idle\n");
			hdmi_state = HDMI_State_Idle;
			return 0;
	}
}

s32 hdmi_core_hpd_check(void)
{
	if (hdmi_state >= HDMI_State_HPD_Done)
		return 1;
	else
		return 0;
}

s32 hdmi_core_get_video_info(s32 vic)
{
	s32 i,count;

	count = sizeof(video_timing)/sizeof(struct disp_video_timings);
	for (i=0;i<count;i++) {
		if (vic == video_timing[i].vic)
			return i;
	}
	__wrn("can't find the video timing parameters\n");
	return -1;
}

s32 hdmi_core_get_audio_info(s32 sample_rate)
{
	//ACR_N 32000 44100 48000 88200 96000 176400 192000
	//		4096  6272  6144  12544 12288  25088  24576
	__inf("sample_rate:%d in hdmi_core_get_audio_info\n", sample_rate);

	switch (sample_rate) {
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

	if ((glb_video_para.vic == HDMI1440_480I) || (glb_video_para.vic == HDMI1440_576I) ||
		/*(glb_video_para.vic == HDMI480P)	 || */(glb_video_para.vic == HDMI576P)) {
		audio_info.CTS =   ((27000000/100) *(audio_info.ACR_N /128)) / (sample_rate/100);
	} else if ( (glb_video_para.vic == HDMI720P_50 )||(glb_video_para.vic == HDMI720P_60 ) ||
				 (glb_video_para.vic == HDMI1080I_50)||(glb_video_para.vic == HDMI1080I_60) ||
				 (glb_video_para.vic == HDMI1080P_24)||(glb_video_para.vic == HDMI1080P_25) ||
				 (glb_video_para.vic == HDMI1080P_30)) {
		audio_info.CTS =   ((74250000/100) *(audio_info.ACR_N /128)) / (sample_rate/100);
	} else if ( (glb_video_para.vic == HDMI1080P_50)||(glb_video_para.vic == HDMI1080P_60)	   ||
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

s32 hdmi_core_set_hdcp_enable(u32 enable)
{
	hdcp_enable = enable;
	/* change the hdmi state, video will be reconfig if it output currently */
	hdmi_state = HDMI_State_Idle;
	if (1 == video_on)
		hdmi_clk_disable_prepare();
	video_on = 0;
	audio_on = 0;

	return 0;
}

u32 hdmi_core_get_hdcp_enable(void)
{
	return hdcp_enable;
}

s32 hdmi_core_set_cts_enable(u32 enable)
{
	cts_enable = enable;

	return 0;
}

u32 hdmi_core_get_cts_enable(void)
{
	return cts_enable;
}

u32 hdmi_core_get_csc_type(void)
{
	int csc = 1;

	if ((hdmi_core_get_cts_enable() == 1) &&(hdmi_edid_is_yuv() == 0))
		csc = 0;

	if ((is_exp == 1) &&
		((glb_video_para.vic == HDMI1080P_24)
		|| (glb_video_para.vic == HDMI1080P_24_3D_FP)
		|| (glb_video_para.vic == HDMI3840_2160P_24)
		|| (glb_video_para.vic == HDMI3840_2160P_30)
		|| (glb_video_para.vic == HDMI3840_2160P_25))
	) {
		csc = 0;
	}

	return csc;
}

s32 hdmi_core_set_audio_enable(bool enable)
{
	int ret = 0;

	mutex_lock(&hdmi_lock);
	__inf("set_audio_enable = %x!\n",enable);

	if (true == enable)
		audio_cfged = true;
	audio_enable = enable;

	if (((glb_audio_para.type != 1) && (true == audio_enable)) ||
		((glb_audio_para.type == 1) && (audio_cfged == true) && (true == audio_enable)) ) {
		if (audio_config_internal())
		{
			__wrn("audio_config_internal err!\n");
			ret = -1;
		}
	}
	mutex_unlock(&hdmi_lock);
	return ret;
}

bool hdmi_core_get_audio_enable(void)
{
	bool ret;
	mutex_lock(&hdmi_lock);
	ret = audio_enable;
	mutex_unlock(&hdmi_lock);
	return ret;
}

static s32 audio_config_internal(void)
{
	__inf("audio_config_internal, type code:%d\n", glb_audio_para.type);
	__inf("audio_config_internal, sample_rate:%d\n", glb_audio_para.sample_rate);
	__inf("audio_config_internal, sample_bit:%d\n", glb_audio_para.sample_bit);
	__inf("audio_config_internal, channel_num:%d\n", glb_audio_para.ch_num);
	__inf("audio_config_internal, channel allocation:%d\n", glb_audio_para.ca);

	if (video_on)
	{
		__inf("audio_config_internal when video on");
		if ((cts_enable==1) && (isHDMI == 0))
		{
			__inf("sink is not hdmi, not sending audio\n");
			return 0;
		}

		if (bsp_hdmi_audio(&glb_audio_para))
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

s32 hdmi_core_audio_config(hdmi_audio_t *audio_param)
{
	int ret = 0;
	mutex_lock(&hdmi_lock);
	__inf("hdmi_core_audio_config\n");

	glb_audio_para.type = audio_param->data_raw;
	glb_audio_para.sample_rate = audio_param->sample_rate;
	glb_audio_para.sample_bit = audio_param->sample_bit;
	glb_audio_para.ch_num = audio_param->channel_num;
	glb_audio_para.ca = audio_param->ca;

	mutex_unlock(&hdmi_lock);
	return ret;
}

u32 hdmi_core_get_video_mode(void)
{
	u32 ret;
	mutex_lock(&hdmi_lock);
	if (video_enable == 0)
		ret = 0;
	else
		ret =  glb_video_para.vic;
	mutex_unlock(&hdmi_lock);
	return ret;
}

s32 hdmi_core_set_video_mode(u32 vic)
{
	u32 ret = 0;

	__inf("hdmi_core_set_video_mode = %x\n",vic);
	mutex_lock(&hdmi_lock);
	glb_video_para.vic = vic;
	glb_audio_para.vic = vic;
	mutex_unlock(&hdmi_lock);

	return ret;
}

s32 hdmi_core_set_video_enable(bool enable)
{
	int ret = 0;

	mutex_lock(&hdmi_lock);
	__inf("hdmi_core_set_video_enable enable=%x, video_on=%d!\n",enable, video_on);
	if ((hdmi_state == HDMI_State_HPD_Done) && enable && (0 == video_on))
	{
		video_config(glb_video_para.vic);
		__inf("hdmi_core_set_video_enable, vic:%d,is_hdmi:%d,is_yuv:%d,is_hcts:%d\n",
			glb_video_para.vic, glb_video_para.is_hdmi,glb_video_para.is_yuv, glb_video_para.is_hcts);
		if (bsp_hdmi_video(&glb_video_para))
		{
			__wrn("set hdmi video error!\n");
			ret = -1;
			goto video_en_end;
		}

		hdmi_clk_enable_prepare();
		bsp_hdmi_set_video_en(enable);
		video_on = 1;

		if (((glb_audio_para.type != 1) && (true == audio_enable)) ||
			((glb_audio_para.type == 1) && (audio_cfged == true)) ) {
			if (audio_config_internal())
			{
				__wrn("set audio_config_internal error!\n");
				ret = -1;
				goto video_en_end;
			}
		}
	}
	else
	{
		if ((1 == video_on) && (!enable)) {
			bsp_hdmi_set_video_en(enable);
			hdmi_clk_disable_prepare();
		}
		video_on = 0;
	}

	video_enable = enable;

video_en_end:
	mutex_unlock(&hdmi_lock);
	return ret;
}

bool hdmi_core_get_video_enable(void)
{
	bool ret;
	mutex_lock(&hdmi_lock);
	ret = video_enable;
	mutex_unlock(&hdmi_lock);
	return ret;
}

s32 hdmi_core_get_list_num(void)
{
	return sizeof(video_timing)/sizeof(struct disp_video_timings);
}

static s32 video_config(u32 vic)
{
	int ret = 0;

	isHDMI = hdmi_edid_is_hdmi();
	YCbCr444_Support = hdmi_edid_is_yuv();

	__inf("video_config, vic:%d,cts_enable:%d,isHDMI:%d,YCbCr444_Support:%d,hdcp_enable:%d\n",
		vic,cts_enable,isHDMI,YCbCr444_Support,hdcp_enable);

	glb_video_para.vic = vic;
	if ((cts_enable==1) && (isHDMI == 0))
		glb_video_para.is_hdmi = 0;
	else
		glb_video_para.is_hdmi = 1;

	glb_video_para.is_yuv = hdmi_core_get_csc_type();

	if (hdcp_enable)
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

s32 hdmi_core_enter_lp(void)
{
	__inf("video enter lp\n");

	hdmi_state = HDMI_State_Idle;
	bsp_hdmi_standby();
	hdmi_para_reset();

	return 0;
}

s32 hdmi_core_exit_lp(void)
{
	bsp_hdmi_init();
	return 0;
}

s32 hdmi_core_mode_support(u32 mode)
{
	if (0 == hdmi_core_hpd_check())
		return 0;

	return Device_Support_VIC[mode];
}

s32 hdmi_core_dvi_support(void)
{
	if ((hdmi_edid_is_hdmi() == 0) && (hdmi_core_get_cts_enable() == 1))
		return 1;
	else
		return 0;
}

s32 hdmi_core_update_detect_time(u32 time_val)
{
	hdmi_detect_time = time_val;

	return 0;
}

int hdmi_core_cec_get_simple_msg(unsigned char *msg)
{
	int ret = -1;

	mutex_lock(&hdmi_lock);
	ret = bsp_hdmi_cec_get_simple_msg(msg);
	mutex_unlock(&hdmi_lock);

	return ret;
}
