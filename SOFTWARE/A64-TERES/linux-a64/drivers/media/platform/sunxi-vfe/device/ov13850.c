/*
 * A V4L2 driver for ov13850 cameras.
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
MODULE_DESCRIPTION("A low-level driver for ov13850 camera sensors");
MODULE_LICENSE("GPL");

//for internel driver debug
#define DEV_DBG_EN      1 
#if(DEV_DBG_EN == 1)    
#define vfe_dev_dbg(x,arg...) printk("[ov13850]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...) 
#endif
#define vfe_dev_err(x,arg...) printk("[ov13850]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[ov13850]"x,##arg)

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
#define V4L2_IDENT_SENSOR 0x13850
int ov13850_sensor_vts;




/*
 * Our nominal (default) frame rate.
 */

#define SENSOR_FRAME_RATE 30


/*
 * The ov13850 sits on i2c with ID 0x6c or 0x20
 */
//#define I2C_ADDR 0x6c
#define SENSOR_NAME "ov13850"
//static struct delayed_work sensor_s_ae_ratio_work;
static struct v4l2_subdev *glb_sd;
//#define 4k_video

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
};

//for capture                                                                         
static struct regval_list sensor_13mega_regs[] = {
 // ;XVCLK=24Mhz, SCLK=4x120Mhz, MIPI 640Mbps, DACCLK=240Mhz
	{0x0103,0x01}, // ; software reset

	{0x0300,0x01}, //; PLL
	{0x0301,0x00}, //; PLL
	{0x0302,0x28}, //; PLL
	{0x0303,0x00}, // ; PLL
	{0x030a,0x00}, // ; PLL
	//{//0xffff, 50},
	{0x300f,0x11}, // ; MIPI 10-bit mode
	{0x3010,0x01}, // ; MIPI PHY
	{0x3011,0x76}, // ; MIPI PHY
	{0x3012,0x41}, // ; MIPI 4 lane
	{0x3013,0x12}, // ; MIPI control
	{0x3014,0x11}, // ; MIPI control
	{0x301f,0x03}, //
	{0x3106,0x00}, //
	{0x3210,0x47}, //
	{0x3500,0x00}, // ; exposure HH
	{0x3501,0x67}, // ; exposure H
	{0x3502,0x80}, // ; exposure L
	{0x3506,0x00}, // ; short exposure HH
	{0x3507,0x02}, // ; short exposure H
	{0x3508,0x00}, // ; shour exposure L
	{0x3509,0x10},//00},//8},
	{0x350a,0x00}, // ; gain H
	{0x350b,0x10}, // ; gain L
	{0x350e,0x00}, // ; short gain H
	{0x350f,0x10}, // ; short gain L
	{0x3600,0x40}, // ; analog control
	{0x3601,0xfc}, // ; analog control
	{0x3602,0x02}, // ; analog control
	{0x3603,0x48}, // ; analog control
	{0x3604,0xa5}, // ; analog control
	{0x3605,0x9f}, // ; analog control
	{0x3607,0x00}, // ; analog control
	{0x360a,0x40}, // ; analog control
	{0x360b,0x91}, // ; analog control
	{0x360c,0x49}, // ; analog control
	{0x360f,0x8a}, //
	{0x3611,0x10}, // ; PLL2
	//{0x3612,0x23}, // ; PLL2	
	{0x3612,0x13}, // ; PLL2	
	//{0x3613,0x33}, // ; PLL2
	{0x3613,0x22}, // ; PLL2
	{0x3615,0x08}, //
	{0x3641,0x02}, 
	{0x3660,0x82}, 
	{0x3668,0x54}, 
	{0x3669,0x40}, 
	{0x3667,0xa0}, 
	{0x3702,0x40}, 
	{0x3703,0x44}, 
	{0x3704,0x2c}, 
	{0x3705,0x24}, 
	{0x3706,0x50}, 
	{0x3707,0x44}, 
	{0x3708,0x3c}, 
	{0x3709,0x1f}, 
	{0x370a,0x26}, 
	{0x370b,0x3c}, 
	{0x3720,0x66}, 
	{0x3722,0x84}, 
	{0x3728,0x40}, 
	{0x372a,0x00}, 
	{0x372f,0x90}, 
	{0x3710,0x28}, 
	{0x3716,0x03}, 
	{0x3718,0x10}, 
	{0x3719,0x08}, 
	{0x371c,0xfc}, 
	{0x3760,0x13}, 
	{0x3761,0x34}, 
	{0x3767,0x24}, 
	{0x3768,0x06}, 
	{0x3769,0x45}, 
	{0x376c,0x23}, 
	{0x3d84,0x00}, // ; OTP program disable
	{0x3d85,0x17}, // ; OTP power up load data enable, power load setting enable, software load setting 
	{0x3d8c,0x73}, // ; OTP start address H
	{0x3d8d,0xbf}, // ; OTP start address L
	{0x3800,0x00}, // ; H crop start H
	{0x3801,0x08}, // ; H crop start L
	{0x3802,0x00}, // ; V crop start H
	{0x3803,0x04}, // ; V crop start L
	{0x3804,0x10}, // ; H crop end H
	{0x3805,0x97}, // ; H crop end L
	{0x3806,0x0c}, // ; V crop end H
	{0x3807,0x4b}, // ; V crop end L
	{0x3808,0x08}, // ; H output size H
	{0x3809,0x40}, // ; H output size L
			  
	{0x380a,0x06}, // ; V output size H
	{0x380b,0x20}, // ; V output size L
	{0x380c,0x25}, // ; HTS H
	{0x380d,0x80}, // ; HTS L
	{0x380e,0x06}, // ; VTS H
	{0x380f,0x80}, // ; VTS L
	{0x3810,0x00}, // ; H win off H
	{0x3811,0x04}, // ; H win off L
	{0x3812,0x00}, // ; V win off H
	{0x3813,0x02}, // ; V win off L
	{0x3814,0x31}, // ; H inc
	{0x3815,0x31}, // ; V inc
	{0x3820,0x02}, // ; V flip off, V bin on
	{0x3821,0x05}, // ; H mirror on, H bin on
	{0x3834,0x00}, //
	{0x3835,0x1c}, // ; cut_en, vts_auto, blk_col_dis
	{0x3836,0x08}, //
	{0x3837,0x02}, //
	{0x4000,0xf1},//c1}, // ; BLC offset trig en, format change trig en, gain trig en, exp trig en, median en
	{0x4001,0x00}, // ; BLC
	{0x400b,0x0c}, // ; BLC
	{0x4011,0x00}, // ; BLC
	{0x401a,0x00}, // ; BLC
	{0x401b,0x00}, // ; BLC
	{0x401c,0x00}, // ; BLC
	{0x401d,0x00}, // ; BLC
	{0x4020,0x00}, // ; BLC
	{0x4021,0xe4}, // ; BLC
	{0x4022,0x07}, // ; BLC
	{0x4023,0x5f}, // ; BLC
	{0x4024,0x08}, // ; BLC
	{0x4025,0x44}, // ; BLC
	{0x4026,0x08}, // ; BLC
	{0x4027,0x47}, // ; BLC
	{0x4028,0x00}, // ; BLC
	{0x4029,0x02}, // ; BLC
	{0x402a,0x04}, // ; BLC
	{0x402b,0x08}, // ; BLC
	{0x402c,0x02}, // ; BLC
	{0x402d,0x02}, // ; BLC
	{0x402e,0x0c}, // ; BLC
	{0x402f,0x08}, // ; BLC
	{0x403d,0x2c}, //
	{0x403f,0x7f}, // 
	{0x4500,0x82}, // ; BLC
	{0x4501,0x38}, // ; BLC
	{0x4601,0x04}, //
	{0x4602,0x22}, //
			   
	{0x4603,0x01}, //; VFIFO
	{0x4837,0x19}, //; MIPI global timing

	{0x4d00,0x04}, // ; temperature monitor
	{0x4d01,0x42}, //	; temperature monitor
	{0x4d02,0xd1}, //	; temperature monitor
	{0x4d03,0x90}, //	; temperature monitor
	{0x4d04,0x66}, //	; temperature monitor
	{0x4d05,0x65}, // ; temperature monitor
	{0x5000,0x0e}, // ; windowing enable, BPC on, WPC on, Lenc on
	{0x5001,0x03}, // ; BLC enable, MWB on
	{0x5002,0x07}, //
	{0x5013,0x40}, 
	{0x501c,0x00}, 
	{0x501d,0x10}, 
 	//{0x5057,0x56},//add
	{0x5242,0x00}, 
	{0x5243,0xb8}, 
	{0x5244,0x00}, 
	{0x5245,0xf9}, 
	{0x5246,0x00}, 
	{0x5247,0xf6}, 
	{0x5248,0x00}, 
	{0x5249,0xa6}, 
	{0x5300,0xfc}, 
	{0x5301,0xdf}, 
	{0x5302,0x3f}, 
	{0x5303,0x08}, 
	{0x5304,0x0c}, 
	{0x5305,0x10}, 
	{0x5306,0x20}, 
	{0x5307,0x40}, 
	{0x5308,0x08}, 
	{0x5309,0x08}, 
	{0x530a,0x02}, 
	{0x530b,0x01}, 
	{0x530c,0x01}, 
	{0x530d,0x0c}, 
	{0x530e,0x02}, 
	{0x530f,0x01}, 
	{0x5310,0x01}, 
	{0x5400,0x00}, 
	{0x5401,0x61}, 
	{0x5402,0x00}, 
	{0x5403,0x00}, 
	{0x5404,0x00}, 
	{0x5405,0x40}, 
	{0x540c,0x05}, 
	{0x5b00,0x00}, 
	{0x5b01,0x00}, 

	{0x5b02,0x01}, 
	{0x5b03,0xff}, 
	{0x5b04,0x02}, 
	{0x5b05,0x6c}, 
	{0x5b09,0x02}, //
	{0x5e00,0x00}, // ; test pattern disable
	{0x5e10,0x1c}, // ; ISP test disable

	/*above  for init*/
	{0x0300,0x01},// ; PLL
	{0x0302,0x38},// ; PLL
	{0xffff, 50},

	{0x3501,0xcf},// ; Exposure H
	{0x370a,0x24},// 
	{0x372a,0x04},// 
	{0x372f,0xa0},// 
	{0x3801,0x14},// ; H crop start L
	{0x3803,0x0c},// ; V crop start L
	{0x3805,0x8b},// ; H crop end L
	{0x3807,0x43},// ; V crop end L
	{0x3808,0x10}, //; H output size H
	{0x3809,0x70}, //; H output size L
	{0x380a,0x0c}, //; V output size H
	{0x380b,0x30}, //; V output size L
	{0x380c,0x25}, //; HTS H
	{0x380d,0x80}, //; HTS L
	{0x380e,0x0d},//6}, //; VTS H
	{0x380f,0x00},//80}, //; VTS L
	{0x3813,0x04}, //; V win off
	{0x3814,0x11}, //; H inc
	{0x3815,0x11}, //; V inc
	{0x3820,0x00}, //; V flip off, V bin off
	{0x3821,0x04}, //; H mirror on, H bin off
	{0x3836,0x04},
	{0x3837,0x01},
	{0x4020,0x02}, 
	{0x4021,0x3c},
	{0x4022,0x0e},
	{0x4023,0x37},
	{0x4024,0x0f},
	{0x4025,0x1c},
	{0x4026,0x0f},
	{0x4027,0x1f},
	{0x4603,0x01},// ; VFIFO
	{0x4837,0x19},// ; MIPI global timing
	{0x4826,0x12},	 //default 0x32  trail
	{0x5401,0x71},// 
	{0x5405,0x80},// 
	{0x0100,0x01},// ; wake up, streaming
};

//for video
static struct regval_list sensor_4k_videos[]=
{
	{0x0103,0x01}, // ; software reset

	{0x030a,0x00}, // ; PLL
	//{//0xffff, 50},
	{0x300f,0x11}, // ; MIPI 10-bit mode
	{0x3010,0x01}, // ; MIPI PHY
	{0x3011,0x76}, // ; MIPI PHY
	{0x3012,0x41}, // ; MIPI 4 lane
	{0x3013,0x12}, // ; MIPI control
	{0x3014,0x11}, // ; MIPI control
	{0x301f,0x03}, //
	{0x3106,0x00}, //
	{0x3210,0x47}, //
	{0x3500,0x00}, // ; exposure HH
	{0x3501,0x67}, // ; exposure H
	{0x3502,0x80}, // ; exposure L
	{0x3506,0x00}, // ; short exposure HH
	{0x3507,0x02}, // ; short exposure H
	{0x3508,0x00}, // ; shour exposure L
	{0x3509,0x10},//00},//8},
	{0x350a,0x00}, // ; gain H
	{0x350b,0x10}, // ; gain L
	{0x350e,0x00}, // ; short gain H
	{0x350f,0x10}, // ; short gain L
	{0x3600,0x40}, // ; analog control
	{0x3601,0xfc}, // ; analog control
	{0x3602,0x02}, // ; analog control
	{0x3603,0x48}, // ; analog control
	{0x3604,0xa5}, // ; analog control
	{0x3605,0x9f}, // ; analog control
	{0x3607,0x00}, // ; analog control
	{0x360a,0x40}, // ; analog control
	{0x360b,0x91}, // ; analog control
	{0x360c,0x49}, // ; analog control
	{0x360f,0x8a}, //
	{0x3611,0x10}, // ; PLL2
	//{0x3612,0x13}, // ; PLL2			   
	{0x3613,0x11}, // ; PLL2
	{0xffff, 50},
	{0x3615,0x08}, //
	{0x3641,0x02}, 
	{0x3660,0x82}, 
	{0x3668,0x54}, 
	{0x3669,0x40}, 
	{0x3667,0xa0}, 
	{0x3702,0x40}, 
	{0x3703,0x44}, 
	{0x3704,0x2c}, 
	{0x3705,0x24}, 
	{0x3706,0x50}, 
	{0x3707,0x44}, 
	{0x3708,0x3c}, 
	{0x3709,0x1f}, 
	{0x370a,0x26}, 
	{0x370b,0x3c}, 
	{0x3720,0x66}, 
	{0x3722,0x84}, 
	{0x3728,0x40}, 
	{0x372a,0x00}, 
	{0x372f,0x90}, 
	{0x3710,0x28}, 
	{0x3716,0x03}, 
	{0x3718,0x10}, 
	{0x3719,0x08}, 
	{0x371c,0xfc}, 
	{0x3760,0x13}, 
	{0x3761,0x34}, 
	{0x3767,0x24}, 
	{0x3768,0x06}, 
	{0x3769,0x45}, 
	{0x376c,0x23}, 
	{0x3d84,0x00}, // ; OTP program disable
	{0x3d85,0x17}, // ; OTP power up load data enable, power load setting enable, software load setting 
				  
	{0x3d8c,0x73}, // ; OTP start address H
	{0x3d8d,0xbf}, // ; OTP start address L
	{0x3800,0x00}, // ; H crop start H
	{0x3801,0x08}, // ; H crop start L
	{0x3802,0x00}, // ; V crop start H
	{0x3803,0x04}, // ; V crop start L
	{0x3804,0x10}, // ; H crop end H
	{0x3805,0x97}, // ; H crop end L
	{0x3806,0x0c}, // ; V crop end H
	{0x3807,0x4b}, // ; V crop end L
	{0x3808,0x08}, // ; H output size H
	{0x3809,0x40}, // ; H output size L
              
	{0x380a,0x06}, // ; V output size H
	{0x380b,0x20}, // ; V output size L
	{0x380c,0x25}, // ; HTS H
	{0x380d,0x80}, // ; HTS L
	{0x380e,0x06}, // ; VTS H
	{0x380f,0x80}, // ; VTS L
	{0x3810,0x00}, // ; H win off H
	{0x3811,0x04}, // ; H win off L
	{0x3812,0x00}, // ; V win off H
	{0x3813,0x02}, // ; V win off L
	{0x3814,0x31}, // ; H inc
	{0x3815,0x31}, // ; V inc
	{0x3820,0x02}, // ; V flip off, V bin on
	{0x3821,0x05}, // ; H mirror on, H bin on
	{0x3834,0x00}, //
	{0x3835,0x1c}, // ; cut_en, vts_auto, blk_col_dis
	{0x3836,0x08}, //
	{0x3837,0x02}, //
	{0x4000,0xf1},//c1}, // ; BLC offset trig en, format change trig en, gain trig en, exp trig en, median en
	{0x4001,0x00}, // ; BLC
	{0x400b,0x0c}, // ; BLC
	{0x4011,0x00}, // ; BLC
	{0x401a,0x00}, // ; BLC
	{0x401b,0x00}, // ; BLC
	{0x401c,0x00}, // ; BLC
	{0x401d,0x00}, // ; BLC
	{0x4020,0x00}, // ; BLC
	{0x4021,0xe4}, // ; BLC
	{0x4022,0x07}, // ; BLC
	{0x4023,0x5f}, // ; BLC
	{0x4024,0x08}, // ; BLC
	{0x4025,0x44}, // ; BLC
	{0x4026,0x08}, // ; BLC
	{0x4027,0x47}, // ; BLC
	{0x4028,0x00}, // ; BLC
	{0x4029,0x02}, // ; BLC
	{0x402a,0x04}, // ; BLC
	{0x402b,0x08}, // ; BLC
	{0x402c,0x02}, // ; BLC
	{0x402d,0x02}, // ; BLC
	{0x402e,0x0c}, // ; BLC
	{0x402f,0x08}, // ; BLC
	{0x403d,0x2c}, //
	{0x403f,0x7f}, // 
	{0x4500,0x82}, // ; BLC
	{0x4501,0x38}, // ; BLC
	{0x4601,0x04}, //
	{0x4602,0x22}, //

	{0x4603,0x01}, //; VFIFO
	{0x4837,0x19}, //; MIPI global timing
	{0x4800,0x04},
	{0x4802,0x42},	//default 0x00
	{0x481a,0x00},
	{0x481b,0x1c},	 //default 0x3c  prepare
	{0x4826,0x12},	 //default 0x32  trail

	{0x4d00,0x04}, // ; temperature monitor
	{0x4d01,0x42}, //  ; temperature monitor
	{0x4d02,0xd1}, //  ; temperature monitor
	{0x4d03,0x90}, //  ; temperature monitor
	{0x4d04,0x66}, //  ; temperature monitor
	{0x4d05,0x65}, // ; temperature monitor
	{0x5000,0x0e}, // ; windowing enable, BPC on, WPC on, Lenc on
	{0x5001,0x03}, // ; BLC enable, MWB on
	{0x5002,0x07}, //
	{0x5013,0x40}, 
	{0x501c,0x00}, 
	{0x501d,0x10}, 
	{0x5242,0x00}, 
	{0x5243,0xb8}, 
	{0x5244,0x00}, 
	{0x5245,0xf9}, 
	{0x5246,0x00}, 
	{0x5247,0xf6}, 
	{0x5248,0x00}, 
	{0x5249,0xa6}, 
	{0x5300,0xfc}, 
	{0x5301,0xdf}, 
	{0x5302,0x3f}, 
	{0x5303,0x08}, 
	{0x5304,0x0c}, 
	{0x5305,0x10}, 
	{0x5306,0x20}, 
	{0x5307,0x40}, 
	{0x5308,0x08}, 
	{0x5309,0x08}, 
	{0x530a,0x02}, 
	{0x530b,0x01}, 
	{0x530c,0x01}, 
	{0x530d,0x0c}, 
	{0x530e,0x02}, 
	{0x530f,0x01}, 
	{0x5310,0x01}, 
	{0x5400,0x00}, 
	{0x5401,0x61}, 
	{0x5402,0x00}, 
	{0x5403,0x00}, 
	{0x5404,0x00}, 
	{0x5405,0x40}, 
	{0x540c,0x05}, 
	{0x5b00,0x00}, 
	{0x5b01,0x00}, 

	{0x5b02,0x01}, 
	{0x5b03,0xff}, 
	{0x5b04,0x02}, 
	{0x5b05,0x6c}, 
	{0x5b09,0x02}, //
	{0x5e00,0x00}, // ; test pattern disable
	{0x5e10,0x1c}, // ; ISP test disable
			  
	{0x3813,0x04}, //; V win off
	{0x3814,0x11}, //; H inc
	{0x3815,0x11}, //; V inc
	{0x3820,0x00}, //; V flip off, V bin off
	{0x3821,0x04}, //; H mirror on, H bin off
	{0x3836,0x04},
	{0x3837,0x01},

	{0x4837,0x0a},// ; MIPI global timing
	{0x4826,0x12},	 //default 0x32  trail
	{0x5401,0x71},// 
	{0x5405,0x80},// 

	{0x3612,0x07},

	{0x0300,0x00},
	{0x0301,0x00},
	{0x0302,0x20},//8},//28 OK!},//1e},

	{0x0303,0x00},
	{0x4837,0x0d},//8},//13},

	{0x370a,0x24},// 
	{0x372a,0x04},// 
	{0x372f,0xa0},// 
	{0x3800,0x01},//0}, // ; H crop start H
	{0x3801,0x4c},//CC}, // ; H crop start L
	{0x3802,0x02},//1}, // ; V crop start H
	{0x3803,0x8C}, // ; V crop start L
	{0x3804,0x10},//0F}, // ; H crop end H
	{0x3805,0x53},//D3}, // ; H crop end L
	{0x3806,0x0B},//A}, // ; V crop end H
	{0x3807,0x03}, // ; V crop end L
	{0x3808,0x0F}, // ; H output size H
	{0x3809,0x0 }, // ; H output size L
	{0x380A,0x08},
	{0x380B,0x70},
	{0x380C,0x1a},
	{0x380D,0x90},
	{0x380E,0x0b},
	{0x380F,0xb0},

	{0x3810,0x00},
	{0x3811,0x04},
	{0x3812,0x00},
	{0x3813,0x04},
	{0x3836,0x04},
	{0x3837,0x01},
	{0x4020,0x0 },
	{0x4021,0xE6},
	{0x4022,0xE },
	{0x4023,0x1E},
	{0x4024,0xF },
	{0x4025,0x0 },
	{0x4026,0xF },
	{0x4027,0x6 },

	{0x0100,0x01},

};

static struct regval_list sensor_1080p_regs[] = { 
//;XVCLK=24Mhz, SCLK=4x120Mhz, MIPI 640Mbps, DACCLK=240Mhz
/*
* using quarter size to scale down
*/
	{0x0103,0x01}, // ; software reset
	
	{0x0300,0x01}, //; PLL
	{0x0301,0x00}, //; PLL
	{0x0302,0x28}, //; PLL
	{0x0303,0x00}, // ; PLL
	{0x030a,0x00}, // ; PLL
	//{0xffff, 20},
	{0x300f,0x11}, // ; MIPI 10-bit mode
	{0x3010,0x01}, // ; MIPI PHY
	{0x3011,0x76}, // ; MIPI PHY
	{0x3012,0x41}, // ; MIPI 4 lane
	{0x3013,0x12}, // ; MIPI control
	{0x3014,0x11}, // ; MIPI control
	{0x301f,0x03}, //
	{0x3106,0x00}, //
	{0x3210,0x47}, //
	{0x3500,0x00}, // ; exposure HH
	{0x3501,0x67}, // ; exposure H
	{0x3502,0x80}, // ; exposure L
	{0x3506,0x00}, // ; short exposure HH
	{0x3507,0x02}, // ; short exposure H
	{0x3508,0x00}, // ; shour exposure L
	{0x3509,0x10},//00},//8},
	{0x350a,0x00}, // ; gain H
	{0x350b,0x10}, // ; gain L
	{0x350e,0x00}, // ; short gain H
	{0x350f,0x10}, // ; short gain L
	{0x3600,0x40}, // ; analog control
	{0x3601,0xfc}, // ; analog control
	{0x3602,0x02}, // ; analog control
	{0x3603,0x48}, // ; analog control
	{0x3604,0xa5}, // ; analog control
	{0x3605,0x9f}, // ; analog control
	{0x3607,0x00}, // ; analog control
	{0x360a,0x40}, // ; analog control
	{0x360b,0x91}, // ; analog control
	{0x360c,0x49}, // ; analog control
	{0x360f,0x8a}, //
	{0x3611,0x10}, // ; PLL2
	//{0x3612,0x23}, // ; PLL2
	{0x3612,0x13}, // ; PLL2   
	//{0x3613,0x33}, // ; PLL2
	{0x3613,0x22}, // ; PLL2
	//{0xffff, 50},
	{0x3615,0x08}, //
	{0x3641,0x02}, 
	{0x3660,0x82}, 
	{0x3668,0x54}, 
	{0x3669,0x40}, 
	{0x3667,0xa0}, 
	{0x3702,0x40}, 
	{0x3703,0x44}, 
	{0x3704,0x2c}, 
	{0x3705,0x24}, 
	{0x3706,0x50}, 
	{0x3707,0x44}, 
	{0x3708,0x3c}, 
	{0x3709,0x1f}, 
	{0x370a,0x26}, 
	{0x370b,0x3c}, 
	{0x3720,0x66}, 
	{0x3722,0x84}, 
	{0x3728,0x40}, 
	{0x372a,0x00}, 
	{0x372f,0x90}, 
	{0x3710,0x28}, 
	{0x3716,0x03}, 
	{0x3718,0x10}, 
	{0x3719,0x08}, 
	{0x371c,0xfc}, 
	{0x3760,0x13}, 
	{0x3761,0x34}, 
	{0x3767,0x24}, 
	{0x3768,0x06}, 
	{0x3769,0x45}, 
	{0x376c,0x23}, 
	{0x3d84,0x00}, // ; OTP program disable
	{0x3d85,0x17}, // ; OTP power up load data enable, power load setting enable, software load setting 
	{0x3d8c,0x73}, // ; OTP start address H
	{0x3d8d,0xbf}, // ; OTP start address L
	{0x3800,0x00}, // ; H crop start H
	{0x3801,0x08}, // ; H crop start L
	{0x3802,0x00}, // ; V crop start H
	{0x3803,0x04}, // ; V crop start L
	{0x3804,0x10}, // ; H crop end H
	{0x3805,0x97}, // ; H crop end L
	{0x3806,0x0c}, // ; V crop end H
	{0x3807,0x4b}, // ; V crop end L
	{0x3808,0x08}, // ; H output size H
	{0x3809,0x40}, // ; H output size L
				 
	{0x380a,0x06}, // ; V output size H
	{0x380b,0x20}, // ; V output size L
	{0x380c,0x25}, // ; HTS H
	{0x380d,0x80}, // ; HTS L
	{0x380e,0x06}, // ; VTS H
	{0x380f,0x80}, // ; VTS L
	{0x3810,0x00}, // ; H win off H
	{0x3811,0x04}, // ; H win off L
	{0x3812,0x00}, // ; V win off H
	{0x3813,0x02}, // ; V win off L
	{0x3814,0x31}, // ; H inc
	{0x3815,0x31}, // ; V inc
	{0x3820,0x02}, // ; V flip off, V bin on
	{0x3821,0x05}, // ; H mirror on, H bin on
	{0x3834,0x00}, //
	{0x3835,0x1c}, // ; cut_en, vts_auto, blk_col_dis
	{0x3836,0x08}, //
	{0x3837,0x02}, //
	{0x4000,0xf1},//c1}, // ; BLC offset trig en, format change trig en, gain trig en, exp trig en, median en
	{0x4001,0x00}, // ; BLC
	{0x400b,0x0c}, // ; BLC
	{0x4011,0x00}, // ; BLC
	{0x401a,0x00}, // ; BLC
	{0x401b,0x00}, // ; BLC
	{0x401c,0x00}, // ; BLC
	{0x401d,0x00}, // ; BLC
	{0x4020,0x00}, // ; BLC
	{0x4021,0xe4}, // ; BLC
	{0x4022,0x07}, // ; BLC
	{0x4023,0x5f}, // ; BLC
	{0x4024,0x08}, // ; BLC
	{0x4025,0x44}, // ; BLC
	{0x4026,0x08}, // ; BLC
	{0x4027,0x47}, // ; BLC
	{0x4028,0x00}, // ; BLC
	{0x4029,0x02}, // ; BLC
	{0x402a,0x04}, // ; BLC
	{0x402b,0x08}, // ; BLC
	{0x402c,0x02}, // ; BLC
	{0x402d,0x02}, // ; BLC
	{0x402e,0x0c}, // ; BLC
	{0x402f,0x08}, // ; BLC
	{0x403d,0x2c}, //
	{0x403f,0x7f}, // 
	{0x4500,0x82}, // ; BLC
	{0x4501,0x38}, // ; BLC
	{0x4601,0x04}, //
	{0x4602,0x22}, //
				  
	{0x4603,0x01}, //; VFIFO
	{0x4837,0x19}, //; MIPI global timing
	
	{0x4d00,0x04}, // ; temperature monitor
	{0x4d01,0x42}, //  ; temperature monitor
	{0x4d02,0xd1}, //  ; temperature monitor
	{0x4d03,0x90}, //  ; temperature monitor
	{0x4d04,0x66}, //  ; temperature monitor
	{0x4d05,0x65}, // ; temperature monitor
	{0x5000,0x0e}, // ; windowing enable, BPC on, WPC on, Lenc on
	{0x5001,0x03}, // ; BLC enable, MWB on
	{0x5002,0x07}, //
	{0x5013,0x40}, 
	{0x501c,0x00}, 
	{0x501d,0x10}, 
	//{0x5057,0x56},//add
	{0x5056,0x08},
	{0x5058,0x08},
	{0x505a,0x08},
	{0x5242,0x00}, 
	{0x5243,0xb8}, 
	{0x5244,0x00}, 
	{0x5245,0xf9}, 
	{0x5246,0x00}, 
	{0x5247,0xf6}, 
	{0x5248,0x00}, 
	{0x5249,0xa6}, 
	{0x5300,0xfc}, 
	{0x5301,0xdf}, 
	{0x5302,0x3f}, 
	{0x5303,0x08}, 
	{0x5304,0x0c}, 
	{0x5305,0x10}, 
	{0x5306,0x20}, 
	{0x5307,0x40}, 
	{0x5308,0x08}, 
	{0x5309,0x08}, 
	{0x530a,0x02}, 
	{0x530b,0x01}, 
	{0x530c,0x01}, 
	{0x530d,0x0c}, 
	{0x530e,0x02}, 
	{0x530f,0x01}, 
	{0x5310,0x01}, 
	{0x5400,0x00}, 
	{0x5401,0x61}, 
	{0x5402,0x00}, 
	{0x5403,0x00}, 
	{0x5404,0x00}, 
	{0x5405,0x40}, 
	{0x540c,0x05}, 
	{0x5b00,0x00}, 
	{0x5b01,0x00}, 
	
	{0x5b02,0x01}, 
	{0x5b03,0xff}, 
	{0x5b04,0x02}, 
	{0x5b05,0x6c}, 
	{0x5b09,0x02}, //
	{0x5e00,0x00}, // ; test pattern disable
	{0x5e10,0x1c}, // ; ISP test disable

	{0x0300,0x01},// ; PLL
	{0x0302,0x28},// ; PLL
	{0xffff, 50},
	{0x3501,0x67},// ; Exposure H
	{0x370a,0x26},// 
	{0x372a,0x00}, 
	{0x372f,0x90}, 
	{0x3801,0x08}, //; H crop start L
	{0x3803,0x04}, //; V crop start L
	{0x3805,0x97}, //; H crop end L
	{0x3807,0x4b}, //; V crop end L
	{0x3808,0x08}, //; H output size H
	{0x3809,0x40}, //; H output size L
	{0x380a,0x06}, //; V output size H
	{0x380b,0x20}, //; V output size L
	{0x380c,0x25}, //; HTS H
	{0x380d,0x80}, //; HTS L
	{0x380e,0x0a},//6}, //; VTS H
	{0x380f,0x80}, //; VTS L
	{0x3813,0x02}, //; V win off
	{0x3814,0x31}, //; H inc
	{0x3815,0x31}, //; V inc
	{0x3820,0x02}, //; V flip off, V bin on
	{0x3821,0x05}, //; H mirror on, H bin on
	{0x3836,0x08}, //
	{0x3837,0x02}, //
	{0x4020,0x00}, //
	{0x4021,0xe4}, //
	{0x4022,0x07}, //
	{0x4023,0x5f}, //
	{0x4024,0x08}, //
	{0x4025,0x44}, //
	{0x4026,0x08}, //
	{0x4027,0x47}, //
	{0x4603,0x01}, //; VFIFO
	{0x4837,0x19}, //; MIPI global timing
	{0x4802,0x42},	//default 0x00
	{0x481a,0x00},
	{0x481b,0x1c},	 //default 0x3c  prepare
	{0x4826,0x12},	 //default 0x32  trail

	{0x5401,0x61}, //
	{0x5405,0x40}, //
	{0x0100,0x01}, //; wake up, streaming

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
	int exp_val, gain_val,frame_length,shutter;
	unsigned char explow=0,expmid=0,exphigh=0;
	unsigned short gainlow=0,gainhigh=0;  
	struct sensor_info *info = to_state(sd);

	exp_val = exp_gain->exp_val;
	gain_val = exp_gain->gain_val;
 
//	printk("exp_val = %d,gain_val = %d %x%x\n",exp_val,gain_val, tmp3, tmp2);

	if(exp_val>0xfffff)
	 exp_val=0xfffff;
	//gain_val =256;
	if(gain_val<1*16)
	 gain_val=16;

	gainlow=(unsigned char)(gain_val&0xff);
	gainhigh=(unsigned char)((gain_val>>8)&0x3);
 
	exp_val >>=4;
	exphigh = (unsigned char) ( (0xffff&exp_val)>>12);  
	expmid  = (unsigned char) ( (0xfff&exp_val)>>4);  
	explow  = (unsigned char) ( (0x0f&exp_val)<<4   );
	
	shutter = exp_val;
	if(shutter	> ov13850_sensor_vts - 6)
		frame_length = shutter + 6;
	else
		frame_length = ov13850_sensor_vts;

	sensor_write(sd, 0x3208, 0x00);//enter group write

	sensor_write(sd, 0x380f, (frame_length & 0xff));
	sensor_write(sd, 0x380e, (frame_length >> 8));

	sensor_write(sd, 0x3502, explow);
	sensor_write(sd, 0x3501, expmid);
	sensor_write(sd, 0x3500, exphigh);
	
	sensor_write(sd, 0x350b, gainlow);
	sensor_write(sd, 0x350a, gainhigh);

	sensor_write(sd, 0x3208, 0x10);//end group write
	sensor_write(sd, 0x3208, 0xa0);//init group write
	
	info->gain = gain_val;
	info->exp = exp_val;
	
	return 0;
}

static int sensor_s_exp(struct v4l2_subdev *sd, unsigned int exp_val)
{
	unsigned char explow,expmid,exphigh;
	struct sensor_info *info = to_state(sd);
	if(exp_val>0xfffff)
		exp_val=0xfffff;
	
	exp_val >>=4;
    exphigh = (unsigned char) ( (0xffff&exp_val)>>12);  
	expmid  = (unsigned char) ( (0xfff&exp_val)>>4);  
	explow  = (unsigned char) ( (0x0f&exp_val)<<4   );
	
	sensor_write(sd, 0x3208, 0x00);//enter group write

	sensor_write(sd, 0x3502, explow);
	sensor_write(sd, 0x3501, expmid);
	sensor_write(sd, 0x3500, exphigh);
	
//	printk("ov13850 sensor_set_exp = %d\n", tmp);
	
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

	gainlow=(unsigned char)(gain_val&0xff);
	gainhigh=(unsigned char)((gain_val>>8)&0x3);

	sensor_write(sd, 0x350b, gainlow);
	sensor_write(sd, 0x350a, gainhigh);
	sensor_write(sd, 0x3208, 0x10);//end group write
	sensor_write(sd, 0x3208, 0xa0);//init group write
	
	//printk("ov13850 sensor_set_gain = %d, Done!\n", gain_val);
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
	if(rdval != 0xD8)
		return -ENODEV;
 	
	LOG_ERR_RET(sensor_read(sd, 0x300b, &rdval))
	if(rdval != 0x50)
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
//	info->width = 4608;
//	info->height = 3456;
	info->hflip = 0;
	info->vflip = 0;
	info->gain = 0;

	info->tpf.numerator = 1;            
	info->tpf.denominator = 15;    /* 30fps */    
 
	ret = sensor_write_array(sd,sensor_default_regs, 0/*ARRAY_SIZE(sensor_default_regs)*/);  
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
#if 1
	/* Fullsize: 4208*3120 */
	{
		.width      = 4208,
		.height     = 3120,
		.hoffset    = 0,
		.voffset    = 0,
		.hts        = 9600/16,
		.vts        = 3328-6,
		.pclk       = (637*1000*1000+800*1000)/16,
		.mipi_bps	 = 850*1000*1000,
		.fps_fixed  = 2,
		.bin_factor = 1,
		.intg_min   = 16,
		.intg_max   = (3328-6)<<4,
		.gain_min   = 20,
		.gain_max   = (16<<4)-1,
		.regs       = sensor_13mega_regs,
		.regs_size  = ARRAY_SIZE(sensor_13mega_regs),
		.set_size   = NULL,
	},
#endif
#if 1
	/*4k video*/
	{
		.width	  = 3840,
		.height	  = 2160,
		.hoffset	  = 0,
		.voffset	  = 0,
		.hts		  = 6800/16,
		.vts		  = 2992,
		.pclk 	  = 468*1000*1000/16,
		.mipi_bps   = 850*1000*1000,
		.fps_fixed  = 2,
		.bin_factor = 1,
		.intg_min   = 16,
		.intg_max   = 2992<<4,
		.gain_min   = 20,
		.gain_max   = (8<<4)-1,
		.regs 	  = sensor_4k_videos,
		.regs_size  = ARRAY_SIZE(sensor_4k_videos),
		.set_size   = NULL,
	},

	/* 2112*1568 */
	{
		.width	  = 1920,
		.height 	  = 1080,
		.hoffset	  = (2112-1920)/2,
		.voffset	  = (1568-1080)/2,
		.hts        = 9600/16,
		.vts        = 1664,
		.pclk 	  = 639*1000*1000/16,
		.mipi_bps   = 640*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 16,
		.intg_max   = 1664<<4,
		.gain_min   = 20,
		.gain_max   = (16<<4)-1,
		.regs		  = sensor_1080p_regs,
		.regs_size  = ARRAY_SIZE(sensor_1080p_regs),
		.set_size	  = NULL,
	},
#endif

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
	ov13850_sensor_vts = wsize->vts;
  
	vfe_dev_print("s_fmt set width = %d, height = %d\n",wsize->width,wsize->height);

	if(info->capture_mode == V4L2_MODE_VIDEO)
	{
	//video
	} else {
	//capture image
	}
	
	//sensor_write_array(sd, sensor_oe_enable_regs, ARRAY_SIZE(sensor_oe_enable_regs));
	vfe_dev_print("s_fmt end\n");
	usleep_range(100000,101000);
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

