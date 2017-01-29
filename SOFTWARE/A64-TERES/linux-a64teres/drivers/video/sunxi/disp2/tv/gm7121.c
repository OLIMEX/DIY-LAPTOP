#include <linux/module.h>
#include <asm/uaccess.h>
#include <asm/memory.h>
#include <asm/unistd.h>
#include "asm-generic/int-ll64.h"
#include "linux/kernel.h"
#include "linux/mm.h"
#include "linux/semaphore.h"
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/dma-mapping.h>
#include <linux/sched.h>   //wake_up_process()
#include <linux/kthread.h> //kthread_create()??��|kthread_run()
#include <linux/err.h> //IS_ERR()??��|PTR_ERR()
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include "../disp/disp_sys_intf.h"
#include <video/sunxi_display2.h>
#include <linux/clk-private.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of_iommu.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>

static char modules_name[32] = "gm7121";
static char key_name[20] = "tv_gm7121_para";

#define GM7121_Config(sub_addr,data) tv_i2c_write(sub_addr, data)
static void gm7121_init(enum disp_tv_mode tv_mode);
static enum disp_tv_mode g_tv_mode = DISP_TV_MOD_PAL;
static u32 tv_i2c_id = 1;
static u32 tv_i2c_used = 0;
static u32 tv_screen_id = 0;
static u32 tv_used = 0;
static u32 tv_power_used = 0;
static char tv_power[16] = {0};

static bool tv_io_used[28];
static disp_gpio_set_t tv_io[28];
static struct i2c_adapter *tv_i2c_adapter;
static struct i2c_client *tv_i2c_client;
static struct disp_device *gm7121_device = NULL;
static struct disp_vdevice_source_ops tv_source_ops;
static s32 tv_i2c_write(u8 sub_addr, u8 data);
//static s32 tv_i2c_read(u8 sub_addr, u8 *data);

extern struct disp_device* disp_vdevice_register(struct disp_vdevice_init_data *data);
extern s32 disp_vdevice_unregister(struct disp_device *vdevice);
extern s32 disp_vdevice_get_source_ops(struct disp_vdevice_source_ops *ops);
extern unsigned int disp_boot_para_parse(void);

static struct disp_video_timings video_timing[] =
{
	/*vic  tv_mode         PCLK    AVI  x   y    HT  HBP HFP HST  VT  VBP VFP VST  H_P V_P I  TRD */
	{0,	DISP_TV_MOD_PAL, 27000000, 0, 720, 576, 864, 137, 3,  2, 625, 20, 25,  2,  0,  0,  1, 0, 0},
	{0,	DISP_TV_MOD_NTSC,27000000, 0, 720, 480, 858, 57, 19, 62, 525, 15,  4,  3,  0,  0,  1, 0, 0},
};

static int tv_parse_config(void)
{
	disp_gpio_set_t  *gpio_info;
	int i, ret;
	char io_name[32];

	for(i=0; i<28; i++) {
		gpio_info = &(tv_io[i]);
		sprintf(io_name, "tv_d%d", i);
		ret = disp_sys_script_get_item(key_name, io_name, (int *)gpio_info, sizeof(disp_gpio_set_t)/sizeof(int));
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

static s32 gm7121_tv_power_on(u32 on_off)
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

static s32 gm7121_tv_open(void)
{
    gm7121_tv_power_on(1);
    tv_pin_config(1);
    msleep(400);
    if(tv_source_ops.tcon_enable)
        tv_source_ops.tcon_enable(gm7121_device);
    msleep(100);
    gm7121_init(g_tv_mode);

    return 0;
}

static s32 gm7121_tv_close(void)
{
    if(tv_source_ops.tcon_disable)
        tv_source_ops.tcon_disable(gm7121_device);
    msleep(100);
    tv_pin_config(0);
    gm7121_tv_power_on(0);
    msleep(500);
    return 0;
}

#if 0
static s32 gm7121_tv_get_mode(void)
{
    return g_tv_mode;
}
#endif

static s32 gm7121_tv_set_mode(enum disp_tv_mode tv_mode)
{
    g_tv_mode = tv_mode;

    return 0;
}

static s32 gm7121_tv_get_hpd_status(void)
{
    return 0;
}

static s32 gm7121_tv_get_mode_support(enum disp_tv_mode tv_mode)
{
    if(tv_mode == DISP_TV_MOD_PAL || tv_mode == DISP_TV_MOD_NTSC)
        return 1;

    return 0;
}

static s32 gm7121_tv_get_video_timing_info(struct disp_video_timings **video_info)
{
	struct disp_video_timings *info;
	int ret = -1;
	int i, list_num;
	info = video_timing;

	list_num = sizeof(video_timing)/sizeof(struct disp_video_timings);
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

static s32 gm7121_tv_get_interface_para(void* para)
{
	struct disp_vdevice_interface_para intf_para;

	intf_para.intf = 0;
	intf_para.sub_intf = 12;
	intf_para.sequence = 2;
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
static s32 gm7121_tv_get_input_csc(void)
{
	return 0;
}

static void gm7121_init(enum disp_tv_mode tv_mode)
{
	if (tv_mode == DISP_TV_MOD_PAL)
		return;
	//disp_tv_mode tv_mode = DISP_TV_MOD_PAL;//DISP_TV_MOD_PAL;//DISP_TV_MOD_NTSC;
	//-------------------SAA7121 START-------------------------------
	pr_warn("[TV]gm7121_init, tv_Mode=%d\n", tv_mode);
	GM7121_Config(0x28,0x21);
	GM7121_Config(0x29,0x1D);
	GM7121_Config(0x2A,0x00);
	GM7121_Config(0x2B,0x00);
	GM7121_Config(0x2C,0x00);
	GM7121_Config(0x2D,0x00);
	GM7121_Config(0x2E,0x00);
	GM7121_Config(0x2F,0x00);
	GM7121_Config(0x30,0x00);
	GM7121_Config(0x31,0x00);
	GM7121_Config(0x32,0x00);
	GM7121_Config(0x33,0x00);
	GM7121_Config(0x34,0x00);
	GM7121_Config(0x35,0x00);
	GM7121_Config(0x36,0x00);
	GM7121_Config(0x37,0x00);
	GM7121_Config(0x38,0x00);
	GM7121_Config(0x39,0x00);

	//GM7121_Config(0x3A,0x93);    //color  strape
	GM7121_Config(0x3A,0x13);   //data

	GM7121_Config(0x5A,0x00);
	GM7121_Config(0x5B,0x6d);
	GM7121_Config(0x5C,0x9f);
	//GM7121_Config(0x5D,0x1e);
	GM7121_Config(0x5E,0x1c);
	GM7121_Config(0x5F,0x35);
	GM7121_Config(0x60,0x00);

	if (tv_mode == DISP_TV_MOD_NTSC)
	{
		GM7121_Config(0x5D,0x1e);
		GM7121_Config(0x61,0x01);    //NTSC
		GM7121_Config(0x63,0x1f);    //NTSC
		GM7121_Config(0x64,0x7c);    //NTSC
		GM7121_Config(0x65,0xF0);    //NTSC
		GM7121_Config(0x66,0x21);    //NTSC
	}
	else
	{
		GM7121_Config(0x5D,0x0e);
		GM7121_Config(0x61,0x06);    //PAL
		GM7121_Config(0x63,0xCB);    //PAL
		GM7121_Config(0x64,0x8A);    //PAL
		GM7121_Config(0x65,0x09);    //PAL
		GM7121_Config(0x66,0x2A);    //PAL
	}

	GM7121_Config(0x62,0x3B);     //RTCI Enable
#if 0
	GM7121_Config(0x67,0x00);
	GM7121_Config(0x68,0x00);
	GM7121_Config(0x69,0x00);
	GM7121_Config(0x6A,0x00);
#endif
	GM7121_Config(0x6B,0x12);

	GM7121_Config(0x6C,0x01);

	GM7121_Config(0x6D,0x20);

	GM7121_Config(0x6E,0x80);    //video with color
	GM7121_Config(0x6F,0x00);
	GM7121_Config(0x70,0x14);
	GM7121_Config(0x71,0x00);
	GM7121_Config(0x72,0x00);
	GM7121_Config(0x73,0x00);
	GM7121_Config(0x74,0x00);
	GM7121_Config(0x75,0x00);
	GM7121_Config(0x76,0x00);
	GM7121_Config(0x77,0x00);
	GM7121_Config(0x78,0x00);
	GM7121_Config(0x79,0x00);
	GM7121_Config(0x7A,0x16);
	GM7121_Config(0x7B,0x36);
	GM7121_Config(0x7C,0x40);
	GM7121_Config(0x7D,0x00);
	GM7121_Config(0x7E,0x00);
	GM7121_Config(0x7F,0x00);
}

static int tv_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct disp_vdevice_init_data init_data;

	pr_info("[DISP_I2C] tv_i2c_probe\n");
	memset(&init_data, 0, sizeof(struct disp_vdevice_init_data));
	tv_i2c_client = client;

	init_data.disp = tv_screen_id;
	memcpy(init_data.name, modules_name, 32);
	init_data.type = DISP_OUTPUT_TYPE_TV;
	init_data.fix_timing = 0;

	init_data.func.enable = gm7121_tv_open;
	init_data.func.disable = gm7121_tv_close;
	init_data.func.get_HPD_status = gm7121_tv_get_hpd_status;
	init_data.func.set_mode = gm7121_tv_set_mode;
	init_data.func.mode_support = gm7121_tv_get_mode_support;
	init_data.func.get_video_timing_info = gm7121_tv_get_video_timing_info;
	init_data.func.get_interface_para = gm7121_tv_get_interface_para;
	init_data.func.get_input_csc = gm7121_tv_get_input_csc;
	disp_vdevice_get_source_ops(&tv_source_ops);
	tv_parse_config();

	gm7121_device = disp_vdevice_register(&init_data);

	return 0;
}

static int __exit tv_i2c_remove(struct i2c_client *client)
{
		if(gm7121_device)
			disp_vdevice_unregister(gm7121_device);
		gm7121_device = NULL;
    return 0;
}

static const struct i2c_device_id tv_i2c_id_table[] = {
    { "tv_i2c", 0 },
    {}
};
MODULE_DEVICE_TABLE(i2c, tv_i2c_id_table);

static int tv_i2c_detect(struct i2c_client *client, struct i2c_board_info *info)
{
    if(tv_i2c_id == client->adapter->nr){
        const char *type_name = "tv_i2c";
        tv_i2c_adapter = client->adapter;
        pr_info("[DISP_I2C] tv_i2c_detect, get right i2c adapter, id=%d\n", tv_i2c_adapter->nr);
        strlcpy(info->type, type_name, I2C_NAME_SIZE);
        return 0;
    }
    return -ENODEV;
}

static  unsigned short normal_i2c[] = {0x46, I2C_CLIENT_END};

static const struct of_device_id sunxi_tv_gm7121_match[] = {
	{ .compatible = "allwinner,sunxi_tv_gm7121", },
	{},
};

static struct i2c_driver tv_i2c_driver = {
    .class = I2C_CLASS_HWMON,
    .probe        = tv_i2c_probe,
    .remove        = __exit_p(tv_i2c_remove),
    .id_table    = tv_i2c_id_table,
    .driver    = {
        .name    = "tv_i2c",
        .owner    = THIS_MODULE,
        .of_match_table = sunxi_tv_gm7121_match,
    },
    .detect        = tv_i2c_detect,
    .address_list    = normal_i2c,
};

static int  tv_i2c_init(void)
{
    int ret;
    int value;

	ret = disp_sys_script_get_item(key_name, "tv_twi_used", &value, 1);
    if(1 == ret)
    {
        tv_i2c_used = value;
        if(tv_i2c_used == 1)
        {
             ret = disp_sys_script_get_item(key_name, "tv_twi_id", &value, 1);
            tv_i2c_id = (ret == 1)? value:tv_i2c_id;

            ret = disp_sys_script_get_item(key_name, "tv_twi_addr", &value, 1);
            normal_i2c[0] = (ret == 1)? value:normal_i2c[0];

            return i2c_add_driver(&tv_i2c_driver);
        }
    }
    return 0;
}

static void tv_i2c_exit(void)
{
    i2c_del_driver(&tv_i2c_driver);
}

static s32 tv_i2c_write(u8 sub_addr, u8 data)
{
    s32 ret = 0;
    u8 i2c_data[2];
    struct i2c_msg msg;

    if(tv_i2c_used)
    {
        i2c_data[0] = sub_addr;
        i2c_data[1] = data;
        msg.addr = tv_i2c_client->addr;
        msg.flags = 0;
        msg.len = 2;
        msg.buf = i2c_data;
        ret = i2c_transfer(tv_i2c_adapter, &msg, 1);
    }

	return ret;
}

#if 0
static s32 tv_i2c_read(u8 sub_addr, u8 *data)
{
	s32 ret = 0;
	struct i2c_msg msgs[] = {
		{
			.addr	= tv_i2c_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= &sub_addr,
		},
		{
		.addr	= tv_i2c_client->addr,
		.flags	= I2C_M_RD,
		.len	= 1,
		.buf	= data,
		},
	};

	if(tv_i2c_used) {
		ret = i2c_transfer(tv_i2c_adapter, msgs, 2);
	}

	return ret;
}
#endif

static int __init gm7121_module_init(void)
{
	int ret;
	int value;

	pr_info("[TV]gm7121_module_init begin\n");

	ret = disp_sys_script_get_item(key_name, "tv_used", &value, 1);
	if(1 == ret) {
		tv_used = value;
		if(tv_used)
		{
			unsigned int value, output_type0, output_mode0, output_type1, output_mode1;

			ret = disp_sys_script_get_item(key_name, "tv_power", (int*)tv_power, 32/sizeof(int));
			if(2 == ret) {
				tv_power_used = 1;
				printk("[TV] tv_power: %s\n", tv_power);
			}
			tv_i2c_init();
			value = disp_boot_para_parse();
			output_type0 = (value >> 8) & 0xff;
			output_mode0 = (value) & 0xff;
			output_type1 = (value >> 24)& 0xff;
			output_mode1 = (value >> 16) & 0xff;
			if((output_type0 == DISP_OUTPUT_TYPE_TV) ||
				(output_type1 == DISP_OUTPUT_TYPE_TV))
			{
				printk("[TV]%s:smooth boot", __func__);
				gm7121_tv_power_on(1);
				if(DISP_OUTPUT_TYPE_TV == output_type0)
				{
					g_tv_mode = output_mode0;
				}
				else if(DISP_OUTPUT_TYPE_TV == output_type1)
				{
					g_tv_mode = output_mode1;
				}
			}
		}
	} else
		tv_used = 0;

	return 0;
}

static void __exit gm7121_module_exit(void)
{
    pr_info("gm7121_module_exit\n");
    tv_i2c_exit();
}

late_initcall(gm7121_module_init);
module_exit(gm7121_module_exit);

MODULE_AUTHOR("tyle");
MODULE_DESCRIPTION("gm7121 driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:gm7121");
