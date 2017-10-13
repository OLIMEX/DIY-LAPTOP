/*
 * A V4L2 driver for Novatek nt99252 cameras.
 * Novatek 2013-02-01
 * monitor reloation = 1280x800
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
#include <media/v4l2-mediabus.h>//linux-3.0
#include <linux/io.h>

#include "camera.h"
#include "sensor_helper.h"



MODULE_AUTHOR("raymonxiu");
MODULE_DESCRIPTION("A low-level driver for Novatek nt99252 sensors");
MODULE_LICENSE("GPL");



//for internel driver debug
#define DEV_DBG_EN   		0
#if(DEV_DBG_EN == 1)
#define vfe_dev_dbg(x,arg...) printk(KERN_INFO"[CSI_DEBUG][nt99252]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...)
#endif

#define vfe_dev_err(x,arg...) printk(KERN_INFO"[CSI_ERR][nt99252]"x,##arg)
#define vfe_dev_print(x,arg...) printk(KERN_INFO"[CSI][nt99252]"x,##arg)

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
#define IO_CFG		0						//0:csi back 1:csi front
#define V4L2_IDENT_SENSOR 0x2520


#define REG_ADDR_STEP 2
#define REG_DATA_STEP 1
#define REG_STEP 	     (REG_ADDR_STEP+REG_DATA_STEP)

/*
 * Basic window sizes.  These probably belong somewhere more globally
 * useful.
 */
#define UXGA_WIDTH		1600
#define UXGA_HEIGHT	  1200
#define SXGA_WIDTH		1280
#define SXGA_HEIGHT	       960
#define HD720_WIDTH 	1280
#define HD720_HEIGHT	720
#define SVGA_WIDTH		800
#define SVGA_HEIGHT 	600
#define VGA_WIDTH			640
#define VGA_HEIGHT		480
#define QVGA_WIDTH		320
#define QVGA_HEIGHT		240

#define CIF_WIDTH		352
#define CIF_HEIGHT		288

#define QCIF_WIDTH		176
#define	QCIF_HEIGHT	144

/*
 * Our nominal (default) frame rate.
 */
#define SENSOR_FRAME_RATE 8

/*
 * The nt99252 sits on i2c with ID 0x6C
 */
#define I2C_ADDR             0x6C
#define nt99252_SENSOR_ID    0x2520
#define SENSOR_NAME    "nt99252"
/* Registers */

#define PCLK_68M

//#define NTK_TEST
#define NT99252_AE_TARGET 0x3C	//0x3A

#if (NT99252_AE_TARGET == 0x30)
	//[AE_Target : 0x30]
#define NT99252_REG_0X32B8 0x36
#define NT99252_REG_0X32B9 0x2A
#define NT99252_REG_0X32BC 0x30
#define NT99252_REG_0X32BD 0x33
#define NT99252_REG_0X32BE 0x2D
#elif (NT99252_AE_TARGET == 0x38)
	//[AE_Target : 0x38]
#define NT99252_REG_0X32B8 0x3F
#define NT99252_REG_0X32B9 0x31
#define NT99252_REG_0X32BC 0x38
#define NT99252_REG_0X32BD 0x3C
#define NT99252_REG_0X32BE 0x34
#elif(NT99252_AE_TARGET == 0x3A)
	//[AE_Target : 0x3A]
#define NT99252_REG_0X32B8 0x42
#define NT99252_REG_0X32B9 0x32
#define NT99252_REG_0X32BC 0x3A
#define NT99252_REG_0X32BD 0x3E
#define NT99252_REG_0X32BE 0x36
#elif(NT99252_AE_TARGET == 0x3B)
	//[AE_Target : 0x3B]
#define NT99252_REG_0X32B8 0x43
#define NT99252_REG_0X32B9 0x33
#define NT99252_REG_0X32BC 0x3B
#define NT99252_REG_0X32BD 0x3F
#define NT99252_REG_0X32BE 0x37
#elif(NT99252_AE_TARGET == 0x3C)
	//[AE_Target : 0x3C]
#define NT99252_REG_0X32B8 0x44
#define NT99252_REG_0X32B9 0x34
#define NT99252_REG_0X32BC 0x3C
#define NT99252_REG_0X32BD 0x40
#define NT99252_REG_0X32BE 0x38
#elif(NT99252_AE_TARGET == 0x3D)
	//[AE_Target : 0x3D]
#define NT99252_REG_0X32B8 0x45
#define NT99252_REG_0X32B9 0x35
#define NT99252_REG_0X32BC 0x3D
#define NT99252_REG_0X32BD 0x41
#define NT99252_REG_0X32BE 0x39
#elif(NT99252_AE_TARGET == 0x3E)
	//[AE_Target : 0x3E]
#define NT99252_REG_0X32B8 0x46
#define NT99252_REG_0X32B9 0x36
#define NT99252_REG_0X32BC 0x3E
#define NT99252_REG_0X32BD 0x42
#define NT99252_REG_0X32BE 0x3A
#elif(NT99252_AE_TARGET == 0x3F)
	//[AE_Target : 0x3F]
#define NT99252_REG_0X32B8 0x47
#define NT99252_REG_0X32B9 0x37
#define NT99252_REG_0X32BC 0x3F
#define NT99252_REG_0X32BD 0x43
#define NT99252_REG_0X32BE 0x3B
#elif(NT99252_AE_TARGET == 0x40)
	//[AE_Target : 0x40]
#define NT99252_REG_0X32B8 0x48
#define NT99252_REG_0X32B9 0x38
#define NT99252_REG_0X32BC 0x40
#define NT99252_REG_0X32BD 0x44
#define NT99252_REG_0X32BE 0x3C
#endif

/*
 * Information we maintain about a known sensor.
 */
struct sensor_format_struct;  /* coming later */

#if 0
struct snesor_colorfx_struct; /* coming later */
__csi_subdev_info_t ccm_info_con =
{
	.mclk 	= MCLK,
	.vref 	= VREF_POL,
	.href 	= HREF_POL,
	.clock	= CLK_POL,
	.iocfg	= IO_CFG,
};

struct sensor_info {
	struct v4l2_subdev sd;
	struct sensor_format_struct *fmt;  /* Current format */
	__csi_subdev_info_t *ccm_info;
	int	width;
	int	height;
	int brightness;
	int	contrast;
	int saturation;
	int hue;
	int hflip;
	int vflip;
	int gain;
	int autogain;
	int exp;
	enum v4l2_exposure_auto_type autoexp;
	int autowb;
	enum v4l2_whiteblance wb;
	enum v4l2_colorfx clrfx;
  enum v4l2_flash_mode flash_mode;
	u8 clkrc;			/* Clock divider value */
};
#endif

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
#if 1	//For 10"
	{0x3069, 0x00},
	{0x306A, 0x00},
#else	//For 9.7"
	{0x306A, 0x02},
#endif

//NTK 2013-01-24
	{0x302A, 0x00},
  {0x302c, 0x09},
  {0x302d, 0x01},
	{0x301F, 0x80},
	{0x303f, 0x0e},
	{0x3051,0xE8},
	{0x320A, 0x00},

	{0x302E, 0x01},
	{0x3100, 0x01},
	{0x3101, 0x80},
	{0x3104, 0x03},
	{0x3105, 0x03},
	{0x3106, 0x0D},
	{0x310A, 0x62},
	{0x310D, 0x60},
	{0x3111, 0x5B},
	{0x3131, 0x58},
	{0x3127, 0x01},

	{0x3210, 0x2E},//3A
	{0x3211, 0x2E},//3A
	{0x3212, 0x2E},//3A
	{0x3213, 0x2E},//3A
	{0x3214, 0x21},
	{0x3215, 0x21},
	{0x3216, 0x21},
	{0x3217, 0x21},
	{0x3218, 0x21},
	{0x3219, 0x21},
	{0x321A, 0x21},
	{0x321B, 0x21},
	{0x321C, 0x1F},//22
	{0x321D, 0x1F},//22
	{0x321E, 0x1F},//22
	{0x321F, 0x1F},//22
	{0x3220, 0x01},
	{0x3221, 0x98},
	{0x3222, 0x01},
	{0x3223, 0x90},
	{0x3224, 0x01},
	{0x3225, 0x90},
	{0x3226, 0x01},
	{0x3227, 0x90},
	{0x3228, 0x00},
	{0x3229, 0xF0},
	{0x322A, 0x00},
	{0x322B, 0xF0},
	{0x322C, 0x00},
	{0x322D, 0xF0},
	{0x322E, 0x01},
	{0x322F, 0x04},
	{0x3230, 0x08},//0F
	{0x3231, 0x00},
	{0x3232, 0x00},
	{0x3233, 0x08},//0F
	{0x3234, 0x00},
	{0x3235, 0x00},
	{0x3236, 0x00},
	{0x3237, 0x00},
	{0x3238, 0x28},
	{0x3239, 0x28},
	{0x323A, 0x2E},
	{0x3241, 0x81},
	{0x3243, 0xC3},
	{0x3244, 0x00},
	{0x3245, 0x00},

	{0x3302,0x00}, //[CC_R1]
	{0x3303,0x5b},
	{0x3304,0x00},
	{0x3305,0x6c},
	{0x3306,0x00},
	{0x3307,0x3A},
	{0x3308,0x07},
	{0x3309,0xbf},
	{0x330A,0x06},
	{0x330B,0xf9},
	{0x330C,0x01},
	{0x330D,0x48},
	{0x330E,0x01},
	{0x330F,0x0b},
	{0x3310,0x06},
	{0x3311,0xfd},
	{0x3312,0x07},
	{0x3313,0xFb},

	{0x3270,0x00}, //GammaT1
	{0x3271,0x10},
	{0x3272,0x1c},
	{0x3273,0x31},
	{0x3274,0x44},
	{0x3275,0x54},
	{0x3276,0x6d},
	{0x3277,0x83},
	{0x3278,0x96},
	{0x3279,0xa7},
	{0x327A,0xc5},
	{0x327B,0xdd},
	{0x327C,0xef},
	{0x327D,0xf8},
	{0x327E,0xF8},

	{0x3250, 0x03},		//WB
	{0x3251, 0xFF},
	{0x3252, 0x00},     //83
	{0x3253, 0x03},
	{0x3254, 0xFF},
	{0x3255, 0x00},
	{0x3256, 0x80},
	{0x3257, 0x10},
	{0x329B, 0x00},
	                  //WB limit
	{0x32a1, 0x00},
	{0x32a2, 0xed},
	{0x32a3, 0x01},
	{0x32a4, 0x67},
	{0x32a5, 0x01},
	{0x32a6, 0x54},
	{0x32a7, 0x01},
	{0x32a8, 0xf8},

	{0x32b0, 0x55},    //AE window
	{0x32b1, 0xaa},
	{0x32b2, 0x14},

	{0x3327,0x00}, // EEXT Sel
	{0x3326,0x0c},
	{0x3360,0x08}, // IQ Sel
	{0x3361,0x0E},
	{0x3362,0x14},
	{0x3363,0xB3}, // Auto Control
	{0x3331,0x0C}, // EMap
	{0x3332,0x60},
	{0x3365,0x10},
	{0x3366,0x10},
	{0x3368,0x20}, // Edge Enhance
	{0x3369,0x1c},
	{0x336A,0x18},
	{0x336B,0x14},
	{0x336d,0x14}, // DPC
	{0x336e,0x12},
	{0x336f,0x0c},
	{0x3370,0x08},
	{0x3379,0x0A}, // NR_Comp_Max
	{0x337A,0x10},
	{0x337B,0x14},
	{0x337C,0x18},
	{0x3371,0x38}, // NR_Weight
	{0x3372,0x38},
	{0x3373,0x3F},
	{0x3374,0x3F},
	{0x33A0,0xb0}, // AS
	{0x33A1,0x10},
	{0x33A2,0x18},
	{0x33A3,0x40},
	{0x33A4,0x02},

	{0x33c0,0x03}, //Chroma
	{0x33c9,0xCF},
	{0x33ca,0x24},
	{0x3012,0x02},
	{0x3013,0x00},

	{0x32B8, NT99252_REG_0X32B8}, // AE Target
	{0x32B9, NT99252_REG_0X32B9},
	{0x32BC, NT99252_REG_0X32BC},
	{0x32BD, NT99252_REG_0X32BD},
	{0x32BE, NT99252_REG_0X32BE},

	{0x334A, 0x00},		//[GF_Previous]
	{0x334B, 0x7F},
	{0x334C, 0x1F},
	{0x3201, 0x7F},

//	{0x32AC, 0x02},
//	{0x32AD, 0xB8},

	{0x3060, 0x01},
};

/* 1600X1200 UXGA*/
static struct regval_list sensor_uxga_regs[] ={

	{0x334A, 0x34},
	{0x334B, 0x14},
	{0x334C, 0x10},
	{0x303E, 0x01},

	{0x32F1, 0x00},
	{0x32FC, 0x00},
	{0x32F8, 0x01},

	//[YUYV_1600x1200_8.33_14.01_Fps_50Hz] PCLK 68M
	{0x32BF, 0x60},
    {0x32C0, 0x7A},
    {0x32C1, 0x7A},
    {0x32C2, 0x7A},
    {0x32C3, 0x06},
    {0x32C4, 0x20},
    {0x32C5, 0x20},
    {0x32C6, 0x20},
    {0x32C7, 0x00},
    {0x32C8, 0x91},
    {0x32C9, 0x7A},
    {0x32CA, 0x9A},
    {0x32CB, 0x9A},
    {0x32CC, 0x9A},
    {0x32CD, 0x9A},
    {0x32DB, 0x72},
    {0x3241, 0x86},
    {0x32E0, 0x06},
    {0x32E1, 0x40},
    {0x32E2, 0x04},
    {0x32E3, 0xB0},
    {0x32E4, 0x00},
    {0x32E5, 0x00},
    {0x32E6, 0x00},
    {0x32E7, 0x00},
    {0x3200, 0x3E},
    {0x302A, 0x00},
    {0x302C, 0x0C},
    {0x302C, 0x1B},
    {0x302D, 0x21},
    {0x3022, 0x24},
    {0x3023, 0x24},
	{0x3002, 0x00},
	{0x3003, 0x04},
	{0x3004, 0x00},
	{0x3005, 0x04},
	{0x3006, 0x06},
	{0x3007, 0x43},
	{0x3008, 0x04},
	{0x3009, 0xCC},
    {0x300A, 0x07},
    {0x300B, 0x85},
    {0x300C, 0x04},
    {0x300D, 0xBC},
    {0x300E, 0x06},
    {0x300F, 0x40},
	{0x3010, 0x04},
	{0x3011, 0xB0},
	{0x32BB, 0x87},

	{0x325C, 0x03},
	{0x320A, 0x00},
	{0x3021, 0x06},
	{0x3060, 0x01},

};

/* 1280X960 SXGA */
static struct regval_list sensor_sxga_regs[] =
{
//[YUYV_1280x960_8.33_14.28_Fps_50Hz]  68Mhz
	{0x32fc, 0xf0},
	{0x32f8, 0x01},

    {0x32BF, 0x60},
    {0x32C0, 0x7A},
    {0x32C1, 0x7A},
    {0x32C2, 0x7A},
    {0x32C3, 0x06},
    {0x32C4, 0x20},
    {0x32C5, 0x20},
    {0x32C6, 0x20},
    {0x32C7, 0x00},
    {0x32C8, 0x91},
    {0x32C9, 0x7A},
    {0x32CA, 0x9A},
    {0x32CB, 0x9A},
    {0x32CC, 0x9A},
    {0x32CD, 0x9A},
    {0x32DB, 0x72},
    {0x3241, 0x86},
    {0x32E0, 0x05},
    {0x32E1, 0x00},
    {0x32E2, 0x03},
    {0x32E3, 0xC0},
    {0x32E4, 0x00},
    {0x32E5, 0x40},
    {0x32E6, 0x00},
    {0x32E7, 0x40},
    {0x3200, 0x3E},
    {0x302A, 0x00},
    {0x302C, 0x0C},
    {0x302C, 0x1B},
    {0x302D, 0x21},
    {0x3022, 0x24},
    {0x3023, 0x24},
    {0x3002, 0x00},
    {0x3003, 0x04},
    {0x3004, 0x00},
    {0x3005, 0x04},
    {0x3006, 0x06},
    {0x3007, 0x43},
    {0x3008, 0x04},
    {0x3009, 0xCC},
    {0x300A, 0x07},
    {0x300B, 0x85},
    {0x300C, 0x04},
    {0x300D, 0xBC},
    {0x300E, 0x06},
    {0x300F, 0x40},
    {0x3010, 0x04},
    {0x3011, 0xB0},
    {0x32BB, 0x87},

    {0x325C, 0x03},
    {0x320A, 0x48},
    {0x3021, 0x06},
    {0x3060, 0x01},
};

/* 1280X720 */
static struct regval_list sensor_720p_regs[] =
{//YUYV_1280x720_1600*1200 scaler_10.00_14.28_Fps_50Hz PCLK 68Mhz

	{0x32BF, 0x60},
    {0x32C0, 0x7A},
    {0x32C1, 0x7A},
    {0x32C2, 0x7A},
    {0x32C3, 0x06},
    {0x32C4, 0x20},
    {0x32C5, 0x20},
    {0x32C6, 0x20},
    {0x32C7, 0x00},
    {0x32C8, 0x91},
    {0x32C9, 0x7A},
    {0x32CA, 0x9A},
    {0x32CB, 0x9A},
    {0x32CC, 0x9A},
    {0x32CD, 0x9A},
    {0x32DB, 0x72},
    {0x3241, 0x86},
    {0x32E0, 0x05},
    {0x32E1, 0x00},
    {0x32E2, 0x02},
    {0x32E3, 0xD0},
    {0x32E4, 0x00},
    {0x32E5, 0x40},
    {0x32E6, 0x00},
    {0x32E7, 0xAB},
    {0x3200, 0x3E},
    {0x302A, 0x00},
    {0x302C, 0x0C},
    {0x302C, 0x1B},
	{0x302D, 0x21},
	{0x3022, 0x24},
	{0x3023, 0x24},
	{0x3002, 0x00},
	{0x3003, 0x04},
	{0x3004, 0x00},
	{0x3005, 0x04},
	{0x3006, 0x06},
	{0x3007, 0x43},
	{0x3008, 0x04},
	{0x3009, 0xCC},
	{0x300A, 0x07},
    {0x300B, 0x85},
	{0x300C, 0x04},
	{0x300D, 0xBC},
	{0x300E, 0x06},
	{0x300F, 0x40},
	{0x3010, 0x04},
	{0x3011, 0xB0},
	{0x32BB, 0x87},
	{0x325C, 0x03},
	{0x320A, 0x48},
	{0x3021, 0x06},
	{0x3060, 0x01},
};
/*1024*768*/
//static struct regval_list sensor_xga_regs[] =
//{};

/* 800X600 SVGA*/
static struct regval_list sensor_svga_regs[] ={
//800*600_1600*1200 scaler_8.33~14.28fps PCLK 68Mhz
	{0x334A, 0x00},
	{0x334B, 0x7F},
	{0x334C, 0x1F},
	{0x303e, 0x01},  //08
  {0x3080, 0x00},
  {0x3081, 0x00},
  {0x3082, 0x03},

	//{0x3052, 0x0f},
	{0x32fc, 0xf8},
	{0x32f8, 0x01},
	{0x32BF, 0x60},
    {0x32C0, 0x7A},
    {0x32C1, 0x7A},
    {0x32C2, 0x7A},
    {0x32C3, 0x06},
    {0x32C4, 0x20},
    {0x32C5, 0x20},
    {0x32C6, 0x20},
    {0x32C7, 0x00},
    {0x32C8, 0x91},
    {0x32C9, 0x7A},
    {0x32CA, 0x9A},
    {0x32CB, 0x9A},
    {0x32CC, 0x9A},
    {0x32CD, 0x9A},
    {0x32DB, 0x72},
    {0x3241, 0x86},
    {0x32E0, 0x03},
    {0x32E1, 0x20},
	{0x32E2, 0x02},
	{0x32E3, 0x58},
	{0x32E4, 0x01},
	{0x32E5, 0x00},
	{0x32E6, 0x01},
	{0x32E7, 0x00},
	{0x3200, 0x3E},
	//{0x3201, 0x7F},
	{0x302A, 0x00},
	{0x302C, 0x0C},
    {0x302C, 0x1B},
    {0x302D, 0x21},
    {0x3022, 0x24},
    {0x3023, 0x24},
    {0x3002, 0x00},
    {0x3003, 0x04},
    {0x3004, 0x00},
    {0x3005, 0x04},
    {0x3006, 0x06},
    {0x3007, 0x43},
    {0x3008, 0x04},
    {0x3009, 0xCC},
    {0x300A, 0x07},
    {0x300B, 0x85},
    {0x300C, 0x04},
    {0x300D, 0xBC},
    {0x300E, 0x06},
	{0x300F, 0x40},
	{0x3010, 0x04},
	{0x3011, 0xB0},
	{0x32BB, 0x87},
	{0x325C, 0x03},
	{0x320A, 0x6C},
	{0x3021, 0x06},
	{0x3060, 0x01},
};

/* 640X480 VGA */
static struct regval_list sensor_vga_regs[] =
{
	//640*480_1600*1200 scaler_8.33~14.28fps PCLK 68Mhz
	{0x334A, 0x00},
	{0x334B, 0x7F},
	{0x334C, 0x1F},
	{0x303e, 0x01},
  {0x3080, 0x00},
  {0x3081, 0x00},
  {0x3082, 0x03},

	//{0x3052, 0x0f},
	{0x32fc, 0xf8},
	{0x32f8, 0x01},

	{0x32BF, 0x60},
    {0x32C0, 0x7A},
    {0x32C1, 0x7A},
    {0x32C2, 0x7A},
    {0x32C3, 0x06},
    {0x32C4, 0x20},
    {0x32C5, 0x20},
    {0x32C6, 0x20},
    {0x32C7, 0x00},
    {0x32C8, 0x91},
    {0x32C9, 0x7A},
    {0x32CA, 0x9A},
    {0x32CB, 0x9A},
    {0x32CC, 0x9A},
    {0x32CD, 0x9A},
    {0x32DB, 0x72},
    {0x3241, 0x86},
    {0x32E0, 0x02},
    {0x32E1, 0x80},
    {0x32E2, 0x01},
    {0x32E3, 0xE0},
    {0x32E4, 0x01},
    {0x32E5, 0x81},
    {0x32E6, 0x01},
    {0x32E7, 0x81},
    {0x3200, 0x3E},
    {0x302A, 0x00},
    {0x302C, 0x0C},
    {0x302C, 0x1B},
    {0x302D, 0x21},
	{0x3022, 0x24},
	{0x3023, 0x24},
	{0x3002, 0x00},
	{0x3003, 0x04},
	{0x3004, 0x00},
	{0x3005, 0x04},
	{0x3006, 0x06},
	{0x3007, 0x43},
	{0x3008, 0x04},
	{0x3009, 0xCC},
	{0x300A, 0x07},
    {0x300B, 0x85},
	{0x300C, 0x04},
	{0x300D, 0xBC},
	{0x300E, 0x06},
	{0x300F, 0x40},
	{0x3010, 0x04},
	{0x3011, 0xB0},
	{0x32BB, 0x87},
	{0x325C, 0x03},
	{0x320A, 0x68},
		//{0x3025, 0x02},
	{0x3021, 0x06},
	{0x3060, 0x01},
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
	{0x3201, 0x7F},
};

static struct regval_list sensor_wb_incandescence_regs[] = {
	{0x3201, 0x6F},
	{0x3290, 0x01},
	{0x3291, 0x30},
	{0x3296, 0x01},
	{0x3297, 0xCB},
};

static struct regval_list sensor_wb_fluorescent_regs[] = {
	{0x3201, 0x6F},
	{0x3290, 0x01},
	{0x3291, 0x70},
	{0x3296, 0x01},
	{0x3297, 0xFF},
};

static struct regval_list sensor_wb_tungsten_regs[] = {
	{0x3201, 0x6F},
	{0x3290, 0x01},
	{0x3291, 0x00},
	{0x3296, 0x02},
	{0x3297, 0x30},
};

static struct regval_list sensor_wb_horizon[] = {
//null
};

static struct regval_list sensor_wb_daylight_regs[] = {
	{0x3201, 0x6F},
	{0x3290, 0x01},
	{0x3291, 0x38},
	{0x3296, 0x01},
	{0x3297, 0x68},
};


static struct regval_list sensor_wb_flash[] = {
//null
};

static struct regval_list sensor_wb_cloud_regs[] = {
	{0x3201, 0x6F},
	{0x3290, 0x01},
	{0x3291, 0x51},
	{0x3296, 0x01},
	{0x3297, 0x00},
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
    {0x32f1, 0x00},
   	{0x32F8, 0x01},
};

static struct regval_list sensor_colorfx_bw_regs[] = {
	{0x32F1, 0x01},
    {0x32F8, 0x01},
};

static struct regval_list sensor_colorfx_sepia_regs[] = {
    {0x32f1, 0x02},
    {0x32f6, 0x20},
    {0x32F8, 0x01},
};

static struct regval_list sensor_colorfx_negative_regs[] = {
    {0x32f1, 0x03},
    {0x32F8, 0x01},
};

static struct regval_list sensor_colorfx_emboss_regs[] = {
//NULL
};

static struct regval_list sensor_colorfx_sketch_regs[] = {
//NULL
};

static struct regval_list sensor_colorfx_sky_blue_regs[] = {
    {0x32f1, 0x05},
    {0x32f4, 0xF0},
    {0x32f5, 0x80},
    {0x32F8, 0x01},
};

static struct regval_list sensor_colorfx_grass_green_regs[] = {
    {0x32f1, 0x05},
    {0x32f4, 0x60},
    {0x32f5, 0x20},
    {0x32F8, 0x01},
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
//NULL
};

static struct regval_list sensor_brightness_neg2_regs[] = {
//NULL
};

static struct regval_list sensor_brightness_neg1_regs[] = {
//NULL
};

static struct regval_list sensor_brightness_zero_regs[] = {
//NULL
};

static struct regval_list sensor_brightness_pos1_regs[] = {
	//NULL
};

static struct regval_list sensor_brightness_pos2_regs[] = {
//NULL
};

static struct regval_list sensor_brightness_pos3_regs[] = {
//NULL
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
	{0x32FC, 0x40},
	{0x32F2, 0x40},
	{0x32F8, 0x01},
};

static struct regval_list sensor_contrast_neg3_regs[] = {
	{0x32FC, 0x30},
	{0x32F2, 0x50},
	{0x32F8, 0x01},
};

static struct regval_list sensor_contrast_neg2_regs[] = {
	{0x32FC, 0x20},
	{0x32F2, 0x60},
	{0x32F8, 0x01},
};

static struct regval_list sensor_contrast_neg1_regs[] = {
	{0x32FC, 0x10},
	{0x32F2, 0x70},
	{0x32F8, 0x01},
};

static struct regval_list sensor_contrast_zero_regs[] = {
	{0x32FC, 0xf8},
	{0x32F2, 0x80},
	{0x32F8, 0x01},
};

static struct regval_list sensor_contrast_pos1_regs[] = {
	{0x32FC, 0xF0},
	{0x32F2, 0x90},
	{0x32F8, 0x01},
};

static struct regval_list sensor_contrast_pos2_regs[] = {
	{0x32FC, 0xE0},
	{0x32F2, 0xA0},
	{0x32F8, 0x01},
};

static struct regval_list sensor_contrast_pos3_regs[] = {
	{0x32FC, 0xD0},
	{0x32F2, 0xB0},
	{0x32F8, 0x01},
};

static struct regval_list sensor_contrast_pos4_regs[] = {
	{0x32FC, 0xC0},
	{0x32F2, 0xC0},
	{0x32F8, 0x01},
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
    {0x32F3, 0x40},
	{0x32F8, 0x01},
};

static struct regval_list sensor_saturation_neg3_regs[] = {
    {0x32F3, 0x50},
	{0x32F8, 0x01},
};

static struct regval_list sensor_saturation_neg2_regs[] = {
    {0x32F3, 0x60},
	{0x32F8, 0x01},
};

static struct regval_list sensor_saturation_neg1_regs[] = {
    {0x32F3, 0x70},
	{0x32F8, 0x01},
};

static struct regval_list sensor_saturation_zero_regs[] = {
    {0x32F3, 0x80},
	{0x32F8, 0x01},
};

static struct regval_list sensor_saturation_pos1_regs[] = {
    {0x32F3, 0x90},
	{0x32F8, 0x01},
};

static struct regval_list sensor_saturation_pos2_regs[] = {
    {0x32F3, 0xa0},
	{0x32F8, 0x01},
};

static struct regval_list sensor_saturation_pos3_regs[] = {
    {0x32F3, 0xb0},
	{0x32F8, 0x01},
};

static struct regval_list sensor_saturation_pos4_regs[] = {
    {0x32F3, 0xc0},
	{0x32F8, 0x01},
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
	{0x32F2, 0x30},
	{0x32F8, 0x01},
};

static struct regval_list sensor_ev_neg3_regs[] = {
	{0x32F2, 0x40},
	{0x32F8, 0x01},
};

static struct regval_list sensor_ev_neg2_regs[] = {
	{0x32F2, 0x50},
	{0x32F8, 0x01},
};

static struct regval_list sensor_ev_neg1_regs[] = {
	{0x32F2, 0x60},
	{0x32F8, 0x01},
};

static struct regval_list sensor_ev_zero_regs[] = {
	{0x32F2, 0x70},
	{0x32F8, 0x01},
};

static struct regval_list sensor_ev_pos1_regs[] = {
	{0x32F2, 0x80},
	{0x32F8, 0x01},
};

static struct regval_list sensor_ev_pos2_regs[] = {
	{0x32F2, 0x90},
	{0x32F8, 0x01},
};

static struct regval_list sensor_ev_pos3_regs[] = {
	{0x32F2, 0xa0},
	{0x32F8, 0x01},
};

static struct regval_list sensor_ev_pos4_regs[] = {
	{0x32F2, 0xb0},
	{0x32F8, 0x01},
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

static struct regval_list sensor_oe_disable_regs[] = {
};

static struct regval_list sensor_oe_enable_regs[] = {
};


/*
 * Here we'll try to encapsulate the changes for just the output
 * video format.
 *
 */

static struct regval_list sensor_fmt_yuv422_yuyv[] = {
	//{0x32F0, 0x01},
};


static struct regval_list sensor_fmt_yuv422_yvyu[] = {
	//{0x32F0, 0x03},
};

static struct regval_list sensor_fmt_yuv422_vyuy[] = {
	//{0x32F0, 0x02},
};

static struct regval_list sensor_fmt_yuv422_uyvy[] = {
	//{0x32F0, 0x00},
};

static struct regval_list sensor_fmt_raw[] = {
	//{0x32F0, 0x70},
};

static int sensor_g_hflip(struct v4l2_subdev *sd, __s32 *value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	data_type val;

	ret = sensor_read(sd, 0x3022, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_g_hflip!\n");
		return ret;
	}

	val = (val & 0x02);

	*value = val;

	info->hflip = *value;
	return 0;
}

static int sensor_s_hflip(struct v4l2_subdev *sd, int value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	data_type val;

	ret = sensor_read(sd, 0x3022, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_s_hflip!\n");
		return ret;
	}

	switch (value) {
		case 0:
		  val &= 0xFD;
			break;
		case 1:
			val |= 0x02;
			break;
		default:
			return -EINVAL;
	}
	ret = sensor_write(sd, 0x3022, val);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_hflip!\n");
		return ret;
	}

	mdelay(20);

	info->hflip = value;
	return 0;
}

static int sensor_g_vflip(struct v4l2_subdev *sd, __s32 *value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	data_type val;


	ret = sensor_read(sd, 0x3022, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_g_vflip!\n");
		return ret;
	}

	val = (val & 0x01);

	*value = val;

	info->vflip = *value;
	return 0;
}

static int sensor_s_vflip(struct v4l2_subdev *sd, int value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	data_type val;


	ret = sensor_read(sd, 0x3022, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_s_vflip!\n");
		return ret;
	}

	switch (value) {
		case 0:
		  val &= 0xFE;
			break;
		case 1:
			val |= 0x01;
			break;
		default:
			return -EINVAL;
	}
	ret = sensor_write(sd, 0x3022, val);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_vflip!\n");
		return ret;
	}

	mdelay(20);

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
	int ret;
	struct sensor_info *info = to_state(sd);
	data_type val;


	ret = sensor_read(sd, 0x3201, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_g_autoexp!\n");
		return ret;
	}

	val = ((val& 0x20) >> 5);

	if (val == 0x01) {
		*value = V4L2_EXPOSURE_AUTO;
	}
	else
	{
		*value = V4L2_EXPOSURE_MANUAL;
	}

	info->autoexp = *value;
	return 0;
}

static int sensor_s_autoexp(struct v4l2_subdev *sd,
		enum v4l2_exposure_auto_type value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	data_type val;


	ret = sensor_read(sd, 0x3201, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_s_autoexp!\n");
		return ret;
	}

	switch (value) {
		case V4L2_EXPOSURE_AUTO:
		  val |= 0x20;
			break;
		case V4L2_EXPOSURE_MANUAL:
			val &= 0xDF;
			break;
		case V4L2_EXPOSURE_SHUTTER_PRIORITY:
			return -EINVAL;
		case V4L2_EXPOSURE_APERTURE_PRIORITY:
			return -EINVAL;
		default:
			return -EINVAL;
	}

	ret = sensor_write(sd, 0x3201, val);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_autoexp!\n");
		return ret;
	}

	mdelay(10);

	info->autoexp = value;
	return 0;
}

static int sensor_g_autowb(struct v4l2_subdev *sd, int *value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	data_type val;


	ret = sensor_read(sd, 0x3201, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_g_autowb!\n");
		return ret;
	}

	val = ((val & 0x10) >> 4);

	*value = val;
	info->autowb = *value;

	return 0;
}

static int sensor_s_autowb(struct v4l2_subdev *sd, int value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	data_type val;

	ret = sensor_write_array(sd, sensor_wb_auto_regs, ARRAY_SIZE(sensor_wb_auto_regs));
	if (ret < 0) {
		vfe_dev_err("sensor_write_array err at sensor_s_autowb!\n");
		return ret;
	}

	ret = sensor_read(sd, 0x3201, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_s_autowb!\n");
		return ret;
	}

	switch(value) {
	case 0:
		val &= 0xEF;
		break;
	case 1:
		val |= 0x10;
		break;
	default:
		break;
	}
	ret = sensor_write(sd, 0x3201, val);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_autowb!\n");
		return ret;
	}

	mdelay(10);

	info->autowb = value;
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
	mdelay(10);

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

static int sensor_s_sw_stby(struct v4l2_subdev *sd, int on_off)
{
	int ret=0;
	return ret;
}

/*
 * Stuff that knows about the sensor.
 */

static int sensor_power(struct v4l2_subdev *sd, int on)
{
	int ret;
	cci_lock(sd);
	switch(on)
	{
		case CSI_SUBDEV_STBY_ON:
			vfe_dev_dbg("CSI_SUBDEV_STBY_ON\n");
			vfe_dev_print("disalbe oe!\n");
			ret = sensor_write_array(sd, sensor_oe_disable_regs, ARRAY_SIZE(sensor_oe_disable_regs));
			if(ret < 0)
				vfe_dev_err("disalbe oe falied!\n");
			ret = sensor_s_sw_stby(sd, CSI_GPIO_HIGH);
			if(ret < 0)
				vfe_dev_err("soft stby falied!\n");
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			mdelay(10);
			vfe_set_mclk(sd,OFF);
			break;
		case CSI_SUBDEV_STBY_OFF:
			vfe_dev_dbg("CSI_SUBDEV_STBY_OFF\n");
			ret = sensor_s_sw_stby(sd, CSI_GPIO_LOW);
			if(ret < 0)
				vfe_dev_err("soft stby off falied!\n");
			mdelay(10);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
			mdelay(10);
			vfe_set_mclk_freq(sd,MCLK);
			vfe_set_mclk(sd,ON);
			mdelay(10);
			vfe_dev_print("enable oe!\n");
			ret = sensor_write_array(sd, sensor_oe_enable_regs,  ARRAY_SIZE(sensor_oe_enable_regs));
			if(ret < 0)
				vfe_dev_err("enable oe falied!\n");
			break;
		case CSI_SUBDEV_PWR_ON:
			vfe_dev_dbg("CSI_SUBDEV_PWR_ON\n");
			vfe_gpio_set_status(sd,PWDN,1);//set the gpio to output
			vfe_gpio_set_status(sd,RESET,1);//set the gpio to output
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			mdelay(1);
			vfe_set_mclk_freq(sd,MCLK);
			vfe_set_mclk(sd,ON);
			mdelay(10);
			vfe_gpio_write(sd,POWER_EN,CSI_GPIO_HIGH);
			vfe_set_pmu_channel(sd,IOVDD,ON);
			vfe_set_pmu_channel(sd,AVDD,ON);
			vfe_set_pmu_channel(sd,DVDD,ON);
			vfe_set_pmu_channel(sd,AFVDD,ON);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
			mdelay(10);
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);
			mdelay(30);
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			mdelay(30);
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);
			mdelay(30);
			break;
		case CSI_SUBDEV_PWR_OFF:
			vfe_dev_dbg("CSI_SUBDEV_PWR_OFF\n");
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			mdelay(10);
			vfe_gpio_write(sd,POWER_EN,CSI_GPIO_LOW);
			vfe_set_pmu_channel(sd,AFVDD,OFF);
			vfe_set_pmu_channel(sd,DVDD,OFF);
			vfe_set_pmu_channel(sd,AVDD,OFF);
			vfe_set_pmu_channel(sd,IOVDD,OFF);
			mdelay(10);
			vfe_set_mclk(sd,OFF);
			vfe_gpio_set_status(sd,RESET,0);//set the gpio to input
			vfe_gpio_set_status(sd,PWDN,0);//set the gpio to input
			break;
		default:
			return -EINVAL;
	}
	cci_unlock(sd);
	return 0;
}

static int sensor_reset(struct v4l2_subdev *sd, u32 val)
{
	switch(val)
	{
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		case 3:
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);
			mdelay(10);
			break;
		case 4:
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			mdelay(10);
			break;
		case 5:
			vfe_dev_dbg("CSI_SUBDEV_RST_PUL\n");
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);
			mdelay(10);
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			mdelay(30);
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);
			mdelay(10);
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

static int sensor_detect(struct v4l2_subdev *sd)
{
	int ret;
	unsigned int SENSOR_ID=0;
	unsigned int version=0;
	data_type val;

	ret = sensor_read(sd, 0x3000, &val);
	SENSOR_ID|= (val<< 8);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_detect!\n");
		return ret;
	}

	ret = sensor_read(sd, 0x3001, &val);
	SENSOR_ID|= (val);
	printk("nt99252_SENSOR_ID=%x",SENSOR_ID);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_detect!\n");
		return ret;
	}

	if(SENSOR_ID != nt99252_SENSOR_ID)
		return -ENODEV;
#if 1
	// Test
	ret = sensor_read(sd, 0x307E, &val);

	version = (val<< 8);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_detect!\n");
		return ret;
	}
	vfe_dev_dbg("0x307E: %X\n", version);

	ret = sensor_read(sd, 0x307F, &val);

	version= (val<< 8);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_detect!\n");
		return ret;
	}
	vfe_dev_dbg("0x307E: %X\n", version);
#endif
	return 0;
}

static int sensor_init(struct v4l2_subdev *sd, u32 val)
{
	int ret;
	vfe_dev_dbg("sensor_init\n");
	/*Make sure it is a target sensor*/
	ret = sensor_detect(sd);
	if (ret) {
		vfe_dev_err("chip found is not an target chip.\n");
		return ret;
	}

	return sensor_write_array(sd, sensor_default_regs , ARRAY_SIZE(sensor_default_regs));
}

static long sensor_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	int ret=0;
	return ret;
}


/*
 * Store information about the video data format.
 */
static struct sensor_format_struct {
	__u8 *desc;
	//__u32 pixelformat;
	enum v4l2_mbus_pixelcode mbus_code;//linux-3.0
	struct regval_list *regs;
	int	regs_size;
	int bpp;   /* Bytes per pixel */
} sensor_formats[] = {
	{
		.desc		= "YUYV 4:2:2",
		.mbus_code	= V4L2_MBUS_FMT_YUYV8_2X8,//linux-3.0
		.regs 		= sensor_fmt_yuv422_yuyv,
		.regs_size = ARRAY_SIZE(sensor_fmt_yuv422_yuyv),
		.bpp		= 2,
	},
	{
		.desc		= "YVYU 4:2:2",
		.mbus_code	= V4L2_MBUS_FMT_YVYU8_2X8,//linux-3.0
		.regs 		= sensor_fmt_yuv422_yvyu,
		.regs_size = ARRAY_SIZE(sensor_fmt_yuv422_yvyu),
		.bpp		= 2,
	},
	{
		.desc		= "UYVY 4:2:2",
		.mbus_code	= V4L2_MBUS_FMT_UYVY8_2X8,//linux-3.0
		.regs 		= sensor_fmt_yuv422_uyvy,
		.regs_size = ARRAY_SIZE(sensor_fmt_yuv422_uyvy),
		.bpp		= 2,
	},
	{
		.desc		= "VYUY 4:2:2",
		.mbus_code	= V4L2_MBUS_FMT_VYUY8_2X8,//linux-3.0
		.regs 		= sensor_fmt_yuv422_vyuy,
		.regs_size = ARRAY_SIZE(sensor_fmt_yuv422_vyuy),
		.bpp		= 2,
	},
	{
		.desc		= "Raw RGB Bayer",
		.mbus_code	= V4L2_MBUS_FMT_SBGGR8_1X8,//linux-3.0
		.regs 		= sensor_fmt_raw,
		.regs_size = ARRAY_SIZE(sensor_fmt_raw),
		.bpp		= 1
	},
};
#define N_FMTS ARRAY_SIZE(sensor_formats)




/*
 * Then there is the issue of window sizes.  Try to capture the info here.
 */


static struct sensor_win_size
 sensor_win_sizes[] = {
	/* UXGA */
	{
		.width			= UXGA_WIDTH,
		.height			= UXGA_HEIGHT,
		.regs 			= sensor_uxga_regs,
		.regs_size	= ARRAY_SIZE(sensor_uxga_regs),
		.set_size		= NULL,
	},
		/* SXGA */
	{
		.width			= SXGA_WIDTH,
		.height			= SXGA_HEIGHT,
		.regs 			= sensor_sxga_regs,
		.regs_size	= ARRAY_SIZE(sensor_sxga_regs),
		.set_size		= NULL,
	},
	/* 720p */
	{
		.width			= HD720_WIDTH,
		.height			= HD720_HEIGHT,
		.regs 			= sensor_720p_regs,
		.regs_size	= ARRAY_SIZE(sensor_720p_regs),
		.set_size		= NULL,
	},
	/* SVGA */
	{
		.width			= SVGA_WIDTH,
		.height			= SVGA_HEIGHT,
		.regs				= sensor_svga_regs,
		.regs_size	= ARRAY_SIZE(sensor_svga_regs),
		.set_size		= NULL,
	},
	/* VGA */
	{
		.width			= VGA_WIDTH,
		.height			= VGA_HEIGHT,
		.regs				= sensor_vga_regs,
		.regs_size	= ARRAY_SIZE(sensor_vga_regs),
		.set_size		= NULL,
	},
};

#define N_WIN_SIZES (ARRAY_SIZE(sensor_win_sizes))


static int sensor_enum_fmt(struct v4l2_subdev *sd, unsigned index,
                 enum v4l2_mbus_pixelcode *code)//linux-3.0
{
	if (index >= N_FMTS)//linux-3.0
		return -EINVAL;

	*code = sensor_formats[index].mbus_code;//linux-3.0

	return 0;
}

static int sensor_try_fmt_internal(struct v4l2_subdev *sd,
		//struct v4l2_format *fmt,
		struct v4l2_mbus_framefmt *fmt,//linux-3.0
		struct sensor_format_struct **ret_fmt,
		struct sensor_win_size **ret_wsize)
{
	int index;
	struct sensor_win_size *wsize;
	vfe_dev_dbg("sensor_try_fmt_internal\n");
	for (index = 0; index < N_FMTS; index++)
		if (sensor_formats[index].mbus_code == fmt->code)//linux-3.0
			break;

	if (index >= N_FMTS) {
		/* default to first format */
		index = 0;
		fmt->code = sensor_formats[0].mbus_code;//linux-3.0
	}

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
	for (wsize = sensor_win_sizes; wsize < sensor_win_sizes + N_WIN_SIZES;
	     wsize++)
		if (fmt->width >= wsize->width && fmt->height >= wsize->height)//linux-3.0
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
             struct v4l2_mbus_framefmt *fmt)//linux-3.0
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
             struct v4l2_mbus_framefmt *fmt)//linux-3.0
{
	int ret;
	struct sensor_format_struct *sensor_fmt;
	struct sensor_win_size *wsize;
	struct sensor_info *info = to_state(sd);

	vfe_dev_dbg("sensor_s_fmt\n");

	ret = sensor_try_fmt_internal(sd, fmt, &sensor_fmt, &wsize);
	if (ret)
		return ret;

	sensor_write_array(sd, sensor_fmt->regs , sensor_fmt->regs_size);

	ret = 0;
	if (wsize->regs)
	{
		ret = sensor_write_array(sd, wsize->regs , wsize->regs_size);
		if (ret < 0)
			return ret;
	}

	if (wsize->set_size)
	{
		ret = wsize->set_size(sd);
		if (ret < 0)
			return ret;
	}

	mdelay(500);
	info->fmt = sensor_fmt;
	info->width = wsize->width;
	info->height = wsize->height;

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
	cp->timeperframe.numerator = 1;

	if (info->width > SVGA_WIDTH && info->height > SVGA_HEIGHT) {
		cp->timeperframe.denominator = SENSOR_FRAME_RATE/2;
	}
	else {
		cp->timeperframe.denominator = SENSOR_FRAME_RATE;
	}

	return 0;
}

static int sensor_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *parms)
{
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
//	case V4L2_CID_BRIGHTNESS:
//		return v4l2_ctrl_query_fill(qc, -4, 4, 1, 1);
//	case V4L2_CID_CONTRAST:
//		return v4l2_ctrl_query_fill(qc, -4, 4, 1, 1);
//	case V4L2_CID_SATURATION:
//		return v4l2_ctrl_query_fill(qc, -4, 4, 1, 1);
//	case V4L2_CID_HUE:
//		return v4l2_ctrl_query_fill(qc, -180, 180, 5, 0);
	case V4L2_CID_VFLIP:
	case V4L2_CID_HFLIP:
		return v4l2_ctrl_query_fill(qc, 0, 1, 1, 0);
//	case V4L2_CID_GAIN:
//		return v4l2_ctrl_query_fill(qc, 0, 255, 1, 128);
//	case V4L2_CID_AUTOGAIN:
//		return v4l2_ctrl_query_fill(qc, 0, 1, 1, 1);
	case V4L2_CID_EXPOSURE:
    case V4L2_CID_AUTO_EXPOSURE_BIAS:
		return v4l2_ctrl_query_fill(qc, -4, 4, 1, 0);
	case V4L2_CID_EXPOSURE_AUTO:
		return v4l2_ctrl_query_fill(qc, 0, 1, 1, 0);
    case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:
		return v4l2_ctrl_query_fill(qc, 0, 5, 1, 0);
	case V4L2_CID_AUTO_WHITE_BALANCE:
		return v4l2_ctrl_query_fill(qc, 0, 1, 1, 1);
	case V4L2_CID_COLORFX:
		return v4l2_ctrl_query_fill(qc, 0, 9, 1, 0);
	case V4L2_CID_FLASH_LED_MODE:
	  	return v4l2_ctrl_query_fill(qc, 0, 4, 1, 0);
	}
	return -EINVAL;
}

static int sensor_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
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
//  case V4L2_CID_POWER_LINE_FREQUENCY:
//    return sensor_g_band_filter(sd, &ctrl->value);
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
	cci_dev_probe_helper(sd, client, &sensor_ops, &cci_drv);

	info->fmt = &sensor_formats[0];

	info->brightness = 0;
	info->contrast = 0;
	info->saturation = 0;
	info->hue = 0;
	info->hflip = 0;
	info->vflip = 0;
	info->gain = 0;
	info->autogain = 1;
	info->exp = 0;
	info->autoexp = 0;
	info->autowb = 1;
	info->wb = 0;
	info->clrfx = 0;

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

