#include "drv_tv_i.h"

#define TV_SOURCE 	"pll_de"
#define TVE_CLK	 	"tve"
#define 	SCREEN_COUNT 1

extern s32 disp_set_tv_func(disp_tv_func * func);
static bool g_used;
bool g_dac_used;
bool g_suspend;
tv_info_t g_tv_info;

static void tve_clk_enable(u32 sel);
static void tve_clk_init(u32 sel);
static void tve_clk_disable(u32 sel);
static void tve_clk_config(u32 sel, u32 tv_mode);
extern s32 disp_set_tv_func(disp_tv_func * func);

static disp_video_timings video_timing[] =
{
	/*vic         tv_mode          PCLK   AVI    x     y    HT  HBP HFP HST VT VBP VFP VST*/
	//{0,	DISP_TV_MOD_PAL,27000000, 0, 720, 576, 864, 137, 3,   2,  625, 20, 25, 2, 0, 0,1, 0, 0},
	//{0,	DISP_TV_MOD_NTSC,13500000, 0,  720,   480,   858,   57,   19,   62,  525,   15,  4,  3,  0,   0,   1,   0,   0},
	{0,   DISP_TV_MOD_NTSC,27000000,  0,  720,   480,   858,   60,   16,   62,  525,   30,  9,  6,  0,   0,   0,   0,   0},
	{0,   DISP_TV_MOD_PAL  ,27000000,  0,  720,   576,   864,   68,   12,   64,  625,   39,  5,  5,  0,   0,   0,   0,   0},
};

s32 tv_get_dac_hpd(u32 sel)
{
	u8 dac[3] = {0};
	s32 i = 0;
	u32  ret = DISP_TV_NONE;

	dac[i] = tve_low_get_dac_status(sel);
       if(dac[i]>1)
            dac[i] = 0;
       if(g_tv_info.screen[sel].dac_source[i] == DISP_TV_DAC_SRC_COMPOSITE && dac[i] == 1) {
		ret = DISP_TV_CVBS;
       }
	return  ret;
}

s32 tv_get_video_info(s32 mode)
{
	s32 i,count;
	count = sizeof(video_timing)/sizeof(disp_video_timings);
	for(i=0;i<count;i++) {
		if(mode == video_timing[i].tv_mode)
			return i;
	}
	return -1;
}

s32 tv_get_list_num(void)
{
	return sizeof(video_timing)/sizeof(disp_video_timings);

}

s32 tv_init(void)
{
	disp_tv_func disp_func;
	s32 i = 0;
	s32   val;
	s32  type;
	char sub_key[20];
	u32 cail = 0x200;
	g_suspend = 0;
	memset(&g_tv_info, 0, sizeof(tv_info_t));
	type = disp_sys_script_get_item("tv_para", "tv_used", &val, 1);
	if(1 == type)
		g_used = val;
	if(g_used) {
		g_tv_info.screen[0].base_address = 0x01e00000;
		for(i=0; i <SCREEN_COUNT; i++) {
			tve_low_set_reg_base(i, g_tv_info.screen[i].base_address);
	    		tve_clk_init(i);
			tve_clk_enable(i);
			tve_low_init(i, cail);
		}
		g_tv_info.screen[0].tv_mode = DISP_TV_MOD_PAL;
		g_tv_info.screen[1].tv_mode = DISP_TV_MOD_PAL;

		type = disp_sys_script_get_item("tv_para", "tv_dac_used", &val, 1);
		if(1 == type)
			g_dac_used = val;
		if(g_dac_used) {
			g_tv_info.dac_count = 1;
			for(i=0; i<4; i++) {
		           	type = disp_sys_script_get_item("tv_para", sub_key, &val, 1);
	            		if(1 == type) {
					g_tv_info.screen[0].dac_source[i] = val;
					g_tv_info.screen[1].dac_source[i] = val;
					g_tv_info.dac_count++;
		            	}
				else
					break;
			}
		}
		tve_low_dac_autocheck_enable(1, 0);
		disp_func.tv_enable = tv_enable;
		disp_func.tv_disable = tv_disable;
		disp_func.tv_resume = tv_resume;
		disp_func.tv_suspend = tv_suspend;
		disp_func.tv_get_mode = tv_get_mode;
		disp_func.tv_set_mode = tv_set_mode;
		disp_func.tv_get_video_timing_info = tv_get_video_timing_info;
		disp_func.tv_get_input_csc = tv_get_input_csc;
		disp_func.tv_mode_support = tv_mode_support;
		disp_func.tv_get_dac_hpd = tv_get_dac_hpd;				//modify add disp_dunc
		disp_set_tv_func(&disp_func);
	}
		return 0;
}

s32 tv_exit(void)
{
	s32 sel;
	for(sel=0; sel <SCREEN_COUNT; sel++) {
		tv_disable(sel);
		tve_low_exit(sel);
	}
	return 0;
}


s32 tv_get_mode(u32 sel)
{
	 //add get mode from gtv

	 return g_tv_info.screen[sel].tv_mode;
}

s32 tv_set_mode(u32 sel, disp_tv_mode tv_mode)
{
	if(tv_mode >= DISP_TV_MODE_NUM) {
      		return -1;
    	}
	g_tv_info.screen[sel].tv_mode = tv_mode;
	return  0;
}

s32 tv_get_input_csc(void)
{
	return 1;   	//support yuv only
}

s32 tv_get_video_timing_info(u32 sel, disp_video_timings **video_info)
{
	disp_video_timings *info;
	int ret = -1;
	int i, list_num;
	info = video_timing;
	list_num = tv_get_list_num();
	for(i=0; i<list_num; i++) {
		if(info->tv_mode == g_tv_info.screen[sel].tv_mode){
			*video_info = info;
			ret = 0;
			break;
		}
		info ++;
	}
	return ret;
}


s32 tv_enable(u32 sel)
{
	if(!g_tv_info.enable) {
		tve_clk_config(sel, g_tv_info.screen[sel].tv_mode);
		tve_low_set_tv_mode(sel, g_tv_info.screen[sel].tv_mode);
		tve_low_dac_cfg(sel, g_tv_info.screen[sel].tv_mode);
		tve_low_open(sel);
		g_tv_info.enable = 1;
	}
	tv_low_print_base_reg();
	return 0;
}

s32 tv_disable(u32 sel)
{
	u32 i;
	if(g_tv_info.enable) {
		for(i=0; i<g_tv_info.dac_count; i++) {
			tve_low_dac_disable(sel, i);
		}
		tve_low_close(sel);
		g_tv_info.enable = 0;
	}
	return 0;
}

s32 tv_suspend(void)
{
	if(g_used && (0 == g_suspend)) {
		g_suspend = true;
		tve_clk_disable(1);
	}

	return 0;
}

s32 tv_resume(void)
{
	if(g_used && (1 == g_suspend)) {
		g_suspend= false;
		tve_clk_enable(1);
	}
	return  0;
}

s32 tv_mode_support(disp_tv_mode mode)
{
	u32 i, list_num;
	disp_video_timings *info;


	info = video_timing;
	list_num = tv_get_list_num();
	for(i=0; i<list_num; i++) {
		if(info->tv_mode == mode) {
			return 1;
		}
		info ++;
	}
	return 0;
}

static void tve_clk_init(u32 sel)
{
	char clk[20] = {0};

	if(SCREEN_COUNT <= sel)
		sel = sel-1;
	sprintf(clk, "tve%d", sel);
	disp_sys_clk_set_parent("tve", TV_SOURCE);
}


static void tve_clk_enable(u32 sel)
{
	char clk[20] = {0};

	if(SCREEN_COUNT <= sel)
		sel = sel-1;
	sprintf(clk, "tve%d", sel);
	disp_sys_clk_enable("tve");
}

static void tve_clk_disable(u32 sel)
{
	char clk[20] = {0};

	if(SCREEN_COUNT <= sel)
		sel = sel-1;
	sprintf(clk, "tve%d", sel);
	disp_sys_clk_disable("tve");
}

static void tve_clk_config(u32 sel, u32 tv_mode)
{
#if 0
	int index = 0;
	char clk[20] = {0};
	disp_video_timings *pinfo;
	if(SCREEN_COUNT <= sel)
		sel = sel-1;
	pinfo = video_timing;
	sprintf(clk, "tve%d", sel);
	index = tv_get_video_info(tv_mode);			//modify tv_mode
#endif
	disp_sys_clk_set_rate("tve", 216000000);
}

