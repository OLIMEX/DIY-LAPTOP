#include "hdmi_hal.h"
#include "../disp_sys_intf.h"

#define HDMI_IO_NUM 5
static bool hdmi_io_used[HDMI_IO_NUM]={0};
static disp_gpio_set_t hdmi_io[HDMI_IO_NUM];

static bool hdmi_used;
static bool bopen;
static char hdmi_power[25];
static u32  g_hdmi_mode;

extern disp_video_timings video_timing[];
void hdmi_delay_ms(__u32 ms)
{
#if defined(__LINUX_PLAT__)
	u32 timeout = ms*HZ/1000;

	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(timeout);
#elif defined(__UBOOT_PLAT__)
    __msdelay(ms);
#endif
	return ;
}

void hdmi_delay_us(unsigned long us)
{
#if defined(__LINUX_PLAT__)
	udelay(us);
#elif defined(__UBOOT_PLAT__)
    __usdelay(us);
#endif
	return ;
}

static int hdmi_parse_io_config(void)
{
	disp_gpio_set_t  *gpio_info;
	int i, ret;
	char io_name[32];

	for(i=0; i<HDMI_IO_NUM; i++) {
		gpio_info = &(hdmi_io[i]);
		sprintf(io_name, "hdmi_io_%d", i);
		ret = disp_sys_script_get_item("hdmi_para", io_name, (int *)gpio_info, sizeof(disp_gpio_set_t)/sizeof(int));
		if(ret == 3)
		  hdmi_io_used[i]= 1;
		else
			hdmi_io_used[i] = 0;
	}

  return 0;
}

static int hdmi_io_config(u32 bon)
{
	int hdl,i;

	for(i=0; i<HDMI_IO_NUM; i++)	{
		if(hdmi_io_used[i]) {
			disp_gpio_set_t  gpio_info[1];

			memcpy(gpio_info, &(hdmi_io[i]), sizeof(disp_gpio_set_t));
			if(!bon) {
				gpio_info->mul_sel = 7;
			}
			hdl = disp_sys_gpio_request(gpio_info, 1);
			disp_sys_gpio_release(hdl, 2);
		}
	}
	return 0;
}

static void hdmi_clk_config(u32 vic)
{
	int index = 0;

	index = Hdmi_hal_get_video_info(vic);
	disp_sys_clk_set_rate("hdmi", video_timing[index].pixel_clk);
}

__s32 Hdmi_open(void)
{
	__inf("[Hdmi_open]\n");

	hdmi_clk_config(g_hdmi_mode);
	Hdmi_hal_video_enable(1);
	//if(ghdmi.bopen == 0)
	//{
	//	up(run_sem);
	//}
	bopen = 1;
	return 0;
}

__s32 Hdmi_close(void)
{
	__inf("[Hdmi_close]\n");

	Hdmi_hal_video_enable(0);
	bopen = 0;
	return 0;
}

struct disp_hdmi_mode hdmi_mode_tbl[] = {
	{DISP_TV_MOD_480I,                HDMI1440_480I,     },
	{DISP_TV_MOD_576I,                HDMI1440_576I,     },
	{DISP_TV_MOD_480P,                HDMI480P,          },
	{DISP_TV_MOD_576P,                HDMI576P,          },
	{DISP_TV_MOD_720P_50HZ,           HDMI720P_50,       },
	{DISP_TV_MOD_720P_60HZ,           HDMI720P_60,       },
	{DISP_TV_MOD_1080I_50HZ,          HDMI1080I_50,      },
	{DISP_TV_MOD_1080I_60HZ,          HDMI1080I_60,      },
	{DISP_TV_MOD_1080P_24HZ,          HDMI1080P_24,      },
	{DISP_TV_MOD_1080P_50HZ,          HDMI1080P_50,      },
	{DISP_TV_MOD_1080P_60HZ,          HDMI1080P_60,      },
	{DISP_TV_MOD_1080P_25HZ,          HDMI1080P_25,      },
	{DISP_TV_MOD_1080P_30HZ,          HDMI1080P_30,      },
	{DISP_TV_MOD_1080P_24HZ_3D_FP,    HDMI1080P_24_3D_FP,},
	{DISP_TV_MOD_720P_50HZ_3D_FP,     HDMI720P_50_3D_FP, },
	{DISP_TV_MOD_720P_60HZ_3D_FP,     HDMI720P_60_3D_FP, },
	{DISP_TV_MOD_3840_2160P_30HZ,     HDMI3840_2160P_30, },
	//{DISP_TV_MOD_3840_2160P_24HZ,     HDMI3840_2160P_24, },
	{DISP_TV_MOD_3840_2160P_25HZ,     HDMI3840_2160P_25, },
};

__s32 Hdmi_set_display_mode(disp_tv_mode mode)
{
	__u32 hdmi_mode;
	__u32 i;
	bool find = false;

	__inf("[Hdmi_set_display_mode],mode:%d\n",mode);

	for(i=0; i<sizeof(hdmi_mode_tbl)/sizeof(struct disp_hdmi_mode); i++)
	{
		if(hdmi_mode_tbl[i].mode == mode) {
			hdmi_mode = hdmi_mode_tbl[i].hdmi_mode;
			find = true;
			break;
		}
	}

	if(find) {
		g_hdmi_mode = hdmi_mode;
		return Hdmi_hal_set_display_mode(hdmi_mode);
	} else {
		__wrn("unsupported video mode %d when set display mode\n", mode);
		return -1;
	}

}

#if 0
__s32 Hdmi_Audio_Enable(__u8 mode, __u8 channel)
{
	__inf("[Hdmi_Audio_Enable],ch:%d\n",channel);
	audio_enable = mode;
	return Hdmi_hal_audio_enable(mode, channel);
}

__s32 Hdmi_Set_Audio_Para(hdmi_audio_t * audio_para)
{
	__inf("[Hdmi_Set_Audio_Para]\n");
	return Hdmi_hal_set_audio_para(audio_para);
}
#endif

__s32 Hdmi_mode_support(disp_tv_mode mode)
{
	__u32 hdmi_mode;
	__u32 i;
	bool find = false;

	for(i=0; i<sizeof(hdmi_mode_tbl)/sizeof(struct disp_hdmi_mode); i++)
	{
		if(hdmi_mode_tbl[i].mode == mode) {
			hdmi_mode = hdmi_mode_tbl[i].hdmi_mode;
			find = true;
			break;
		}
	}

	if(find) {
		return Hdmi_hal_mode_support(hdmi_mode);
	} else {
		return 0;
	}
}

__s32 Hdmi_get_HPD_status(void)
{
	return Hdmi_hal_get_HPD();
}

#if 0
__s32 Hdmi_set_pll(__u32 pll, __u32 clk)
{
	Hdmi_hal_set_pll(pll, clk);
	return 0;
}

__s32 Hdmi_dvi_enable(__u32 mode)
{
	return Hdmi_hal_cts_enable(mode);//Hdmi_hal_dvi_enable(mode);
}

__s32 Hdmi_dvi_support(void)
{
	return Hdmi_hal_dvi_support();
}

__u32 Hdmi_hdcp_enable(__u32 hdcp_en)
{
	return Hdmi_hal_hdcp_enable(hdcp_en);
}

__s32 Hdmi_get_hdcp_enable(void)
{
	return Hdmi_hal_get_hdcp_enable();
}
#endif

__s32 Hdmi_get_video_timming_info(disp_video_timings **video_info)
{
	disp_video_timings *info;
	int ret = -1;
	int i, list_num;

	info = video_timing;
	list_num = Hdmi_hal_get_list_num();
	for(i=0; i<list_num; i++) {
		if(info->vic == g_hdmi_mode) {
			*video_info = info;
			ret = 0;
			break;
		}
		info ++;
	}
	return ret;
}

__s32 Hdmi_get_input_csc(void)
{
	return Hmdi_hal_get_input_csc();
}

int Hdmi_run_thread(void *parg)
{
	while (1) {

		Hdmi_hal_main_task();

		hdmi_delay_ms(200);
	}

	return 0;
}

extern s32 disp_set_hdmi_func(disp_hdmi_func * func);
__s32 Hdmi_init(void)
{
	s32 ret = 0;
	disp_hdmi_func disp_func;
	s32 val;

	hdmi_used = 0;

	ret = script_parser_fetch("hdmi_para", "hdmi_used", &val, 1);
	if(ret == 0) {
		hdmi_used = val;

		if(hdmi_used)
		{
			ret = script_parser_fetch("hdmi_para", "hdmi_power", (int *)hdmi_power, 25);
			if(ret == 0) {
				disp_sys_power_enable(hdmi_power);
			}
			ret = script_parser_fetch("hdmi_para", "hdmi_cts_compatibility", &val, 1);
			if(ret == 0) {
				Hdmi_hal_cts_enable(val);
			}
			ret = script_parser_fetch("hdmi_para", "hdmi_hdcp_enable", &val, 1);
			if(ret == 0) {
				Hdmi_hal_hdcp_enable(val);
			}
			disp_sys_clk_enable("hdmi");

			Hdmi_set_reg_base(HDMI_BASE);
			Hdmi_hal_init();
			hdmi_parse_io_config();
			hdmi_io_config(1);

			disp_func.hdmi_open = Hdmi_open;
			disp_func.hdmi_close = Hdmi_close;
			disp_func.hdmi_set_mode = Hdmi_set_display_mode;
			disp_func.hdmi_mode_support = Hdmi_mode_support;
			disp_func.hdmi_get_HPD_status = Hdmi_get_HPD_status;
			disp_func.hdmi_get_input_csc = Hdmi_get_input_csc;
			disp_func.hdmi_get_video_timing_info = Hdmi_get_video_timming_info;
			disp_set_hdmi_func(&disp_func);
		}
	}
	return 0;
}

__s32 Hdmi_exit(void)
{
	if(hdmi_used) {
		disp_sys_power_disable(hdmi_power);
		Hdmi_hal_exit();
	}

	return 0;
}
