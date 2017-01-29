/*
 * A V4L2 driver for AR0330 Raw cameras.
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

MODULE_AUTHOR("Chomoly");
MODULE_DESCRIPTION("A low-level driver for Aptina ar0330 Raw sensors");
MODULE_LICENSE("GPL");

//for internel driver debug
#define DEV_DBG_EN      0 
#if(DEV_DBG_EN == 1)    
#define vfe_dev_dbg(x,arg...) printk("[AR0330 Raw]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...) 
#endif
#define vfe_dev_err(x,arg...) printk("[AR0330 Raw]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[AR0330 Raw]"x,##arg)

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
#define V4L2_IDENT_SENSOR  0x0330

/*
 * Our nominal (default) frame rate.
 */

#define SENSOR_FRAME_RATE 30


/*
 * The ar0330 i2c address
 */
#define I2C_ADDR 0x20
#define SENSOR_NAME "ar0330"
//#define rev_1
//#define sequnser_a
//#define sequnser_b
//#define version_4

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


/*
 * The default register settings
 *
 */

static struct regval_list sensor_default_regs[] = 
{
#ifdef sequnser_a
	0x3064, 0x1802,
	0x3078, 0x0001,
	0x31E0, 0x0703,
	0x306E, 0xFC10,

	//0x300e, 0x0010,
	0x30f0, 0x1200,
	0x3072, 0x0000,
	0x30BA, 0x002C,
	0x30FE, 0x0080,
	0x31E0, 0x0000,
	//0x3ECE, 0x00FF,
	0x3ED0, 0xE4F6,
	0x3ED2, 0x0146,
	0x3ED4, 0x8F6C,
	0x3ED6, 0x66CC,
	0x3ED8, 0x8C42,
	0x3EDA, 0x8822,
	0x3EDC, 0x2222,
	0x3EDE, 0x22C0,
	0x3EE0, 0x1500,
	0x3EE6, 0x0080,
	0x3EE8, 0x2027,
	0x3EEA, 0x001D,
	0x3F06, 0x046A,

	0x301a, 0x10c8,

	0x3088, 0x8000,	
	0x3086, 0x4540,	
	0x3086, 0x6134,	
	0x3086, 0x4A31,	
	0x3086, 0x4342,	
	0x3086, 0x4560,	
	0x3086, 0x2714,	
	0x3086, 0x3DFF,	
	0x3086, 0x3DFF,	
	0x3086, 0x3DEA,	
	0x3086, 0x2704,	
	0x3086, 0x3D10,	
	0x3086, 0x2705,	
	0x3086, 0x3D10,	
	0x3086, 0x2715,	
	0x3086, 0x3527,	
	0x3086, 0x053D,	
	0x3086, 0x1045,	
	0x3086, 0x4027,	
	0x3086, 0x0427,	
	0x3086, 0x143D,	
	0x3086, 0xFF3D,	
	0x3086, 0xFF3D,	
	0x3086, 0xEA62,	
	0x3086, 0x2728,	
	0x3086, 0x3627,	
	0x3086, 0x083D,
	0x3086, 0x6444,
	0x3086, 0x2C2C,
	0x3086, 0x2C2C,
	0x3086, 0x4B01,
	0x3086, 0x432D,
	0x3086, 0x4643,
	0x3086, 0x1647,
	0x3086, 0x435F,
	0x3086, 0x4F50,
	0x3086, 0x2604,
	0x3086, 0x2684,
	0x3086, 0x2027,
	0x3086, 0xFC53,
	0x3086, 0x0D5C,
	0x3086, 0x0D60,
	0x3086, 0x5754,
	0x3086, 0x1709,
	0x3086, 0x5556,
	0x3086, 0x4917,
	0x3086, 0x145C,
	0x3086, 0x0945,
	0x3086, 0x0045,
	0x3086, 0x8026,
	0x3086, 0xA627,
	0x3086, 0xF817,
	0x3086, 0x0227,
	0x3086, 0xFA5C,
	0x3086, 0x0B5F,
	0x3086, 0x5307,
	0x3086, 0x5302,
	0x3086, 0x4D28,
	0x3086, 0x6C4C,
	0x3086, 0x0928,
	0x3086, 0x2C28,
	0x3086, 0x294E,
	0x3086, 0x1718,
	0x3086, 0x26A2,
	0x3086, 0x5C03,
	0x3086, 0x1744,
	0x3086, 0x2809,
	0x3086, 0x27F2,
	0x3086, 0x1714,
	0x3086, 0x2808,
	0x3086, 0x164D,
	0x3086, 0x1A26,
	0x3086, 0x8317,
	0x3086, 0x0145,
	0x3086, 0xA017,
	0x3086, 0x0727,
	0x3086, 0xF317,
	0x3086, 0x2945,
	0x3086, 0x8017,
	0x3086, 0x0827,
	0x3086, 0xF217,
	0x3086, 0x285D,
	0x3086, 0x27FA,
	0x3086, 0x170E,
	0x3086, 0x2681,
	0x3086, 0x5300,
	0x3086, 0x17E6,
	0x3086, 0x5302,
	0x3086, 0x1710,
	0x3086, 0x2683,
	0x3086, 0x2682,
	0x3086, 0x4827,
	0x3086, 0xF24D,
	0x3086, 0x4E28,
	0x3086, 0x094C,
	0x3086, 0x0B17,
	0x3086, 0x6D28,
	0x3086, 0x0817,
	0x3086, 0x014D,
	0x3086, 0x1A17,
	0x3086, 0x0126,
	0x3086, 0x035C,
	0x3086, 0x0045,
	0x3086, 0x4027,
	0x3086, 0x9017,
	0x3086, 0x2A4A,
	0x3086, 0x0A43,
	0x3086, 0x160B,
	0x3086, 0x4327,
	0x3086, 0x9445,
	0x3086, 0x6017,
	0x3086, 0x0727,
	0x3086, 0x9517,
	0x3086, 0x2545,
	0x3086, 0x4017,
	0x3086, 0x0827,
	0x3086, 0x905D,
	0x3086, 0x2808,
	0x3086, 0x530D,
	0x3086, 0x2645,
	0x3086, 0x5C01,
	0x3086, 0x2798,
	0x3086, 0x4B12,
	0x3086, 0x4452,
	0x3086, 0x5117,
	0x3086, 0x0260,
	0x3086, 0x184A,
	0x3086, 0x0343,
	0x3086, 0x1604,
	0x3086, 0x4316,
	0x3086, 0x5843,
	0x3086, 0x1659,
	0x3086, 0x4316,
	0x3086, 0x5A43,
	0x3086, 0x165B,
	0x3086, 0x4327,
	0x3086, 0x9C45,
	0x3086, 0x6017,
	0x3086, 0x0727,
	0x3086, 0x9D17,
	0x3086, 0x2545,
	0x3086, 0x4017,
	0x3086, 0x1027,
	0x3086, 0x9817,
	0x3086, 0x2022,
	0x3086, 0x4B12,
	0x3086, 0x442C,
	0x3086, 0x2C2C,
	0x3086, 0x2C00,
	/*
	*
	*pll setting
	*
	*/

	{0x301a,0x10c0},	
	{0x302A, 0x0006},	
	{0x302C, 0x0001},	
	{0x302E, 0x0004},	
	{0x3030, 0x0062},
	{0x3036, 0x0006},	
	{0x3038, 0x0001},
	{0x31ac, 0x0c0c},
	{0x301a,0x10d0},
	//{0x301a,0x10d8},
#endif
#ifdef sequnser_b
	0x3064, 0x1802,
	0x3078, 0x0001,
	0x31E0, 0x0703,
	0x306E, 0xFC10,

	//0x300e, 0x0010,
	0x30f0, 0x1208,
	0x3072, 0x0000,
	0x30BA, 0x002C,
	0x30FE, 0x0080,
	0x31E0, 0x0000,
	0x3ECE, 0x00FF,
	0x3ED0, 0xE4F6,
	0x3ED2, 0x0146,
	0x3ED4, 0x8F6C,
	0x3ED6, 0x66CC,
	0x3ED8, 0x8C42,
	0x3EDA, 0x889b,
	0x3EDC, 0x8863,
	0x3EDE, 0xaa04,
	0x3EE0, 0x15f0,
	0x3EE6, 0x008c,
	0x3EE8, 0x2024,
	0x3EEA, 0xff1f,
	0x3F06, 0x046A,

	0x301a, 0x10c8,

#endif
#ifdef version_4
	0x30f0, 0x1200,
	0x3072, 0x0007,
	0x3ED2, 0x0146,

	0x3ED6, 0x66CC,
	0x3ED8, 0x8C42,
	{0x3064,0x1802},
	{0x3078,0x0001},
	{0x31E0,0x0703},
	{0x306E,0xFC10},
	{0x305e,0x0080},
	{0x3ed2,0x0146},
	{0x301e,0x0100},

	/*
	*
	*pll setting
	*
	*/

	{0x301a,0x10c0},	
	{0x302A, 0x0006},	
	{0x302C, 0x0001},	
	{0x302E, 0x0004},	
	{0x3030, 0x0062},
	{0x3036, 0x0006},	
	{0x3038, 0x0001},
	{0x31ac, 0x0c0c},
	{0x301a,0x10d0},
	//{0x301a,0x10d8},
#endif
	{0x30f0, 0x1208},
	{0x3072, 0x0008},
	{0x3064,0x1802},
	{0x3078,0x0001},
	{0x31E0,0x0703},
	{0x306E,0xFC10},
	{0x305e,0x00a0},//global gain
	{0x3ed2,0x0146},
	{0x3eda,0x88bc},
	{0x3edc,0xaa63},//last modified

	{0x301e,0x0100},

	/*
	*
	*pll setting
	*
	*/
		  
	{0x301a,0x10c0},  
	{0x302A, 0x0006}, 
	{0x302C, 0x0001}, 
	{0x302E, 0x0004}, 
	{0x3030, 0x0062},
	{0x3036, 0x0006}, 
	{0x3038, 0x0001},
	{0x31ac, 0x0c0c},
	{0x301a,0x10d0},
	//{0x301a,0x10d8},
};

static struct regval_list sensor_1080p_regs[] = { //1080: 1920*1080@30fps EIS
	/*for 1080P*/
	/* 2304*1296 */
	{0x301a,0x10d0},

	{0x31AE,0x0301},
	{0x3004,0x0006},		
	{0x3008,0x0905},
	{0x3002,0x0078}, 
	{0x3006,0x0587},
	//{0x301A,0x10dc},
	
	{0x30A2,0x0001},
	{0x30A6,0x0001},
	
	{0x3040,0x0000},
	{0x300c,0x04e0},
	{0x300A,0x051C},
	//{0x300A,0x028e},
	{0x3014,0x0000},
	//{0x3012,0x051B},
	{0x3042,0x0000},
	{0x30BA,0x002c},

	//{0x3088,0x80BA},
	//{0x3086,0xE653},
	{0x301A,0x10d4}
};

static struct regval_list sensor_720p_regs[] = { //720: 1280*720@60fps
	{0x301a,0x10d8},

	{0x31AE,0x0301},
	{0x3004,0x0200},		
	{0x3008,0x06ff},
	{0x3002,0x019c}, 
	{0x3006,0x046b},
	//{0x301A,0x10dc},
	
	{0x30A2,0x0001},
	{0x30A6,0x0001},
	
	{0x3040,0x0000},
	{0x300c,0x03f6},
	{0x300A,0x0314},
	//{0x300A,0x028e},
	{0x3014,0x0000},
	{0x3012,0x0311},
	{0x3042,0x03c8},
	{0x30BA,0x006c},
	{0x3088,0x80BA},
	{0x3086,0x0253},
	//{0x3086,0x0253},
		
	{0x301A,0x10dc}
};

static struct regval_list sensor_vga_regs[] = { //VGA:  640*480
	//DVP_VGA_60fps
	{0x301a,0x10d8},

	{0x31AE,0x0301},
	{0x3004,0x00c0},		
	{0x3008,0x083f},
	{0x3002,0x0030}, 
	{0x3006,0x05cf},
	//{0x301A,0x10dc},
	
	{0x30A2,0x0005},
	{0x30A6,0x0005},
	
	{0x3040,0x0000},
	{0x300c,0x04da},
	{0x300A,0x0291},
	//{0x300A,0x028e},
	{0x3014,0x0000},
	{0x3012,0x0290},
	{0x3042,0x02a1},
	{0x30BA,0x002c},
	{0x3088,0x80BA},
	{0x3086,0xe653},
	//{0x3086,0x0253},
		
	{0x301A,0x10dc}
};

/*
 * Here we'll try to encapsulate the changes for just the output
 * video format.
 * 
 */

static struct regval_list sensor_fmt_raw[] = {
    //{REG_TERM,VAL_TERM},
};

/*
 * Code for dealing with controls.
 * fill with different sensor module
 * different sensor module has different settings here
 * if not support the follow function ,retrun -EINVAL
 */
static int sensor_g_exp(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);
	
	*value = info->exp;
	vfe_dev_dbg("sensor_get_exposure = %d\n", info->exp);
	return 0;
}

static int sensor_s_exp(struct v4l2_subdev *sd, unsigned int exp_val)
{
    struct sensor_info *info = to_state(sd);

    vfe_dev_dbg("sensor_set_exposure = %d\n", exp_val);
    if(exp_val>0xffffff)
        exp_val=0xfffff0;
    if(exp_val<16)
        exp_val=16;
  
    exp_val=(exp_val)>>4;//rounding to 1
  
    sensor_write(sd, 0x3012,exp_val);//coarse integration time
    
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
	unsigned short tmp_gain = 0;

	 if (gain_val < 16)
	 	gain_val = 16;

    if (16<= gain_val*100 && gain_val*100 < (103*16) )
        sensor_write(sd,0x3060,0x0000);
    else if ((103*16) <= gain_val*100 && gain_val*100 < (107*16))
        sensor_write(sd,0x3060,0x0001);
    else if ((107*16) <= gain_val*100 && gain_val*100 < (110*16))
        sensor_write(sd,0x3060,0x0002);
    else if ((110*16) <= gain_val*100 && gain_val*100 < (114*16))
        sensor_write(sd,0x3060,0x0003);
    else if ((114*16) <= gain_val*100 && gain_val*100 < (119*16))
        sensor_write(sd,0x3060,0x0004);
    else if ((119*16) <= gain_val*100 && gain_val*100 < (123*16))
        sensor_write(sd,0x3060,0x0005);
    else if ((123*16) <= gain_val*100 && gain_val*100 < (128*16))
        sensor_write(sd,0x3060,0x0006);
    else if ((128*16) <= gain_val*100 && gain_val*100 < (133*16))
        sensor_write(sd,0x3060,0x0007);
    else if ((133*16) <= gain_val*100 && gain_val*100 < (139*16))
        sensor_write(sd,0x3060,0x0008);
    else if ((139*16) <= gain_val*100 && gain_val*100 < (145*16))
        sensor_write(sd,0x3060,0x0009);
    else if ((145*16) <= gain_val*100 && gain_val*100 < (152*16))
        sensor_write(sd,0x3060,0x000a);
    else if ((152*16) <= gain_val*100 && gain_val*100 < (160*16))
        sensor_write(sd,0x3060,0x000b);
    else if ((160*16) <= gain_val*100 && gain_val*100 < (168*16))
        sensor_write(sd,0x3060,0x000c);
    else if ((168*16) <= gain_val*100 && gain_val*100 < (178*16))
        sensor_write(sd,0x3060,0x000d);
    else if ((178*16) <= gain_val*100 && gain_val*100 < (188*16))
        sensor_write(sd,0x3060,0x000e);
    else if ((188*16) <= gain_val*100 && gain_val*100 < (200*16))
        sensor_write(sd,0x3060,0x000f);
	else if (200*16 <= gain_val*100 && gain_val*100 < (213*16))
		sensor_write(sd,0x3060,0x0010);
	else if ((213*16) <= gain_val*100 && gain_val*100 < (229*16))
		sensor_write(sd,0x3060,0x0012);
	else if ((229*16) <= gain_val*100 && gain_val*100 < (246*16))
		sensor_write(sd,0x3060,0x0014);
	else if ((246*16) <= gain_val*100 && gain_val*100 < (267*16))
		sensor_write(sd,0x3060,0x0016);
	else if ((267*16) <= gain_val*100 && gain_val*100 < (291*16))
		sensor_write(sd,0x3060,0x0018);
	else if ((291*16) <= gain_val*100 && gain_val*100 < (320*16))
		sensor_write(sd,0x3060,0x001a);
	else if ((320*16) <= gain_val*100 && gain_val*100 < (800*16))
	{
		tmp_gain = (2449700 + gain_val*564520 - gain_val*gain_val*1646)/1000000;
		sensor_write(sd,0x3060,tmp_gain);
	}
	else if ((800*16) <= gain_val*100 )
		sensor_write(sd,0x3060,0x0030);

	//Digit Gain
	if((1*16 <= gain_val) && (gain_val<= 8*16))
		sensor_write(sd,0x305e, 128);
	else if ((8*16 < gain_val) && (gain_val <= 10*16))
		sensor_write(sd,0x305e, gain_val);
   		
	sensor_read(sd,0x3060,&tmp_gain);
		  
	info->gain = gain_val;
	return 0;
}

static int ar0330_sensor_vts;

static int sensor_s_exp_gain(struct v4l2_subdev *sd, struct sensor_exp_gain *exp_gain)
{
    int exp_val, gain_val,shutter,frame_length;  
    struct sensor_info *info = to_state(sd);
    exp_val = exp_gain->exp_val;
    gain_val = exp_gain->gain_val;
    if(gain_val<1*16)
        gain_val=16;
    if(gain_val>64*16-1)
        gain_val=64*16-1;
    if(exp_val>0xfffff)
        exp_val=0xfffff;

    shutter = exp_val/16;
    if(shutter > ar0330_sensor_vts - 4)
    	frame_length = shutter + 4;
    else
    	frame_length = ar0330_sensor_vts;

	printk("norm exp_val = %d,gain_val = %d\n",exp_val,gain_val);
	sensor_write(sd, 0x300A,frame_length);//coarse integration time
	sensor_s_exp(sd,exp_val);
	sensor_s_gain(sd,gain_val);
	info->exp = exp_val;
	info->gain = gain_val;
	return 0;
}
static int sensor_s_sw_stby(struct v4l2_subdev *sd, int on_off)
{
	int ret ;
    data_type rdtmp;
	ret = sensor_read(sd,0x301a,&rdtmp);
	if (ret!=0)
		return ret;
	if (on_off == 1)
		sensor_write(sd,0x301a,(rdtmp & 0xfff8));
	else 
		sensor_write(sd,0x301a,rdtmp );
	return ret;
}
/*
 * Stuff that knows about the sensor.
 */
static int sensor_power(struct v4l2_subdev *sd, int on)
{
	int ret;

	ret = 0;
	switch(on)
	{
		case CSI_SUBDEV_STBY_ON:
			vfe_dev_dbg("CSI_SUBDEV_STBY_ON!\n");
			ret = sensor_s_sw_stby(sd, CSI_GPIO_HIGH);
			if(ret < 0)
				vfe_dev_err("soft stby falied!\n");
			usleep_range(1000,1200);
			cci_lock(sd);
			vfe_set_mclk(sd,OFF);
			cci_unlock(sd);  
			break;
		case CSI_SUBDEV_STBY_OFF:
			vfe_dev_dbg("CSI_SUBDEV_STBY_OFF!\n");
			cci_lock(sd);    
			vfe_set_mclk_freq(sd,MCLK);
			vfe_set_mclk(sd,ON);
			usleep_range(1000,1200);
			cci_unlock(sd);        
			ret = sensor_s_sw_stby(sd, CSI_GPIO_LOW);
			if(ret < 0)
				vfe_dev_err("soft stby off falied!\n");
			usleep_range(1000,1200);
			break;
		case CSI_SUBDEV_PWR_ON:
			vfe_dev_dbg("CSI_SUBDEV_PWR_ON!\n");
			cci_lock(sd);
			vfe_gpio_set_status(sd,RESET,1);//set the gpio to output
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);  
			vfe_set_pmu_channel(sd,AVDD,ON);	
			vfe_set_pmu_channel(sd,DVDD,ON);
			vfe_set_pmu_channel(sd,IOVDD,ON);
			vfe_set_mclk_freq(sd,MCLK);
			vfe_set_mclk(sd,ON);
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			usleep_range(2000,2200);
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);
			usleep_range(6000,7000);
			cci_unlock(sd);  
			break;
		case CSI_SUBDEV_PWR_OFF:
			vfe_dev_dbg("CSI_SUBDEV_PWR_OFF!\n");
			cci_lock(sd);
			vfe_gpio_set_status(sd,PWDN,1);//set the gpio to output
			vfe_gpio_set_status(sd,RESET,1);//set the gpio to output
			vfe_gpio_write(sd,POWER_EN,CSI_GPIO_LOW);
			vfe_set_pmu_channel(sd,IOVDD,OFF);
			vfe_set_pmu_channel(sd,DVDD,OFF);
			vfe_set_pmu_channel(sd,AVDD,OFF);
			vfe_set_mclk(sd,OFF);
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
    unsigned short rdval = 0;
    LOG_ERR_RET(sensor_read(sd, 0x3000, &rdval))
    if(rdval != 0x2604)
    {
        printk(KERN_DEBUG"*********sensor error,read id is %x.\n",rdval);
        return -ENODEV;
    }
    else
    {
        printk(KERN_DEBUG"*********find ar0330 raw data camera sensor now.\n");
  	    return 0;
    }
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
    info->width = HD1080_WIDTH;
    info->height = HD1080_HEIGHT;
	info->hflip = 0;
	info->vflip = 0;
	info->gain = 0;

	info->tpf.numerator = 1;            
	info->tpf.denominator = 30;    /* 30fps */ 
	sensor_write(sd,0x301a,0x00d9);

	usleep_range(34000,35000);
	sensor_write(sd,0x31ae,0x0301);
	sensor_write(sd,0x301a,0x10d8);
	usleep_range(34000,35000);
#ifdef rev_1
	char tmpv=0;
	sensor_write(sd,0x300e,0x10);
	sensor_read(sd,0x300e,&tmpv);
	printk("the 300e value is 0x%x\n",tmpv);
#endif
	ret = sensor_write_array(sd, sensor_default_regs, ARRAY_SIZE(sensor_default_regs));
	if(ret < 0) {
		vfe_dev_err("write sensor_default_regs error\n");
		return ret;
	}
#ifdef sequnser_b
	usleep_range(100000,105000);
	ret = sensor_write_array(sd, default2, ARRAY_SIZE(default2));  
	if(ret < 0) {
		vfe_dev_err("write default2 error\n");
		return ret;
	}
#endif
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
			ret = sensor_s_exp_gain(sd, (struct sensor_exp_gain *)arg);
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
		.desc		= "Raw RGB Bayer",
		.mbus_code = V4L2_MBUS_FMT_SGRBG12_12X1,
		.regs 		= sensor_fmt_raw,
		.regs_size = ARRAY_SIZE(sensor_fmt_raw),
		.bpp		= 1
	},
};
#define N_FMTS ARRAY_SIZE(sensor_formats)

/*
 * Then there is the issue of window sizes.  Try to capture the info here.
 */
static struct sensor_win_size sensor_win_sizes[] = {
	/* qsxga: 2304*1296 */
	{
		.width      = 2304,
		.height     = 1296,
		.hoffset    = 0,
		.voffset    = 0,
		.hts        = 2496,
		.vts        = 1308,
		.pclk       = 98*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 1<<4,
		.intg_max   = 1900<<4,//1308<<4,
		.gain_min   = 1<<4,
		.gain_max   = 10<<4,
		.regs       = sensor_1080p_regs,
		.regs_size  = ARRAY_SIZE(sensor_1080p_regs),
		.set_size   = NULL,
	},
	/* 1080P */
	{
		.width	  = HD1080_WIDTH,
		.height 	  = HD1080_HEIGHT,
		.hoffset	  = (2304-1920)/2,//57,
		.voffset	  = (1296-1080)/2,//108,
		.hts        = 2496,//2484,//2497,
		.vts        = 1308,//1315,//1308,
		.pclk       = 98*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 1<<4,
		.intg_max   = 1308<<4,//1308<<4,//1315<<4,//1308<<4,
		.gain_min   = 1<<4,
		.gain_max   = 10<<4,
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
		.hts        = 2028,
		.vts        = 788,
		.pclk       = 98*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 1,
		.intg_max   = 788<<4,
		.gain_min   = 1<<4,
		.gain_max   = 8<<4,
		.regs			  = sensor_720p_regs,//
		.regs_size	= ARRAY_SIZE(sensor_720p_regs),//
		.set_size		= NULL,
	},
	/* VGA */
	{
		.width	  = VGA_WIDTH,
		.height 	  = VGA_HEIGHT,
		.hoffset	  = 0,
		.voffset	  = 0,
		.hts        = 2484,//limited by sensor
		.vts        = 657,
		.pclk       = 98*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 1,
		.intg_max   = 657<<4,
		.gain_min   = 1<<4,
		.gain_max   = 8<<4,
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
	/*
	* Note the size we'll actually handle.
	*/
	fmt->width = wsize->width;
	fmt->height = wsize->height;
	info->current_wins = wsize;
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
  cfg->type = V4L2_MBUS_PARALLEL;
  cfg->flags = V4L2_MBUS_MASTER | VREF_POL | HREF_POL | CLK_POL ;
  
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
	
    LOG_ERR_RET(sensor_write_array(sd, sensor_fmt->regs, sensor_fmt->regs_size))
    ret = 0;
    if (wsize->regs)
        LOG_ERR_RET(sensor_write_array(sd, wsize->regs, wsize->regs_size))
    if (wsize->set_size)
        LOG_ERR_RET(wsize->set_size(sd))

    info->fmt = sensor_fmt;
    info->width = wsize->width;
    info->height = wsize->height;
    ar0330_sensor_vts = wsize->vts;
    // show_regs_array(sd,sensor_1080p_regs);

    vfe_dev_print("s_fmt set width = %d, height = %d\n",wsize->width,wsize->height);
    if(info->capture_mode == V4L2_MODE_VIDEO)
    {
        //video
    } else {
        //capture image
    }
    //sensor_write_array(sd, sensor_oe_enable_regs, ARRAY_SIZE(sensor_oe_enable_regs));
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
    	return v4l2_ctrl_query_fill(qc, 1*16, 16*9-1, 1, 16);
    case V4L2_CID_EXPOSURE:
    	return v4l2_ctrl_query_fill(qc, 1, 65536*16, 1, 1);
    case V4L2_CID_FRAME_RATE:
    	return v4l2_ctrl_query_fill(qc, 15, 120, 1, 30);
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
    .data_width = CCI_BITS_16,
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
