/*
 * A V4L2 driver for OV8858 cameras.
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

MODULE_AUTHOR("lwj");
MODULE_DESCRIPTION("A low-level driver for OV8858 sensors");
MODULE_LICENSE("GPL");

//for internel driver debug
#define DEV_DBG_EN      1 
#if(DEV_DBG_EN == 1)    
#define vfe_dev_dbg(x,arg...) printk("[OV8858]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...) 
#endif
#define vfe_dev_err(x,arg...) printk("[OV8858]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[OV8858]"x,##arg)

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
#define V4L2_IDENT_SENSOR 0x8858
int ov8858_sensor_vts;



/*
 * Our nominal (default) frame rate.
 */

#define SENSOR_FRAME_RATE 30


/*
 * The ov8858 sits on i2c with ID 0x6c
 */
#define I2C_ADDR 0x6c
#define SENSOR_NAME "ov8858"
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
	//2lane initial
	{0x0103, 0x01},// ; software reset
	{0x0100, 0x00},// ; software standby
	{0x0100, 0x00},// ;
	{0x0100, 0x00},// ;
	
	{0x0302, 0x1e},// ; pll1_multi
	{0x0303, 0x00},// ; pll1_divm
	{0x0304, 0x03},// ; pll1_div_mipi
	{0x030e, 0x00},// ; pll2_rdiv
	{0x030f, 0x09},// ; pll2_divsp
	{0x0312, 0x01},// ; pll2_pre_div0, pll2_r_divdac
	{0x031e, 0x0c},// ; pll1_no_lat
	{0x3600, 0x00},//
	{0x3601, 0x00},//
	{0x3602, 0x00},//
	{0x3603, 0x00},//
	{0x3604, 0x22},//
	{0x3605, 0x30},//
	{0x3606, 0x00},//
	{0x3607, 0x20},//
	{0x3608, 0x11},//
	{0x3609, 0x28},//
	{0x360a, 0x00},//
	{0x360b, 0x06},//
	{0x360c, 0xdc},//
	{0x360d, 0x40},//
	{0x360e, 0x0c},//
	{0x360f, 0x20},//
	{0x3610, 0x07},//
	{0x3611, 0x20},//
	{0x3612, 0x88},//
	{0x3613, 0x80},//
	{0x3614, 0x58},//
	{0x3615, 0x00},//
	{0x3616, 0x4a},//
	{0x3617, 0xb0},//
	{0x3618, 0x56},//
	{0x3619, 0x70},//
	{0x361a, 0x99},//
	{0x361b, 0x00},//
	{0x361c, 0x07},//
	{0x361d, 0x00},//
	{0x361e, 0x00},//
	{0x361f, 0x00},//
	{0x3638, 0xff},//
	{0x3633, 0x0c},//
	{0x3634, 0x0c},//
	{0x3635, 0x0c},//
	{0x3636, 0x0c},//
	{0x3645, 0x13},//
	{0x3646, 0x83},//
	{0x364a, 0x07},//
	{0x3015, 0x01},// ; 
	
	{0x3018, 0x32},// ; MIPI 2 lane
	{0x3020, 0x93},// ; Clock switch output normal, pclk_div =/1
	{0x3022, 0x01},// ; pd_mipi enable when rst_sync
	{0x3031, 0x0a},// ; MIPI 10-bit mode
	{0x3034, 0x00},//
	{0x3106, 0x01},// ; sclk_div, sclk_pre_div
	{0x3305, 0xf1},//
	{0x3308, 0x00},//
	{0x3309, 0x28},//
	{0x330a, 0x00},//
	{0x330b, 0x20},//
	{0x330c, 0x00},//
	{0x330d, 0x00},//
	{0x330e, 0x00},//
	{0x330f, 0x40},//
	{0x3307, 0x04},//
	{0x3500, 0x00},// ; exposure H
	{0x3501, 0x4d},// ; exposure M
	{0x3502, 0x40},// ; exposure L
	{0x3503, 0x04},// ; gain delay 1 frame, exposure delay 1 frame, sensor gain
	{0x3505, 0x80},// ; gain option
	{0x3508, 0x04},// ; gain H
	{0x3509, 0x00},// ; gain L
	{0x350c, 0x00},// ; short gain H
	{0x350d, 0x80},// ; short gain L
	{0x3510, 0x00},// ; short exposure H
	{0x3511, 0x02},// ; short exposure M
	{0x3512, 0x00},// ; short exposure L
	{0x3700, 0x18},//
	{0x3701, 0x0c},//
	{0x3702, 0x28},//
	{0x3703, 0x19},//
	{0x3704, 0x14},//
	{0x3705, 0x00},//
	{0x3706, 0x35},//
	{0x3707, 0x04},//
	{0x3708, 0x24},//
	{0x3709, 0x33},//
	{0x370a, 0x00},//
	{0x370b, 0xb5},//
	{0x370c, 0x04},//
	{0x3718, 0x12},//
	{0x3719, 0x31},//
	{0x3712, 0x42},//
	{0x3714, 0x24},//
	{0x371e, 0x19},//
	{0x371f, 0x40},//
	{0x3720, 0x05},//
	
	{0x3721, 0x05},//
	{0x3724, 0x06},//
	{0x3725, 0x01},//
	{0x3726, 0x06},//
	{0x3728, 0x05},//
	{0x3729, 0x02},//
	{0x372a, 0x03},//
	{0x372b, 0x53},//
	{0x372c, 0xa3},//
	{0x372d, 0x53},//
	{0x372e, 0x06},//
	{0x372f, 0x10},//
	{0x3730, 0x01},//
	{0x3731, 0x06},//
	{0x3732, 0x14},//
	{0x3733, 0x10},//
	{0x3734, 0x40},//
	{0x3736, 0x20},//
	{0x373a, 0x05},//
	{0x373b, 0x06},//
	{0x373c, 0x0a},//
	{0x373e, 0x03},//
	{0x3755, 0x10},//
	{0x3758, 0x00},//
	{0x3759, 0x4c},//
	{0x375a, 0x06},//
	{0x375b, 0x13},//
	{0x375c, 0x20},//
	{0x375d, 0x02},//
	{0x375e, 0x00},//
	{0x375f, 0x14},//
	{0x3768, 0x22},//
	{0x3769, 0x44},//
	{0x376a, 0x44},//
	{0x3761, 0x00},//
	{0x3762, 0x00},//
	{0x3763, 0x00},//
	{0x3766, 0xff},//
	{0x376b, 0x00},//
	{0x3772, 0x23},//
	{0x3773, 0x02},//
	{0x3774, 0x16},//
	{0x3775, 0x12},//
	{0x3776, 0x04},//
	{0x3777, 0x00},//
	{0x3778, 0x1b},//
	{0x37a0, 0x44},//
	{0x37a1, 0x3d},//
	
	{0x37a2, 0x3d},//
	{0x37a3, 0x00},//
	{0x37a4, 0x00},//
	{0x37a5, 0x00},//
	{0x37a6, 0x00},//
	{0x37a7, 0x44},//
	{0x37a8, 0x4c},//
	{0x37a9, 0x4c},//
	{0x3760, 0x00},//
	{0x376f, 0x01},//
	{0x37aa, 0x44},//
	{0x37ab, 0x2e},//
	{0x37ac, 0x2e},//
	{0x37ad, 0x33},//
	{0x37ae, 0x0d},//
	{0x37af, 0x0d},//
	{0x37b0, 0x00},//
	{0x37b1, 0x00},//
	{0x37b2, 0x00},//
	{0x37b3, 0x42},//
	{0x37b4, 0x42},//
	{0x37b5, 0x33},//
	{0x37b6, 0x00},//
	{0x37b7, 0x00},//
	{0x37b8, 0x00},//
	{0x37b9, 0xff},//
	{0x3800, 0x00},// ; x start H
	{0x3801, 0x0c},// ; x start L
	{0x3802, 0x00},// ; y start H
	{0x3803, 0x0c},// ; y start L
	{0x3804, 0x0c},// ; x end H
	{0x3805, 0xd3},// ; x end L
	{0x3806, 0x09},// ; y end H
	{0x3807, 0xa3},// ; y end L
	{0x3808, 0x06},// ; x output size H
	{0x3809, 0x60},// ; x output size L
	{0x380a, 0x04},// ; y output size H
	{0x380b, 0xc8},// ; y output size L
	{0x380c, 0x07},// ; 03 ; HTS H
	{0x380d, 0x88},// ; c4 ; HTS L
	{0x380e, 0x04},// ; VTS H
	{0x380f, 0xdc},// ; VTS L
	{0x3810, 0x00},// ; ISP x win H
	{0x3811, 0x04},// ; ISP x win L
	{0x3813, 0x02},// ; ISP y win L
	{0x3814, 0x03},// ; x odd inc
	{0x3815, 0x01},// ; x even inc
	{0x3820, 0x00},// ; vflip off
	
	{0x3821, 0x67},// ; mirror on, bin on
	{0x382a, 0x03},// ; y odd inc
	{0x382b, 0x01},// ; y even inc
	{0x3830, 0x08},//
	{0x3836, 0x02},//
	{0x3837, 0x18},//
	{0x3841, 0xff},// ; window auto size enable
	{0x3846, 0x48},//
	{0x3d85, 0x14},// ; OTP power up load data enable, OTP powerr up load setting disable
	{0x3f08, 0x08},//
	{0x3f0a, 0x80},//
	{0x4000, 0xf1},// ; out_range_trig, format_chg_trig, gain_trig, exp_chg_trig, median filter enable
	{0x4001, 0x10},// ; total 128 black column
	{0x4005, 0x10},// ; BLC target L
	{0x4002, 0x27},// ; value used to limit BLC offset
	{0x4009, 0x81},// ; final BLC offset limitation enable
	{0x400b, 0x0c},// ; DCBLC on, DCBLC manual mode on
	{0x401b, 0x00},// ; zero line R coefficient
	{0x401d, 0x00},// ; zoro line T coefficient
	{0x4020, 0x00},// ; Anchor left start H
	{0x4021, 0x04},// ; Anchor left start L
	{0x4022, 0x04},// ; Anchor left end H
	{0x4023, 0xb9},// ; Anchor left end L
	{0x4024, 0x05},// ; Anchor right start H
	{0x4025, 0x2a},// ; Anchor right start L
	{0x4026, 0x05},// ; Anchor right end H
	{0x4027, 0x2b},// ; Anchor right end L
	{0x4028, 0x00},// ; top zero line start
	{0x4029, 0x02},// ; top zero line number
	{0x402a, 0x04},// ; top black line start
	{0x402b, 0x04},// ; top black line number
	{0x402c, 0x02},// ; bottom zero line start
	{0x402d, 0x02},// ; bottom zoro line number
	{0x402e, 0x08},// ; bottom black line start
	{0x402f, 0x02},// ; bottom black line number
	{0x401f, 0x00},// ; interpolation x disable, interpolation y disable, Anchor one disable
	{0x4034, 0x3f},//
	{0x403d, 0x04},// ; md_precison_en
	{0x4300, 0xff},// ; clip max H
	{0x4301, 0x00},// ; clip min H
	{0x4302, 0x0f},// ; clip min L, clip max L
	{0x4316, 0x00},//
	{0x4500, 0x38},//
	{0x4503, 0x18},//
	{0x4600, 0x00},//
	{0x4601, 0xcb},//
	{0x481f, 0x32},// ; clk prepare min
	{0x4837, 0x16},// ; global timing
	
	{0x4850, 0x10},// ; lane 1 = 1, lane 0 = 0
	{0x4851, 0x32},// ; lane 3 = 3, lane 2 = 2
	{0x4b00, 0x2a},//
	{0x4b0d, 0x00},//
	{0x4d00, 0x04},// ; temperature sensor
	{0x4d01, 0x18},// ;
	{0x4d02, 0xc3},// ;
	{0x4d03, 0xff},// ;
	{0x4d04, 0xff},// ;
	{0x4d05, 0xff},// ; temperature sensor
	{0x5000, 0x7f},// ; slave AWB gain enable, slave AWB statistics enable, master AWB gain enable, master AWB statistics enable, BPC on, WPC on
	{0x5001, 0x01},// ; BLC on
	{0x5002, 0x08},// ; H scale off, WBMATCH select slave sensor's gain, WBMATCH off, OTP_DPC off,
	{0x5003, 0x20},// ; DPC_DBC buffer control enable, WB
	{0x5046, 0x12},//

		//DPC

	{0x5780, 0xfc},// ; add in V28 for stronger DPC control
	{0x5781, 0x13},// ; add in V28 for stronger DPC control
	{0x5782, 0x03},// ; add in V28 for stronger DPC control
	{0x5786, 0x20},// ; add in V28 for stronger DPC control
	{0x5787, 0x60},//
	{0x5788, 0x08},// ; add in V28 for stronger DPC control
	{0x5789, 0x08},// ; add in V28 for stronger DPC control
	{0x578a, 0x02},// ; add in V28 for stronger DPC control
	{0x578b, 0x01},// ; add in V28 for stronger DPC control
	{0x578c, 0x01},// ; add in V28 for stronger DPC control
	{0x578d, 0x0c},// ; add in V28 for stronger DPC control
	{0x578e, 0x02},// ; add in V28 for stronger DPC control
	{0x578f, 0x01},// ; add in V28 for stronger DPC control
	{0x5790, 0x01},// ; add in V28 for stronger DPC control

	{0x5901, 0x00},// ; H skip off, V skip off
	{0x5e00, 0x00},// ; test pattern off
	{0x5e01, 0x41},// ; window cut enable
	{0x382d, 0x7f},//
	{0x4825, 0x3a},// ; lpx_p_min
	{0x4826, 0x40},// ; hs_prepare_min
	{0x4808, 0x25},// ; wake up delay in 1/1024 s
	{0x0100, 0x01},//
};

//for capture                                                                         
static struct regval_list sensor_quxga_regs[] = {
	//@@3264_2448_2lane_2.5fps_110Mbps/lane
	//MIPI=720Mbps, SysClk=72Mhz,Dac Clock=360Mhz
	{0x0100, 0x00},//
	{0x030e, 0x02},// ; pll2_rdiv
	{0x030f, 0x04},// ; pll2_divsp
	{0x0312, 0x03},// ; pll2_pre_div0, pll2_r_divdac
	{0x3015, 0x00},//
	{0x3501, 0x9a},//
	{0x3502, 0x20},//
	{0x3508, 0x02},//
	{0x3706, 0x6a},//
	{0x370a, 0x01},//
	{0x370b, 0x6a},//
	{0x3778, 0x32},//
	{0x3800, 0x00},// ; x start H
	{0x3801, 0x0c},// ; x start L
	{0x3802, 0x00},// ; y start H
	{0x3803, 0x0c},// ; y start L
	{0x3804, 0x0c},// ; x end H
	{0x3805, 0xd3},// ; x end L
	{0x3806, 0x09},// ; y end H
	{0x3807, 0xa3},// ; y end L
	{0x3808, 0x0c},// ; x output size H
	{0x3809, 0xc0},// ; x output size L
	{0x380a, 0x09},// ; y output size H
	{0x380b, 0x90},// ; y output size L
	{0x380c, 0x07},// ; HTS H
	{0x380d, 0x94},// ; HTS L
	{0x380e, 0x09},// ; VTS H
	{0x380f, 0xaa},// ; VTS L
	
	{0x3814, 0x01},// ; x odd inc
	{0x3821, 0x46},// ; mirror on, bin off
	{0x382a, 0x01},// ; y odd inc
	{0x3830, 0x06},//
	{0x3836, 0x01},//
	{0x3f0a, 0x00},//
	{0x4001, 0x00},// ; total 256 black column
	{0x4022, 0x0b},// ; Anchor left end H
	{0x4023, 0xc3},// ; Anchor left end L
	{0x4024, 0x0c},// ; Anchor right start H
	{0x4025, 0x36},// ; Anchor right start L
	{0x4026, 0x0c},// ; Anchor right end H
	{0x4027, 0x37},// ; Anchor right end L
	{0x402b, 0x08},// ; top black line number
	{0x402e, 0x0c},// ; bottom black line start
	{0x4500, 0x58},//
	{0x4600, 0x01},//
	{0x4601, 0x97},//
	{0x382d, 0xff},//
	{0x0100, 0x01},//

};


//for video
static struct regval_list sensor_1080p_regs[] = { 
	//@@1920_1080_2Lane_5fps_120Mbps/lane
	{0x0100, 0x00},//
	{0x030e, 0x00},// ; pll2_rdiv
	{0x030f, 0x04},// ; pll2_divsp
	{0x0312, 0x01},// ; pll2_pre_div0, pll2_r_divdac
	{0x3015, 0x01},//
	{0x3501, 0x74},//
	{0x3502, 0x80},//
	{0x3508, 0x02},//
	{0x3706, 0x6a},//
	{0x370a, 0x01},//
	{0x370b, 0x6a},//
	{0x3778, 0x16},//
	{0x3800, 0x00},// ; x start H
	{0x3801, 0x0c},// ; x start L
	{0x3802, 0x00},// ; y start H
	{0x3803, 0x0c},// ; y start L
	{0x3804, 0x0c},// ; x end H
	{0x3805, 0xd3},// ; x end L
	{0x3806, 0x09},// ; y end H
	{0x3807, 0xa3},// ; y end L
	{0x3808, 0x07},// ; x output size H
	{0x3809, 0x80},// ; x output size L
	{0x380a, 0x04},// ; y output size H
	{0x380b, 0x38},// ; y output size L
	{0x380c, 0x0d},// ; HTS H
	{0x380d, 0xf2},// ; HTS L
	{0x380e, 0x05},// ; VTS H
	{0x380f, 0x40},// ; VTS L
	
	{0x3814, 0x01},// ; x odd inc
	{0x3821, 0x46},// ; mirror on, bin off
	{0x382a, 0x01},// ; y odd inc
	{0x3830, 0x06},//
	{0x3836, 0x01},//
	{0x3f0a, 0x00},//
	{0x4001, 0x00},// ; total 256 black column
	{0x4022, 0x07},// ; Anchor left end H
	{0x4023, 0x2d},// ; Anchor left end L
	{0x4024, 0x07},// ; Anchor right start H
	{0x4025, 0x9e},// ; Anchor right start L
	{0x4026, 0x07},// ; Anchor right end H
	{0x4027, 0x9f},// ; Anchor right end L
	{0x402b, 0x08},// ; top black line number
	{0x402e, 0x0c},// ; bottom black line start
	{0x4500, 0x58},//
	{0x4600, 0x00},//
	{0x4601, 0xe8},//
	{0x382d, 0xff},//
	{0x0100, 0x01},//
};

static struct regval_list sensor_sxga_regs[] = {
	//MIPI=720Mbps, SysClk=144Mhz,Dac Clock=360Mhz.
	{0x0100, 0x00},//
	{0x030e, 0x00},// ; pll2_rdiv
	{0x030f, 0x09},// ; pll2_divsp
	{0x0312, 0x01},// ; pll2_pre_div0, pll2_r_divdac
	{0x3015, 0x01},//
	{0x3501, 0x4d},// ; exposure M
	{0x3502, 0x40},// ; exposure L
	{0x3508, 0x04},// ; gain H
	{0x3706, 0x35},//
	{0x370a, 0x00},//
	{0x370b, 0xb5},//
	{0x3778, 0x1b},//
	{0x3808, 0x06},// ; x output size H
	{0x3809, 0x60},// ; x output size L
	{0x380a, 0x04},// ; y output size H
	{0x380b, 0xc8},// ; y output size L
	{0x380c, 0x07},// ; HTS H

	{0x380d, 0x88},// ; HTS L
	{0x380e, 0x04},// ; VTS H
	{0x380f, 0xdc},// ; VTS L
	{0x3814, 0x03},// ; x odd inc
	{0x3821, 0x67},// ; mirror on, bin on
	{0x382a, 0x03},// ; y odd inc
	{0x3830, 0x08},//
	{0x3836, 0x02},//
	{0x3f0a, 0x80},//
	{0x4001, 0x10},// ; total 128 black column
	{0x4022, 0x04},// ; Anchor left end H
	{0x4023, 0xb9},// ; Anchor left end L
	{0x4024, 0x05},// ; Anchor right start H
	{0x4025, 0x2a},// ; Anchor right start L
	{0x4026, 0x05},// ; Anchor right end H
	{0x4027, 0x2b},// ; Anchor right end L
	{0x402b, 0x04},// ; top black line number
	{0x402e, 0x08},// ; bottom black line start
	{0x4500, 0x38},//
	{0x4600, 0x00},//
	{0x4601, 0xcb},//
	{0x382d, 0x7f},//
	{0x0100, 0x01},//
};

static struct regval_list sensor_720p_regs[] = {
	//MIPI=720Mbps, SysClk=144Mhz,Dac Clock=360Mhz.
	{0x0100, 0x00},//
	{0x030e, 0x00},// ; pll2_rdiv
	{0x030f, 0x09},// ; pll2_divsp
	{0x0312, 0x01},// ; pll2_pre_div0, pll2_r_divdac
	{0x3015, 0x01},//
	{0x3501, 0x4d},// ; exposure M
	{0x3502, 0x40},// ; exposure L
	{0x3508, 0x04},// ; gain H
	{0x3706, 0x35},//
	{0x370a, 0x00},//
	{0x370b, 0xb5},//
	{0x3778, 0x1b},//
	{0x3808, 0x06},// ; x output size H
	{0x3809, 0x60},// ; x output size L
	{0x380a, 0x04},// ; y output size H
	{0x380b, 0xc8},// ; y output size L
	{0x380c, 0x07},// ; HTS H
	
	{0x380d, 0x88},// ; HTS L
	{0x380e, 0x04},// ; VTS H
	{0x380f, 0xdc},// ; VTS L
	{0x3814, 0x03},// ; x odd inc
	{0x3821, 0x67},// ; mirror on, bin on
	{0x382a, 0x03},// ; y odd inc
	{0x3830, 0x08},//
	{0x3836, 0x02},//
	{0x3f0a, 0x80},//
	{0x4001, 0x10},// ; total 128 black column
	{0x4022, 0x04},// ; Anchor left end H
	{0x4023, 0xb9},// ; Anchor left end L
	{0x4024, 0x05},// ; Anchor right start H
	{0x4025, 0x2a},// ; Anchor right start L
	{0x4026, 0x05},// ; Anchor right end H
	{0x4027, 0x2b},// ; Anchor right end L
	{0x402b, 0x04},// ; top black line number
	{0x402e, 0x08},// ; bottom black line start
	{0x4500, 0x38},//
	{0x4600, 0x00},//
	{0x4601, 0xcb},//
	{0x382d, 0x7f},//
	{0x0100, 0x01},//

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
  
	gain_val *= 8;

	if (gain_val < 2*16*8)
	{
		gainhigh = 0;
		gainlow = gain_val;
	}
	else if (2*16*8 <=gain_val && gain_val < 4*16*8)
	{
		gainhigh = 1;
		gainlow = gain_val/2-8;
	}
	else if (4*16*8 <= gain_val && gain_val < 8*16*8)
	{
		gainhigh = 3;
		gainlow = gain_val/4-12;
	}
	else 
	{
		gainhigh = 7;
		gainlow = gain_val/8-8;
	}
  
	exphigh	= (unsigned char) ( (0x0f0000&exp_val)>>16);
	expmid	= (unsigned char) ( (0x00ff00&exp_val)>>8);
	explow	= (unsigned char) ( (0x0000ff&exp_val)	 );
  
	shutter = exp_val/16;  
	if(shutter  > ov8858_sensor_vts- 4)
		frame_length = shutter + 4;
	else
		frame_length = ov8858_sensor_vts;
  
	sensor_write(sd, 0x3208, 0x00);//enter group write
  
	sensor_write(sd, 0x380f, (frame_length & 0xff));
	sensor_write(sd, 0x380e, (frame_length >> 8));
  
	sensor_write(sd, 0x3509, gainlow);
	sensor_write(sd, 0x3508, gainhigh);
  
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

	vfe_dev_dbg("sensor_set_exposure = %d\n", exp_val>>4);
	if(exp_val>0xfffff)
		exp_val=0xfffff;
	
	//if(info->exp == exp_val && exp_val <= (2480)*16)
	//	return 0;
  
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

	gain_val *= 8;

	if (gain_val < 2*16*8)
	{
	  gainhigh = 0;
	  gainlow = gain_val;
	}
	else if (2*16*8 <=gain_val && gain_val < 4*16*8)
	{
	  gainhigh = 1;
	  gainlow = gain_val/2-8;
	}
	else if (4*16*8 <= gain_val && gain_val < 8*16*8)
	{
	  gainhigh = 3;
	  gainlow = gain_val/4-12;
	}
	else 
	{
	  gainhigh = 7;
	  gainlow = gain_val/8-8;
	}
	
	sensor_write(sd, 0x3509, gainlow);
	sensor_write(sd, 0x3508, gainhigh);
	sensor_write(sd, 0x3208, 0x10);//end group write
	sensor_write(sd, 0x3208, 0xa0);//init group write
	
	//printk("8858 sensor_set_gain = %d, Done!\n", gain_val);
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
			vfe_set_mclk_freq(sd,MCLK);
			vfe_set_mclk(sd,ON);
			usleep_range(10000,12000);
			vfe_set_pmu_channel(sd,IOVDD,ON);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			usleep_range(10000,12000);
			vfe_set_pmu_channel(sd,AVDD,ON);
			usleep_range(10000,12000);
			vfe_set_pmu_channel(sd,DVDD,ON);
			vfe_set_pmu_channel(sd,AFVDD,ON);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);
			usleep_range(10000,12000);
			cci_unlock(sd);    
			break;
		case CSI_SUBDEV_PWR_OFF:
			vfe_dev_dbg("CSI_SUBDEV_PWR_OFF!\n");
			cci_lock(sd);   
			vfe_set_mclk(sd,OFF);
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			usleep_range(10000,12000);
			vfe_set_pmu_channel(sd,AFVDD,OFF);
			vfe_set_pmu_channel(sd,DVDD,OFF);
			usleep_range(10000,12000);
			vfe_set_pmu_channel(sd,AVDD,OFF);
			usleep_range(10000,12000);
			vfe_set_pmu_channel(sd,IOVDD,OFF);  
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
			usleep_range(10000,12000);
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
	if(rdval != 0x00)
		return -ENODEV;
  	
	LOG_ERR_RET(sensor_read(sd, 0x300b, &rdval))
	if(rdval != 0x88)
		return -ENODEV;
	  
	LOG_ERR_RET(sensor_read(sd, 0x300c, &rdval))
	if(rdval != 0x58)
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
	info->width = QUXGA_WIDTH;
	info->height = QUXGA_HEIGHT;
	info->hflip = 0;
	info->vflip = 0;
	info->gain = 0;

	info->tpf.numerator = 1;            
	info->tpf.denominator = 15;    /* 30fps */    
  
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
	  /* quxga: 3264*2448 */
	  {
      .width      = QUXGA_WIDTH,//3280,
      .height     = QUXGA_HEIGHT,//2464,
      .hoffset    = 0,
      .voffset    = 0,
      .hts        = 1940,
      .vts        = 2474,
      .pclk       = 72*1000*1000,
      .mipi_bps	  = 720*1000*1000,
      .fps_fixed  = 2,
      .bin_factor = 1,
      .intg_min   = 16,
      .intg_max   = (2474-4)<<4,
      .gain_min   = 1<<4,
      .gain_max   = 15<<4,
      .regs       = sensor_quxga_regs,
      .regs_size  = ARRAY_SIZE(sensor_quxga_regs),
      .set_size   = NULL,
    },

    /* 1080P */
    {
      .width	  = HD1080_WIDTH,
      .height 	  = HD1080_HEIGHT,
      .hoffset    = 0,
      .voffset    = 0,
	  .hts		  = 3570,
	  .vts		  = 1344,
	  .pclk 	  = 144*1000*1000,
	  .mipi_bps   = 720*1000*1000,
      .fps_fixed  = 1,
      .bin_factor = 1,
      .intg_min   = 1<<4,
      .intg_max   = (1344-4)<<4,
      .gain_min   = 1<<4,
      .gain_max   = 15<<4,
      .regs       = sensor_1080p_regs,//
      .regs_size  = ARRAY_SIZE(sensor_1080p_regs),//
      .set_size	  = NULL,
    },

  	/* SXGA */
    {
      .width	  = SXGA_WIDTH,
      .height 	  = SXGA_HEIGHT,
      .hoffset	  = 176,
      .voffset	  = 132,
      .hts        = 1928,
      .vts        = 1244,
	  .pclk 	  = 144*1000*1000,
	  .mipi_bps   = 720*1000*1000,
      .fps_fixed  = 1,
      .bin_factor = 1,
      .intg_min   = 1,
      .intg_max   = (1244-4)<<4,
      .gain_min   = 1<<4,
      .gain_max   = 15<<4,
      .regs		  = sensor_sxga_regs,
      .regs_size  = ARRAY_SIZE(sensor_sxga_regs),
      .set_size	  = NULL,
    },

    /* 720p */
    {
      .width      = HD720_WIDTH,
      .height     = HD720_HEIGHT,
      .hoffset    = 176,
      .voffset    = 252,
	  .hts		  = 1928,
	  .vts		  = 1244,
	  .pclk 	  = 144*1000*1000,
	  .mipi_bps   = 720*1000*1000,
      .fps_fixed  = 1,
      .bin_factor = 1,
      .intg_min   = 16,
      .intg_max   = (1244-4)<<4,
      .gain_min   = 1<<4,
      .gain_max   = 15<<4,
      .regs		  = sensor_720p_regs,//
      .regs_size  = ARRAY_SIZE(sensor_720p_regs),//
      .set_size	  = NULL,
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
	cfg->type = V4L2_MBUS_CSI2;
	cfg->flags = 0|V4L2_MBUS_CSI2_2_LANE|V4L2_MBUS_CSI2_CHANNEL_0;
  
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
	ov8858_sensor_vts = wsize->vts;
   
	vfe_dev_print("s_fmt = %x, width = %d, height = %d\n",sensor_fmt->mbus_code,wsize->width,wsize->height);

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

