/*
 * A V4L2 driver for OV12830 cameras.
 *
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/videodev2.h>
#include <linux/clk.h>
#include <media/v4l2-device.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-mediabus.h>
#include <linux/io.h>
#include "camera.h"
#include "sensor_helper.h"

MODULE_AUTHOR("raymonxiu");
MODULE_DESCRIPTION("A low-level driver for OV12830 sensors");
MODULE_LICENSE("GPL");

//for internel driver debug
#define DEV_DBG_EN      1 
#if(DEV_DBG_EN == 1)    
#define vfe_dev_dbg(x,arg...) printk("[OV12830]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...) 
#endif
#define vfe_dev_err(x,arg...) printk("[OV12830]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[OV12830]"x,##arg)

#define LOG_ERR_RET(x)  { \
                          int ret;  \
                          ret = x; \
                          if(ret < 0) {\
                            vfe_dev_err("error at %s\n",__func__);  \
                            return ret; \
                          } \
                        }

//define module timing
#define MCLK              (24*1000*1000)
#define VREF_POL          V4L2_MBUS_VSYNC_ACTIVE_HIGH
#define HREF_POL          V4L2_MBUS_HSYNC_ACTIVE_HIGH
#define CLK_POL           V4L2_MBUS_PCLK_SAMPLE_RISING
#define V4L2_IDENT_SENSOR 0xc830




/*
 * Our nominal (default) frame rate.
 */

#define SENSOR_FRAME_RATE 30


/*
 * The ov12830 sits on i2c with ID 0x6c
 */
#define I2C_ADDR 0x6c
#define SENSOR_NAME "ov12830"
//static struct delayed_work sensor_s_ae_ratio_work;
static struct v4l2_subdev *glb_sd;

/*
 * Information we maintain about a known sensor.
 */
struct sensor_format_struct;  /* coming later */

struct cfg_array { /* coming later */
	struct regval_list * regs;
	int size;
};

static inline struct sensor_info *to_state(struct v4l2_subdev *sd)
{
	return container_of(sd, struct sensor_info, sd);
}

static int ov12830_sensor_vts;
/*
 * The default register settings
 *
 */


static struct regval_list sensor_default_regs[] = {
// MIPI = 880MBps, 4 lane, SysCLK = 120Mhz, DAC clock = 272Mhz
// initialize to 2112x1500 30fps with binning
	{0x0103,0x01},// software reset
	{0xffff,0x0a},// mdelay 10ms
	{0x0100,0x00}, // software standby
	{0x3001,0x06},// driver 2x, fsin disable, frex_in disable
	{0x3002,0x80},// vsync oen, href, pclk, fres, fsin, sda odis
	{0x3011,0x41},// 4 lane MIPI
	{0x3014,0x16},//
	{0x3015,0x0b},// 4 lane enable
	{0x4800,0x04},// [5]: gating clk, [2]: set mipi11 when no packet
	{0x3022,0x03},// debug mode
	{0x3090,0x02},// PLL_PLL10
	{0x3091,0x0a},// PLL_PLL11
	{0x3092,0x00},// PLL_PLL12
	{0x3093,0x00},// PLL_PLL13
	{0x3098,0x03},// PLL
	{0x3099,0x11},// PLL
	{0x309c,0x01},// PLL
	{0x30b3,0x5b},// PLL_multilier 0x5b:20fps
	{0x30b4,0x03},// PLL_PLL1_PRE
	{0x30b5,0x04},// PLL_PLL1_OP_PIX_CLK_DIV
	{0x30b6,0x01},// PLL_PLL1_OP_SYS_CLK_DIV
	{0x3106,0x01},// sclk = PLL_sclk/2
	{0x3304,0x28},
	{0x3305,0x41},
	{0x3306,0x30},
	{0x3308,0x00},
	{0x3309,0xc8},
	{0x330a,0x01},
	{0x330b,0x90},
	{0x330c,0x02},
	{0x330d,0x58},
	{0x330e,0x03},
	{0x330f,0x20},
	{0x3300,0x00},
	
	{0x3400,0x06},// MWB R
	{0x3401,0x0a},// 
	{0x3402,0x04},// MWB G
	{0x3403,0x00},// 
	{0x3404,0x05},// MWB B
	{0x3405,0xb9},// 			//8d
	{0x3406,0x01},// MWB enable

	{0x3500,0x00},// exposure = 1508
	{0x3501,0x5e},// exposure
	{0x3502,0x40},// exposure
	{0x3503,0x07},// VTS manual, AGC manual, AEC manual
	{0x3509,0x10},// use sensor gain
	{0x350a,0x00},// gain = 8x
	{0x350b,0x80},// gain
	// analog
	{0x3602,0x28},
	{0x3612,0x80},
	{0x3621,0xb5},
	{0x3622,0x0b},
	{0x3623,0x28},
	{0x3631,0xb3},
	{0x3634,0x04},
	{0x3660,0x80},
	{0x3662,0x10},
	{0x3663,0xf0},
	{0x3667,0x00},
	{0x366f,0x20},
	{0x3680,0xb5},
	{0x3682,0x00},
	// sensor timing
	{0x3701,0x12},
	{0x3702,0x88},
	{0x3708,0xe6},
	{0x3709,0xc7},
	{0x370b,0xa0},
	{0x370d,0x11},
	{0x370e,0x00},
	{0x371c,0x01},
	{0x371f,0x1b},
	{0x3726,0x00},
	{0x372a,0x09},
	{0x3739,0xb0},
	{0x373c,0x40},
	{0x376b,0x44},
	{0x377b,0x44},
	// PSRAM
	{0x3780,0x22},
	{0x3781,0xc8},
	{0x3783,0x31},
	{0x3786,0x16},
	{0x3787,0x02},
	{0x3796,0x84},
	{0x379c,0x0c},
	// FREX
	{0x37c5,0x00},
	{0x37c6,0x00},
	{0x37c7,0x00},
	{0x37c9,0x00},
	{0x37ca,0x00},
	{0x37cb,0x00},
	{0x37cc,0x00},
	{0x37cd,0x00},
	{0x37ce,0x10},
	{0x37cf,0x00},
	{0x37d0,0x00},
	{0x37d1,0x00},
	{0x37d2,0x00},
	{0x37de,0x00},
	{0x37df,0x00},
	//blc
	{0x4000,0x18}, // window select as blc does, use reg 0x4006 as K for DC BLC
	{0x4001,0x06}, // blc start line
	{0x4002,0x45}, // [7]:redo after format change; [6]: blc auto, do 5 frames blc after reset
	{0x4004,0x02}, // number of black lines
	{0x4005,0x19}, // no black line output, apply one channel offset (0x400c, 0x400d) to all channes
	{0x4007,0x90}, // enable media filter, window do not contain first 1/16 and last 1/16
	{0x4008,0x20},
	{0x4009,0x10}, // BLC target
	{0x4100,0x2d},
	{0x4101,0x22},
	{0x4102,0x04},
	{0x4104,0x5c},
	{0x4109,0xa3},
	{0x410a,0x03},
	{0x4300,0xff}, // data max
	{0x4303,0x00},
	{0x4304,0x08},
	{0x4307,0x30},
	{0x4311,0x04}, // vsync width
	{0x4511,0x05},
	{0x4816,0x52},
	{0x481f,0x30},
	{0x4826,0x2c},
	// lvds
	{0x4a00,0xaa},
	{0x4a03,0x01},
	{0x4a05,0x08},
	// temperature
	{0x4d00,0x05},
	{0x4d01,0x19},
	{0x4d02,0xfd},
	{0x4d03,0xd1},
	{0x4d04,0xff},
	{0x4d05,0xff},
	{0x4d07,0x04},
	{0x4837,0x09}, // MIPI global timing
	{0x5000,0x06}, // lenc off, bpc on, wpc on
	{0x5001,0x01}, // mwb on
	{0x5002,0x00}, // scale off
	{0x5003,0x21},
	{0x5043,0x48},
	{0x5013,0x80},
	{0x501f,0x00},
	{0x5e00,0x00},
	{0x5a01,0x00}, // win x start
	{0x5a02,0x00}, // win y start
	{0x5a03,0x00}, // win y start
	{0x5a04,0x10}, // win width = 4256
	{0x5a05,0xa0}, // win width
	{0x5a06,0x0c}, // win height = 3192
	{0x5a07,0x78}, // win height
	{0x5a08,0x00}, // window size manual off
	{0x5e00,0x00},
	{0x5e01,0x41}, // window cut enable
	{0x5e11,0x30},

};

//for capture                                                                         
static struct regval_list sensor_hxga_regs[] = { //hxga: 4000*3000@20/15fps
// MIPI=880Mbps, SysClk=324Mhz,Dac Clock=272Mhz.
// 4000x3000 20fps
	{0x0100,0x00}, // software standby
	{REG_DLY,0x32},
	{0x3091,0x16}, // PLL_PLL11 0x16:20fps
	{0x3708,0xe3},
	{0x3709,0xc3},
	//timing
	{0x3800,0x00}, // s start = 112
	{0x3801,0x70}, // x start
	{0x3802,0x00}, // y start = 8
	{0x3803,0x00}, // y start
	{0x3804,0x10}, // x end = 4143
	{0x3805,0x2f}, // x end
	{0x3806,0x0b}, // y end = 3007
	{0x3807,0xc0}, // y end
	{0x3808,0x0f}, // x output size = 4000
	{0x3809,0xa0}, // x output size
	{0x380a,0x0b}, // y output size = 3000
	{0x380b,0xb8}, // y output size
	{0x380c,0x11}, // HTS = 4432
	{0x380d,0x50}, // HTS
	{0x380e,0x0b}, // VTS = 3044
	{0x380f,0xe4}, // VTS
	{0x3810,0x00}, // isp x win = 16
	{0x3811,0x10}, // isp x win
	{0x3812,0x00}, // isp y win = 8
	{0x3813,0x08}, // isp y win
	{0x3814,0x11}, // x inc
	{0x3815,0x11}, // y inc
	{0x3820,0x10}, // V bin off, flip off
	{0x3821,0x0e}, // mirror on, h bin off
	{0x4004,0x08}, // number of black lines
	{0x5002,0x00}, // scale off
	{0x0100,0x01}, // wake up from software standby
};


//for video
static struct regval_list sensor_1080p_regs[] = { //1080: 1920*1080@30fps
// MIPI=880Mbps, SysClk=120Mhz,Dac Clock=272Mhz.
// 1920x1080 30fps
	{0x0100,0x00}, // software standby
	{REG_DLY,0x32},
	{0x3091,0x14}, // PLL_PLL11  0x0a:30fps 0x14 60fps
	{0x3708,0xe6},
	{0x3709,0xc7},
	//timing
	{0x3800,0x00}, // x start = 0
	{0x3801,0x00}, // x start
	{0x3802,0x01}, // y start = 312
	{0x3803,0x38}, // y start
	{0x3804,0x10}, // x end = 4255
	{0x3805,0x9f}, // x end
	{0x3806,0x0a}, // y end = 2703
	{0x3807,0x8f}, // y end
	{0x3808,0x07}, // x output size = 1920
	{0x3809,0x80}, // x output size
	{0x380a,0x04}, // y output size = 1080
	{0x380b,0x38}, // y output size
	{0x380c,0x0c}, // HTS = 3278
	{0x380d,0xce}, // HTS
	{0x380e,0x04}, // VTS = 1220
	{0x380f,0xc4}, // VTS
	{0x3810,0x00}, // isp x win = 8
	{0x3811,0x08}, // isp x win
	{0x3812,0x00}, // isp y win = 2
	{0x3813,0x02}, // isp y win
	{0x3814,0x31}, // x inc
	{0x3815,0x31}, // y inc
	{0x3820,0x14}, // V fast bin on, flip off
	{0x3821,0x0f}, // mirror on, h bin on
	{0x4004,0x02}, // number of black lines
	{0x5002,0x80}, // scale on
	{0x0100,0x01}, // wake up from software standby
};

static struct regval_list sensor_720p_regs[] = { //720: 1280*720@30fps
// MIPI=880Mbps, SysClk=120Mhz,Dac Clock=272Mhz.
// 1280x720 60fps
	{0x0100,0x00}, // software standby
	{REG_DLY,0x32},
	{0x3091,0x21}, // PLL_PLL11  0x0a:30fps 0x14 60fps
	{0x3708,0xe6}, 
	{0x3709,0xc7}, 
	//timing
	{0x3800,0x00}, // x start = 64
	{0x3801,0x40}, // x start
	{0x3802,0x01}, // y start = 348
	{0x3803,0x5c}, // y start
	{0x3804,0x10}, // x end = 4191
	{0x3805,0x5f}, // x end
	{0x3806,0x0a}, // y end = 2667
	{0x3807,0x6b}, // y end
	{0x3808,0x05}, // x output size = 1280
	{0x3809,0x00}, // x output size
	{0x380a,0x02}, // y output size = 720
	{0x380b,0xd0}, // y output size
	{0x380c,0x0c}, // HTS = 3264
	{0x380d,0xc0}, // HTS
	{0x380e,0x04}, // VTS = 1220
	{0x380f,0xc4}, // VTS
	{0x3810,0x00}, // isp x win = 16
	{0x3811,0x10}, // isp x win
	{0x3812,0x00}, // isp y win = 8
	{0x3813,0x08}, // isp y win
	{0x3814,0x31}, // x inc
	{0x3815,0x31}, // y inc
	{0x3820,0x14}, // V fast bin on, flip off
	{0x3821,0x0f}, // mirror on, h bin on
	{0x4004,0x02}, // number of black lines
	{0x5002,0x80}, // scale on
	{0x0100,0x01}, // wake up from software standby
};


static struct regval_list sensor_vga_regs[] = { //VGA:  640*480
// MIPI=880Mbps, SysClk=240Mhz,Dac Clock=272Mhz.
// 640x480 120fps
	{0x0100,0x00}, // software standby
	{REG_DLY,0x32},
	{0x3091,0x14}, // PLL_PLL11  0x0a:30fps 0x14 60fps
	{0x3708,0xe4},
	{0x3709,0xcb},
	//timing
	{0x3800,0x00}, // x start = 64
	{0x3801,0x40}, // x start
	{0x3802,0x00}, // y start = 0
	{0x3803,0x00}, // y start
	{0x3804,0x10}, // x end = 4159
	{0x3805,0x3f}, // x end
	{0x3806,0x0a}, // y end = 2667
	{0x3807,0x6b}, // y end
	{0x3808,0x02}, // x output size = 640
	{0x3809,0x80}, // x output size
	{0x380a,0x01}, // y output size = 480
	{0x380b,0xe0}, // y output size
	{0x380c,0x0a}, // HTS = 2560
	{0x380d,0x00}, // HTS
	{0x380e,0x03}, // VTS = 780
	{0x380f,0x0c}, // VTS
	{0x3810,0x00}, // isp x win = 16
	{0x3811,0x10}, // isp x win
	{0x3812,0x00}, // isp y win = 2
	{0x3813,0x02}, // isp y win
	{0x3814,0x71}, // x inc
	{0x3815,0x53}, // y inc
	{0x3820,0x14}, // V fast bin on, flip off
	{0x3821,0x0f}, // mirror on, h bin on
	{0x4004,0x02}, // number of black lines
	{0x5002,0x80}, // scale on
	{0x0100,0x01}, // wake up from software standby
};


/*
 * Here we'll try to encapsulate the changes for just the output
 * video format.
 * 
 */

static struct regval_list sensor_fmt_raw[] = {

};


static int sensor_g_exp(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);

	*value = info->exp;
	vfe_dev_dbg("sensor_get_exposure = %d\n", info->exp);
	return 0;
}

static int sensor_s_exp_gain(struct v4l2_subdev *sd, struct sensor_exp_gain *exp_gain)
{
	int exp_val, gain_val,shutter,frame_length;  
	unsigned char explow=0,expmid=0,exphigh=0;
	unsigned char gainlow=0,gainhigh=0;  
	struct sensor_info *info = to_state(sd);
    
	exp_val = exp_gain->exp_val;
	gain_val = exp_gain->gain_val;
	if(gain_val<1*16)
		gain_val=16;
	if(gain_val>64*16-1)
		gain_val=64*16-1;

	if(exp_val>0xfffff)
		exp_val=0xfffff;

	gainlow=(unsigned char)(gain_val&0xff);
	gainhigh=(unsigned char)((gain_val>>8)&0x3);

	exphigh	= (unsigned char) ( (0x0f0000&exp_val)>>16);
	expmid	= (unsigned char) ( (0x00ff00&exp_val)>>8);
	explow	= (unsigned char) ( (0x0000ff&exp_val)	 );
	shutter = exp_val/16;
	//printk("sensor_vts = %d\n",sensor_vts);
  
	if(shutter  > ov12830_sensor_vts- 4)
	    frame_length = shutter + 4;
	else
	    frame_length = ov12830_sensor_vts;
  
	//printk("exp_val = %d,gain_val = %d\n",exp_val,gain_val);
	sensor_write(sd, 0x3208, 0x00);//enter group write
  
	sensor_write(sd, 0x3503, 0x07);

	sensor_write(sd, 0x380f, (frame_length & 0xff));
	sensor_write(sd, 0x380e, (frame_length >> 8));

	sensor_write(sd, 0x350b, gainlow);
	sensor_write(sd, 0x350a, gainhigh);
	
	sensor_write(sd, 0x3502, explow);
	sensor_write(sd, 0x3501, expmid);
	sensor_write(sd, 0x3500, exphigh);	
	sensor_write(sd, 0x3208, 0x10);//end group write
	sensor_write(sd, 0x3208, 0xa0);//init group write
	info->exp = exp_val;
	info->gain = gain_val;
	return 0;
}

static int sensor_s_exp(struct v4l2_subdev *sd, unsigned int exp_val)
{
	unsigned char explow,expmid,exphigh;
	struct sensor_info *info = to_state(sd);

	if(exp_val>0xfffff)
		exp_val=0xfffff;
	
//	if(info->exp == exp_val && exp_val <= (1968)*16)
//		return 0;
  
    exphigh = (unsigned char) ( (0x0f0000&exp_val)>>16);
    expmid  = (unsigned char) ( (0x00ff00&exp_val)>>8);
    explow  = (unsigned char) ( (0x0000ff&exp_val)   );
	
	sensor_write(sd, 0x3208, 0x00);//enter group write
	sensor_write(sd, 0x3502, explow);
	sensor_write(sd, 0x3501, expmid);
	sensor_write(sd, 0x3500, exphigh);	
	
	info->exp = exp_val;
	return 0;
}

static int sensor_g_gain(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);
	
	*value = info->gain;
	vfe_dev_dbg("sensor_get_gain = %d\n", info->gain);
	return 0;
}

static int sensor_s_gain(struct v4l2_subdev *sd, int gain_val)
{
	struct sensor_info *info = to_state(sd);
	unsigned char gainlow=0;
	unsigned char gainhigh=0;

	if(gain_val<1*16)
		gain_val=16;
	if(gain_val>64*16-1)
		gain_val=64*16-1;
	vfe_dev_dbg("sensor_set_gain = %d\n", gain_val);
    //if(info->gain == gain_val)
    //    return 0;

	gainlow=(unsigned char)(gain_val&0xff);
	gainhigh=(unsigned char)((gain_val>>8)&0x3);
	
	sensor_write(sd, 0x3503, 0x17);
	sensor_write(sd, 0x350b, gainlow);
	sensor_write(sd, 0x350a, gainhigh);
	sensor_write(sd, 0x3208, 0x10);//end group write
	sensor_write(sd, 0x3208, 0xa0);//init group write
	
	//printk("5647 sensor_set_gain = %d, Done!\n", gain_val);
	info->gain = gain_val;
	
	return 0;
}


static int sensor_s_sw_stby(struct v4l2_subdev *sd, int on_off)
{
	int ret;
	data_type rdval;
	
	ret=sensor_read(sd, 0x0100, &rdval);
	if(ret!=0)
		return ret;
	
	if(on_off==CSI_GPIO_LOW)//sw stby on
	{
		ret=sensor_write(sd, 0x0100, rdval&0xfe);
	}
	else//sw stby off
	{
		ret=sensor_write(sd, 0x0100, rdval|0x01);
	}
	return ret;
}

/*
 * Stuff that knows about the sensor.
 */
 
static int sensor_power(struct v4l2_subdev *sd, int on)
{
	int ret = 0;
	switch(on)
	{
		case CSI_SUBDEV_STBY_ON:
			vfe_dev_dbg("CSI_SUBDEV_STBY_ON!\n");
			ret = sensor_s_sw_stby(sd, CSI_GPIO_LOW);
			if(ret < 0)
				vfe_dev_err("soft stby falied!\n");
			usleep_range(10000,12000);
			cci_lock(sd);    
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
			cci_unlock(sd);    
			vfe_set_mclk(sd,OFF);
			break;
		case CSI_SUBDEV_STBY_OFF:
			vfe_dev_dbg("CSI_SUBDEV_STBY_OFF!\n");
			cci_lock(sd);    
			vfe_set_mclk_freq(sd,MCLK);
			vfe_set_mclk(sd,ON);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			usleep_range(10000,12000);
			ret = sensor_s_sw_stby(sd, CSI_GPIO_HIGH);
			if(ret < 0)
				vfe_dev_err("soft stby off falied!\n");
			cci_unlock(sd);    
			break;
		case CSI_SUBDEV_PWR_ON:
			vfe_dev_dbg("CSI_SUBDEV_PWR_ON!\n");
			cci_lock(sd);    
			vfe_gpio_set_status(sd,PWDN,1);//set the gpio to output
			vfe_gpio_set_status(sd,RESET,1);//set the gpio to output
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			usleep_range(1000,1200);
			vfe_set_mclk_freq(sd,MCLK);
			vfe_set_mclk(sd,ON);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,POWER_EN,CSI_GPIO_HIGH);
			vfe_set_pmu_channel(sd,IOVDD,ON);
			vfe_set_pmu_channel(sd,AVDD,ON);
			vfe_set_pmu_channel(sd,DVDD,ON);
			vfe_set_pmu_channel(sd,AFVDD,ON);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);
			usleep_range(30000,31000);
			cci_unlock(sd);    
			break;
		case CSI_SUBDEV_PWR_OFF:
			vfe_dev_dbg("CSI_SUBDEV_PWR_OFF!\n");
			cci_lock(sd);   
			vfe_set_mclk(sd,OFF);
			vfe_gpio_write(sd,POWER_EN,CSI_GPIO_LOW);
			vfe_set_pmu_channel(sd,AFVDD,OFF);
			vfe_set_pmu_channel(sd,DVDD,OFF);
			vfe_set_pmu_channel(sd,AVDD,OFF);
			vfe_set_pmu_channel(sd,IOVDD,OFF);  
     		usleep_range(10000,12000);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			vfe_gpio_set_status(sd,RESET,0);//set the gpio to input
			vfe_gpio_set_status(sd,PWDN,0);//set the gpio to input
			cci_unlock(sd);    
			break;
		default:
			return -EINVAL;
	}   
	return 0;
}
 
static int sensor_reset(struct v4l2_subdev *sd, u32 val)
{
	switch(val)
	{
		case 0:
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);
			usleep_range(10000,12000);
			break;
		case 1:
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			usleep_range(10000,12000);
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

static int sensor_detect(struct v4l2_subdev *sd)
{
	data_type rdval;
  
	LOG_ERR_RET(sensor_read(sd, 0x300a, &rdval))
	if(rdval != 0xc8)
		return -ENODEV;
  	
	LOG_ERR_RET(sensor_read(sd, 0x300b, &rdval))
	if(rdval != 0x30)
		return -ENODEV;
  
	return 0;
}

static int sensor_init(struct v4l2_subdev *sd, u32 val)
{
	int ret;
	struct sensor_info *info = to_state(sd);
  
	vfe_dev_dbg("sensor_init\n");
  
	/*Make sure it is a target sensor*/
	ret = sensor_detect(sd);
	if (ret) {
		vfe_dev_err("chip found is not an target chip.\n");
		return ret;
	}
  
	vfe_get_standby_mode(sd,&info->stby_mode);
  
	if((info->stby_mode == HW_STBY || info->stby_mode == SW_STBY) \
			&& info->init_first_flag == 0) {
		vfe_dev_print("stby_mode and init_first_flag = 0\n");
		return 0;
	}
  
	info->focus_status = 0;
	info->low_speed = 0;
	info->width = HXGA_WIDTH;
	info->height = HXGA_HEIGHT;
	info->hflip = 0;
	info->vflip = 0;
	info->gain = 0;

	info->tpf.numerator = 1;            
	info->tpf.denominator = 30;    /* 30fps */    
  
	ret = sensor_write_array(sd, sensor_default_regs, ARRAY_SIZE(sensor_default_regs));  
	if(ret < 0) {
		vfe_dev_err("write sensor_default_regs error\n");
		return ret;
	}
  
	if(info->stby_mode == 0)
		info->init_first_flag = 0;
  
	info->preview_first_flag = 1;
  
	return 0;
}

static long sensor_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	int ret=0;
	struct sensor_info *info = to_state(sd);
	switch(cmd) {
		case GET_CURRENT_WIN_CFG:
			if(info->current_wins != NULL)
			{
				memcpy( arg,
				        info->current_wins,
				        sizeof(struct sensor_win_size) );
				ret=0;
			}
			else
			{
				vfe_dev_err("empty wins!\n");
				ret=-1;
			}
			break;
		case SET_FPS:
			break;
		case ISP_SET_EXP_GAIN:
			sensor_s_exp_gain(sd, (struct sensor_exp_gain *)arg);
			break;
		default:
			return -EINVAL;
	}
	return ret;
}


/*
 * Store information about the video data format. 
 */
static struct sensor_format_struct {
	__u8 *desc;
	//__u32 pixelformat;
	enum v4l2_mbus_pixelcode mbus_code;
	struct regval_list *regs;
	int regs_size;
	int bpp;   /* Bytes per pixel */
}sensor_formats[] = {
	{
		.desc				= "Raw RGB Bayer",
		.mbus_code	= V4L2_MBUS_FMT_SBGGR10_10X1,//V4L2_MBUS_FMT_SGRBG10_10X1,
		.regs 			= sensor_fmt_raw,
		.regs_size 	= ARRAY_SIZE(sensor_fmt_raw),
		.bpp				= 1
	},
};
#define N_FMTS ARRAY_SIZE(sensor_formats)

  

/*
 * Then there is the issue of window sizes.  Try to capture the info here.
 */


static struct sensor_win_size sensor_win_sizes[] = {
	/* hxga: 4000*3000 */
	{
		.width      = HXGA_WIDTH,
		.height     = HXGA_HEIGHT,
		.hoffset    = 0,
		.voffset    = 0,
		.hts        = 4432,
		.vts        = 3044,
		.pclk       = 264*1000*1000,
		.mipi_bps   = 200*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 1<<4,
		.intg_max   = (3044-4)<<4,
		.gain_min   = 1<<4,
		.gain_max   = 16<<4,
		.regs       = sensor_hxga_regs,
		.regs_size  = ARRAY_SIZE(sensor_hxga_regs),
		.set_size   = NULL,
	},

	/* 1080P */
	{
		.width      = HD1080_WIDTH,
		.height     = HD1080_HEIGHT,
		.hoffset    = 0,
		.voffset    = 0,
		.hts        = 3278,
		.vts        = 1220,
		.pclk       = 240*1000*1000,
		.mipi_bps   = 200*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 2,
		.intg_min   = 1<<4,
		.intg_max   = (1220-4)<<4,
		.gain_min   = 1<<4,
		.gain_max   = 16<<4,
		.regs       = sensor_1080p_regs,//
		.regs_size  = ARRAY_SIZE(sensor_1080p_regs),//
		.set_size		= NULL,
	},
	/* 720p */
	{
		.width      = HD720_WIDTH,
		.height     = HD720_HEIGHT,
		.hoffset    = 0,
		.voffset    = 0,
		.hts        = 3278,
		.vts        = 1220,
		.pclk       = 120*1000*1000,
		.mipi_bps   = 200*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 2,
		.intg_min   = 1<<4,
		.intg_max   = (1220-4)<<4,
		.gain_min   = 1<<4,
		.gain_max   = 16<<4,
		.regs       = sensor_720p_regs,//
		.regs_size  = ARRAY_SIZE(sensor_720p_regs),//
		.set_size   = NULL,
	},

	/* VGA */
	{
		.width      = VGA_WIDTH,
		.height     = VGA_HEIGHT,
		.hoffset    = 0,
		.voffset    = 0,
		.hts        = 3278,
		.vts        = 1220,
		.pclk       = 120*1000*1000,
		.mipi_bps   = 200*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 2,
		.intg_min   = 1<<4,
		.intg_max   = (1220-4)<<4,
		.gain_min   = 1<<4,
		.gain_max   = 16<<4,
		.regs       = sensor_vga_regs,
		.regs_size  = ARRAY_SIZE(sensor_vga_regs),
		.set_size   = NULL,
	},
};

#define N_WIN_SIZES (ARRAY_SIZE(sensor_win_sizes))

static int sensor_enum_fmt(struct v4l2_subdev *sd, unsigned index,
                 enum v4l2_mbus_pixelcode *code)
{
	if (index >= N_FMTS)
		return -EINVAL;

	*code = sensor_formats[index].mbus_code;
	return 0;
}

static int sensor_enum_size(struct v4l2_subdev *sd,
                            struct v4l2_frmsizeenum *fsize)
{
	if(fsize->index > N_WIN_SIZES-1)
		return -EINVAL;
  
	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->discrete.width = sensor_win_sizes[fsize->index].width;
	fsize->discrete.height = sensor_win_sizes[fsize->index].height;
  
	return 0;
}


static int sensor_try_fmt_internal(struct v4l2_subdev *sd,
    struct v4l2_mbus_framefmt *fmt,
    struct sensor_format_struct **ret_fmt,
    struct sensor_win_size **ret_wsize)
{
	int index;
	struct sensor_win_size *wsize;
	struct sensor_info *info = to_state(sd);

	for (index = 0; index < N_FMTS; index++)
		if (sensor_formats[index].mbus_code == fmt->code)
			break;

	if (index >= N_FMTS)
		return -EINVAL;

	if (ret_fmt != NULL)
		*ret_fmt = sensor_formats + index;

	/*
	* Fields: the sensor devices claim to be progressive.
	*/
	fmt->field = V4L2_FIELD_NONE;

	/*
	* Round requested image size down to the nearest
	* we support, but not below the smallest.
	*/
	for (wsize = sensor_win_sizes; wsize < sensor_win_sizes + N_WIN_SIZES; wsize++)
		if (fmt->width >= wsize->width && fmt->height >= wsize->height)
			break;

	if (wsize >= sensor_win_sizes + N_WIN_SIZES)
		wsize--;   /* Take the smallest one */
	if (ret_wsize != NULL)
		*ret_wsize = wsize;
	info->current_wins = wsize;  
	/*
	* Note the size we'll actually handle.
	*/
	fmt->width = wsize->width;
	fmt->height = wsize->height;

	return 0;
}

static int sensor_try_fmt(struct v4l2_subdev *sd, 
             struct v4l2_mbus_framefmt *fmt)
{
	return sensor_try_fmt_internal(sd, fmt, NULL, NULL);
}

static int sensor_g_mbus_config(struct v4l2_subdev *sd,
           struct v4l2_mbus_config *cfg)
{
	cfg->type = V4L2_MBUS_CSI2;
	cfg->flags = 0|V4L2_MBUS_CSI2_4_LANE|V4L2_MBUS_CSI2_CHANNEL_0;
  
	return 0;
}


/*
 * Set a format.
 */
static int sensor_s_fmt(struct v4l2_subdev *sd, 
             struct v4l2_mbus_framefmt *fmt)
{
	int ret;
	struct sensor_format_struct *sensor_fmt;
	struct sensor_win_size *wsize;
	struct sensor_info *info = to_state(sd);

	vfe_dev_dbg("sensor_s_fmt\n");
    
	//sensor_write_array(sd, sensor_oe_disable_regs, ARRAY_SIZE(sensor_oe_disable_regs));

	ret = sensor_try_fmt_internal(sd, fmt, &sensor_fmt, &wsize);
	if (ret)
		return ret;

	if(info->capture_mode == V4L2_MODE_VIDEO)
	{
	//video
	}
	else if(info->capture_mode == V4L2_MODE_IMAGE)
	{
	//image
	}
  
	sensor_write_array(sd, sensor_fmt->regs, sensor_fmt->regs_size);

	ret = 0;
	if (wsize->regs)
		LOG_ERR_RET(sensor_write_array(sd, wsize->regs, wsize->regs_size))

	if (wsize->set_size)
		LOG_ERR_RET(wsize->set_size(sd))

	info->fmt = sensor_fmt;
	info->width = wsize->width;
	info->height = wsize->height;
	ov12830_sensor_vts = wsize->vts;
   
	vfe_dev_print("s_fmt set width = %d, height = %d\n",wsize->width,wsize->height);

	if(info->capture_mode == V4L2_MODE_VIDEO)
	{
	//video
	} else {
	//capture image
	}
	
	//sensor_write_array(sd, sensor_oe_enable_regs, ARRAY_SIZE(sensor_oe_enable_regs));
	vfe_dev_print("s_fmt end\n");
	return 0;
}

/*
 * Implement G/S_PARM.  There is a "high quality" mode we could try
 * to do someday; for now, we just do the frame rate tweak.
 */
static int sensor_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *parms)
{
	struct v4l2_captureparm *cp = &parms->parm.capture;
	struct sensor_info *info = to_state(sd);

	if (parms->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;
  
	memset(cp, 0, sizeof(struct v4l2_captureparm));
	cp->capability = V4L2_CAP_TIMEPERFRAME;
	cp->capturemode = info->capture_mode;
     
	return 0;
}

static int sensor_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *parms)
{
	struct v4l2_captureparm *cp = &parms->parm.capture;
	struct sensor_info *info = to_state(sd);
  
	vfe_dev_dbg("sensor_s_parm\n");
  
	if (parms->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;
  
	if (info->tpf.numerator == 0)
		return -EINVAL;
    
	info->capture_mode = cp->capturemode;
  
	return 0;
}


static int sensor_queryctrl(struct v4l2_subdev *sd,
    struct v4l2_queryctrl *qc)
{
	/* Fill in min, max, step and default value for these controls. */
	/* see include/linux/videodev2.h for details */
  
	switch (qc->id) {
	case V4L2_CID_GAIN:
		return v4l2_ctrl_query_fill(qc, 1*16, 64*16-1, 1, 1*16);
	case V4L2_CID_EXPOSURE:
		return v4l2_ctrl_query_fill(qc, 0, 65535*16, 1, 0);
	case V4L2_CID_FRAME_RATE:
		return v4l2_ctrl_query_fill(qc, 15, 120, 1, 120);
	}
	return -EINVAL;
}

static int sensor_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	switch (ctrl->id) {
	case V4L2_CID_GAIN:
		return sensor_g_gain(sd, &ctrl->value);
	case V4L2_CID_EXPOSURE:
		return sensor_g_exp(sd, &ctrl->value);
	}
	return -EINVAL;
}

static int sensor_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct v4l2_queryctrl qc;
	int ret;

	qc.id = ctrl->id;
	ret = sensor_queryctrl(sd, &qc);
	if (ret < 0) {
		return ret;
	}

	if (ctrl->value < qc.minimum || ctrl->value > qc.maximum) {
		vfe_dev_err("max gain qurery is %d,min gain qurey is %d\n",qc.maximum,qc.minimum);
		return -ERANGE;
	}

	switch (ctrl->id) {
	case V4L2_CID_GAIN:
		return sensor_s_gain(sd, ctrl->value);
	case V4L2_CID_EXPOSURE:
		return sensor_s_exp(sd, ctrl->value);
	}
	return -EINVAL;
}


static int sensor_g_chip_ident(struct v4l2_subdev *sd,
    struct v4l2_dbg_chip_ident *chip)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	return v4l2_chip_ident_i2c_client(client, chip, V4L2_IDENT_SENSOR, 0);
}


/* ----------------------------------------------------------------------- */

static const struct v4l2_subdev_core_ops sensor_core_ops = {
	.g_chip_ident = sensor_g_chip_ident,
	.g_ctrl = sensor_g_ctrl,
	.s_ctrl = sensor_s_ctrl,
	.queryctrl = sensor_queryctrl,
	.reset = sensor_reset,
	.init = sensor_init,
	.s_power = sensor_power,
	.ioctl = sensor_ioctl,
};

static const struct v4l2_subdev_video_ops sensor_video_ops = {
	.enum_mbus_fmt = sensor_enum_fmt,
	.enum_framesizes = sensor_enum_size,
	.try_mbus_fmt = sensor_try_fmt,
	.s_mbus_fmt = sensor_s_fmt,
	.s_parm = sensor_s_parm,
	.g_parm = sensor_g_parm,
	.g_mbus_config = sensor_g_mbus_config,
};

static const struct v4l2_subdev_ops sensor_ops = {
	.core = &sensor_core_ops,
	.video = &sensor_video_ops,
};

/* ----------------------------------------------------------------------- */
static struct cci_driver cci_drv = {
	.name = SENSOR_NAME,
	.addr_width = CCI_BITS_16,
	.data_width = CCI_BITS_8,
};

static int sensor_probe(struct i2c_client *client,
      const struct i2c_device_id *id)
{
	struct v4l2_subdev *sd;
	struct sensor_info *info;
	info = kzalloc(sizeof(struct sensor_info), GFP_KERNEL);
	if (info == NULL)
		return -ENOMEM;
	sd = &info->sd;
	glb_sd = sd;
	cci_dev_probe_helper(sd, client, &sensor_ops, &cci_drv);

	info->fmt = &sensor_formats[0];
	info->af_first_flag = 1;
	info->init_first_flag = 1;

	return 0;
}
static int sensor_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd;
	sd = cci_dev_remove_helper(client, &cci_drv);
	kfree(to_state(sd));
	return 0;
}

static const struct i2c_device_id sensor_id[] = {
	{SENSOR_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, sensor_id);


static struct i2c_driver sensor_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = SENSOR_NAME,
	},
	.probe = sensor_probe,
	.remove = sensor_remove,
	.id_table = sensor_id,
};
static __init int init_sensor(void)
{
	return cci_dev_init_helper(&sensor_driver);
}

static __exit void exit_sensor(void)
{
	cci_dev_exit_helper(&sensor_driver);
}

module_init(init_sensor);
module_exit(exit_sensor);

