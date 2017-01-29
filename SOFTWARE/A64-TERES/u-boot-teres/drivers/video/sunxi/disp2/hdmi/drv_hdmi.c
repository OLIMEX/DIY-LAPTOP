#include "drv_hdmi_i.h"
#include "hdmi_core.h"
#include "../disp/disp_sys_intf.h"

#define HDMI_IO_NUM 5
#define __CLK_SUPPORT__
//#define __CLK_OF__
static bool hdmi_io_used[HDMI_IO_NUM]={0};
static disp_gpio_set_t hdmi_io[HDMI_IO_NUM];
static u32 io_enable_count = 0;
#if defined(__LINUX_PLAT__)
static struct semaphore *run_sem = NULL;
static struct task_struct * HDMI_task;
#endif
static char hdmi_power[25];
static bool hdmi_power_used;
static bool hdmi_used;
static bool boot_hdmi = false;
#if defined(__CLK_SUPPORT__)
static struct clk *hdmi_clk = NULL;
static struct clk *hdmi_ddc_clk = NULL;
#endif
static u32 power_enable_count = 0;
static u32 clk_enable_count = 0;
static struct mutex mlock;
#if defined(CONFIG_SND_SUNXI_SOC_HDMIAUDIO)
static bool audio_enable = false;
#endif
static bool b_hdmi_suspend;
static bool b_hdmi_suspend_pre;

hdmi_info_t ghdmi;

void hdmi_delay_ms(unsigned long ms)
{
#if defined(__LINUX_PLAT__)
	u32 timeout = ms*HZ/1000;
	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(timeout);
#elif defined(__UBOOT_PLAT__)
	__msdelay(ms);
#endif
}

void hdmi_delay_us(unsigned long us)
{
#if defined(__LINUX_PLAT__)
	udelay(us);
#elif defined(__UBOOT_PLAT__)
	__usdelay(us);
#endif
}

unsigned int hdmi_get_soc_version(void)
{
    unsigned int version = 0;
#if defined(CONFIG_ARCH_SUN8IW7)
	unsigned int chip_ver = sunxi_get_soc_ver();

	switch (chip_ver) {
		case SUN8IW7P1_REV_A:
		case SUN8IW7P2_REV_A:
			version = 0;
			break;
		case SUN8IW7P1_REV_B:
		case SUN8IW7P2_REV_B:
			version = 1;
	}
#endif
	return version;
}

static int hdmi_parse_io_config(void)
{
	disp_gpio_set_t  *gpio_info;
	int i, ret;
	char io_name[32];

	for (i=0; i<HDMI_IO_NUM; i++) {
		gpio_info = &(hdmi_io[i]);
		sprintf(io_name, "hdmi_io_%d", i);
		ret = disp_sys_script_get_item("hdmi", io_name, (int *)gpio_info, sizeof(disp_gpio_set_t)/sizeof(int));
		if (ret == 3)
		  hdmi_io_used[i]= 1;
		else
			hdmi_io_used[i] = 0;
	}

  return 0;
}

static int hdmi_io_config(u32 bon)
{
	int hdl,i;

	for (i=0; i<HDMI_IO_NUM; i++)	{
		if (hdmi_io_used[i]) {
			disp_gpio_set_t  gpio_info[1];

			memcpy(gpio_info, &(hdmi_io[i]), sizeof(disp_gpio_set_t));
			if (!bon) {
				gpio_info->mul_sel = 7;
			}
			hdl = disp_sys_gpio_request(gpio_info, 1);
			disp_sys_gpio_release(hdl, 2);
		}
	}
	return 0;
}
#if defined(__CLK_SUPPORT__)
static int hdmi_clk_enable(void)
{
	int ret = 0;

	if (hdmi_clk)
		ret = clk_prepare_enable(hdmi_clk);
	if (0 != ret) {
		__wrn("enable hdmi clk fail\n");
		return ret;
	}

	if (hdmi_ddc_clk)
		ret = clk_prepare_enable(hdmi_ddc_clk);

	if (0 != ret) {
		__wrn("enable hdmi ddc clk fail\n");
		clk_disable(hdmi_clk);
	}

	return ret;
}

static int hdmi_clk_disable(void)
{
	if (hdmi_clk)
		clk_disable(hdmi_clk);
	if (hdmi_ddc_clk)
		clk_disable(hdmi_ddc_clk);

	return 0;
}

static int hdmi_clk_config(u32 vic)
{
	int index = 0;

	index = hdmi_core_get_video_info(vic);
	if (hdmi_clk)
		clk_set_rate(hdmi_clk, video_timing[index].pixel_clk);

	return 0;
}
#else
static int hdmi_clk_enable(void){return 0;}
static int hdmi_clk_disable(void){return 0;}
static int hdmi_clk_config(u32 vic){return 0;}
#endif

static int hdmi_power_enable(char *name)
{
	return disp_sys_power_enable(name);
}

static int hdmi_power_disable(char *name)
{
	return disp_sys_power_disable(name);
}

static s32 hdmi_enable(void)
{
	__inf("[hdmi_enable]\n");

	mutex_lock(&mlock);
	if (1 != ghdmi.bopen) {
#if defined(__UBOOT_PLAT__)
		/* if force output hdmi when hdmi not plug in
			then detect one more time
		*/
		if (!hdmi_core_hpd_check())
			hdmi_core_loop();
#endif
		hdmi_clk_config(ghdmi.mode);
		hdmi_core_set_video_enable(1);
		ghdmi.bopen = 1;
	}
	mutex_unlock(&mlock);
	return 0;
}

static s32 hdmi_disable(void)
{
	__inf("[hdmi_disable]\n");

	mutex_lock(&mlock);
	if (0 != ghdmi.bopen) {
		hdmi_core_set_video_enable(0);
		ghdmi.bopen = 0;
	}
	mutex_unlock(&mlock);
	return 0;
}

static struct disp_hdmi_mode hdmi_mode_tbl[] = {
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
	{DISP_TV_MOD_3840_2160P_25HZ,     HDMI3840_2160P_25, },
};

u32 hdmi_get_vic(u32 mode)
{
	u32 hdmi_mode = DISP_TV_MOD_720P_50HZ;
	u32 i;
	bool find = false;

	for (i=0; i<sizeof(hdmi_mode_tbl)/sizeof(struct disp_hdmi_mode); i++)
	{
		if (hdmi_mode_tbl[i].mode == mode) {
			hdmi_mode = hdmi_mode_tbl[i].hdmi_mode;
			find = true;
			break;
		}
	}

	if (false == find)
		__wrn("[HDMI]can't find vic for mode(%d)\n", mode);

	return hdmi_mode;
}

static s32 hdmi_set_display_mode(u32 mode)
{
	u32 hdmi_mode;
	u32 i;
	bool find = false;

	__inf("[hdmi_set_display_mode],mode:%d\n",mode);

	for (i=0; i<sizeof(hdmi_mode_tbl)/sizeof(struct disp_hdmi_mode); i++)
	{
		if (hdmi_mode_tbl[i].mode == (enum disp_tv_mode)mode) {
			hdmi_mode = hdmi_mode_tbl[i].hdmi_mode;
			find = true;
			break;
		}
	}

	if (find) {
		ghdmi.mode = hdmi_mode;
		return hdmi_core_set_video_mode(hdmi_mode);
	} else {
		__wrn("unsupported video mode %d when set display mode\n", mode);
		return -1;
	}

}

#if defined(CONFIG_SND_SUNXI_SOC_HDMIAUDIO)
static s32 hdmi_audio_enable(u8 mode, u8 channel)
{
	__inf("[hdmi_audio_enable],mode:%d,ch:%d\n",mode, channel);
	mutex_lock(&mlock);
	audio_enable = mode;
	mutex_unlock(&mlock);
	return hdmi_core_set_audio_enable(audio_enable);
}

static s32 hdmi_set_audio_para(hdmi_audio_t * audio_para)
{
	__inf("[hdmi_set_audio_para]\n");
	return hdmi_core_audio_config(audio_para);
}
#endif

static s32 hdmi_mode_support(u32 mode)
{
	u32 hdmi_mode;
	u32 i;
	bool find = false;

	for (i=0; i<sizeof(hdmi_mode_tbl)/sizeof(struct disp_hdmi_mode); i++)
	{
		if (hdmi_mode_tbl[i].mode == (enum disp_tv_mode)mode) {
			hdmi_mode = hdmi_mode_tbl[i].hdmi_mode;
			find = true;
			break;
		}
	}

	if (find) {
		return hdmi_core_mode_support(hdmi_mode);
	} else {
		return 0;
	}
}

static s32 hdmi_get_HPD_status(void)
{
#if defined(__UBOOT_PLAT__)
	/* check hw status once when get hpd status
		no need on linux plat while there's detect thread
	*/
	hdmi_core_loop();
#endif
	return hdmi_core_hpd_check();
}

#if defined(__LINUX_PLAT__)
static s32 hdmi_get_hdcp_enable(void)
{
	return hdmi_core_get_hdcp_enable();
}
#endif

static s32 hdmi_get_video_timming_info(struct disp_video_timings **video_info)
{
	struct disp_video_timings *info;
	int ret = -1;
	int i, list_num;

	info = video_timing;
	list_num = hdmi_core_get_list_num();
	for (i=0; i<list_num; i++) {
		if (info->vic == ghdmi.mode) {
			*video_info = info;
			ret = 0;
			break;
		}
		info ++;
	}
	return ret;
}

static s32 hdmi_get_input_csc(void)
{
	return hdmi_core_get_csc_type();
}

#if defined(__LINUX_PLAT__)
static int hdmi_run_thread(void *parg)
{
	while (1) {
		if (kthread_should_stop()) {
			break;
		}

		mutex_lock(&mlock);
		if (false == b_hdmi_suspend) {
			/* normal state */
			b_hdmi_suspend_pre = b_hdmi_suspend;
			mutex_unlock(&mlock);
			hdmi_core_loop();

			if (false == b_hdmi_suspend) {
				if (hdmi_get_hdcp_enable()==1)
					hdmi_delay_ms(100);
				else
					hdmi_delay_ms(200);
			}
		} else {
			/* suspend state */
			if (false == b_hdmi_suspend_pre) {
				/* first time after enter suspend state */
				//hdmi_core_enter_lp();
			}
			b_hdmi_suspend_pre = b_hdmi_suspend;
			mutex_unlock(&mlock);
		}
	}

	return 0;
}

#if defined(CONFIG_SWITCH) || defined(CONFIG_ANDROID_SWITCH)
static struct switch_dev hdmi_switch_dev = {
	.name = "hdmi",
};

s32 disp_set_hdmi_detect(bool hpd);
static void hdmi_report_hpd_work(struct work_struct *work)
{
	if (hdmi_get_HPD_status())	{
		if (hdmi_switch_dev.dev)
			switch_set_state(&hdmi_switch_dev, 1);
		disp_set_hdmi_detect(1);
		__inf("switch_set_state 1\n");
	}	else {
		if (hdmi_switch_dev.dev)
			switch_set_state(&hdmi_switch_dev, 0);
		disp_set_hdmi_detect(0);
		__inf("switch_set_state 0\n");
	}
}

s32 hdmi_hpd_state(u32 state)
{
	if (state == 0) {
		if (hdmi_switch_dev.dev)
			switch_set_state(&hdmi_switch_dev, 0);
	} else {
		if (hdmi_switch_dev.dev)
			switch_set_state(&hdmi_switch_dev, 1);
	}

	return 0;
}
#else
static void hdmi_report_hpd_work(struct work_struct *work)
{
}

s32 hdmi_hpd_state(u32 state)
{
	return 0;
}
#endif
#endif
/**
 * hdmi_hpd_report - report hdmi hot plug state to user space
 * @hotplug:	0: hdmi plug out;   1:hdmi plug in
 *
 * always return success.
 */
s32 hdmi_hpd_event(void)
{
#if defined(__LINUX_PLAT__)
	schedule_work(&ghdmi.hpd_work);
#endif
	return 0;
}

static s32 hdmi_suspend(void)
{
#if defined(__LINUX_PLAT__)
	hdmi_core_update_detect_time(0);
	mutex_lock(&mlock);
	if (hdmi_used && (false == b_hdmi_suspend)) {
		b_hdmi_suspend = true;
		if (HDMI_task) {
			kthread_stop(HDMI_task);
			HDMI_task = NULL;
		}
		hdmi_core_enter_lp();
		if (0 != clk_enable_count) {
			hdmi_clk_disable();
			clk_enable_count --;
		}
		if (0 != io_enable_count) {
			hdmi_io_config(0);
			io_enable_count --;
		}
		if ((hdmi_power_used) && (0 != power_enable_count)) {
			hdmi_power_disable(hdmi_power);
			power_enable_count --;
		}
		__wrn("[HDMI]hdmi suspend\n");
	}
	mutex_unlock(&mlock);
#endif
	return 0;
}

static s32 hdmi_resume(void)
{
	int ret = 0;
#if defined(__LINUX_PLAT__)
	mutex_lock(&mlock);
	if (hdmi_used && (true == b_hdmi_suspend)) {
		/* normal state */
		if (clk_enable_count == 0) {
			ret = hdmi_clk_enable();
			if (0 == ret)
				clk_enable_count ++;
			else {
				pr_warn("fail to enable hdmi's clock\n");
				goto exit;
			}
		}
		if ((hdmi_power_used) && (power_enable_count == 0)) {
			hdmi_power_enable(hdmi_power);
			power_enable_count ++;
		}
		if (io_enable_count == 0) {
			hdmi_io_config(1);
			io_enable_count ++;
		}
		/* first time after exit suspend state */
		hdmi_core_exit_lp();

		HDMI_task = kthread_create(hdmi_run_thread, (void*)0, "hdmi proc");
		if (IS_ERR(HDMI_task)) {
			s32 err = 0;
			pr_warn("Unable to start kernel thread %s.\n\n", "hdmi proc");
			err = PTR_ERR(HDMI_task);
			HDMI_task = NULL;
		} else
			wake_up_process(HDMI_task);

		__wrn("[HDMI]hdmi resume\n");
	}

exit:
	mutex_unlock(&mlock);

	hdmi_core_update_detect_time(200);//200ms
	b_hdmi_suspend = false;
#endif
	return  ret;
}

#if defined(CONFIG_SND_SUNXI_SOC_HDMIAUDIO)
extern void audio_set_hdmi_func(__audio_hdmi_func * hdmi_func);
#endif
extern s32 disp_set_hdmi_func(struct disp_device_func * func);
extern unsigned int disp_boot_para_parse(void);

#if defined(__LINUX_PLAT__)
s32 hdmi_init(struct platform_device *pdev)
#else
s32 hdmi_init(void)
#endif
{
#if defined(CONFIG_SND_SUNXI_SOC_HDMIAUDIO)
	__audio_hdmi_func audio_func;
#if defined (CONFIG_SND_SUNXI_SOC_AUDIOHUB_INTERFACE)
	__audio_hdmi_func audio_func_muti;
#endif
#endif
#if defined(__LINUX_PLAT__)
	unsigned int output_type0, output_mode0, output_type1, output_mode1;
#endif
	struct disp_device_func disp_func;
	int ret = 0;
	uintptr_t reg_base;
	int value;

	hdmi_used = 0;
	b_hdmi_suspend_pre = b_hdmi_suspend = false;
	hdmi_power_used = 0;
	hdmi_used = 1;
#if defined(__LINUX_PLAT__)
	/*  parse boot para */
	value = disp_boot_para_parse();
	output_type0 = (value >> 8) & 0xff;
	output_mode0 = (value) & 0xff;

	output_type1 = (value >> 24)& 0xff;
	output_mode1 = (value >> 16) & 0xff;
	if ((output_type0 == DISP_OUTPUT_TYPE_HDMI) ||
		(output_type1 == DISP_OUTPUT_TYPE_HDMI)) {
		boot_hdmi = true;
		ghdmi.bopen = 1;
		ghdmi.mode = (output_type0 == DISP_OUTPUT_TYPE_HDMI)?output_mode0:output_mode1;
		ghdmi.mode = hdmi_get_vic(ghdmi.mode);
	}
#endif
	/* iomap */
	reg_base = disp_getprop_regbase("hdmi", "reg", 0);
	if (0 == reg_base) {
		__wrn("unable to map hdmi registers\n");
		ret = -EINVAL;
		goto err_iomap;
	}
	hdmi_core_set_base_addr(reg_base);

#if defined(__CLK_OF__)
	/* get clk */
	hdmi_clk = of_clk_get(pdev->dev.of_node, 0);
	if (IS_ERR(hdmi_clk)) {
		__wrn("fail to get clk for hdmi\n");
		goto err_clk_get;
	}
	clk_enable_count = hdmi_clk->enable_count;
	hdmi_ddc_clk = of_clk_get(pdev->dev.of_node, 1);
	if (IS_ERR(hdmi_ddc_clk)) {
		__wrn("fail to get clk for hdmi ddc\n");
		goto err_clk_get;
	}
#else
	hdmi_clk = clk_get(NULL, "hdmi");
	if (IS_ERR(hdmi_clk)) {
		__wrn("fail to get clk for hdmi\n");
		goto err_clk_get;
	}
	hdmi_ddc_clk = clk_get(NULL, "hdmi_slow");
	if (IS_ERR(hdmi_ddc_clk)) {
		__wrn("fail to get clk for hdmi_ddc_clk\n");
		goto err_clk_get;
	}
#endif
	/* parse io config */
	hdmi_parse_io_config();
	mutex_init(&mlock);

	if ((hdmi_power_used) && (power_enable_count == 0)) {
		hdmi_power_enable(hdmi_power);
		power_enable_count ++;
	}
	if (io_enable_count == 0) {
		hdmi_io_config(1);
		io_enable_count ++;
	}
	mutex_lock(&mlock);
	if (clk_enable_count == 0) {
		ret = hdmi_clk_enable();
		clk_enable_count ++;
	}
	mutex_unlock(&mlock);
	if (0 != ret) {
		clk_enable_count --;
		__wrn("fail to enable hdmi clk\n");
		goto err_clk_enable;
	}

#if defined(__LINUX_PLAT__)
	INIT_WORK(&ghdmi.hpd_work, hdmi_report_hpd_work);
#endif
#if defined(CONFIG_SWITCH) || defined(CONFIG_ANDROID_SWITCH)
	switch_dev_register(&hdmi_switch_dev);
#endif

	ret = disp_sys_script_get_item("hdmi", "hdmi_power", (int*)hdmi_power, 2);
	if (2 == ret) {
		hdmi_power_used = 1;
		if (hdmi_power_used) {
			__inf("[HDMI] power %s\n", hdmi_power);
			mutex_lock(&mlock);
			ret = hdmi_power_enable(hdmi_power);
			power_enable_count ++;
			mutex_unlock(&mlock);
			if (0 != ret) {
				power_enable_count --;
				__wrn("fail to enable hdmi power %s\n", hdmi_power);
				goto err_power;
			}
		}
	}

	ret = disp_sys_script_get_item("hdmi", "hdmi_cts_compatibility", &value, 1);
	if (1 == ret) {
		hdmi_core_set_cts_enable(value);
	}
	ret = disp_sys_script_get_item("hdmi", "hdmi_hdcp_enable", &value, 1);
	if (1 == ret) {
		hdmi_core_set_hdcp_enable(value);
	}

	ret = disp_sys_script_get_item("hdmi", "hdmi_hpd_mask", &value, 1);
	if (1 == ret) {
		hdmi_hpd_mask = value;
	}

#if defined(__UBOOT_PLAT__)
	hdmi_core_update_detect_time(10);
#endif
	hdmi_core_initial(boot_hdmi);

#if defined(__LINUX_PLAT__)
	run_sem = kmalloc(sizeof(struct semaphore),GFP_KERNEL | __GFP_ZERO);
	if (!run_sem) {
		__wrn("fail to kmalloc memory for run_sem\n");
		goto err_sem;
	}
	sema_init((struct semaphore*)run_sem,0);

	HDMI_task = kthread_create(hdmi_run_thread, (void*)0, "hdmi proc");
	if (IS_ERR(HDMI_task)) {
		s32 err = 0;
		__wrn"Unable to start kernel thread %s.\n\n", "hdmi proc");
		err = PTR_ERR(HDMI_task);
		HDMI_task = NULL;
		goto err_thread;
	}
	wake_up_process(HDMI_task);
#endif

#if defined(CONFIG_SND_SUNXI_SOC_HDMIAUDIO)
	audio_func.hdmi_audio_enable = hdmi_audio_enable;
	audio_func.hdmi_set_audio_para = hdmi_set_audio_para;
	audio_set_hdmi_func(&audio_func);
#if defined (CONFIG_SND_SUNXI_SOC_AUDIOHUB_INTERFACE)
	audio_func_muti.hdmi_audio_enable = hdmi_audio_enable;
	audio_func_muti.hdmi_set_audio_para = hdmi_set_audio_para;
	audio_set_muti_hdmi_func(&audio_func_muti);
#endif
#endif
	memset(&disp_func, 0, sizeof(struct disp_device_func));
	disp_func.enable = hdmi_enable;
	disp_func.disable = hdmi_disable;
	disp_func.set_mode = hdmi_set_display_mode;
	disp_func.mode_support = hdmi_mode_support;
	disp_func.get_HPD_status = hdmi_get_HPD_status;
	disp_func.get_input_csc = hdmi_get_input_csc;
	disp_func.get_video_timing_info = hdmi_get_video_timming_info;
	disp_func.suspend = hdmi_suspend;
	disp_func.resume = hdmi_resume;
	disp_set_hdmi_func(&disp_func);

	return 0;

#if defined(__LINUX_PLAT__)
err_thread:
	kfree(run_sem);
err_sem:
#endif
	hdmi_power_disable(hdmi_power);
err_power:
	hdmi_clk_disable();
err_clk_enable:
#if defined(__CLK_SUPPORT__)
err_clk_get:
#endif
err_iomap:
	return -1;
}

s32 hdmi_exit(void)
{
	if (hdmi_used) {
		hdmi_core_exit();
#if defined(__LINUX_PLAT__)
		if (run_sem)	{
			kfree(run_sem);
		}

		run_sem = NULL;
		if (HDMI_task) {
			kthread_stop(HDMI_task);
			HDMI_task = NULL;
		}
#endif
		if ((1 == hdmi_power_used) && (0 != power_enable_count)) {
			hdmi_power_disable(hdmi_power);
		}
		if (0 != clk_enable_count) {
			hdmi_clk_disable();
			clk_enable_count--;
		}
	}

	return 0;
}
