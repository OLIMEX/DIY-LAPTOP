/*
 * A V4L2 driver for Hynix HI257 cameras.
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
MODULE_DESCRIPTION("A low-level driver for Hynix HI257 sensors");
MODULE_LICENSE("GPL");



//for internel driver debug
#define DEV_DBG_EN   		1 
#if(DEV_DBG_EN == 1)		
#define vfe_dev_dbg(x,arg...) printk("[CSI_DEBUG][HI257]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...) 
#endif
#define vfe_dev_err(x,arg...) printk("[CSI_ERR][HI257]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[CSI][HI257]"x,##arg)

#define LOG_ERR_RET(x)  { \
                          int ret;  \
                          ret = x; \
                          if(ret < 0) {\
                            vfe_dev_err("error at %s\n",__func__);  \
                            return ret; \
                          } \
                        }

//define module timing
#define MCLK (27*1000*1000)
#define VREF_POL          V4L2_MBUS_VSYNC_ACTIVE_LOW
#define HREF_POL          V4L2_MBUS_HSYNC_ACTIVE_HIGH
#define CLK_POL           V4L2_MBUS_PCLK_SAMPLE_FALLING
#define V4L2_IDENT_SENSOR 0x257

/*
 * Our nominal (default) frame rate.
 */
#define SENSOR_FRAME_RATE 20

/*
 * The hi257 sits on i2c with ID 0x40
 */
#define I2C_ADDR 0x40
#define SENSOR_NAME "hi257"
//For hi257
//Sensor address in two-wire serial bus :
// 40H(write) , 41H(read) 

/* Registers */


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
	{0x01, 0x01}, //sleep on
	{0x01, 0x03}, //sleep off
	{0x01, 0x01}, //sleep on
	
	{0x03, 0x00}, //Dummy 750us
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	
	{0x08, 0x00},
	{0x09, 0x77},// pad strength = max
	{0x0a, 0x07},// pad strength = max
	
	{0x0e, 0x03}, //PLL On
	{0x0e, 0x73}, //PLLx2
	
	{0x03, 0x00}, //Dummy 750us
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	
	
	///// PAGE 20 /////
	{0x03, 0x20}, //page 20
	{0x10, 0x1c}, //AE off 50hz
	
	///// PAGE 22 /////
	{0x03, 0x22}, //page 22
	{0x10, 0x69}, //AWB off
	
	
	///// Initial Start /////
	///// PAGE 0 Start /////
	{0x03, 0x00}, //page 0
	{0x10, 0x00},
	{0x11, 0x90}, //Windowing On + 1Frame Skip
	{0x12, 0x00}, //Rinsing edge 0x04 // Falling edge 0x00
	{0x14, 0x05},
	
	{0x20, 0x00}, //Row H
	{0x21, 0x02}, //Row L
	{0x22, 0x00}, //Col H
	{0x23, 0x04}, //Col L
	
	{0x24, 0x04}, //Window height_H //= 1200
	{0x25, 0xb0}, //Window height_L //
	{0x26, 0x06}, //Window width_H	//= 1600
	{0x27, 0x40}, //Window wight_L
	
	{0x40, 0x01}, //Hblank_360
	{0x41, 0x78},
	{0x42, 0x00}, //Vblank
	{0x43, 0x14}, //Flick Stop
	
	{0x50, 0x00}, //Test Pattern
	
	///// BLC /////
	{0x80, 0x2e},
	{0x81, 0x7e},
	{0x82, 0x90},
	{0x83, 0x00},
	{0x84, 0xcc}, //20130604 0x0c->0xcc
	{0x85, 0x00},
	{0x86, 0x00},
	{0x87, 0x0f},
	{0x88, 0x34},
	{0x8a, 0x0b},
	{0x8e, 0x80}, //Pga Blc Hold
	
	{0x90, 0x0c}, //BLC_TIME_TH_ON
	{0x91, 0x0c}, //BLC_TIME_TH_OFF
	{0x92, 0x78}, //BLC_AG_TH_ON
	{0x93, 0x70}, //BLC_AG_TH_OFF
	{0x96, 0xdc}, //BLC Outdoor Th On
	{0x97, 0xfe}, //BLC Outdoor Th Off
	{0x98, 0x38},
	
	//OutDoor  BLC
	{0x99, 0x43}, //R,Gr,B,Gb Offset
	
	//Dark BLC
	{0xa0, 0x00}, //R,Gr,B,Gb Offset
	
	//Normal BLC
	{0xa8, 0x44}, //R,Gr,B,Gb Offset
	///// PAGE 0 END /////
	
	///// PAGE 2 START /////
	{0x03, 0x02},
	{0x10, 0x00},
	{0x13, 0x00},
	{0x14, 0x00},
	{0x18, 0xcc},
	{0x19, 0x01}, // pmos switch on (for cfpn)
	{0x1A, 0x39}, //20130604 0x09->0xcc
	{0x1B, 0x00},
	{0x1C, 0x1a}, // for ncp
	{0x1D, 0x14}, // for ncp
	{0x1E, 0x30}, // for ncp
	{0x1F, 0x10},
	
	{0x20, 0x77},
	{0x21, 0xde},
	{0x22, 0xa7},
	{0x23, 0x30},
	{0x24, 0x77},
	{0x25, 0x10},
	{0x26, 0x10},
	{0x27, 0x3c},
	{0x2b, 0x80},
	{0x2c, 0x02},
	{0x2d, 0x58},
	{0x2e, 0x11},//20130604 0xde->0x11
	{0x2f, 0x11},//20130604 0xa7->0x11
	
	{0x30, 0x00},
	{0x31, 0x99},
	{0x32, 0x00},
	{0x33, 0x00},
	{0x34, 0x22},
	{0x36, 0x75},
	{0x38, 0x88},
	{0x39, 0x88},
	{0x3d, 0x03},
	{0x3f, 0x02},
	
	{0x49, 0xc1},//20130604 0x87->0xd1 --> mode Change Issue modify -> 0xc1 
	{0x4a, 0x10},
	
	{0x50, 0x21},
	{0x53, 0xb1},
	{0x54, 0x10},
	{0x55, 0x1c}, // for ncp
	{0x56, 0x11},
	{0x58, 0x3a},//20130604 add
	{0x59, 0x38},//20130604 add
	{0x5d, 0xa2},
	{0x5e, 0x5a},
	
	{0x60, 0x87},
	{0x61, 0x98},
	{0x62, 0x88},
	{0x63, 0x96},
	{0x64, 0x88},
	{0x65, 0x96},
	{0x67, 0x3f},
	{0x68, 0x3f},
	{0x69, 0x3f},
	
	{0x72, 0x89},
	{0x73, 0x95},
	{0x74, 0x89},
	{0x75, 0x95},
	{0x7C, 0x84},
	{0x7D, 0xaf},
	
	{0x80, 0x01},
	{0x81, 0x7a},
	{0x82, 0x13},
	{0x83, 0x24},
	{0x84, 0x78},
	{0x85, 0x7c},
	
	{0x92, 0x44},
	{0x93, 0x59},
	{0x94, 0x78},
	{0x95, 0x7c},
	
	{0xA0, 0x02},
	{0xA1, 0x74},
	{0xA4, 0x74},
	{0xA5, 0x02},
	{0xA8, 0x85},
	{0xA9, 0x8c},
	{0xAC, 0x10},
	{0xAD, 0x16},
	
	{0xB0, 0x99},
	{0xB1, 0xa3},
	{0xB4, 0x9b},
	{0xB5, 0xa2},
	{0xB8, 0x9b},
	{0xB9, 0x9f},
	{0xBC, 0x9b},
	{0xBD, 0x9f},
	
	{0xc4, 0x29},
	{0xc5, 0x40},
	{0xc6, 0x5c},
	{0xc7, 0x72},
	{0xc8, 0x2a},
	{0xc9, 0x3f},
	{0xcc, 0x5d},
	{0xcd, 0x71},
	
	{0xd0, 0x10},
	{0xd1, 0x14},
	{0xd2, 0x20},
	{0xd3, 0x00},
	{0xd4, 0x0c}, //DCDC_TIME_TH_ON
	{0xd5, 0x0c}, //DCDC_TIME_TH_OFF 
	{0xd6, 0x78}, //DCDC_AG_TH_ON
	{0xd7, 0x70}, //DCDC_AG_TH_OFF
	{0xdc, 0x00},
	{0xdd, 0xa3},
	{0xde, 0x00},
	{0xdf, 0x84},
	
	{0xe0, 0xa4},
	{0xe1, 0xa4},
	{0xe2, 0xa4},
	{0xe3, 0xa4},
	{0xe4, 0xa4},
	{0xe5, 0x01},
	{0xe8, 0x00},
	{0xe9, 0x00},
	{0xea, 0x77},
	
	{0xF0, 0x00},
	{0xF1, 0x00},
	{0xF2, 0x00},
	
	///// PAGE 2 END /////
	
	///// PAGE 10 START /////
	{0x03, 0x10}, //page 10
	{0x10, 0x01}, //S2D enable _ YUYV Order
	{0x11, 0x03},
	{0x12, 0xf0},
	{0x13, 0x03},
	
	{0x20, 0x00},
	{0x21, 0x40},
	{0x22, 0x0f},
	{0x24, 0x20},
	{0x25, 0x10},
	{0x26, 0x01},
	{0x27, 0x02},
	{0x28, 0x11},
	
	{0x40, 0x00},
	{0x41, 0x00}, //D-YOffset Th
	{0x42, 0x05}, //Cb Offset
	{0x43, 0x05}, //Cr Offset
	{0x44, 0x80},
	{0x45, 0x80},
	{0x46, 0xf0},
	{0x48, 0x85},
	{0x4a, 0x80},
	
	{0x50, 0xa0}, //D-YOffset AG
	
	{0x60, 0x4f}, //Outdoor Saturation
	{0x61, 0x94}, //Sat B
	{0x62, 0x92}, //Sat R
	{0x63, 0x80}, //Auto-De Color
	
	{0x66, 0x42},
	{0x67, 0x22},
	
	{0x6a, 0x8c}, //White Protection Offset Dark/Indoor
	{0x74, 0x0c}, //White Protection Offset Outdoor
	{0x76, 0x01}, //White Protection Enable
	///// PAGE 10 END /////
	
	///// PAGE 11 START /////
	{0x03, 0x11}, //page 11
	
	//LPF Auto Control
	{0x20, 0x00},
	{0x21, 0x00},
	{0x26, 0x5a}, // Double_AG
	{0x27, 0x58}, // Double_AG
	{0x28, 0x0f},
	{0x29, 0x10},
	{0x2b, 0x30},
	{0x2c, 0x32},
	
	//GBGR 
	{0x70, 0x2b},
	{0x74, 0x30},
	{0x75, 0x18},
	{0x76, 0x30},
	{0x77, 0xff},
	{0x78, 0xa0},
	{0x79, 0xff}, //Dark GbGr Th
	{0x7a, 0x30},
	{0x7b, 0x20},
	{0x7c, 0xf4}, //Dark Dy Th B[7:4]
	///// PAGE 11 END /////
	
	///// PAGE 12 START /////
	{0x03, 0x12}, //page 11
	
	//YC2D
	{0x10, 0x03}, //Y DPC Enable
	{0x11, 0x08}, //
	{0x12, 0x10}, //0x30 -> 0x10
	{0x20, 0x53}, //Y_lpf_enable
	{0x21, 0x03}, //C_lpf_enable_on
	{0x22, 0xe6}, //YC2D_CrCbY_Dy
	
	{0x23, 0x14}, //Outdoor Dy Th
	{0x24, 0x20}, //Indoor Dy Th // For reso Limit 0x20
	{0x25, 0x30}, //Dark Dy Th
	
	//Outdoor LPF Flat
	{0x30, 0xff}, //Y Hi Th
	{0x31, 0x00}, //Y Lo Th
	{0x32, 0xf0}, //Std Hi Th //Reso Improve Th Low //50
	{0x33, 0x00}, //Std Lo Th
	{0x34, 0x00}, //Median ratio
	
	//Indoor LPF Flat
	{0x35, 0xff}, //Y Hi Th
	{0x36, 0x00}, //Y Lo Th
	{0x37, 0xff}, //Std Hi Th //Reso Improve Th Low //50
	{0x38, 0x00}, //Std Lo Th
	{0x39, 0x00}, //Median ratio
	
	//Dark LPF Flat
	{0x3a, 0xff}, //Y Hi Th
	{0x3b, 0x00}, //Y Lo Th
	{0x3c, 0x93}, //Std Hi Th //Reso Improve Th Low //50
	{0x3d, 0x00}, //Std Lo Th
	{0x3e, 0x00}, //Median ratio
	
	//Outdoor Cindition
	{0x46, 0xa0}, //Out Lum Hi
	{0x47, 0x40}, //Out Lum Lo
	
	//Indoor Cindition
	{0x4c, 0xb0}, //Indoor Lum Hi
	{0x4d, 0x40}, //Indoor Lum Lo
	
	//Dark Cindition
	{0x52, 0xb0}, //Dark Lum Hi
	{0x53, 0x50}, //Dark Lum Lo
	
	//C-Filter
	{0x70, 0x10}, //Outdoor(2:1) AWM Th Horizontal
	{0x71, 0x0a}, //Outdoor(2:1) Diff Th Vertical
	{0x72, 0x10}, //Indoor,Dark1 AWM Th Horizontal
	{0x73, 0x0a}, //Indoor,Dark1 Diff Th Vertical
	{0x74, 0x18}, //Dark(2:3) AWM Th Horizontal
	{0x75, 0x0f}, //Dark(2:3) Diff Th Vertical
	
	//DPC
	{0x90, 0x7d},
	{0x91, 0x34},
	{0x99, 0x28},
	{0x9c, 0x14},
	{0x9d, 0x15},
	{0x9e, 0x28},
	{0x9f, 0x28},
	{0xb0, 0x0e}, //Zipper noise Detault change (0x75->0x0e)
	{0xb8, 0x44},
	{0xb9, 0x15},
	///// PAGE 12 END /////
	
	///// PAGE 13 START /////
	{0x03, 0x13}, //page 13
	
	{0x80, 0xfd}, //Sharp2D enable _ YUYV Order
	{0x81, 0x07}, //Sharp2D Clip/Limit
	{0x82, 0x73}, //Sharp2D Filter
	{0x83, 0x00}, //Sharp2D Low Clip
	{0x85, 0x00},
	
	{0x92, 0x33}, //Sharp2D Slop n/p
	{0x93, 0x30}, //Sharp2D LClip
	{0x94, 0x02}, //Sharp2D HiClip1 Th
	{0x95, 0xf0}, //Sharp2D HiClip2 Th
	{0x96, 0x1e}, //Sharp2D HiClip2 Resolution
	{0x97, 0x40}, 
	{0x98, 0x80},
	{0x99, 0x40},
	
	//Sharp Lclp
	{0xa2, 0x03}, //Outdoor Lclip_N
	{0xa3, 0x04}, //Outdoor Lclip_P
	{0xa4, 0x02}, //Indoor Lclip_N 0x03 For reso Limit 0x0e
	{0xa5, 0x03}, //Indoor Lclip_P 0x0f For reso Limit 0x0f
	{0xa6, 0x80}, //Dark Lclip_N
	{0xa7, 0x80}, //Dark Lclip_P
	
	//Outdoor Slope
	{0xb6, 0x28}, //Lum negative Hi
	{0xb7, 0x20}, //Lum negative middle
	{0xb8, 0x24}, //Lum negative Low
	{0xb9, 0x28}, //Lum postive Hi
	{0xba, 0x20}, //Lum postive middle
	{0xbb, 0x24}, //Lum postive Low
	
	//Indoor Slope
	{0xbc, 0x20}, //Lum negative Hi
	{0xbd, 0x20}, //Lum negative middle
	{0xbe, 0x20}, //Lum negative Low
	{0xbf, 0x20}, //Lum postive Hi
	{0xc0, 0x20}, //Lum postive middle
	{0xc1, 0x20}, //Lum postive Low
	
	//Dark Slope
	{0xc2, 0x14}, //Lum negative Hi
	{0xc3, 0x24}, //Lum negative middle
	{0xc4, 0x1d}, //Lum negative Low
	{0xc5, 0x14}, //Lum postive Hi
	{0xc6, 0x24}, //Lum postive middle
	{0xc7, 0x1d}, //Lum postive Low
	///// PAGE 13 END /////
	
	///// PAGE 14 START /////
	{0x03, 0x14}, //page 14
	{0x10, 0x01},
	
	{0x20, 0xb0}, //X-Center
	{0x21, 0x90}, //Y-Center
	
	{0x22, 0x10}, //LSC R 1b->15 20130125
	{0x23, 0x0d}, //LSC G
	{0x24, 0x0d}, //LSC B
	
	{0x25, 0xf0}, //LSC Off
	{0x26, 0xf0}, //LSC On
	///// PAGE 14 END /////
	
	/////// PAGE 15 START ///////
	{0x03, 0x15}, //page 15
	{0x10, 0x21},
	{0x14, 0x44},
	{0x15, 0x34},
	{0x16, 0x26},
	{0x17, 0x2f},
	
	//CMC
	{0x30, 0xdd},
	{0x31, 0x68},
	{0x32, 0x0b},
	{0x33, 0x2c},
	{0x34, 0xce},
	{0x35, 0x22},
	{0x36, 0x0e},
	{0x37, 0x42},
	{0x38, 0xd0},
	
	//CMC OFS
	{0x40, 0x90},
	{0x41, 0x16},
	{0x42, 0x86},
	{0x43, 0x01},
	{0x44, 0x10},
	{0x45, 0x91},
	{0x46, 0x82},
	{0x47, 0x9c},
	{0x48, 0x1d},
	
	//CMC POFS
	{0x50, 0x0d},
	{0x51, 0x91},
	{0x52, 0x04},
	{0x53, 0x07},
	{0x54, 0x19},
	{0x55, 0xa0},
	{0x56, 0x00},
	{0x57, 0x98},
	{0x58, 0x18},
	///// PAGE 15 END /////
	
	///// PAGE 16 START /////
	{0x03, 0x16}, //page 16
	{0x10, 0x31},
	{0x18, 0x5a},// Double_AG 5e->37
	{0x19, 0x58},// Double_AG 5e->36
	{0x1a, 0x0e},
	{0x1b, 0x01},
	{0x1c, 0xdc},
	{0x1d, 0xfe},
	
	//Indoor
	{0x30, 0x00},
	{0x31, 0x12},
	{0x32, 0x1d},
	{0x33, 0x31},
	{0x34, 0x51},
	{0x35, 0x6c},
	{0x36, 0x82},
	{0x37, 0x96},
	{0x38, 0xa7},
	{0x39, 0xb6},
	{0x3a, 0xc3},
	{0x3b, 0xd0},
	{0x3c, 0xd9},
	{0x3d, 0xe1},
	{0x3e, 0xe9},
	{0x3f, 0xef},
	{0x40, 0xf6},
	{0x41, 0xfa},
	{0x42, 0xff},
	
	
	//Outdoor
	{0x50, 0x00},
	{0x51, 0x03},
	{0x52, 0x10},
	{0x53, 0x26},
	{0x54, 0x43},
	{0x55, 0x5d},
	{0x56, 0x79},
	{0x57, 0x8c},
	{0x58, 0x9f},
	{0x59, 0xaa},
	{0x5a, 0xb6},
	{0x5b, 0xc3},
	{0x5c, 0xce},
	{0x5d, 0xd9},
	{0x5e, 0xe1},
	{0x5f, 0xe9},
	{0x60, 0xf0},
	{0x61, 0xf4},
	{0x62, 0xf5},
	
	//Dark
	{0x70, 0x00},
	{0x71, 0x10},
	{0x72, 0x1c},
	{0x73, 0x2e},
	{0x74, 0x4e},
	{0x75, 0x6c},
	{0x76, 0x82},
	{0x77, 0x96},
	{0x78, 0xa7},
	{0x79, 0xb6},
	{0x7a, 0xc4},
	{0x7b, 0xd0},
	{0x7c, 0xda},
	{0x7d, 0xe2},
	{0x7e, 0xea},
	{0x7f, 0xf0},
	{0x80, 0xf6},
	{0x81, 0xfa},
	{0x82, 0xff},
	///// PAGE 16 END /////
	
	///// PAGE 17 START /////
	{0x03, 0x17}, //page 17
	{0xc1, 0x00},
	{0xc4, 0x4b},
	{0xc5, 0x3f},
	{0xc6, 0x02},
	{0xc7, 0x20},
	///// PAGE 17 END /////
	
	///// PAGE 19 START /////
	{0x03, 0x19}, //Page 0x19
	{0x10, 0x7f}, //mcmc_ctl1 MCMC Enable B:[0]
	{0x11, 0x7f}, //mcmc_ctl2
	{0x12, 0x1e}, //mcmc_delta1
	{0x13, 0x32}, //mcmc_center1
	{0x14, 0x1e}, //mcmc_delta2
	{0x15, 0x6e}, //mcmc_center2
	{0x16, 0x0a}, //mcmc_delta3
	{0x17, 0xb8}, //mcmc_center3
	{0x18, 0x1e}, //mcmc_delta4
	{0x19, 0xe6}, //mcmc_center4
	{0x1a, 0x9e}, //mcmc_delta5
	{0x1b, 0x22}, //mcmc_center5
	{0x1c, 0x9e}, //mcmc_delta6
	{0x1d, 0x5e}, //mcmc_center6
	{0x1e, 0x3b}, //mcmc_sat_gain1
	{0x1f, 0x3e}, //mcmc_sat_gain2
	{0x20, 0x4b}, //mcmc_sat_gain3
	{0x21, 0x52}, //mcmc_sat_gain4
	{0x22, 0x59}, //mcmc_sat_gain5
	{0x23, 0x46}, //mcmc_sat_gain6
	{0x24, 0x00}, //mcmc_hue_angle1
	{0x25, 0x01}, //mcmc_hue_angle2
	{0x26, 0x0e}, //mcmc_hue_angle3
	{0x27, 0x04}, //mcmc_hue_angle4
	{0x28, 0x00}, //mcmc_hue_angle5
	{0x29, 0x8c}, //mcmc_hue_angle6
	
	{0x53, 0x10}, //mcmc_ctl3
	
	{0x6c, 0xff}, //mcmc_lum_ctl1 sat hue offset
	{0x6d, 0x3f}, //mcmc_lum_ctl2 gain
	{0x6e, 0x00}, //mcmc_lum_ctl3 hue
	{0x6f, 0x00}, //mcmc_lum_ctl4 rgb offset
	{0x70, 0x00}, //mcmc_lum_ctl5 rgb scale
	
	{0x71, 0x3f}, //mcmc_lum_gain_wgt_th1 R1 magenta
	{0x72, 0x3f}, //mcmc_lum_gain_wgt_th2 R1
	{0x73, 0x3f}, //mcmc_lum_gain_wgt_th3 R1
	{0x74, 0x3f}, //mcmc_lum_gain_wgt_th4 R1
	{0x75, 0x30}, //mcmc_rg1_lum_sp1	  R1
	{0x76, 0x50}, //mcmc_rg1_lum_sp2	  R1
	{0x77, 0x80}, //mcmc_rg1_lum_sp3	  R1
	{0x78, 0xb0}, //mcmc_rg1_lum_sp4	  R1
	
	{0x79, 0x3f}, //mcmc_lum_gain_wgt_th1 R2 bright green
	{0x7a, 0x3f}, //mcmc_lum_gain_wgt_th2 R2
	{0x7b, 0x3f}, //mcmc_lum_gain_wgt_th3 R2
	{0x7c, 0x3f}, //mcmc_lum_gain_wgt_th4 R2
	{0x7d, 0x28}, //mcmc_rg2_lum_sp1	  R2
	{0x7e, 0x50}, //mcmc_rg2_lum_sp2	  R2
	{0x7f, 0x80}, //mcmc_rg2_lum_sp3	  R2
	{0x80, 0xb0}, //mcmc_rg2_lum_sp4	  R2
	
	{0x81, 0x28}, //mcmc_lum_gain_wgt_th1 R3 dark green
	{0x82, 0x3f}, //mcmc_lum_gain_wgt_th2 R3
	{0x83, 0x3f}, //mcmc_lum_gain_wgt_th3 R3
	{0x84, 0x3f}, //mcmc_lum_gain_wgt_th4 R3
	{0x85, 0x28}, //mcmc_rg3_lum_sp1	  R3
	{0x86, 0x50}, //mcmc_rg3_lum_sp2	  R3
	{0x87, 0x80}, //mcmc_rg3_lum_sp3	  R3
	{0x88, 0xb0}, //mcmc_rg3_lum_sp4	  R3
	
	{0x89, 0x1a}, //mcmc_lum_gain_wgt_th1 R4 skin
	{0x8a, 0x28}, //mcmc_lum_gain_wgt_th2 R4
	{0x8b, 0x3f}, //mcmc_lum_gain_wgt_th3 R4
	{0x8c, 0x3f}, //mcmc_lum_gain_wgt_th4 R4
	{0x8d, 0x10}, //mcmc_rg3_lum_sp1	  R4
	{0x8e, 0x30}, //mcmc_rg3_lum_sp2	  R4
	{0x8f, 0x60}, //mcmc_rg3_lum_sp3	  R4
	{0x90, 0x90}, //mcmc_rg3_lum_sp4	  R4
	
	{0x91, 0x1a}, //mcmc_rg5_gain_wgt_th1 R5 cyan
	{0x92, 0x28}, //mcmc_rg5_gain_wgt_th2 R5
	{0x93, 0x3f}, //mcmc_rg5_gain_wgt_th3 R5
	{0x94, 0x3f}, //mcmc_rg5_gain_wgt_th4 R5
	{0x95, 0x28}, //mcmc_rg5_lum_sp1	  R5
	{0x96, 0x50}, //mcmc_rg5_lum_sp2	  R5
	{0x97, 0x80}, //mcmc_rg5_lum_sp3	  R5
	{0x98, 0xb0}, //mcmc_rg5_lum_sp4	  R5
	
	{0x99, 0x1a}, //mcmc_rg6_gain_wgt_th1 R6 blue
	{0x9a, 0x28}, //mcmc_rg6_gain_wgt_th2 R6
	{0x9b, 0x3f}, //mcmc_rg6_gain_wgt_th3 R6
	{0x9c, 0x3f}, //mcmc_rg6_gain_wgt_th4 R6
	{0x9d, 0x28}, //mcmc_rg6_lum_sp1	  R6
	{0x9e, 0x50}, //mcmc_rg6_lum_sp2	  R6
	{0x9f, 0x80}, //mcmc_rg6_lum_sp3	  R6
	{0xa0, 0xb0}, //mcmc_rg6_lum_sp4	  R6
	
	{0xe5, 0x80}, //add 20120709 Bit[7] On MCMC --> YC2D_LPF
	
	
	/////// PAGE 20 START ///////
	{0x03, 0x20},
	{0x10, 0x1c},
	{0x11, 0x14},
	{0x18, 0x30},
	{0x20, 0x25}, //8x8 Ae weight 0~7 Outdoor / Weight Outdoor On B[5]
	{0x21, 0x30},
	{0x22, 0x10},
	{0x23, 0x00},
	
	{0x28, 0xe7},
	{0x29, 0x0d},
	{0x2a, 0xf0},
	{0x2b, 0x34}, //Adaptive Off,1/100 Flicker
	
	{0x2c, 0x83}, //AE After CI
	{0x2d, 0x03},
	{0x2e, 0x13},
	{0x2f, 0x0b},
	
	{0x30, 0x78},
	{0x31, 0xd7},
	{0x32, 0x10},
	{0x33, 0x2e},
	{0x34, 0x20},
	{0x35, 0xd4},
	{0x36, 0xfe},
	{0x37, 0x32},
	{0x38, 0x04},
	{0x39, 0x22},
	{0x3a, 0xde},
	{0x3b, 0x22},
	{0x3c, 0xde},
	{0x3d, 0xe1},
	
	{0x50, 0x45},
	{0x51, 0x88},
	
	{0x56, 0x1f}, // for tracking
	{0x57, 0xa6}, // for tracking
	{0x58, 0x1a}, // for tracking
	{0x59, 0x7a}, // for tracking
	
	{0x5a, 0x04},
	{0x5b, 0x04},
	
	{0x5e, 0xc7},
	{0x5f, 0x95},
	
	{0x62, 0x10},
	{0x63, 0xc0},
	{0x64, 0x10},
	{0x65, 0x8a},
	{0x66, 0x58},
	{0x67, 0x58},
	
	{0x70, 0x54}, //6c
	{0x71, 0x89}, //81(+4),89(-4)
	
	{0x76, 0x32},
	{0x77, 0xa1},
	{0x78, 0x22}, //24
	{0x79, 0x30}, // Y Target 70 => 25, 72 => 26 //
	{0x7a, 0x23}, //23
	{0x7b, 0x22}, //22
	{0x7d, 0x23},
	
	{0x03, 0x20}, //Page 20
	{0x83, 0x02}, //EXP Normal 33.33 fps 
	{0x84, 0xbf}, 
	{0x85, 0x20}, 
	{0x86, 0x01}, //EXPMin 6000.00 fps
	{0x87, 0xf4}, 
	{0x88, 0x0b}, //EXP Max(120Hz) 8.00 fps 
	{0x89, 0x71}, 
	{0x8a, 0xb0}, 
	{0xa5, 0x0a}, //EXP Max(100Hz) 8.33 fps 
	{0xa6, 0xfc}, 
	{0xa7, 0x80}, 
	{0x8B, 0xea}, //EXP100 
	{0x8C, 0x60}, 
	{0x8D, 0xc3}, //EXP120 
	{0x8E, 0x50}, 
	{0x9c, 0x0d}, //EXP Limit 857.14 fps 
	{0x9d, 0xac}, 
	{0x9e, 0x01}, //EXP Unit 
	{0x9f, 0xf4}, 
	
	
	{0xa3, 0x00},
	{0xa4, 0xfd}, //Exp 12000
	
	{0xb0, 0x15},
	{0xb1, 0x14},
	{0xb2, 0x80},
	{0xb3, 0x15},
	{0xb4, 0x16},
	{0xb5, 0x3c},
	{0xb6, 0x29},
	{0xb7, 0x23},
	{0xb8, 0x20},
	{0xb9, 0x1e},
	{0xba, 0x1c},
	{0xbb, 0x1b},
	{0xbc, 0x1b},
	{0xbd, 0x1a},
	
	{0xc0, 0x10},
	{0xc1, 0x40},
	{0xc2, 0x40},
	{0xc3, 0x40},
	{0xc4, 0x06},
	
	{0xc8, 0x80},
	{0xc9, 0x80},
	///// PAGE 20 END /////
	
	///// PAGE 21 START /////
	{0x03, 0x21}, //page 21
	
	//Indoor Weight
	{0x20, 0x11},
	{0x21, 0x11},
	{0x22, 0x11},
	{0x23, 0x11},
	{0x24, 0x14},
	{0x25, 0x44},
	{0x26, 0x44},
	{0x27, 0x41},
	{0x28, 0x14},
	{0x29, 0x44},
	{0x2a, 0x44},
	{0x2b, 0x41},
	{0x2c, 0x14},
	{0x2d, 0x47},
	{0x2e, 0x74},
	{0x2f, 0x41},
	{0x30, 0x14},
	{0x31, 0x47},
	{0x32, 0x74},
	{0x33, 0x41},
	{0x34, 0x14},
	{0x35, 0x44},
	{0x36, 0x44},
	{0x37, 0x41},
	{0x38, 0x14},
	{0x39, 0x44},
	{0x3a, 0x44},
	{0x3b, 0x41},
	{0x3c, 0x11},
	{0x3d, 0x11},
	{0x3e, 0x11},
	{0x3f, 0x11},
	
	//Outdoor Weight
	{0x40, 0x11},
	{0x41, 0x11},
	{0x42, 0x11},
	{0x43, 0x11},
	{0x44, 0x14},
	{0x45, 0x44},
	{0x46, 0x44},
	{0x47, 0x41},
	{0x48, 0x14},
	{0x49, 0x44},
	{0x4a, 0x44},
	{0x4b, 0x41},
	{0x4c, 0x14},
	{0x4d, 0x47},
	{0x4e, 0x74},
	{0x4f, 0x41},
	{0x50, 0x14},
	{0x51, 0x47},
	{0x52, 0x74},
	{0x53, 0x41},
	{0x54, 0x14},
	{0x55, 0x44},
	{0x56, 0x44},
	{0x57, 0x41},
	{0x58, 0x14},
	{0x59, 0x44},
	{0x5a, 0x44},
	{0x5b, 0x41},
	{0x5c, 0x11},
	{0x5d, 0x11},
	{0x5e, 0x11},
	{0x5f, 0x11},
	
	
	///// PAGE 22 START /////
	{0x03, 0x22}, //page 22
	{0x10, 0xfd},
	{0x11, 0x2e},
	{0x19, 0x02}, //Low On //
	{0x20, 0x30}, //For AWB Speed
	{0x21, 0x80},
	{0x22, 0x00},
	{0x23, 0x00},
	{0x24, 0x01},
	{0x25, 0x7e}, //for Tracking setting
	
	{0x30, 0x80},
	{0x31, 0x80},
	{0x38, 0x11},
	{0x39, 0x34},
	{0x40, 0xe4}, //Stb Yth
	{0x41, 0x33}, //Stb cdiff
	{0x42, 0x22}, //Stb csum
	{0x43, 0xf3}, //Unstb Yth
	{0x44, 0x55}, //Unstb cdiff
	{0x45, 0x33}, //Unstb csum
	{0x46, 0x00},
	{0x47, 0xa2},
	{0x48, 0x02},
	{0x49, 0x0a},
	
	{0x60, 0x04},
	{0x61, 0xc4},
	{0x62, 0x04},
	{0x63, 0x92},
	{0x66, 0x04},
	{0x67, 0xc4},
	{0x68, 0x04},
	{0x69, 0x92},
	
	{0x80, 0x36},
	{0x81, 0x20},
	{0x82, 0x2a},
	
	{0x83, 0x58},
	{0x84, 0x16},
	{0x85, 0x4f},
	{0x86, 0x15},
	
	{0x87, 0x3b},
	{0x88, 0x30},
	{0x89, 0x29},
	{0x8a, 0x18},
	
	{0x8b, 0x3c},
	{0x8c, 0x32},
	{0x8d, 0x2a},
	{0x8e, 0x1b},
	
	{0x8f, 0x4d}, //4e
	{0x90, 0x46}, //4d
	{0x91, 0x40}, //4c
	{0x92, 0x3a}, //4a
	{0x93, 0x2f}, //46
	{0x94, 0x21},
	{0x95, 0x19},
	{0x96, 0x16},
	{0x97, 0x13},
	{0x98, 0x12},
	{0x99, 0x11},
	{0x9a, 0x10},
	
	{0x9b, 0x88},
	{0x9c, 0x77},
	{0x9d, 0x48},
	{0x9e, 0x38},
	{0x9f, 0x30},
	
	{0xa0, 0x70},
	{0xa1, 0x54},
	{0xa2, 0x6f},
	{0xa3, 0xff},
	
	{0xa4, 0x14}, //1536fps
	{0xa5, 0x2c}, //698fps
	{0xa6, 0xcf}, //148fps
	
	{0xad, 0x2e},
	{0xae, 0x2a},
	
	{0xaf, 0x28}, //Low temp Rgain
	{0xb0, 0x26}, //Low temp Rgain
	
	{0xb1, 0x08},
	{0xb4, 0xbf}, //For Tracking AWB Weight
	{0xb8, 0x91}, //(0+,1-)High Cb , (0+,1-)Low Cr
	{0xb9, 0xb0},
	/////// PAGE 22 END ///////
	
	/////// PAGE 20 ///////
	{0x03, 0x20},
	{0x10, 0x9c}, //AE On 50hz
	
	/////// PAGE 22 ///////
	{0x03, 0x22},
	{0x10, 0xe9}, //AWB On
	
	/////// PAGE 0 ///////
	{0x03, 0x00},
	{0x0e, 0x03},
	{0x0e, 0x73},
	
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	{0x03, 0x00},
	
	{0x03, 0x00},
	{0x01, 0x00},
	
	{0xff,0xff},
	{0xff,0xff},

};

static struct regval_list sensor_uxga_regs[] = {
	//Resolution Setting : 1600*1200
	{0x03,0x00},	//PAGEMODE(0x03)

	{0x10,0x00},//VGA Size
	{0x12,0x04},
	
	{0x20,0x00},
	{0x21,0x04},
	{0x22,0x00},
	{0x23,0x07},
	
	{0x24,0x04},
	{0x25,0xb0},
	{0x26,0x06},
	{0x27,0x40},
	
	{0x40,0x01},//HBLANK: 0x70 = 112
	{0x41,0x98},
	{0x42,0x00},//VBLANK: 0x04 = 4
	{0x43,0x47},
	
	{0x03,0x18},	//PAGEMODE(0x03)
	{0x10,0x00},	 
};

static struct regval_list sensor_hd720_regs[] = {
	//Resolution Setting : 1280*720
	{0x03,0x00},	//PAGEMODE(0x03)

	{0x10,0x00},//VGA Size
	{0x12,0x05},
	
	{0x20,0x00},
	{0x21,0xf0},
	{0x22,0x00},
	{0x23,0xa0},
	
	{0x24,0x02},
	{0x25,0xd0},
	{0x26,0x05},
	{0x27,0x00},
	
	{0x40,0x01},//HBLANK: 0x70 = 112
	{0x41,0x98},
	{0x42,0x00},//VBLANK: 0x04 = 4
	{0x43,0x47},
	
	{0x03,0x18},	//PAGEMODE(0x03)
	{0x10,0x00},
};

static struct regval_list sensor_svga_regs[] = {
	//Resolution Setting : 800*600
	{0x03,0x00},	//PAGEMODE(0x03)

	{0x10,0x10},//VGA Size
	
	{0x20,0x00},
	{0x21,0x04},
	{0x22,0x00},
	{0x23,0x07},
	
	{0x24,0x04},
	{0x25,0xb0},
	{0x26,0x06},
	{0x27,0x40},
	
	{0x40,0x01},//HBLANK: 0x70 = 112
	{0x41,0x98},
	{0x42,0x00},//VBLANK: 0x04 = 4
	{0x43,0x47},
	
	{0x03,0x18},	//PAGEMODE(0x03)
	{0x10,0x00},	

};

static struct regval_list sensor_vga_regs[] = {
	//Resolution Setting : 600*480
	{0x03,0x00},	//PAGEMODE(0x03)

	{0x10,0x10},//VGA Size
	
	{0x20,0x00},
	{0x21,0x04},
	{0x22,0x00},
	{0x23,0x07},
	
	{0x40,0x01},//HBLANK: 0x70 = 112
	{0x41,0x98},
	{0x42,0x00},//VBLANK: 0x04 = 4
	{0x43,0x47},
         

	{0x03,0x18},
	{0x12,0x20},
	{0x10,0x05},
	{0x11,0x00},
	{0x20,0x05},
	{0x21,0x00},
	{0x22,0x03},
	{0x23,0xc0},
	{0x24,0x00},
	{0x25,0x00},
	{0x26,0x00},
	{0x27,0x00},
	{0x28,0x05},
	{0x29,0x00},
	{0x2a,0x03},
	{0x2b,0xc0},
	{0x2c,0x0a},
	{0x2d,0x00},
	{0x2e,0x0a},
	{0x2f,0x00},
	{0x30,0x44},
};

static struct regval_list sensor_qvga_regs[] = {
	//Resolution Setting : 320*240
	{0x03,0x00},	//PAGEMODE(0x03)

	{0x10,0x11},//VGA Size
	
	{0x20,0x00},
	{0x21,0x04},
	{0x22,0x00},
	{0x23,0x07},
	
	{0x40,0x01},//HBLANK: 0x70 = 112
	{0x41,0x98},
	{0x42,0x00},//VBLANK: 0x04 = 4
	{0x43,0x47},
         
	{0x03,0x18},
	{0x12,0x20},
	{0x10,0x07},
	{0x11,0x00},
	{0x20,0x02},
	{0x21,0x80},
	{0x22,0x00},
	{0x23,0xf0},
	{0x24,0x00},
	{0x25,0x06},
	{0x26,0x00},
	{0x27,0x00},
	{0x28,0x02},
	{0x29,0x86},
	{0x2a,0x00},
	{0x2b,0xf0},
	{0x2c,0x14},
	{0x2d,0x00},
	{0x2e,0x14},
	{0x2f,0x00},
	{0x30,0x65},
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
	{0x03,0x22},			
	{0x11,0x2e},				
			     
	{0x83,0x5e},
	{0x84,0x1e},
	{0x85,0x5e},
	{0x86,0x22},		
};

static struct regval_list sensor_wb_incandescence_regs[] = {
	//bai re guang
	{0x03,0x22},
	{0x11,0x28},		  
	{0x80,0x29},
	{0x82,0x54},
	{0x83,0x2e},
	{0x84,0x23},
	{0x85,0x58},
	{0x86,0x4f},
};

static struct regval_list sensor_wb_fluorescent_regs[] = {
	//ri guang deng
	{0x03,0x22},
	{0x11,0x28},
	{0x80,0x41},
	{0x82,0x42},
	{0x83,0x44},
	{0x84,0x34},
	{0x85,0x46},
	{0x86,0x3a},
};

static struct regval_list sensor_wb_tungsten_regs[] = {
	//wu si deng
	{0x03,0x22},
	{0x80,0x24},
	{0x81,0x20},
	{0x82,0x58},
	{0x83,0x27},
	{0x84,0x22},
	{0x85,0x58},
	{0x86,0x52},
};
static struct regval_list sensor_wb_horizon[] = { 
//null
};

static struct regval_list sensor_wb_daylight_regs[] = {
	//tai yang guang
	{0x03,0x22},
	{0x11,0x28},		  
	{0x80,0x59},
	{0x82,0x29},
	{0x83,0x60},
	{0x84,0x50},
	{0x85,0x2f},
	{0x86,0x23},
};

static struct regval_list sensor_wb_flash[] = { 
//null
};

static struct regval_list sensor_wb_cloud_regs[] = {
	{0x03,0x22},
	{0x11,0x28},
	{0x80,0x71},
	{0x82,0x2b},
	{0x83,0x72},
	{0x84,0x70},
	{0x85,0x2b},
	{0x86,0x28},
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
	{0x03, 0x10},  
	{0x11, 0x03},
	{0x12, 0xf0},
	{0x42, 0x05},
	{0x43, 0x05},
	{0x44, 0x80},
	{0x45, 0x80},

};

static struct regval_list sensor_colorfx_bw_regs[] = {
	{0x03, 0x10},  
	{0x11, 0x03},
	{0x12, 0x33},
	{0x42, 0x00},
	{0x43, 0x00},
	{0x44, 0x80},
	{0x45, 0x80},

};

static struct regval_list sensor_colorfx_sepia_regs[] = {
	{0x03, 0x10},  
	{0x11, 0x03},
	{0x12, 0x33},
	{0x42, 0x00},
	{0x43, 0x00},
	{0x44, 0x60},
	{0x45, 0xa3},

};

static struct regval_list sensor_colorfx_negative_regs[] = {
	{0x03, 0x10},  
	{0x11, 0x03},
	{0x12, 0x38},
	{0x42, 0x00},
	{0x43, 0x00},
	{0x44, 0x80},
	{0x45, 0x80},

};

static struct regval_list sensor_colorfx_emboss_regs[] = {
	//NULL
};

static struct regval_list sensor_colorfx_sketch_regs[] = {
	//NULL
};

static struct regval_list sensor_colorfx_sky_blue_regs[] = {
	{0x03, 0x10},  
	{0x11, 0x03},
	{0x12, 0x33},
	{0x42, 0x00},
	{0x43, 0x00},
	{0x44, 0xb0},
	{0x45, 0x40},

};

static struct regval_list sensor_colorfx_grass_green_regs[] = {
	{0x03, 0x10},  
	{0x11, 0x03},
	{0x12, 0x33},
	{0x42, 0x00},
	{0x43, 0x00},
	{0x44, 0x80},
	{0x45, 0x04},

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
	{0x03, 0x10},
	{0x4a, 0x60},
};

static struct regval_list sensor_brightness_neg3_regs[] = {
	{0x03, 0x10},
	{0x4a, 0x68},
};

static struct regval_list sensor_brightness_neg2_regs[] = {
	{0x03, 0x10},
	{0x4a, 0x70},
};

static struct regval_list sensor_brightness_neg1_regs[] = {
	{0x03, 0x10},
	{0x4a, 0x78},
};

static struct regval_list sensor_brightness_zero_regs[] = {
	{0x03, 0x10},
	{0x4a, 0x80},
};

static struct regval_list sensor_brightness_pos1_regs[] = {
	{0x03, 0x10},
	{0x4a, 0x88},
};

static struct regval_list sensor_brightness_pos2_regs[] = {
	{0x03, 0x10},
	{0x4a, 0x90},
};

static struct regval_list sensor_brightness_pos3_regs[] = {
	{0x03, 0x10},
	{0x4a, 0x98},
};

static struct regval_list sensor_brightness_pos4_regs[] = {
	{0x03, 0x10},
	{0x4a, 0xa0},
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
	{0x03},{0x10},
	{0x48},{0x44},
};

static struct regval_list sensor_contrast_neg3_regs[] = {
	{0x03},{0x10},
	{0x48},{0x54},
};

static struct regval_list sensor_contrast_neg2_regs[] = {
	{0x03},{0x10},
	{0x48},{0x64},
};

static struct regval_list sensor_contrast_neg1_regs[] = {
	{0x03},{0x10},
	{0x48},{0x74},
};

static struct regval_list sensor_contrast_zero_regs[] = {
	{0x03},{0x10},
	{0x48},{0x88},
};

static struct regval_list sensor_contrast_pos1_regs[] = {
	{0x03},{0x10},
	{0x48},{0x94},
};

static struct regval_list sensor_contrast_pos2_regs[] = {
	{0x03},{0x10},
	{0x48},{0xa4},
};

static struct regval_list sensor_contrast_pos3_regs[] = {
	{0x03},{0x10},
	{0x48},{0xb4},
};

static struct regval_list sensor_contrast_pos4_regs[] = {
	{0x03},{0x10},
	{0x48},{0xc4},
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
	{0x03},{0x10},
	{0x61},{0x50},
	{0x62},{0x50},
};

static struct regval_list sensor_saturation_neg3_regs[] = {
	{0x03},{0x10},
	{0x61},{0x60},
	{0x62},{0x60},
};

static struct regval_list sensor_saturation_neg2_regs[] = {
	{0x03},{0x10},
	{0x61},{0x70},
	{0x62},{0x70},
};

static struct regval_list sensor_saturation_neg1_regs[] = {
	{0x03},{0x10},
	{0x61},{0x80},
	{0x62},{0x80},
};

static struct regval_list sensor_saturation_zero_regs[] = {
	{0x03},{0x10},
	{0x61},{0x80},
	{0x62},{0x80},
};

static struct regval_list sensor_saturation_pos1_regs[] = {
	{0x03},{0x10},
	{0x61},{0xa0},
	{0x62},{0xa0},
};

static struct regval_list sensor_saturation_pos2_regs[] = {
	{0x03},{0x10},
	{0x61},{0xb0},
	{0x62},{0xb0},
};

static struct regval_list sensor_saturation_pos3_regs[] = {
	{0x03},{0x10},
	{0x61},{0xc0},
	{0x62},{0xc0},
};

static struct regval_list sensor_saturation_pos4_regs[] = {
	{0x03},{0x10},
	{0x61},{0xd0},
	{0x62},{0xd0},
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
	{0x03,0x10},
	{0x40,0xe0},

};

static struct regval_list sensor_ev_neg3_regs[] = {
	{0x03,0x10},
	{0x40,0xc0},

};

static struct regval_list sensor_ev_neg2_regs[] = {
	{0x03,0x10},
	{0x40,0xa0},

};

static struct regval_list sensor_ev_neg1_regs[] = {
	{0x03,0x10},
	{0x40,0x90},

};

static struct regval_list sensor_ev_zero_regs[] = {
	{0x03,0x10},
	{0x40,0x80},

};

static struct regval_list sensor_ev_pos1_regs[] = {
	{0x03,0x10},
	{0x40,0x10},

};

static struct regval_list sensor_ev_pos2_regs[] = {
	{0x03,0x10},
	{0x40,0x20},

};

static struct regval_list sensor_ev_pos3_regs[] = {
	{0x03,0x10},
	{0x40,0x30},

};

static struct regval_list sensor_ev_pos4_regs[] = {
	{0x03,0x10},
	{0x40,0x40},

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
	{0x03},{0x10},//PAGEMODE 0x10
	{0x10},{0x03},	//YCbYCr
};


static struct regval_list sensor_fmt_yuv422_yvyu[] = {
	{0x03},{0x10},//PAGEMODE 0x10
	{0x10},{0x02},	//YCrYCb
};

static struct regval_list sensor_fmt_yuv422_vyuy[] = {
	{0x03},{0x10},//PAGEMODE 0x10
	{0x10},{0x00},	//CrYCbY
};

static struct regval_list sensor_fmt_yuv422_uyvy[] = {
	{0x03},{0x10},//PAGEMODE 0x10
	{0x10},{0x01},	//CbYCrY
};

//static struct regval_list sensor_fmt_raw[] = {
//	
//};

static int sensor_g_hflip(struct v4l2_subdev *sd, __s32 *value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	data_type val;
	
	ret = sensor_write(sd, 0x03, 0x00);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_g_hflip!\n");
		return ret;
	}
	
	ret = sensor_read(sd, 0x11, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_g_hflip!\n");
		return ret;
	}
	
	val &= (1<<0);
	val = val>>0;		//0x11 bit0 is hflip enable
	
	*value = val;
	info->hflip = val;
	return 0;
}

static int sensor_s_hflip(struct v4l2_subdev *sd, int value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	data_type val;
	
	ret = sensor_write(sd, 0x03, 0x00);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_hflip!\n");
		return ret;
	}
	
	ret = sensor_read(sd, 0x11, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_s_hflip!\n");
		return ret;
	}
	
	switch(value) {
	case 0:
		val &= 0xfe;
		break;
	case 1:
		val |= 0x01;
		break;
	default:
			return -EINVAL;
	}	
	ret = sensor_write(sd, 0x11, val);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_hflip!\n");
		return ret;
	}
	msleep(100);
	info->hflip = value;
	return 0;
}

static int sensor_g_vflip(struct v4l2_subdev *sd, __s32 *value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	data_type val;
	
	ret = sensor_write(sd, 0x03, 0x00);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_g_vflip!\n");
		return ret;
	}
	
	ret = sensor_read(sd, 0x11, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_g_vflip!\n");
		return ret;
	}
	
	val &= (1<<1);
	val = val>>1;		//0x11 bit1 is vflip enable
	
	*value = val;
	info->hflip = val;
	return 0;
}

static int sensor_s_vflip(struct v4l2_subdev *sd, int value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	data_type val;
	
	ret = sensor_write(sd, 0x03, 0x00);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_vflip!\n");
		return ret;
	}
	
	ret = sensor_read(sd, 0x11, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_s_vflip!\n");
		return ret;
	}
	
	switch(value) {
	case 0:
		val &= 0xfd;
		break;
	case 1:
		val |= 0x02;
		break;
	default:
			return -EINVAL;
	}	
	ret = sensor_write(sd, 0x11, val);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_g_vflip!\n");
		return ret;
	}
	msleep(100);
	info->hflip = value;
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
	
	ret = sensor_write(sd, 0x03, 0x00);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_g_autoexp!\n");
		return ret;
	}
	
	ret = sensor_read(sd, 0x10, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_g_autoexp!\n");
		return ret;
	}
	
	val &= 0x80;
	if (val == 0x80) {
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
	
	ret = sensor_write(sd, 0x03, 0x20);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_autoexp!\n");
		return ret;
	}
	
	ret = sensor_read(sd, 0x10, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_s_autoexp!\n");
		return ret;
	}
	
	switch (value) {
		case V4L2_EXPOSURE_AUTO:
		  val |= 0x80;
			break;
		case V4L2_EXPOSURE_MANUAL:
			val &= 0x7f;
			break;
		case V4L2_EXPOSURE_SHUTTER_PRIORITY:
			return -EINVAL;    
		case V4L2_EXPOSURE_APERTURE_PRIORITY:
			return -EINVAL;
		default:
			return -EINVAL;
	}
		
	ret = sensor_write(sd, 0x10, val);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_autoexp!\n");
		return ret;
	}
	usleep_range(10000,12000);
	info->autoexp = value;
	
	return 0;
}

static int sensor_g_autowb(struct v4l2_subdev *sd, int *value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	data_type val;
	
	ret = sensor_write(sd, 0x03, 0x22);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_g_autowb!\n");
		return ret;
	}
	
	ret = sensor_read(sd, 0x10, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_g_autowb!\n");
		return ret;
	}
	
	val &= (1<<7);
	val = val>>7;		//0x10 bit7 is awb enable
		
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
	
	ret = sensor_read(sd, 0x10, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_s_autowb!\n");
		return ret;
	}
	
	switch(value) {
	case 0:
		val &= 0x7f;
		break;
	case 1:
		val |= 0x80;
		break;
	default:
		break;
	}	
	ret = sensor_write(sd, 0x10, val);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_autowb!\n");
		return ret;
	}
	usleep_range(10000,12000);
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
	cci_lock(sd);
	switch(on)
	{
		case CSI_SUBDEV_STBY_ON:
			vfe_dev_dbg("CSI_SUBDEV_STBY_ON\n");
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			usleep_range(10000,12000);
			vfe_set_mclk(sd,OFF);
			usleep_range(10000,12000);
			break;
		case CSI_SUBDEV_STBY_OFF:
			vfe_dev_dbg("CSI_SUBDEV_STBY_OFF\n");
			vfe_set_mclk(sd,ON);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
			usleep_range(10000,12000);
			break;
		case CSI_SUBDEV_PWR_ON:
			vfe_dev_dbg("CSI_SUBDEV_PWR_ON\n");
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
			usleep_range(10000,12000);
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			msleep(100);
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);
			usleep_range(10000,12000);
			break;
		case CSI_SUBDEV_PWR_OFF:
			vfe_dev_dbg("CSI_SUBDEV_PWR_OFF\n");
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,POWER_EN,CSI_GPIO_LOW);
			vfe_set_pmu_channel(sd,AFVDD,OFF);
			vfe_set_pmu_channel(sd,DVDD,OFF);
			vfe_set_pmu_channel(sd,AVDD,OFF);
			vfe_set_pmu_channel(sd,IOVDD,OFF);  
			usleep_range(10000,12000);
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
	int ret;
	data_type val;
	
	ret = sensor_write(sd, 0x03, 0x00);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_detect!\n");
		return ret;
	}
	
	ret = sensor_read(sd, 0x04, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_detect!\n");
		return ret;
	}

	if(val != 0xc4)
		return -ENODEV;
	
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
//	{
//		.desc		= "Raw RGB Bayer",
//		.mbus_code	= V4L2_MBUS_FMT_SBGGR8_1X8,//linux-3.0
//		.regs 		= sensor_fmt_raw,
//		.regs_size = ARRAY_SIZE(sensor_fmt_raw),
//		.bpp		= 1
//	},
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
	    .hoffset    = 0,
	    .voffset    = 0,
		.regs 			= sensor_uxga_regs,
		.regs_size	= ARRAY_SIZE(sensor_uxga_regs),
		.set_size		= NULL,
	},
	/* 720p */
	{
		.width			= HD720_WIDTH,
		.height			= HD720_HEIGHT,
	    .hoffset    = 0,
	    .voffset    = 0,
		.regs				= sensor_hd720_regs,
		.regs_size	= ARRAY_SIZE(sensor_hd720_regs),
		.set_size		= NULL,
	},
	/* SVGA */
	{
		.width			= SVGA_WIDTH,
		.height			= SVGA_HEIGHT,
	    .hoffset    = 0,
	    .voffset    = 0,
		.regs				= sensor_svga_regs,
		.regs_size	= ARRAY_SIZE(sensor_svga_regs),
		.set_size		= NULL,
	},
	/* VGA */
	{
		.width			= VGA_WIDTH,
		.height			= VGA_HEIGHT,
	    .hoffset    = 0,
	    .voffset    = 0,
		.regs				= sensor_vga_regs,
		.regs_size	= ARRAY_SIZE(sensor_vga_regs),
		.set_size		= NULL,
	},
	/* QVGA */
	{
		.width		= QVGA_WIDTH,
		.height		= QVGA_HEIGHT,
	    .hoffset    = 0,
	    .voffset    = 0,
		.regs 		= sensor_qvga_regs,
		.regs_size	= ARRAY_SIZE(sensor_qvga_regs),
		.set_size		= NULL,
	}
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
	
	info->fmt = sensor_fmt;
	info->width = wsize->width;
	info->height = wsize->height;
	msleep(500);//�����ӳٽ�����շ�������
	return 0;
}

/*
 * Implement G/S_PARM.  There is a "high quality" mode we could try
 * to do someday; for now, we just do the frame rate tweak.
 */
static int sensor_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *parms)
{
	struct v4l2_captureparm *cp = &parms->parm.capture;
	//struct sensor_info *info = to_state(sd);

	if (parms->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	memset(cp, 0, sizeof(struct v4l2_captureparm));
	cp->capability = V4L2_CAP_TIMEPERFRAME;
	cp->timeperframe.numerator = 1;
	cp->timeperframe.denominator = SENSOR_FRAME_RATE;
	
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
		return sensor_g_colorfx(sd, &ctrl->value);
	case V4L2_CID_FLASH_LED_MODE:
		return sensor_g_flash_mode(sd, &ctrl->value);
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

	if (qc.type == V4L2_CTRL_TYPE_MENU ||
		qc.type == V4L2_CTRL_TYPE_INTEGER ||
		qc.type == V4L2_CTRL_TYPE_BOOLEAN)
	{
		if (ctrl->value < qc.minimum || ctrl->value > qc.maximum) {
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
		return sensor_s_autoexp(sd, (enum v4l2_exposure_auto_type) ctrl->value);
    case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:
  		return sensor_s_wb(sd, (enum v4l2_auto_n_preset_white_balance) ctrl->value); 
    case V4L2_CID_AUTO_WHITE_BALANCE:
		return sensor_s_autowb(sd, ctrl->value);
    case V4L2_CID_COLORFX:
		return sensor_s_colorfx(sd, (enum v4l2_colorfx) ctrl->value);
    case V4L2_CID_FLASH_LED_MODE:
		return sensor_s_flash_mode(sd, (enum v4l2_flash_led_mode) ctrl->value);
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
	.addr_width = CCI_BITS_8,
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
