/*
 * A V4L2 driver for ov5640 cameras.
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
MODULE_DESCRIPTION("A low-level driver for ov7736 sensors");
MODULE_LICENSE("GPL");

#define AF_WIN_NEW_COORD
//for internel driver debug
#define DEV_DBG_EN      01
#if(DEV_DBG_EN == 1)
#define vfe_dev_dbg(x,arg...) printk("[OV7736]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...)
#endif
#define vfe_dev_err(x,arg...) printk("[OV7736]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[OV7736]"x,##arg)

#define CAP_BDG 0
#if(CAP_BDG == 1)
#define vfe_dev_cap_dbg(x,arg...) printk("[OV7736_CAP_DBG]"x,##arg)
#else
#define vfe_dev_cap_dbg(x,arg...)
#endif

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
#define VREF_POL          V4L2_MBUS_VSYNC_ACTIVE_LOW
#define HREF_POL          V4L2_MBUS_HSYNC_ACTIVE_HIGH
#define CLK_POL           V4L2_MBUS_PCLK_SAMPLE_RISING
#define V4L2_IDENT_SENSOR 0x7736

#define SENSOR_NAME "ov7736"

/*
 * Our nominal (default) frame rate.
 */
#define SENSOR_FRAME_RATE 30

/*
 * The ov5640 sits on i2c with ID 0x78
 */
#define I2C_ADDR 0x42

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

static struct regval_list sensor_default_regs[] = {
	{0x3008,0x82}, // software reset
	// delay 5ms here
	{0x3008,0x42}, // software power down
	{0x3630,0x11},
	{0x3104,0x03},
	{0x3017,0x7f}, // output enable
	{0x3018,0xfc}, // output enable
	{0x3600,0x1c},
	{0x3602,0x04},
	{0x3611,0x44},
	{0x3612,0x63},
	{0x3631,0x22},
	{0x3622,0x00}, // binning
	{0x3633,0x25},
	{0x370b,0x43},

	{0x401c,0x00},
	{0x401e,0x11},
	{0x4702,0x01},
	{0x3a00,0x7a}, // night mode off
	{0x3a02,0x04},
	{0x3a03,0xe0},
	{0x3a14,0x04},
	{0x3a15,0xe0},
	{0x3a18,0x00}, // gain ceiling
	{0x3a19,0x30}, // gain ceiling
	{0x300f,0x08}, // PLL	//88
	{0x3011,0x04}, // PLL
	//--- format ---
	{0x4303,0xff}, // Y max
	{0x4307,0xff}, // U max
	{0x430b,0xff}, // V max
	{0x4305,0x00}, // Y min
	{0x4309,0x00}, // U min
	{0x430d,0x00}, // V min
	{0x4001,0x02},
	{0x4004,0x06},
	//	{0x4708,0x07},

	//--- timing ---
	{0x3800,0x00}, // H start
	{0x3801,0x8e}, // H start
	{0x3810,0x08}, // H off
	{0x3811,0x02}, // V off
	{0x380c,0x03}, // HTS
	{0x380d,0x20}, // HTS
	{0x380e,0x01}, // VTS
	{0x380f,0xf4}, // VTS
	//---banding filter ---
	{0x3a09,0x96}, // B50 step

	{0x3a0b,0x7d}, // B60 step
	{0x4300,0x32}, // YUV 422
	{0x501f,0x01}, // YUV 422
	{0x5000,0x4f}, // ISP lenc off, gamma on, awb gain on, white pixel on, black pixel on,

	{0x5001,0x47}, // ISP SDE on, UV average on, color matrix on, awb on
	{0x370d,0x0b}, // vertical binning
	{0x3715,0x1a},
	{0x370e,0x00},
	{0x3713,0x08},
	{0x3703,0x2c},
	{0x3620,0xc2},
	{0x3714,0x36},
	{0x3716,0x01},
	{0x3623,0x03},
	//--- 50/60 ---
	{0x3c00,0x04}, // manual 50hz
	{0x3c01,0xb2}, // auto detection off
	{0x3c04,0x12}, // detection TH sum1
	{0x3c05,0x60}, // detection TH sum2
	{0x3c06,0x00}, // TH luminance 1
	{0x3c07,0x20}, // TH luminance 1
	{0x3c08,0x00}, // TH luminance 2
	{0x3c09,0xc2}, // TH luminance 2
	{0x300d,0x22},
	{0x3c0a,0x9c}, // sample number
	{0x3c0b,0x40}, // sample number
	{0x3008,0x02}, // wake up from software power down
	{0x5180,0x02}, // awb
	{0x5181,0x02}, // awb

	{0x3818,0x80},

	{0x5000,0xcf}, // ISP lenc on, gamma on, awb gain on, white pixel on, black pixel on,

	//--- color matrix ---
	{0x5380,0x42},
	{0x5381,0x33},
	{0x5382,0x0f},
	{0x5383,0x0b},
	{0x5384,0x42},
	{0x5385,0x4d},
	{0x5392,0x1e},
	//--- lens correction ---
	//--- Sekonix Lens ---
	{0x5801,0x00},
	{0x5802,0x50},
	{0x5803,0x40},
	{0x5804,0x1c},
	{0x5805,0x12},
	{0x5806,0x10},
	//--- special effects ---
	{0x5001,0xc7},
	{0x5580,0x06},
	{0x5583,0x40},
	{0x5584,0x26},
	{0x5585,0x20},
	{0x5589,0x10},
	{0x558a,0x00},
	{0x558b,0x3e},
};

static struct regval_list sensor_vga_regs[] = { //VGA:  640*480
	{0x3008,0x42}, // software power down
	{0x3630,0x11},
	{0x3104,0x03},
	{0x3017,0x7f}, // output enable
	{0x3018,0xfc}, // output enable
	{0x3600,0x1c},
	{0x3602,0x04},
	{0x3611,0x44},
	{0x3612,0x63},
	{0x3631,0x22},
	{0x3622,0x00}, // binning
	{0x3633,0x25},
	{0x370b,0x43},

	
	{0x401c,0x00},
	{0x401e,0x11},
	{0x4702,0x01},
	{0x3a00,0x7a}, // night mode off
	{0x3a18,0x00}, // gain ceiling
	{0x3a19,0x30}, // gain ceiling
	{0x300f,0x08}, // PLL	//88
	{0x3011,0x04}, // PLL
	//--- format ---
	{0x4303,0xff}, // Y max
	{0x4307,0xff}, // U max
	{0x430b,0xff}, // V max
	{0x4305,0x00}, // Y min
	{0x4309,0x00}, // U min
	{0x430d,0x00}, // V min
	{0x4001,0x02},
	{0x4004,0x06},
	//	  {0x4708,0x07},

	//--- timing ---
	{0x3800,0x00}, // H start
	{0x3801,0x8e}, // H start
	{0x3810,0x08}, // H off
	{0x3811,0x02}, // V off
	{0x380c,0x03}, // HTS
	{0x380d,0x20}, // HTS
	{0x380e,0x01}, // VTS
	{0x380f,0xf4}, // VTS
	//---banding filter ---
	{0x3a09,0x96}, // B50 step

	{0x3a0b,0x7d}, // B60 step
	{0x4300,0x32}, // YUV 422
	{0x501f,0x01}, // YUV 422
	{0x5000,0x4f}, // ISP lenc off, gamma on, awb gain on, white pixel on, black pixel on,

	{0x5001,0x47}, // ISP SDE on, UV average on, color matrix on, awb on
	{0x370d,0x0b}, // vertical binning
	{0x3715,0x1a},
	{0x370e,0x00},
	{0x3713,0x08},
	{0x3703,0x2c},
	{0x3620,0xc2},
	{0x3714,0x36},
	{0x3716,0x01},
	{0x3623,0x03},
	//--- 50/60 ---
	{0x3c00,0x04}, // manual 50hz
	{0x3c01,0xb2}, // auto detection off
	{0x3c04,0x12}, // detection TH sum1
	{0x3c05,0x60}, // detection TH sum2
	{0x3c06,0x00}, // TH luminance 1
	{0x3c07,0x20}, // TH luminance 1
	{0x3c08,0x00}, // TH luminance 2
	{0x3c09,0xc2}, // TH luminance 2
	{0x300d,0x22},
	{0x3c0a,0x9c}, // sample number
	{0x3c0b,0x40}, // sample number
	{0x3008,0x02}, // wake up from software power down
	{0x5180,0x02}, // awb
	{0x5181,0x02}, // awb

	//--- color matrix ---
	{0x5380,0x42},
	{0x5381,0x33},
	{0x5382,0x0f},
	{0x5383,0x0b},
	{0x5384,0x42},
	{0x5385,0x4d},
	{0x5392,0x1e},
	//--- lens correction ---
	//--- Sekonix Lens ---
	{0x5801,0x00},
	{0x5802,0x50},
	{0x5803,0x40},
	{0x5804,0x25},
	{0x5805,0x12},
	{0x5806,0x10},
	//--- special effects ---
	{0x5001,0xc7},
	{0x5580,0x06},
	{0x5583,0x40},
	{0x5584,0x26},
	{0x5585,0x20},
	{0x5589,0x10},
	{0x558a,0x00},
	{0x558b,0x3e},
	{0x5801,0x00},
	{0x5802,0x00},
	{0x5803,0x00}, 
	{0x5804,0x2b}, //0x31
	{0x5805,0x22},
	{0x5806,0x20},
	{0x5807,0x00},
	{0x5808,0x01},
	{0x5809,0x50}, 
	{0x580a,0x00},
	{0x580b,0xf0},
	//;;gamma
	{0x5481,0x03},
	{0x5482,0x06},
	{0x5483,0x0d},
	{0x5484,0x23},
	{0x5485,0x34},
	{0x5486,0x42},
	{0x5487,0x56},
	{0x5488,0x63},
	{0x5489,0x73},
	{0x548a,0x82},
	{0x548b,0x95},
	{0x548c,0xa6},
	{0x548d,0xbf},
	{0x548e,0xd6},
	{0x548f,0xe8},
	{0x5490,0x20},
	//;;martix
	{0x5380,0x42}, //;38
	{0x5381,0x33}, //;2b
	{0x5382,0x0f}, // ;d
	{0x5383,0x0b}, // ;9
	{0x5384,0x42}, //;38
	{0x5385,0x4d}, //;41
	//;;sharp and de-noise
	
	{0x5300,0x0f}, // sharpen mt th1
	{0x5301,0x30}, // sharpen mt th2
	{0x5302,0x10}, // sharpen offset 1
	{0x5303,0x09},//5}, // sharpen offset 2
	{0x5304,0x0e}, // de-noise th1
	{0x5305,0x30}, // de-noise th2
	{0x5306,0x10}, // de-noise offset 1
	{0x5307,0xd0}, // de-noise offset 2
	
	{0x5680,0x00},
	{0x5681,0x50},
	{0x5682,0x00},
	{0x5683,0x3c},
	{0x5684,0x11},
	{0x5685,0xe0},
	{0x5686,0x0d},
	{0x5687,0x68},
	{0x5688,0x03},
	{0x3008,0x02},
	//;; gain
	{0x3a18,0x00},
	{0x3a19,0x30},
	//;;ae
	{0x3a0f,0x4e},  
	{0x3a10,0x46},  
	{0x3a11,0x86},  
	{0x3a1b,0x4e},  
	{0x3a1e,0x46},  
	{0x3a1f,0x26},  

	{0x3212,0x00}, 
	{0x5380,0x35}, 
	{0x5381,0x29}, 
	{0x5382,0x0c}, 
	{0x5383,0x09}, 
	{0x5384,0x35}, 
	{0x5385,0x3e}, 
	{0x5392,0x1e}, 
	{0x3212,0x10}, 
	{0x3212,0xa0}, 

};
/*
 * The white balance settings
 * Here only tune the R G B channel gain.
 * The white balance enalbe bit is modified in sensor_s_autowb and sensor_s_wb
 */
static struct regval_list sensor_wb_manual[] = {
	//null
};

static struct regval_list sensor_wb_auto_regs[] = {
	{0x3212,0x00}, // enable group 0
	{0x5186,0x02}, //AWB auto
	{0x3212,0x10}, // end group 0
	{0x3212,0xa0}, // launch group 0
};

static struct regval_list sensor_wb_incandescence_regs[] = {
	//bai re guang
	{0x3212,0x00},
	{0x5186,0x03}, //AWB manual
	{0x5052,0x08}, // b gain
	{0x5053,0xab},
	{0x5050,0x04}, // g gain
	{0x5051,0x00},
	{0x504e,0x05}, // r gain
	{0x504f,0x41},
	{0x3212,0x10},
	{0x3212,0xa0},
};

static struct regval_list sensor_wb_fluorescent_regs[] = {
	//ri guang deng
	{0x3212,0x00},
	{0x5186,0x03}, //AWB manual
	{0x5052,0x09}, // b gain
	{0x5053,0x18},
	{0x5050,0x04}, // g gain
	{0x5051,0x06},
	{0x504e,0x04}, // r gain
	{0x504f,0x00},
	{0x3212,0x10},
	{0x3212,0xa0},
};

static struct regval_list sensor_wb_tungsten_regs[] = {
//null
};

static struct regval_list sensor_wb_horizon[] = {
//null
};

static struct regval_list sensor_wb_daylight_regs[] = {
	//tai yang guang
	{0x3212,0x00}, // enable group 0
	{0x5186,0x03}, //AWB manual
	{0x5052,0x05}, // b gain high
	{0x5053,0xd7}, // b gain low
	{0x5050,0x04}, // g gain high
	{0x5051,0x00}, // g gain low
	{0x504e,0x06}, // r gain high
	{0x504f,0x35}, // r gain low
	{0x3212,0x10}, // end group 0
	{0x3212,0xa0}, // launch group 0
};

static struct regval_list sensor_wb_flash[] = {
//null
};

static struct regval_list sensor_wb_cloud_regs[] = {
	{0x3212,0x00},
	{0x5186,0x03}, //AWB manual
	{0x5052,0x04}, // b gain
	{0x5053,0xb4},
	{0x5050,0x04}, // g gain
	{0x5051,0x00},
	{0x504e,0x07}, // r gain
	{0x504f,0x6d},
	{0x3212,0x10},
	{0x3212,0xa0},
};

static struct regval_list sensor_wb_shade[] = {
//null
};

static struct cfg_array sensor_wb[] = {
	{
		.regs = sensor_wb_manual,             //V4L2_WHITE_BALANCE_MANUAL
		.size = ARRAY_SIZE(sensor_wb_manual),
	},
	{
		.regs = sensor_wb_auto_regs,          //V4L2_WHITE_BALANCE_AUTO
		.size = ARRAY_SIZE(sensor_wb_auto_regs),
	},
	{
		.regs = sensor_wb_incandescence_regs, //V4L2_WHITE_BALANCE_INCANDESCENT
		.size = ARRAY_SIZE(sensor_wb_incandescence_regs),
	},
	{
		.regs = sensor_wb_fluorescent_regs,   //V4L2_WHITE_BALANCE_FLUORESCENT
		.size = ARRAY_SIZE(sensor_wb_fluorescent_regs),
	},
	{
		.regs = sensor_wb_tungsten_regs,      //V4L2_WHITE_BALANCE_FLUORESCENT_H
		.size = ARRAY_SIZE(sensor_wb_tungsten_regs),
	},
	{
		.regs = sensor_wb_horizon,            //V4L2_WHITE_BALANCE_HORIZON
		.size = ARRAY_SIZE(sensor_wb_horizon),
	},
	{
		.regs = sensor_wb_daylight_regs,      //V4L2_WHITE_BALANCE_DAYLIGHT
		.size = ARRAY_SIZE(sensor_wb_daylight_regs),
	},
	{
		.regs = sensor_wb_flash,              //V4L2_WHITE_BALANCE_FLASH
		.size = ARRAY_SIZE(sensor_wb_flash),
	},
	{
		.regs = sensor_wb_cloud_regs,         //V4L2_WHITE_BALANCE_CLOUDY
		.size = ARRAY_SIZE(sensor_wb_cloud_regs),
	},
	{
		.regs = sensor_wb_shade,              //V4L2_WHITE_BALANCE_SHADE
		.size = ARRAY_SIZE(sensor_wb_shade),
	},
};


/*
 * The color effect settings
 */
static struct regval_list sensor_colorfx_none_regs[] = {
	{0x3212,0x00}, // enable group 0
	{0x5001,0xc7},
	{0x5580,0x06},
	{0x5583,0x40},
	{0x5584,0x26},
	{0x3212,0x10}, // end group 0
	{0x3212,0xa0}, // launch group 0
};

static struct regval_list sensor_colorfx_bw_regs[] = {
	{0x3212,0x00},
	{0x5001,0xc7},
	{0x5580,0x24},
	{0x5583,0x80},
	{0x5584,0x80},
	{0x3212,0x10},
	{0x3212,0xa0},
};

static struct regval_list sensor_colorfx_sepia_regs[] = {
	{0x3212,0x00},
	{0x5001,0xc7},
	{0x5580,0x1c},
	{0x5583,0x40},
	{0x5584,0xa0},
	{0x3212,0x10},
	{0x3212,0xa0},
};

static struct regval_list sensor_colorfx_negative_regs[] = {
	{0x3212,0x00},
	{0x5001,0xc7},
	{0x5580,0x44},
	{0x5583,0x40},
	{0x5584,0x26},
	{0x3212,0x10},
	{0x3212,0xa0},
};

static struct regval_list sensor_colorfx_emboss_regs[] = {
//NULL  
};

static struct regval_list sensor_colorfx_sketch_regs[] = {
	//NULL
};

static struct regval_list sensor_colorfx_sky_blue_regs[] = {
	//NULL
};

static struct regval_list sensor_colorfx_grass_green_regs[] = {
	//NULL
};

static struct regval_list sensor_colorfx_skin_whiten_regs[] = {
//NULL
};

static struct regval_list sensor_colorfx_vivid_regs[] = {
//NULL
};

static struct regval_list sensor_colorfx_aqua_regs[] = {
//null
};

static struct regval_list sensor_colorfx_art_freeze_regs[] = {
//null
};

static struct regval_list sensor_colorfx_silhouette_regs[] = {
//null
};

static struct regval_list sensor_colorfx_solarization_regs[] = {
//null
};

static struct regval_list sensor_colorfx_antique_regs[] = {
//null
};

static struct regval_list sensor_colorfx_set_cbcr_regs[] = {
//null
};

static struct cfg_array sensor_colorfx[] = {
	{
		.regs = sensor_colorfx_none_regs,         //V4L2_COLORFX_NONE = 0,
		.size = ARRAY_SIZE(sensor_colorfx_none_regs),
	},
	{
		.regs = sensor_colorfx_bw_regs,           //V4L2_COLORFX_BW   = 1,
		.size = ARRAY_SIZE(sensor_colorfx_bw_regs),
	},
	{
		.regs = sensor_colorfx_sepia_regs,        //V4L2_COLORFX_SEPIA  = 2,
		.size = ARRAY_SIZE(sensor_colorfx_sepia_regs),
	},
	{
		.regs = sensor_colorfx_negative_regs,     //V4L2_COLORFX_NEGATIVE = 3,
		.size = ARRAY_SIZE(sensor_colorfx_negative_regs),
	},
	{
		.regs = sensor_colorfx_emboss_regs,       //V4L2_COLORFX_EMBOSS = 4,
		.size = ARRAY_SIZE(sensor_colorfx_emboss_regs),
	},
	{
		.regs = sensor_colorfx_sketch_regs,       //V4L2_COLORFX_SKETCH = 5,
		.size = ARRAY_SIZE(sensor_colorfx_sketch_regs),
	},
	{
		.regs = sensor_colorfx_sky_blue_regs,     //V4L2_COLORFX_SKY_BLUE = 6,
		.size = ARRAY_SIZE(sensor_colorfx_sky_blue_regs),
	},
	{
		.regs = sensor_colorfx_grass_green_regs,  //V4L2_COLORFX_GRASS_GREEN = 7,
		.size = ARRAY_SIZE(sensor_colorfx_grass_green_regs),
	},
	{
		.regs = sensor_colorfx_skin_whiten_regs,  //V4L2_COLORFX_SKIN_WHITEN = 8,
		.size = ARRAY_SIZE(sensor_colorfx_skin_whiten_regs),
	},
	{
		.regs = sensor_colorfx_vivid_regs,        //V4L2_COLORFX_VIVID = 9,
		.size = ARRAY_SIZE(sensor_colorfx_vivid_regs),
	},
	{
		.regs = sensor_colorfx_aqua_regs,         //V4L2_COLORFX_AQUA = 10,
		.size = ARRAY_SIZE(sensor_colorfx_aqua_regs),
	},
	{
		.regs = sensor_colorfx_art_freeze_regs,   //V4L2_COLORFX_ART_FREEZE = 11,
		.size = ARRAY_SIZE(sensor_colorfx_art_freeze_regs),
	},
	{
		.regs = sensor_colorfx_silhouette_regs,   //V4L2_COLORFX_SILHOUETTE = 12,
		.size = ARRAY_SIZE(sensor_colorfx_silhouette_regs),
	},
	{
		.regs = sensor_colorfx_solarization_regs, //V4L2_COLORFX_SOLARIZATION = 13,
		.size = ARRAY_SIZE(sensor_colorfx_solarization_regs),
	},
	{
		.regs = sensor_colorfx_antique_regs,      //V4L2_COLORFX_ANTIQUE = 14,
		.size = ARRAY_SIZE(sensor_colorfx_antique_regs),
	},
	{
		.regs = sensor_colorfx_set_cbcr_regs,     //V4L2_COLORFX_SET_CBCR = 15,
		.size = ARRAY_SIZE(sensor_colorfx_set_cbcr_regs),
	},
};

/*
 * The brightness setttings
 */
static struct regval_list sensor_brightness_neg4_regs[] = {
//NULL
};

static struct regval_list sensor_brightness_neg3_regs[] = {
	{0x3212,0x00},
	{0x5001,0xc7},
	{0x5587,0x30},
	{0x5588,0x09},
	{0x3212,0x10},
	{0x3212,0xa0},
};

static struct regval_list sensor_brightness_neg2_regs[] = {
	{0x3212,0x00},
	{0x5001,0xc7},
	{0x5587,0x20},
	{0x5588,0x09},
	{0x3212,0x10},
	{0x3212,0xa0},
};

static struct regval_list sensor_brightness_neg1_regs[] = {
	{0x3212,0x00},
	{0x5001,0xc7},
	{0x5587,0x10},
	{0x5588,0x09},
	{0x3212,0x10},
	{0x3212,0xa0},
};

static struct regval_list sensor_brightness_zero_regs[] = {
	{0x3212,0x00},
	{0x5001,0xc7},
	{0x5587,0x00},
	{0x5588,0x01},
	{0x3212,0x10},
	{0x3212,0xa0},
};

static struct regval_list sensor_brightness_pos1_regs[] = {
	{0x3212,0x00},
	{0x5001,0xc7},
	{0x5587,0x10},
	{0x5588,0x01},
	{0x3212,0x10},
	{0x3212,0xa0},
};

static struct regval_list sensor_brightness_pos2_regs[] = {
	{0x3212,0x00},
	{0x5001,0xc7},
	{0x5587,0x20},
	{0x5588,0x01},
	{0x3212,0x10},
	{0x3212,0xa0},
};

static struct regval_list sensor_brightness_pos3_regs[] = {
	{0x3212,0x00}, // enable group 0
	{0x5001,0xc7},
	{0x5587,0x30},
	{0x5588,0x01},
	{0x3212,0x10}, // end group 0
	{0x3212,0xa0}, // launch group 0
};

static struct regval_list sensor_brightness_pos4_regs[] = {
//NULL
};

static struct cfg_array sensor_brightness[] = {
	{
		.regs = sensor_brightness_neg4_regs,
		.size = ARRAY_SIZE(sensor_brightness_neg4_regs),
	},
	{
		.regs = sensor_brightness_neg3_regs,
		.size = ARRAY_SIZE(sensor_brightness_neg3_regs),
	},
	{
		.regs = sensor_brightness_neg2_regs,
		.size = ARRAY_SIZE(sensor_brightness_neg2_regs),
	},
	{
		.regs = sensor_brightness_neg1_regs,
		.size = ARRAY_SIZE(sensor_brightness_neg1_regs),
	},
	{
		.regs = sensor_brightness_zero_regs,
		.size = ARRAY_SIZE(sensor_brightness_zero_regs),
	},
	{
		.regs = sensor_brightness_pos1_regs,
		.size = ARRAY_SIZE(sensor_brightness_pos1_regs),
	},
	{
		.regs = sensor_brightness_pos2_regs,
		.size = ARRAY_SIZE(sensor_brightness_pos2_regs),
	},
	{
		.regs = sensor_brightness_pos3_regs,
		.size = ARRAY_SIZE(sensor_brightness_pos3_regs),
	},
	{
		.regs = sensor_brightness_pos4_regs,
		.size = ARRAY_SIZE(sensor_brightness_pos4_regs),
	},
};

/*
 * The contrast setttings
 */
static struct regval_list sensor_contrast_neg4_regs[] = {
//NULL
};

static struct regval_list sensor_contrast_neg3_regs[] = {
	{0x3212,0x00},
	{0x5001,0xc7},
	{0x5585,0x14},
	{0x5586,0x14},
	{0x3212,0x10},
	{0x3212,0xa0},
};

static struct regval_list sensor_contrast_neg2_regs[] = {
	{0x3212,0x00},
	{0x5001,0xc7},
	{0x5585,0x18},
	{0x5586,0x18},
	{0x3212,0x10},
	{0x3212,0xa0},
};

static struct regval_list sensor_contrast_neg1_regs[] = {
	{0x3212,0x00},
	{0x5001,0xc7},
	{0x5585,0x1c},
	{0x5586,0x1c},
	{0x3212,0x10},
	{0x3212,0xa0},
};

static struct regval_list sensor_contrast_zero_regs[] = {
	{0x3212,0x00},
	{0x5001,0xc7},
	{0x5585,0x20},
	{0x5586,0x20},
	{0x3212,0x10},
	{0x3212,0xa0},
};

static struct regval_list sensor_contrast_pos1_regs[] = {
	{0x3212,0x00},
	{0x5001,0xc7},
	{0x5585,0x24},
	{0x5586,0x24},
	{0x3212,0x10},
	{0x3212,0xa0},
};

static struct regval_list sensor_contrast_pos2_regs[] = {
	{0x3212,0x00},
	{0x5001,0xc7},
	{0x5585,0x28},
	{0x5586,0x28},
	{0x3212,0x10},
	{0x3212,0xa0},
};

static struct regval_list sensor_contrast_pos3_regs[] = {
	{0x3212,0x00}, // enable group 0
	{0x5001,0xc7},
	{0x5585,0x2c},
	{0x5586,0x2c},
	{0x3212,0x10}, // end group 0
	{0x3212,0xa0}, // launch group 0
};

static struct regval_list sensor_contrast_pos4_regs[] = {
//NULL
};

static struct cfg_array sensor_contrast[] = {
	{
		.regs = sensor_contrast_neg4_regs,
		.size = ARRAY_SIZE(sensor_contrast_neg4_regs),
	},
	{
		.regs = sensor_contrast_neg3_regs,
		.size = ARRAY_SIZE(sensor_contrast_neg3_regs),
	},
	{
		.regs = sensor_contrast_neg2_regs,
		.size = ARRAY_SIZE(sensor_contrast_neg2_regs),
	},
	{
		.regs = sensor_contrast_neg1_regs,
		.size = ARRAY_SIZE(sensor_contrast_neg1_regs),
	},
	{
		.regs = sensor_contrast_zero_regs,
		.size = ARRAY_SIZE(sensor_contrast_zero_regs),
	},
	{
		.regs = sensor_contrast_pos1_regs,
		.size = ARRAY_SIZE(sensor_contrast_pos1_regs),
	},
	{
		.regs = sensor_contrast_pos2_regs,
		.size = ARRAY_SIZE(sensor_contrast_pos2_regs),
	},
	{
		.regs = sensor_contrast_pos3_regs,
		.size = ARRAY_SIZE(sensor_contrast_pos3_regs),
	},
	{
		.regs = sensor_contrast_pos4_regs,
		.size = ARRAY_SIZE(sensor_contrast_pos4_regs),
	},
};

/*
 * The saturation setttings
 */
static struct regval_list sensor_saturation_neg4_regs[] = {
//NULL
};

static struct regval_list sensor_saturation_neg3_regs[] = {
	{0x3212,0x00},
	{0x5380,0x1a},
	{0x5381,0x14},
	{0x5382,0x04},
	{0x5383,0x1a},
	{0x5384,0x1e},
	{0x5385,0x7c},
	{0x5392,0x1e},
	{0x3212,0x10},
	{0x3212,0xa0},
};

static struct regval_list sensor_saturation_neg2_regs[] = {
	{0x3212,0x00},
	{0x5380,0x28},
	{0x5381,0x1f},
	{0x5382,0x09},
	{0x5383,0x07},
	{0x5384,0x28},
	{0x5385,0x2e},
	{0x5392,0x1e},
	{0x3212,0x10},
	{0x3212,0xa0},
};

static struct regval_list sensor_saturation_neg1_regs[] = {
	{0x3212,0x00},
	{0x5380,0x35},
	{0x5381,0x29},
	{0x5382,0x0c},
	{0x5383,0x09},
	{0x5384,0x35},
	{0x5385,0x3e},
	{0x5392,0x1e},
	{0x3212,0x10},
	{0x3212,0xa0},
};

static struct regval_list sensor_saturation_zero_regs[] = {
	{0x3212,0x00}, 
	{0x5380,0x35}, 
	{0x5381,0x29}, 
	{0x5382,0x0c}, 
	{0x5383,0x09}, 
	{0x5384,0x35}, 
	{0x5385,0x3e}, 
	{0x5392,0x1e}, 
	{0x3212,0x10}, 
	{0x3212,0xa0}, 
};

static struct regval_list sensor_saturation_pos1_regs[] = {
	{0x3212,0x00},
	{0x5380,0x4f},
	{0x5381,0x3d},
	{0x5382,0x12},
	{0x5383,0x0d},
	{0x5384,0x4f},
	{0x5385,0x5c},
	{0x5392,0x1e},
	{0x3212,0x10},
	{0x3212,0xa0},
};

static struct regval_list sensor_saturation_pos2_regs[] = {
	{0x3212,0x00},
	{0x5380,0x5c},
	{0x5381,0x47},
	{0x5382,0x15},
	{0x5383,0x0f},
	{0x5384,0x5c},
	{0x5385,0x6b},
	{0x5392,0x1e},
	{0x3212,0x10},
	{0x3212,0xa0},
};

static struct regval_list sensor_saturation_pos3_regs[] = {
	{0x3212,0x00}, // enable group 0
	{0x5380,0x6a},
	{0x5381,0x52},
	{0x5382,0x18},
	{0x5383,0x12},
	{0x5384,0x6a},
	{0x5385,0x7c},
	{0x5392,0x1e},
	{0x3212,0x10}, // end group 0
	{0x3212,0xa0}, // launch group 0
};

static struct regval_list sensor_saturation_pos4_regs[] = {
//NULL
};

static struct cfg_array sensor_saturation[] = {
	{
		.regs = sensor_saturation_neg4_regs,
		.size = ARRAY_SIZE(sensor_saturation_neg4_regs),
	},
	{
		.regs = sensor_saturation_neg3_regs,
		.size = ARRAY_SIZE(sensor_saturation_neg3_regs),
	},
	{
		.regs = sensor_saturation_neg2_regs,
		.size = ARRAY_SIZE(sensor_saturation_neg2_regs),
	},
	{
		.regs = sensor_saturation_neg1_regs,
		.size = ARRAY_SIZE(sensor_saturation_neg1_regs),
	},
	{
		.regs = sensor_saturation_zero_regs,
		.size = ARRAY_SIZE(sensor_saturation_zero_regs),
	},
	{
		.regs = sensor_saturation_pos1_regs,
		.size = ARRAY_SIZE(sensor_saturation_pos1_regs),
	},
	{
		.regs = sensor_saturation_pos2_regs,
		.size = ARRAY_SIZE(sensor_saturation_pos2_regs),
	},
	{
		.regs = sensor_saturation_pos3_regs,
		.size = ARRAY_SIZE(sensor_saturation_pos3_regs),
	},
	{
		.regs = sensor_saturation_pos4_regs,
		.size = ARRAY_SIZE(sensor_saturation_pos4_regs),
	},
};

/*
 * The exposure target setttings
 */
static struct regval_list sensor_ev_neg4_regs[] = {
};

static struct regval_list sensor_ev_neg3_regs[] = {
	{0x3a0f,0x10},
	{0x3a10,0x08},
	{0x3a11,0x10},
	{0x3a1b,0x08},
	{0x3a1e,0x20},
	{0x3a1f,0x10},
};

static struct regval_list sensor_ev_neg2_regs[] = {
	{0x3a0f,0x18},
	{0x3a10,0x10},
	{0x3a11,0x18},
	{0x3a1b,0x10},
	{0x3a1e,0x30},
	{0x3a1f,0x10},
};

static struct regval_list sensor_ev_neg1_regs[] = {
	{0x3a0f,0x28},
	{0x3a10,0x20},
	{0x3a11,0x51},
	{0x3a1b,0x28},
	{0x3a1e,0x20},
  //{REG_TERM,VAL_TERM},
};

static struct regval_list sensor_ev_zero_regs[] = {
	{0x3a0f,0x4e},  
	{0x3a10,0x46},  
	{0x3a11,0x86},  
	{0x3a1b,0x4e},  
	{0x3a1e,0x46},  
	{0x3a1f,0x26},  
};

static struct regval_list sensor_ev_pos1_regs[] = {
	{0x3a0f,0x48},
	{0x3a10,0x40},
	{0x3a11,0x80},
	{0x3a1b,0x48},
	{0x3a1e,0x40},
	{0x3a1f,0x20},
};

static struct regval_list sensor_ev_pos2_regs[] = {
	{0x3a0f,0x58},
	{0x3a10,0x50},
	{0x3a11,0x91},
	{0x3a1b,0x58},
	{0x3a1e,0x50},
	{0x3a1f,0x20},
};

static struct regval_list sensor_ev_pos3_regs[] = {
	{0x3a0f,0x60}, // stable range in high
	{0x3a10,0x58}, // stable range in low
	{0x3a11,0xa0}, // fast zone high
	{0x3a1b,0x60}, // stable range out high
	{0x3a1e,0x58}, // stable range out low
	{0x3a1f,0x20}, // fast zone low
};

static struct regval_list sensor_ev_pos4_regs[] = {
};

static struct cfg_array sensor_ev[] = {
	{
		.regs = sensor_ev_neg4_regs,
		.size = ARRAY_SIZE(sensor_ev_neg4_regs),
	},
	{
		.regs = sensor_ev_neg3_regs,
		.size = ARRAY_SIZE(sensor_ev_neg3_regs),
	},
	{
		.regs = sensor_ev_neg2_regs,
		.size = ARRAY_SIZE(sensor_ev_neg2_regs),
	},
	{
		.regs = sensor_ev_neg1_regs,
		.size = ARRAY_SIZE(sensor_ev_neg1_regs),
	},
	{
		.regs = sensor_ev_zero_regs,
		.size = ARRAY_SIZE(sensor_ev_zero_regs),
	},
	{
		.regs = sensor_ev_pos1_regs,
		.size = ARRAY_SIZE(sensor_ev_pos1_regs),
	},
	{
		.regs = sensor_ev_pos2_regs,
		.size = ARRAY_SIZE(sensor_ev_pos2_regs),
	},
	{
		.regs = sensor_ev_pos3_regs,
		.size = ARRAY_SIZE(sensor_ev_pos3_regs),
	},
	{
		.regs = sensor_ev_pos4_regs,
		.size = ARRAY_SIZE(sensor_ev_pos4_regs),
	},
};

/*
 * Here we'll try to encapsulate the changes for just the output
 * video format.
 *
 */

static struct regval_list sensor_fmt_yuv422_yuyv[] = {  
	{0x4300,0x30},	//YCbYCr
};

static struct regval_list sensor_fmt_yuv422_yvyu[] = {
	{0x4300,0x31},	//YCrYCb
};

static struct regval_list sensor_fmt_yuv422_vyuy[] = {
	{0x4300,0x33},	//CbYCrY
};

static struct regval_list sensor_fmt_yuv422_uyvy[] = {
	{0x4300,0x32},	//CrYCbY
};



static unsigned int flash_auto_level=0x1c;
static void sensor_get_lum(struct v4l2_subdev *sd, unsigned int *lum)
{
	data_type temp = 0;

	sensor_read(sd, 0x56a1, &temp);
	*lum = temp;
	vfe_dev_dbg("check luminance=0x%x\n", *lum);
}

static void sensor_g_flash_flag(struct v4l2_subdev *sd, unsigned int *flash_flag)
{
	unsigned int current_lum = 0; 
	
	sensor_get_lum(sd, &current_lum);

	if( current_lum < flash_auto_level )
		*flash_flag = 1;
	else
		*flash_flag = 0;
}

/* *********************************************begin of ******************************************** */

static int sensor_g_hflip(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);
	data_type val;

	LOG_ERR_RET(sensor_read(sd, 0x3818, &val))

	val &= (1<<6);
	val >>= 6;

	*value = val;

	info->hflip = *value;
	return 0;
}

static int sensor_s_hflip(struct v4l2_subdev *sd, int value)
{
	struct sensor_info *info = to_state(sd);
	data_type val;

	if(info->hflip == value)
		return 0;

	LOG_ERR_RET(sensor_write(sd, 0x3212, 0x00))
	LOG_ERR_RET(sensor_read(sd, 0x3818, &val))

	switch (value) {
	case 0:
	  val &= 0xbf;
	  break;
	case 1:
	  val |= 0x40;
	  break;
	default:
	  return -EINVAL;
	}

	LOG_ERR_RET(sensor_write(sd, 0x3818, val))
	LOG_ERR_RET(sensor_write(sd, 0x3212, 0x10))
	LOG_ERR_RET(sensor_write(sd, 0x3212, 0xa0))

	usleep_range(10000,12000);
	info->hflip = value;
	return 0;
}

static int sensor_g_vflip(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);
	data_type val;

	LOG_ERR_RET(sensor_read(sd, 0x3818, &val))

	val &= (1<<5);
	*value = val;
	val >>= 5;

	info->vflip = *value;
	return 0;
}

static int sensor_s_vflip(struct v4l2_subdev *sd, int value)
{
	struct sensor_info *info = to_state(sd);
	data_type rdval;

	if(info->vflip == value)
		return 0;

	LOG_ERR_RET(sensor_write(sd, 0x3212, 0x00))
	LOG_ERR_RET(sensor_read(sd, 0x3818, &rdval))

	switch (value) {
	case 0:
		rdval &= 0xdf;
		break;
	case 1:
		rdval |= 0x20;
		break;
	default:
		return -EINVAL;
	}

	LOG_ERR_RET(sensor_write(sd, 0x3818, rdval))

	LOG_ERR_RET(sensor_write(sd, 0x3212, 0x10))
	LOG_ERR_RET(sensor_write(sd, 0x3212, 0xa0))
	usleep_range(10000,12000);
	info->vflip = value;
	return 0;
}

static int sensor_g_autogain(struct v4l2_subdev *sd, __s32 *value)
{
	return -EINVAL;
}

static int sensor_s_autogain(struct v4l2_subdev *sd, int value)
{
	return -EINVAL;
}

static int sensor_g_autoexp(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);
	data_type rdval;

	LOG_ERR_RET(sensor_read(sd, 0x3013, &rdval))

	if ((rdval&0x01) == 0x01) {
		*value = V4L2_EXPOSURE_MANUAL;
	}
	else
	{
		*value = V4L2_EXPOSURE_AUTO;
	}

	info->autoexp = *value;
	return 0;
}

static int sensor_s_autoexp(struct v4l2_subdev *sd,
    enum v4l2_exposure_auto_type value)
{
	struct sensor_info *info = to_state(sd);
	data_type rdval;

	LOG_ERR_RET(sensor_read(sd, 0x3013, &rdval))

	switch (value) {
	case V4L2_EXPOSURE_AUTO:
		rdval &= 0xfe;
		break;
	case V4L2_EXPOSURE_MANUAL:
		rdval |= 0x01;
		break;
	case V4L2_EXPOSURE_SHUTTER_PRIORITY:
		return -EINVAL;
	case V4L2_EXPOSURE_APERTURE_PRIORITY:
		return -EINVAL;
	default:
		return -EINVAL;
	}

	LOG_ERR_RET(sensor_write(sd, 0x3013, rdval))

	info->autoexp = value;
	return 0;
}

static int sensor_g_autowb(struct v4l2_subdev *sd, int *value)
{
	return 0;
}

static int sensor_s_autowb(struct v4l2_subdev *sd, int value)
{
	return 0;
}

static int sensor_g_hue(struct v4l2_subdev *sd, __s32 *value)
{
	return -EINVAL;
}

static int sensor_s_hue(struct v4l2_subdev *sd, int value)
{
	return -EINVAL;
}

static int sensor_g_gain(struct v4l2_subdev *sd, __s32 *value)
{
	return -EINVAL;
}

static int sensor_s_gain(struct v4l2_subdev *sd, int value)
{
	return -EINVAL;
}

/* *********************************************end of ******************************************** */

static int sensor_g_brightness(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);

	*value = info->brightness;
	return 0;
}

static int sensor_s_brightness(struct v4l2_subdev *sd, int value)
{
	struct sensor_info *info = to_state(sd);

	if(info->brightness == value)
		return 0;

	if(value < -4 || value > 4)
		return -ERANGE;

	LOG_ERR_RET(sensor_write_array(sd, sensor_brightness[value+4].regs, sensor_brightness[value+4].size))

	info->brightness = value;
	return 0;
}

static int sensor_g_contrast(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);

	*value = info->contrast;
	return 0;
}

static int sensor_s_contrast(struct v4l2_subdev *sd, int value)
{
	struct sensor_info *info = to_state(sd);

	if(info->contrast == value)
		return 0;

	if(value < -4 || value > 4)
		return -ERANGE;

	LOG_ERR_RET(sensor_write_array(sd, sensor_contrast[value+4].regs, sensor_contrast[value+4].size))

	info->contrast = value;
	return 0;
}

static int sensor_g_saturation(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);

	*value = info->saturation;
	return 0;
}

static int sensor_s_saturation(struct v4l2_subdev *sd, int value)
{
	struct sensor_info *info = to_state(sd);

	if(info->saturation == value)
		return 0;

	if(value < -4 || value > 4)
		return -ERANGE;

	LOG_ERR_RET(sensor_write_array(sd, sensor_saturation[value+4].regs, sensor_saturation[value+4].size))

	info->saturation = value;
	return 0;
}

static int sensor_g_exp_bias(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);

	*value = info->exp_bias;
	return 0;
}

static int sensor_s_exp_bias(struct v4l2_subdev *sd, int value)
{
	struct sensor_info *info = to_state(sd);

	if(info->exp_bias == value)
		return 0;

	if(value < -4 || value > 4)
		return -ERANGE;

	LOG_ERR_RET(sensor_write_array(sd, sensor_ev[value+4].regs, sensor_ev[value+4].size))

	info->exp_bias = value;
	return 0;
}

static int sensor_g_wb(struct v4l2_subdev *sd, int *value)
{
	struct sensor_info *info = to_state(sd);
	enum v4l2_auto_n_preset_white_balance *wb_type = (enum v4l2_auto_n_preset_white_balance*)value;

	*wb_type = info->wb;

	return 0;
}

static int sensor_s_wb(struct v4l2_subdev *sd,
    enum v4l2_auto_n_preset_white_balance value)
{
	struct sensor_info *info = to_state(sd);

	if(info->capture_mode == V4L2_MODE_IMAGE)
		return 0;

	if(info->wb == value)
		return 0;

	LOG_ERR_RET(sensor_write_array(sd, sensor_wb[value].regs ,sensor_wb[value].size) )

	if (value == V4L2_WHITE_BALANCE_AUTO)
		info->autowb = 1;
	else
		info->autowb = 0;

	info->wb = value;
	return 0;
}

static int sensor_g_colorfx(struct v4l2_subdev *sd,
    __s32 *value)
{
	struct sensor_info *info = to_state(sd);
	enum v4l2_colorfx *clrfx_type = (enum v4l2_colorfx*)value;

	*clrfx_type = info->clrfx;
	return 0;
}

static int sensor_s_colorfx(struct v4l2_subdev *sd,
    enum v4l2_colorfx value)
{
	struct sensor_info *info = to_state(sd);

	if(info->clrfx == value)
		return 0;

	LOG_ERR_RET(sensor_write_array(sd, sensor_colorfx[value].regs, sensor_colorfx[value].size))

	info->clrfx = value;
	return 0;
}

static int sensor_g_flash_mode(struct v4l2_subdev *sd,
    __s32 *value)
{
	struct sensor_info *info = to_state(sd);
	enum v4l2_flash_led_mode *flash_mode = (enum v4l2_flash_led_mode*)value;

	*flash_mode = info->flash_mode;
	return 0;
}

static int sensor_s_flash_mode(struct v4l2_subdev *sd,
    enum v4l2_flash_led_mode value)
{
	struct sensor_info *info = to_state(sd);

	info->flash_mode = value;
	return 0;
}

/*
 * Stuff that knows about the sensor.
 */

static int sensor_power(struct v4l2_subdev *sd, int on)
{
	switch(on) {
	case CSI_SUBDEV_STBY_ON:
		vfe_dev_dbg("CSI_SUBDEV_STBY_ON!\n");
		cci_lock(sd);
		vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
		cci_unlock(sd);
		vfe_set_mclk(sd,OFF);
		break;
	case CSI_SUBDEV_STBY_OFF:
		vfe_dev_dbg("CSI_SUBDEV_STBY_OFF!\n");
		cci_lock(sd);
		vfe_set_mclk_freq(sd,MCLK);
		vfe_set_mclk(sd,ON);
		usleep_range(10000,12000);
		vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
		usleep_range(10000,12000);
		cci_unlock(sd);
		break;
	case CSI_SUBDEV_PWR_ON:
		vfe_dev_dbg("CSI_SUBDEV_PWR_ON!\n");
		cci_lock(sd);
		vfe_gpio_set_status(sd,PWDN,1);//set the gpio to output
		vfe_gpio_set_status(sd,RESET,1);//set the gpio to output
		vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
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
		vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
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
		vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
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
	switch(val) {
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

	if(rdval != 0x77)
		return -ENODEV;

	LOG_ERR_RET(sensor_read(sd, 0x300b, &rdval))

	if(rdval != 0x36)
		return -ENODEV;

	return 0;
}

static int sensor_init(struct v4l2_subdev *sd, u32 val)
{
	int ret;
	struct sensor_info *info = to_state(sd);

	vfe_dev_dbg("sensor_init 0x%x\n",val);

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
	info->width = 0;
	info->height = 0;
	info->brightness = 0;
	info->contrast = 0;
	info->saturation = 0;
	info->hue = 0;
	info->hflip = 0;
	info->vflip = 0;
	info->gain = 0;
	info->autogain = 1;
	info->exp_bias = 0;
	info->autoexp = 1;
	info->autowb = 1;
	info->wb = V4L2_WHITE_BALANCE_AUTO;
	info->clrfx = V4L2_COLORFX_NONE;
	info->band_filter = V4L2_CID_POWER_LINE_FREQUENCY_50HZ;

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
	switch(cmd) {
		case GET_FLASH_FLAG:
			sensor_g_flash_flag(sd,(unsigned int *)arg);
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
} sensor_formats[] = {
	{
		.desc   = "YUYV 4:2:2",
		.mbus_code  = V4L2_MBUS_FMT_YUYV8_2X8,
		.regs     = sensor_fmt_yuv422_yuyv,
		.regs_size = ARRAY_SIZE(sensor_fmt_yuv422_yuyv),
		.bpp    = 2,
	},
	{
		.desc   = "YVYU 4:2:2",
		.mbus_code  = V4L2_MBUS_FMT_YVYU8_2X8,
		.regs     = sensor_fmt_yuv422_yvyu,
		.regs_size = ARRAY_SIZE(sensor_fmt_yuv422_yvyu),
		.bpp    = 2,
	},
	{
		.desc   = "UYVY 4:2:2",
		.mbus_code  = V4L2_MBUS_FMT_UYVY8_2X8,
		.regs     = sensor_fmt_yuv422_uyvy,
		.regs_size = ARRAY_SIZE(sensor_fmt_yuv422_uyvy),
		.bpp    = 2,
	},
	{
		.desc   = "VYUY 4:2:2",
		.mbus_code  = V4L2_MBUS_FMT_VYUY8_2X8,
		.regs     = sensor_fmt_yuv422_vyuy,
		.regs_size = ARRAY_SIZE(sensor_fmt_yuv422_vyuy),
		.bpp    = 2,
	},
};
#define N_FMTS ARRAY_SIZE(sensor_formats)

/*
 * Then there is the issue of window sizes.  Try to capture the info here.
 */


static struct sensor_win_size sensor_win_sizes[] = {
	/* VGA */
	{
		.width      = VGA_WIDTH,
		.height     = VGA_HEIGHT,
		.hoffset    = 0,
		.voffset    = 0,
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
	for (index = 0; index < N_FMTS; index++)
		if (sensor_formats[index].mbus_code == fmt->code)//linux-3.0
			break;

	if (index >= N_FMTS)
		return -EINVAL;

	if (ret_fmt != NULL)
		*ret_fmt = sensor_formats + index;

	/*
	 * Fields: the sensor devices claim to be progressive.
	 */
	fmt->field = V4L2_FIELD_NONE;//linux-3.0

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
	fmt->width = wsize->width;//linux-3.0
	fmt->height = wsize->height;//linux-3.0

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
		ret = sensor_s_autoexp(sd,V4L2_EXPOSURE_MANUAL);
		if (ret < 0)
			vfe_dev_err("sensor_s_autoexp off err when capturing image!\n");
		ret = sensor_s_autogain(sd,0);
		if (ret < 0)
			vfe_dev_err("sensor_s_autogain off err when capturing image!\n");
		ret = sensor_s_autowb(sd,0); //lock wb
		if (ret < 0)
			vfe_dev_err("sensor_s_autowb off err when capturing image!\n");
	}
	sensor_write_array(sd, sensor_fmt->regs , sensor_fmt->regs_size);

	if (wsize->regs)
		LOG_ERR_RET(sensor_write_array(sd, wsize->regs, wsize->regs_size))
	if (wsize->set_size)
		LOG_ERR_RET(wsize->set_size(sd))

	sensor_s_hflip(sd,info->hflip);
	sensor_s_vflip(sd,info->vflip);

	if(info->capture_mode == V4L2_MODE_VIDEO ||
		info->capture_mode == V4L2_MODE_PREVIEW)
	{
		ret = sensor_s_autoexp(sd,V4L2_EXPOSURE_AUTO);
		if (ret < 0)
			vfe_dev_err("sensor_s_autoexp on err when capturing video!\n");
		ret = sensor_s_autogain(sd,1);
		if (ret < 0)
			vfe_dev_err("sensor_s_autogain on err when capturing video!\n");
		if (info->wb == V4L2_WHITE_BALANCE_AUTO) {
			ret = sensor_s_autowb(sd,1); //unlock wb
			if (ret < 0)
				vfe_dev_err("sensor_s_autowb on err when capturing image!\n");
    	}

		if(info->low_speed == 1) {
			if(info->preview_first_flag == 1) {
				info->preview_first_flag = 0;
				msleep(600);
			} else {
				msleep(200);
			}
		}
	}
	info->fmt = sensor_fmt;
	info->width = wsize->width;
	info->height = wsize->height;

	vfe_dev_print("s_fmt set width = %d, height = %d\n",wsize->width,wsize->height);
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
	cp->timeperframe.numerator = info->tpf.numerator;
	cp->timeperframe.denominator = info->tpf.denominator;

	return 0;
}

static int sensor_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *parms)
{
	struct v4l2_captureparm *cp = &parms->parm.capture;
	struct v4l2_fract *tpf = &cp->timeperframe;
	struct sensor_info *info = to_state(sd);
	unsigned char div;

	vfe_dev_dbg("sensor_s_parm\n");

	if (parms->type != V4L2_BUF_TYPE_VIDEO_CAPTURE){
		vfe_dev_dbg("parms->type!=V4L2_BUF_TYPE_VIDEO_CAPTURE\n");
		return -EINVAL;
	}

	if (info->tpf.numerator == 0){
		vfe_dev_dbg("info->tpf.numerator == 0\n");
		return -EINVAL;
	}

	info->capture_mode = cp->capturemode;

	if (info->capture_mode == V4L2_MODE_IMAGE) {
		vfe_dev_dbg("capture mode is not video mode,can not set frame rate!\n");
		return 0;
	}

	if (tpf->numerator == 0 || tpf->denominator == 0) {
		tpf->numerator = 1;
		tpf->denominator = SENSOR_FRAME_RATE;/* Reset to full rate */
		vfe_dev_err("sensor frame rate reset to full rate!\n");
	}

	div = SENSOR_FRAME_RATE/(tpf->denominator/tpf->numerator);
	if(div > 15 || div == 0)
	{
		vfe_dev_print("SENSOR_FRAME_RATE=%d\n",SENSOR_FRAME_RATE);
		vfe_dev_print("tpf->denominator=%d\n",tpf->denominator);
		vfe_dev_print("tpf->numerator=%d\n",tpf->numerator);
		return -EINVAL;
	}

	vfe_dev_dbg("set frame rate %d\n",tpf->denominator/tpf->numerator);

	info->tpf.denominator = SENSOR_FRAME_RATE;
	info->tpf.numerator = div;

	if(info->tpf.denominator/info->tpf.numerator < 30)
		info->low_speed = 1;

	return 0;
}


/*
 * Code for dealing with controls.
 * fill with different sensor module
 * different sensor module has different settings here
 * if not support the follow function ,retrun -EINVAL
 */

/* *********************************************begin of ******************************************** */
static int sensor_queryctrl(struct v4l2_subdev *sd,
    struct v4l2_queryctrl *qc)
{
	/* Fill in min, max, step and default value for these controls. */
	/* see include/linux/videodev2.h for details */
	/* see sensor_s_parm and sensor_g_parm for the meaning of value */
	switch (qc->id) {
//  case V4L2_CID_BRIGHTNESS:
//    return v4l2_ctrl_query_fill(qc, -4, 4, 1, 1);
//  case V4L2_CID_CONTRAST:
//    return v4l2_ctrl_query_fill(qc, -4, 4, 1, 1);
//  case V4L2_CID_SATURATION:
//    return v4l2_ctrl_query_fill(qc, -4, 4, 1, 1);
//  case V4L2_CID_HUE:
//    return v4l2_ctrl_query_fill(qc, -180, 180, 5, 0);
	case V4L2_CID_VFLIP:
	case V4L2_CID_HFLIP:
		return v4l2_ctrl_query_fill(qc, 0, 1, 1, 0);
//  case V4L2_CID_GAIN:
//    return v4l2_ctrl_query_fill(qc, 0, 255, 1, 128);
//  case V4L2_CID_AUTOGAIN:
//    return v4l2_ctrl_query_fill(qc, 0, 1, 1, 1);
	case V4L2_CID_EXPOSURE:
    case V4L2_CID_AUTO_EXPOSURE_BIAS:
		return v4l2_ctrl_query_fill(qc, -4, 4, 1, 0);
	case V4L2_CID_EXPOSURE_AUTO:
		return v4l2_ctrl_query_fill(qc, 0, 1, 1, 0);
    case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:
		return v4l2_ctrl_query_fill(qc, 0, 9, 1, 1);
	case V4L2_CID_AUTO_WHITE_BALANCE:
		return v4l2_ctrl_query_fill(qc, 0, 1, 1, 1);
	case V4L2_CID_COLORFX:
		return v4l2_ctrl_query_fill(qc, 0, 15, 1, 0);
	case V4L2_CID_FLASH_LED_MODE:
	  	return v4l2_ctrl_query_fill(qc, 0, 4, 1, 0);
	}
	return -EINVAL;
}

static int sensor_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	//vfe_dev_dbg("sensor_g_ctrl ctrl->id=0x%8x\n", ctrl->id);
  	switch (ctrl->id) {
    case V4L2_CID_BRIGHTNESS:
		return sensor_g_brightness(sd, &ctrl->value);
	case V4L2_CID_CONTRAST:
		return sensor_g_contrast(sd, &ctrl->value);
	case V4L2_CID_SATURATION:
		return sensor_g_saturation(sd, &ctrl->value);
	case V4L2_CID_HUE:
		return sensor_g_hue(sd, &ctrl->value);
	case V4L2_CID_VFLIP:
		return sensor_g_vflip(sd, &ctrl->value);
	case V4L2_CID_HFLIP:
		return sensor_g_hflip(sd, &ctrl->value);
	case V4L2_CID_GAIN:
		return sensor_g_gain(sd, &ctrl->value);
	case V4L2_CID_AUTOGAIN:
		return sensor_g_autogain(sd, &ctrl->value);
	case V4L2_CID_EXPOSURE:
	case V4L2_CID_AUTO_EXPOSURE_BIAS:
		return sensor_g_exp_bias(sd, &ctrl->value);
	case V4L2_CID_EXPOSURE_AUTO:
		return sensor_g_autoexp(sd, &ctrl->value);
	case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:
		return sensor_g_wb(sd, &ctrl->value);
	case V4L2_CID_AUTO_WHITE_BALANCE:
		return sensor_g_autowb(sd, &ctrl->value);
	case V4L2_CID_COLORFX:
		return sensor_g_colorfx(sd,	&ctrl->value);
	case V4L2_CID_FLASH_LED_MODE:
		return sensor_g_flash_mode(sd, &ctrl->value);
	}
	return -EINVAL;
}

static int sensor_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct v4l2_queryctrl qc;
	int ret;

	//vfe_dev_dbg("sensor_s_ctrl ctrl->id=0x%8x\n", ctrl->id);
	qc.id = ctrl->id;
	ret = sensor_queryctrl(sd, &qc);
	if (ret < 0) {
		return ret;
	}

	if (qc.type == V4L2_CTRL_TYPE_MENU ||
		qc.type == V4L2_CTRL_TYPE_INTEGER ||
		qc.type == V4L2_CTRL_TYPE_BOOLEAN)
	{
		if (ctrl->value < qc.minimum || ctrl->value > qc.maximum) 
		{
			return -ERANGE;
		}
	}

  	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		return sensor_s_brightness(sd, ctrl->value);
	case V4L2_CID_CONTRAST:
		return sensor_s_contrast(sd, ctrl->value);
	case V4L2_CID_SATURATION:
		return sensor_s_saturation(sd, ctrl->value);
	case V4L2_CID_HUE:
		return sensor_s_hue(sd, ctrl->value);
	case V4L2_CID_VFLIP:
		return sensor_s_vflip(sd, ctrl->value);
	case V4L2_CID_HFLIP:
		return sensor_s_hflip(sd, ctrl->value);
	case V4L2_CID_GAIN:
		return sensor_s_gain(sd, ctrl->value);
	case V4L2_CID_AUTOGAIN:
		return sensor_s_autogain(sd, ctrl->value);
	case V4L2_CID_EXPOSURE:
	case V4L2_CID_AUTO_EXPOSURE_BIAS:
  		return sensor_s_exp_bias(sd, ctrl->value);
	case V4L2_CID_EXPOSURE_AUTO:
		return sensor_s_autoexp(sd,(enum v4l2_exposure_auto_type) ctrl->value);
    case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:
		return sensor_s_wb(sd,(enum v4l2_auto_n_preset_white_balance) ctrl->value);
	case V4L2_CID_AUTO_WHITE_BALANCE:
		return sensor_s_autowb(sd, ctrl->value);
	case V4L2_CID_COLORFX:
		return sensor_s_colorfx(sd,(enum v4l2_colorfx) ctrl->value);
	case V4L2_CID_FLASH_LED_MODE:
	  	return sensor_s_flash_mode(sd,(enum v4l2_flash_led_mode) ctrl->value);

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
	info->auto_focus = 0;

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
	{ SENSOR_NAME, 0 },
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

