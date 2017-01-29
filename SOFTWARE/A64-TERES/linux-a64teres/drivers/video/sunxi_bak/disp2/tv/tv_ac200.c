#include "tv_ac200.h"
#include "tv_ac200_lowlevel.h"
#include <linux/switch.h>
#include <linux/clk-private.h>

/* clk */
#define DE_LCD_CLK "lcd0"
#define DE_LCD_CLK_SRC "pll_video0"
#define CLK_INIT_ENABLE_COUNT 2

static struct clk *tv_clk = NULL;
static char modules_name[32] = "tv_ac200";
static char key_name[20] = "tv_ac200_para";
static enum disp_tv_mode g_tv_mode = DISP_TV_MOD_PAL;

static u32 tv_screen_id = 0;
static u32 tv_used = 0;

static struct mutex mlock;
static bool tv_suspend_status;
static unsigned int  tv_clk_enable_count = 0;

static bool tv_io_used[28];
static disp_gpio_set_t tv_io[28];

static struct disp_device *tv_device = NULL;
static struct disp_vdevice_source_ops tv_source_ops;

struct ac200_tv_priv tv_priv;
struct disp_video_timings tv_video_timing[] =
{
 /* vic  tv_mode         PCLK     AVI   x   y   HT  HBP HFP HST  VT  VBP VFP VST  H_P V_P I vas TRD */	
 
 	{0, DISP_TV_MOD_NTSC,54000000, 0, 720, 480, 858, 57, 19, 62, 525, 15, 4,  3,  0,  0,  0, 0, 0},
	{0,	DISP_TV_MOD_PAL, 54000000, 0, 720, 576, 864, 69, 12, 63, 625, 19, 2,  3,  0,  0,  0, 0, 0},
};

extern struct disp_device* disp_vdevice_register(struct disp_vdevice_init_data *data);
extern s32 disp_vdevice_unregister(struct disp_device *vdevice);
extern s32 disp_vdevice_get_source_ops(struct disp_vdevice_source_ops *ops);
extern unsigned int disp_boot_para_parse(void);

#if defined(CONFIG_SWITCH) || defined(CONFIG_ANDROID_SWITCH)

static struct task_struct * tv_hpd_task;
static u32 tv_hpd_sourc = 0;

static struct switch_dev cvbs_switch_dev = {
	.name = "cvbs",
};

void tv_report_hpd_work(void)
{
	switch(tv_hpd_sourc) {

	case DISP_TV_NONE:
		switch_set_state(&cvbs_switch_dev, STATUE_CLOSE);
		break;

	case DISP_TV_CVBS:
		switch_set_state(&cvbs_switch_dev, STATUE_OPEN);
		break;
		
	default:
		switch_set_state(&cvbs_switch_dev, STATUE_CLOSE);

		break;
	}
}


s32 tv_detect_thread(void *parg)
{
	s32 hpd;
	while(1) {
		if(kthread_should_stop()) {
			break;
		}
		if(!tv_suspend_status) {
				hpd = aw1683_tve_plug_status();
			if(hpd != tv_hpd_sourc) {
				tv_hpd_sourc = hpd;
				tv_report_hpd_work();
			}
		}
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(20);
	}
	return 0;
}

s32 tv_detect_enable(void)
{
	tv_hpd_task = kthread_create(tv_detect_thread, (void*)0, "tve detect");
	if(IS_ERR_OR_NULL(tv_hpd_task)) {
		s32 err = 0;
		err = PTR_ERR(tv_hpd_task);
		tv_hpd_task = NULL;
		return err;
	}
	else {
		pr_debug("tv_hpd_task is ok!\n");
	}
	wake_up_process(tv_hpd_task);
	return 0;
}

s32 tv_detect_disable(void)
{
	if(tv_hpd_task) {
		kthread_stop(tv_hpd_task);
		tv_hpd_task = NULL;
	}
	return 0;
}
#else
void tv_report_hpd_work(void)
{
	pr_debug("there is null report hpd work,you need support the switch class!");
}

s32 tv_detect_thread(void *parg)
{
	pr_debug("there is null tv_detect_thread,you need support the switch class!");
	return -1;
}

s32 tv_detect_enable(void)
{
	pr_debug("there is null tv_detect_enable,you need support the switch class!");
	return -1;
}

s32 tv_detect_disable(void)
{
	pr_debug("there is null tv_detect_disable,you need support the switch class!");
    	return -1;
}
#endif

#if 0
static s32 tv_power_on(u32 on_off)
{
	if(tv_power_used == 0)
	{
		return 0;
	}
    if(on_off)
    {
        disp_sys_power_enable(tv_power);
    }
    else
    {
        disp_sys_power_disable(tv_power);
    }

    return 0;
}
#endif

static s32 tv_clk_init(void)
{
	disp_sys_clk_set_parent(DE_LCD_CLK, DE_LCD_CLK_SRC);

	return 0;
}

static s32 tv_clk_exit(void)
{

	return 0;
}

static s32 tv_clk_config(u32 mode)
{
	unsigned long pixel_clk, pll_rate, lcd_rate, dclk_rate;//hz
	unsigned long pll_rate_set, lcd_rate_set, dclk_rate_set;//hz
	u32 pixel_repeat, tcon_div, lcd_div;

	if(11 == mode) {
		pixel_clk = tv_video_timing[1].pixel_clk;
		pixel_repeat = tv_video_timing[1].pixel_repeat;
	}
	else {
		pixel_clk = tv_video_timing[0].pixel_clk;
		pixel_repeat = tv_video_timing[0].pixel_repeat;
	}
	lcd_div = 1;
	dclk_rate = pixel_clk * (pixel_repeat + 1);
	tcon_div = 8;//fixme
	lcd_rate = dclk_rate * tcon_div;
	pll_rate = lcd_rate * lcd_div;
	
	disp_sys_clk_set_rate(DE_LCD_CLK_SRC, pll_rate);
	pll_rate_set = disp_sys_clk_get_rate(DE_LCD_CLK_SRC);
	lcd_rate_set = pll_rate_set / lcd_div;
	disp_sys_clk_set_rate(DE_LCD_CLK, lcd_rate_set);
	lcd_rate_set = disp_sys_clk_get_rate(DE_LCD_CLK_SRC);
	dclk_rate_set = lcd_rate_set / tcon_div;
	if(dclk_rate_set != dclk_rate)
		pr_info("pclk=%ld, cur=%ld\n", dclk_rate, dclk_rate_set);

	return 0;
}

static s32 tv_clk_enable(u32 mode)
{
	int ret = 0;

	tv_clk_config(mode);
	if (tv_clk)
		ret = clk_prepare_enable(tv_clk);

	return ret;
}

static s32 tv_clk_disable(void)
{
	if (tv_clk)
		clk_disable(tv_clk);

	return 0;
}

static int tv_parse_config(void)
{
	disp_gpio_set_t  *gpio_info;
	int i, ret;
	char io_name[32];

	for(i=0; i<28; i++) {
		gpio_info = &(tv_io[i]);
		sprintf(io_name, "tv_d%d", i);
		ret = disp_sys_script_get_item(key_name, io_name, (int *)gpio_info,
										sizeof(disp_gpio_set_t)/sizeof(int));
		if(ret == 3) {
		  tv_io_used[i]= 1;
		}
	}

  return 0;
}

static int tv_pin_config(u32 bon)
{
	int hdl,i;

	for(i=0; i<28; i++)	{
		if(tv_io_used[i]) {
			disp_gpio_set_t  gpio_info[1];

			memcpy(gpio_info, &(tv_io[i]), sizeof(disp_gpio_set_t));
			if(!bon) {
				gpio_info->mul_sel = 7;
			}
			hdl = disp_sys_gpio_request(gpio_info, 1);
			disp_sys_gpio_release(hdl, 2);
		}
	}
	return 0;
}

static s32 tv_open(void)
{
	tv_pin_config(1);
	mdelay(550);
	if(tv_source_ops.tcon_enable){
    	tv_source_ops.tcon_enable(tv_device);
		mutex_lock(&mlock);
		tv_clk_enable_count++;
		mutex_unlock(&mlock);
    }
    aw1683_tve_set_mode(g_tv_mode);
	aw1683_tve_open();

    return 0;
}

static s32 tv_close(void)
{
	tv_pin_config(0);
	aw1683_tve_close();
	if(tv_source_ops.tcon_disable) {
    	tv_source_ops.tcon_disable(tv_device);
		mutex_lock(&mlock);
		tv_clk_enable_count--;
		mutex_unlock(&mlock);
    }

    if(tv_source_ops.tcon_simple_enable)
    	tv_source_ops.tcon_simple_enable(tv_device);
    return 0;
}

static s32 tv_set_mode(enum disp_tv_mode tv_mode)
{

	mutex_lock(&mlock);
    g_tv_mode = tv_mode;
	mutex_unlock(&mlock);
    return 0;
}

static s32 tv_get_hpd_status(void)
{
	return tv_hpd_sourc;
}

static s32 tv_get_mode_support(enum disp_tv_mode tv_mode)
{
    if(tv_mode == DISP_TV_MOD_PAL || tv_mode == DISP_TV_MOD_NTSC)
		return 1;

    return 0;
}

static s32 tv_get_video_timing_info(struct disp_video_timings **video_info)
{
	struct disp_video_timings *info;
	int ret = -1;
	int i, list_num;
	info = tv_video_timing;

	list_num = sizeof(tv_video_timing)/sizeof(struct disp_video_timings);
	for(i=0; i<list_num; i++) {
		if(info->tv_mode == g_tv_mode){
			*video_info = info;
			ret = 0;
			break;
		}

		info ++;
	}
	return ret;
}

static s32 tv_get_interface_para(void* para)
{
	struct disp_vdevice_interface_para intf_para;

	intf_para.intf = 0;
	intf_para.sub_intf = 12;
	intf_para.sequence = 0;
	intf_para.clk_phase = 2;
	intf_para.sync_polarity = 0;
	if(g_tv_mode == DISP_TV_MOD_NTSC)
		intf_para.fdelay = 1;//ntsc
	else
		intf_para.fdelay = 2;//pal

	if(para)
		memcpy(para, &intf_para, sizeof(struct disp_vdevice_interface_para));

	return 0;
}

//0:rgb;  1:yuv
static s32 tv_get_input_csc(void)
{
	return 1;
}

s32 tv_suspend(void)
{
	mutex_lock(&mlock);
	if(tv_used && (false == tv_suspend_status)) {
		tv_suspend_status = true;
		tv_detect_disable();
		if(tv_source_ops.tcon_disable) {
			tv_source_ops.tcon_disable(tv_device);
			tv_clk_enable_count--;
		}
		if(tv_clk_enable_count) {
			tv_clk_disable();
			tv_clk_enable_count--;
		}
	}
	mutex_unlock(&mlock);

	return 0;
}

s32 tv_resume(void)
{
	mutex_lock(&mlock);
	if(tv_used && (true == tv_suspend_status)) {
		tv_suspend_status= false;
		tv_clk_enable(g_tv_mode);
		tv_clk_enable_count++;
		tv_detect_enable();
		if(tv_source_ops.tcon_simple_enable)
			tv_source_ops.tcon_simple_enable(tv_device);
	}
	mutex_unlock(&mlock);

	return  0;
}

static const struct of_device_id sunxi_tv_ac200_match[] = {
	{ .compatible = "allwinner,sunxi_tv_ac200", },
	{},
};

static struct disp_device* tv_ac200_register(void)
{
	struct disp_vdevice_init_data init_data;
	struct disp_device* device;
	memset(&init_data, 0, sizeof(struct disp_vdevice_init_data));
	init_data.disp = tv_screen_id;
	memcpy(init_data.name, modules_name, 32);
	init_data.type = DISP_OUTPUT_TYPE_TV;
	init_data.fix_timing = 0;
	init_data.func.enable = tv_open;
	init_data.func.disable = tv_close;
	init_data.func.get_HPD_status = tv_get_hpd_status;
	init_data.func.set_mode = tv_set_mode;
	init_data.func.mode_support = tv_get_mode_support;
	init_data.func.get_video_timing_info = tv_get_video_timing_info;
	init_data.func.get_interface_para = tv_get_interface_para;
	init_data.func.get_input_csc = tv_get_input_csc;
	
	disp_vdevice_get_source_ops(&tv_source_ops);
	device = disp_vdevice_register(&init_data);
	
	return device;
}

static int tv_init(struct platform_device *pdev)
{
	unsigned int value, output_type0, output_mode0, output_type1, output_mode1;

	/* parse boot para */
	value = disp_boot_para_parse();
	output_type0 = (value >> 8) & 0xff;
	output_mode0 = (value) & 0xff;
	output_type1 = (value >> 24)& 0xff;
	output_mode1 = (value >> 16) & 0xff;
	if((output_type0 == DISP_OUTPUT_TYPE_TV) ||
				(output_type1 == DISP_OUTPUT_TYPE_TV)) {
		printk("[TV]%s:smooth boot, type0 = %d, type1 = %d\n",
				__func__, output_type0, output_type1);

		if(DISP_OUTPUT_TYPE_TV == output_type0)
		{
			g_tv_mode = output_mode0;
		}
		else if(DISP_OUTPUT_TYPE_TV == output_type1)
		{
			g_tv_mode = output_mode1;
		}
	}

	/* if support switch class,register it for cvbs hot plugging detect */
#if defined(CONFIG_SWITCH) || defined(CONFIG_ANDROID_SWITCH)
	switch_dev_register(&cvbs_switch_dev);
#endif

	/*register extern tv module to vdevice*/
	tv_device = tv_ac200_register();
	if(IS_ERR_OR_NULL(tv_device)) {
		dev_err(&pdev->dev, "register tv device failed.\n");
		goto err_register;
	}
	
	/* parse io config */
	tv_parse_config();

	/* get clk */
	tv_clk = of_clk_get(pdev->dev.of_node, 0);  //modify when mfd is ready.
	if (IS_ERR_OR_NULL(tv_clk)) {
		dev_err(&pdev->dev, "fail to get clk for hdmi\n");
		goto err_register;
	}
	tv_clk_enable_count = tv_clk->enable_count;

	/* init param*/
	tv_suspend_status = 0;
	tv_used = 1;
	mutex_init(&mlock);
	tv_detect_enable();
	tv_clk_init();
	tv_clk_enable(g_tv_mode);

	return 0;

err_register:
	return -1;
}

static int tv_ac200_probe(struct platform_device *pdev)
{
	struct ac200_tv_priv *acx00_pr;

	pr_info("%s tv_ac200_probe.\n",__func__);
	acx00_pr = devm_kzalloc(&pdev->dev, sizeof(struct ac200_tv_priv), GFP_KERNEL);
	if (acx00_pr == NULL) {
		pr_info("%s devm_kzalloc failed\n",__func__);
		return -ENOMEM;
	}
	platform_set_drvdata(pdev, acx00_pr);
	tv_priv.acx00 = dev_get_drvdata(pdev->dev.parent);

	tv_init(pdev);
	return 0;
}


static void tv_shutdown(struct platform_device *pdev)
{
	#if 0
	struct acx00_priv *acx00 = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = acx00->codec;

	/*disable lineout*/
	snd_soc_update_bits(codec, LINEOUT_CTRL, (0x1<<LINEOUTEN), (0<<LINEOUTEN));
	/*disable pa_ctrl*/
	gpio_set_value(item.gpio.gpio, 0);
	#endif
}


static int  tv_remove(struct platform_device *pdev) //delete __devexit
{
	if(tv_device)
		disp_vdevice_unregister(tv_device);
	tv_device = NULL;
	tv_clk_exit();
    return 0;
}


static struct platform_driver tv_ac200_driver = {
	.driver = {
		.name = "tv",
		.owner = THIS_MODULE,
		.of_match_table = sunxi_tv_ac200_match,
	},
	.probe = tv_ac200_probe,
	.remove = tv_remove,//delete __devexit_p(tv_remove)
	.shutdown = tv_shutdown,
};


module_platform_driver(tv_ac200_driver);

MODULE_AUTHOR("zengqi");
MODULE_DESCRIPTION("tv_ac200 driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:tv_ac200");
