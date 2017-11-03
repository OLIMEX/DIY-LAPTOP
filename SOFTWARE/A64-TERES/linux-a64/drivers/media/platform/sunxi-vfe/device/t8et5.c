/*
 * A V4L2 driver for t8et5 cameras.
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
MODULE_DESCRIPTION("A low-level driver for t8et5 sensors");
MODULE_LICENSE("GPL");

//for internel driver debug
#define DEV_DBG_EN      0 
#if(DEV_DBG_EN == 1)    
#define vfe_dev_dbg(x,arg...) printk("[t8et5]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...) 
#endif
#define vfe_dev_err(x,arg...) printk("[t8et5]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[t8et5]"x,##arg)

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
#define CLK_POL           V4L2_MBUS_PCLK_SAMPLE_FALLING
#define V4L2_IDENT_SENSOR 0x8e05




/*
 * Our nominal (default) frame rate.
 */
#ifdef FPGA
#define SENSOR_FRAME_RATE 15
#else
#define SENSOR_FRAME_RATE 30
#endif

/*
 * The t8et5 sits on i2c with ID 0x78
 */
#define I2C_ADDR (0x3c<<1)
#define SENSOR_NAME "t8et5"
static struct v4l2_subdev *glb_sd;
static int t8et5_sensor_vts = 0;

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
	{0x00,0x01},//MODE_SEL / VREVON / HREVON / SWRST / GRHOLD /(2)/(1)/OUT_FORMAT;
	{0xFE,0x20},//enable modified regs
	{0x01,0x02},//INTGTIM[15:8];
	{0x02,0x70},//INTGTIM[7:0];
	//{0x03,0x00},//(7)/(6)/(5)/(4)/ ANAGAIN[11:8];
	//{0x04,0x9A},//ANAGAIN[7:0];
	{0x05,0x01},//(7)/(6)/(5)/(4)/(3)/(2)/ MWBGAINGR[9:8];
	{0x06,0x00},//MWBGAINGR[7:0];
	{0x07,0x01},//(7)/(6)/(5)/(4)/(3)/(2)/ MWBGAINR[9:8];
	{0x08,0x00},//MWBGAINR[7:0];
	{0x09,0x01},//(7)/(6)/(5)/(4)/(3)/(2)/ MWBGAINB[9:8];
	{0x0A,0x00},//MWBGAINB[7:0];
	{0x0B,0x01},//(7)/(6)/(5)/(4)/(3)/(2)/ MWBGAINBR[9:8];
	{0x0C,0x00},//MWBGAINBR[7:0];
	{0x0D,0x40},//(7)/ PIXCKDIV[2:0] /(4)/ SYSCKDIV[2:0];
	{0x0E,0x01},//(7)/(6)/(5)/(4)/(3)/(2)/ PRECKDIV[1:0];
	{0x0F,0x00},//(7)/(6)/(5)/(4)/(3)/ OPSYSDIV[2:0];
	{0x10,0x00},//(7)/(6)/(5)/(4)/(3)/(2)/(1) PLLMULT[8];
	{0x11,0x42},//PLLMULT[7:0];
	{0x12,0x07},//TTLLINE[15:8];
	{0x13,0xDC},//TTLLINE[7:0];
	{0x14,0x0C},//(7)/(6)/ TTLDOT[13:8];
	{0x15,0xD0},//TTLDOT[7:0];
	{0x16,0x00},//--;
	{0x17,0x00},//--;
	{0x18,0x00},//--;
	{0x19,0x00},//--;
	{0x1A,0x00},//--;
	{0x1B,0x00},//--;
	{0x1C,0x00},//--;
	{0x1D,0x00},//--;
	{0x1E,0x0A},//(7)/(6)/(5)/(4)/ HOUTSIZ[11:8];
	{0x1F,0x30},//HOUTSIZ[7:0];
	{0x20,0x07},//(7)/(6)/(5)/(4)/(3)/ VOUTSIZ[10:8];
	{0x21,0xA8},//VOUTSIZ[7:0];
	{0x22,0x00},//HANABIN /(6)/(5)/(4)/(3)/ VMONI[2:0];
	{0x23,0x00},//(7)/(6)/(5)/(4)/(3)/(2)/ SCALMODE[1:0];
	{0x24,0x00},//MVALUE[7:0];
	{0x25,0x00},//--;
	{0x26,0x02},//TPAT_SEL[2:0]/(4)/(3)/(2)/ TPAT_R[9:8];
	{0x27,0xC0},//TPAT_R[7:0];
	{0x28,0x02},//(7)/(6)/(5)/(4)/(3)/(2)/ TPAT_GR[9:8];
	{0x29,0xC0},//TPAT_GR[7:0];
	{0x2A,0x02},//(7)/(6)/(5)/(4)/(3)/(2)/ TPAT_B[9:8];
	{0x2B,0xC0},//TPAT_B[7:0];
	{0x2C,0x02},//(7)/(6)/(5)/(4)/(3)/(2)/ TPAT_GB[9:8];
	{0x2D,0xC0},//TPAT_GB[7:0];
	{0x2E,0x00},//(7)/(6)/(5)/(4)/ CURHW[11:8];
	{0x2F,0x00},//CURHW[7:0];
	{0x30,0x00},//(7)/(6)/(5)/(4)/ CURHST[11:8];
	{0x31,0x00},//CURHST[7:0];
	{0x32,0x00},//(7)/(6)/(5)/(4)/(3)/ CORVW[10:8];
	{0x33,0x00},//CORVW[7:0];
	{0x34,0x00},//(7)/(6)/(5)/(4)/(3)/ CURVST[10:8];
	{0x35,0x00},//CURVST[7:0];
	{0x36,0x00},//(7)/(6)/(5)/(4)/ ANA_GA_MIN[11:8];
	{0x37,0x1A},//ANA_GA_MIN[7:0];
	{0x38,0x01},//(7)/(6)/(5)/(4)/ ANA_GA_MAX[11:8];
	{0x39,0x60},//ANA_GA_MAX[7:0];
	{0x3A,0x28},//VerNUM[7:5];
	{0x3B,0x00},//--;
	{0x3C,0x00},//--;
	{0x3D,0x00},//--;
	{0x3E,0x00},//--;
	{0x3F,0x00},//--;
	{0x40,0xCA},//--;
	{0x41,0x63},//--;
	{0x42,0x35},//--;
	{0x43,0x16},//--;
	{0x44,0x23},//--;
	{0x45,0x0A},//--;
	{0x46,0x20},//--;
	{0x47,0x0C},//--;
	{0x48,0x03},//--;
	{0x49,0x00},//--;
	{0x4A,0x00},//--;
	{0x4B,0x00},//--;
	{0x4C,0x00},//--;
	{0x4D,0x00},//--;
	{0x4E,0x00},//--;
	{0x4F,0x00},//--;
	{0x50,0x04},//--;
	{0x51,0x84},//--;
	{0x52,0x34},//--;
	{0x53,0x14},//--;
	{0x54,0x34},//--;
	{0x55,0x68},//--;
	{0x56,0x05},//--;
	{0x57,0x40},//--;
	{0x58,0x00},//--;
	{0x59,0x34},//--;
	{0x5A,0x66},//--;
	{0x5B,0x00},//--;
	{0x5C,0x00},//--;
	{0x5D,0x12},//--;
	{0x5E,0x46},//--;
	{0x5F,0x69},//--;
	{0x60,0x78},//--;
	{0x61,0x05},//--;
	{0x62,0x40},//--;
	{0x63,0xC2},//--;
	{0x64,0x82},//--;
	{0x65,0xAA},//--;
	{0x66,0x00},//--;
	{0x67,0x00},//--;
	{0x68,0x20},//--;
	{0x69,0x08},//--;
	{0x6a,0x40},

	{0x6B,0x20},//--;
	{0x6C,0xc4},//--;
	{0x6D,0x00},//--;
	{0x6E,0x00},//--;
	{0x6F,0x00},//--;
	{0x70,0x30},//--;
	{0x71,0x80},//--;
	{0x72,0x60},//--;
	{0x73,0xC8},//WBPCMODE/BBPCMOED/(5)/(4)/(3)/(2)/(1)/ABPCTH;
	{0x74,0x03},//BBPCLV[7:0];
	{0x75,0x03},//WBPCLV[7:0];
	{0x76,0x00},//--;
	{0x77,0x3A},//--;
	{0x78,0x00},//--;
	{0x79,0x00},//--;
	{0x7A,0x27},//--;
	{0x7B,0x00},//--;
	{0x7C,0x1F},//--;
	//{0x7D,0x8F},//LSSCON ///(4)/;
	{0x7D,0x0F},

	{0x7E,0x00},//LSHGA[3:0] / LSVGA[3:0];
	{0x7F,0x00},//LSHOFS[7:0] // H Center position;
	{0x80,0x00},//LSVOFS[7:0] // V Center position;
	{0x81,0x0A},//LSALGR[7:0]// Up Left GR ### 1st order ###;
	{0x82,0x0A},//LSALGB[7:0]//         GB;
	{0x83,0x0A},//LSALR[7:0] //         R;
	{0x84,0x0A},//LSALB[7:0] //         B;
	{0x85,0x00},//LSARGR[7:0]// Up Right GR;
	{0x86,0x00},//LSARGB[7:0]//          GB;
	{0x87,0x00},//LSARR[7:0] //          R;
	{0x88,0x06},//LSARB[7:0] //          B;
	{0x89,0x06},//LSAUGR[7:0]// Bottom Left GR;
	{0x8A,0x06},//LSAUGB[7:0]//             GB;
	{0x8B,0x0A},//LSAUR[7:0] //             R;
	{0x8C,0x0A},//LSAUB[7:0] //             B;
	{0x8D,0x06},//LSADGR[7:0]// Bottom Right GR;
	{0x8E,0x06},//LSADGB[7:0]//              GB;
	{0x8F,0x0A},//LSADR[7:0] //              R;
	{0x90,0x0A},//LSADB[7:0] //              B;
	{0x91,0x32},//LSBLGR[7:0]// Left GR ### 2st order ###;
	{0x92,0x32},//LSBLGB[7:0]//      GB;
	{0x93,0x3A},//LSBLR[7:0] //      R;
	{0x94,0x26},//LSBLB[7:0] //      B;
	{0x95,0x31},//LSBRGR[7:0]// Right GR;
	{0x96,0x31},//LSBRGB[7:0]//       GB;
	{0x97,0x3E},//LSBRR[7:0] //        R;
	{0x98,0x2C},//LSBRB[7:0] //        B;
	{0x99,0x29},//LSCUGR[7:0]// Left GR;
	{0x9A,0x29},//LSCUGB[7:0]//      GB;
	{0x9B,0x2F},//LSCUR[7:0] //      R;
	{0x9C,0x28},//LSCUB[7:0] //      B;
	{0x9D,0x25},//LSCDGR[7:0]// Right GR;
	{0x9E,0x25},//LSCDGB[7:0]//       GB;
	{0x9F,0x28},//LSCDR[7:0] //       R;
	{0xA0,0x1F},//LSCDB[7:0] //       B;
	{0xA1,0x00},//LSDLGR[7:0]// Left GR ### 4st order ###;
	{0xA2,0x00},//LSDLGB[7:0]//      GB;
	{0xA3,0x00},//LSDLR[7:0] //      R;
	{0xA4,0x00},//LSDLB[7:0] //      B;
	{0xA5,0x00},//LSDRGR[7:0]// Right GR;
	{0xA6,0x00},//LSDRGB[7:0]//       GB;
	{0xA7,0x00},//LSDRR[7:0] //        R;
	{0xA8,0x00},//LSDRB[7:0] //        B;
	{0xA9,0x00},//LSEUGR[7:0]// Left GR;
	{0xAA,0x00},//LSEUGB[7:0]//      GB;
	{0xAB,0x2C},//LSEUR[7:0] //      R;
	{0xAC,0x00},//LSEUB[7:0] //      B;
	{0xAD,0x02},//LSEDGR[7:0]// Right GR;
	{0xAE,0x02},//LSEDGB[7:0]//       GB;
	{0xAF,0x28},//LSEDR[7:0] //       R;
	{0xB0,0x28},//LSEDB[7:0] //       B;
	{0xB1,0x00},//--;
	{0xB2,0x00},//--;
	{0xB3,0x00},//--;
	{0xB4,0xFF},//--;
	{0xB5,0xFF},//--;
	{0xB6,0xCE},//--;
	{0xB7,0x19},//AGMAX;(AG*0x1A-15)/16//org 0xCE~=127X(0x67~=32X 0x19~=16X recommanded)
	//{0xB7,0x0c},

	//{0xB7,0x0c},

	{0xB8,0x1a},//--;0x01
	{0xB9,0x00},//--;
	{0xBA,0x00},//--;
	{0xBB,0x00},//--;
	{0xBC,0x00},//--;
	{0xBD,0x00},//--;
	{0xBE,0x00},//--;
	{0xBF,0x00},//--;
	{0xC0,0x80},//--;
	{0xC1,0x00},//--;
	{0xC2,0x44},//(7)/(6)/(5)/(4)/(3)/ MIPI1L /(1)/(0);
	{0xC3,0x04},//--;
	{0xC4,0x03},//(7)/(6)/(5)/(4)/(3)/(2)/ PARALLEL_OUT_SW[1:0];
	{0xC5,0x78},//--;
	{0xC6,0x95},//--;
	{0xC7,0x55},//--;
	{0xC8,0xD6},//--;
	{0xC9,0xA7},//--;
	{0xCA,0x04},//(7)/(6)/(5)/(4)/(3)/ PARALLEL_MODE /(1)/(0);
	{0xCB,0x00},//--;
	{0xCC,0x11},//FS_CODE[7:0];
	{0xCD,0x44},//FE_CODE[7:0];
	{0xCE,0x22},//LS_CODE[7:0];
	{0xCF,0x33},//LE_CODE[7:0];
	{0xD0,0x30},//--;
	{0xD1,0x00},//--;
	{0xD2,0x01},//--;
	{0xD3,0x00},//--;
	{0xD4,0x00},//--;
	{0xD5,0x00},//--;
	{0xD6,0x00},//--;
	{0xD7,0x10},//--;
	{0xD8,0xFF},//--;
	{0xD9,0x00},//--;
	{0xDA,0x10},//--;
	{0xDB,0xFF},//--;
	{0xDC,0x81},//--;
	{0xDD,0x00},//--;
	{0xDE,0x00},//--;
	{0xDF,0x00},//--;
	{0xE0,0x01},//--;
	{0xE1,0x00},//--;
	{0xE2,0x00},//--;
	{0xE3,0x40},//--;
	{0xE4,0x03},//--;
	{0xE5,0x81},//--;
	{0xE6,0x13},//--;
	{0xE7,0xC9},//--;
	{0xE8,0x12},//--;
	{0xE9,0x99},//--;
	//{0xEA,0x00},//--;
	//{0xEB,0x00},//--;
	//{0xEC,0x00},//--;
	//{0xED,0x00},//--;
	//{0xEE,0x00},//--;
	//{0xEF,0x00},//--;
	{0xF0,0x90},//--;
	{0xF1,0x00},//--;
	{0xF2,0x54},//--;
	{0xF3,0x00},//--;
	{0xF4,0x00},//--;
	{0xF5,0x00},//--;
	{0xF6,0x80},//--;
	{0xF7,0x80},//--;
	{0xF8,0x00},//--;
	{0xF9,0x00},//--;
	{0xFA,0x00},//--;
	{0xFB,0x00},//--;
	{0xFC,0x00},//--;
	{0xFD,0x00},//--;
	{0xFE,0x10},//--;ANR
	{0xFF,0x00},//--;
	{0x00,0x81},//MODE_SEL / VREVON / HREVON / SWRST / GRHOLD /(2)/(1)/OUT_FORMAT;


	{0xFE,0x20},//enable modified regs
	{0x73,0xc9},//WBPCMODE/BBPCMOED/(5)/(4)/(3)/(2)/(1)/ABPCTH;manual BPC
	{0x74,0x5c},//BBPCLV[7:0];//0x07
	{0x75,0x07},//WBPCLV[7:0];
	{0xF8,0x55},//--;WBPLV1[3:0]/WBPLV2[3:0]
	{0xF9,0x55},//--;WBPLV1[3:0]/WBPLV2[3:0]
	{0xFD,0x68},//--;AN_CNTLV[7:0]
	{0xFF,0x80},//--;WHT_AG[7:0]

	{0xEA,0x00},//--;
	{0xEB,0x00},//--;
	{0xEC,0xee},//--;
	{0xED,0x00},//--;
	{0xFE,0x20},

	{0xEE,0x80},//--;
	//{0xEF,0x28},//--;
	{0xEF,0x28},

	//{0xB4,0xFF},//--;
	//{0xB5,0xFF},//--;
	//{0xB6,0xCE},//--;

	{0xFE,0x1A},//enable NR
	{0xFf,0x80},

	//{0x00,0x81},//MODE_SEL / VREVON / HREVON / SWRST / GRHOLD /(2)/(1)/OUT_FORMAT;
};

//for capture                                                                         
static struct regval_list sensor_qsxga_regs[] = { //qsxga: 2592*1936@15fps 99MHz
	{0x0D,0x40},//(7)/ PIXCKDIV[2:0] /(4)/ SYSCKDIV[2:0];
	{0x0E,0x01},//(7)/(6)/(5)/(4)/(3)/(2)/ PRECKDIV[1:0];
	{0x0F,0x00},//(7)/(6)/(5)/(4)/(3)/ OPSYSDIV[2:0];
	{0x10,0x00},//(7)/(6)/(5)/(4)/(3)/(2)/(1) PLLMULT[8];
	{0x11,0x42},//PLLMULT[7:0];

	{0x12,0x07},//TTLLINE[15:8];
	{0x13,0xD0},//TTLLINE[7:0];
	{0x14,0x0C},//(7)/(6)/ TTLDOT[13:8];
	{0x15,0xe4},//TTLDOT[7:0];

	{0x1E,0x0A},//(7)/(6)/(5)/(4)/ HOUTSIZ[11:8];
	{0x1F,0x30},//HOUTSIZ[7:0];
	{0x20,0x07},//(7)/(6)/(5)/(4)/(3)/ VOUTSIZ[10:8];
	{0x21,0xA8},//VOUTSIZ[7:0];
	{0x22,0x00},//HANABIN /(6)/(5)/(4)/(3)/ VMONI[2:0];
	{0x23,0x00},//(7)/(6)/(5)/(4)/(3)/(2)/ SCALMODE[1:0];
	{0x24,0x00},//MVALUE[7:0];

};

static struct regval_list sensor_sxga_regs[] = { //SXGA: 1306*980@30fps //63MHz pclk
	//63MHz H bining 1/2 v bining 1/2, resolution lost
	{0x0D,0x40},//(7)/ PIXCKDIV[2:0] /(4)/ SYSCKDIV[2:0];
	{0x0E,0x02},//(7)/(6)/(5)/(4)/(3)/(2)/ PRECKDIV[1:0];
	{0x0F,0x00},//(7)/(6)/(5)/(4)/(3)/ OPSYSDIV[2:0];
	{0x10,0x00},//(7)/(6)/(5)/(4)/(3)/(2)/(1) PLLMULT[8];
	{0x11,0x54},//PLLMULT[7:0];

	{0x12,0x03},//TTLLINE[15:8];
	{0x13,0xe8},//TTLLINE[7:0];
	{0x14,0x08},//(7)/(6)/ TTLDOT[13:8];
	{0x15,0x34},//TTLDOT[7:0];

	{0x1E,0x05},//(7)/(6)/(5)/(4)/ HOUTSIZ[11:8];
	{0x1F,0x18},//HOUTSIZ[7:0];
	{0x20,0x03},//(7)/(6)/(5)/(4)/(3)/ VOUTSIZ[10:8];
	{0x21,0xD4},//VOUTSIZ[7:0];
	{0x22,0x81},//HANABIN /(6)/(5)/(4)/(3)/ VMONI[2:0];
	{0x23,0x02},//(7)/(6)/(5)/(4)/(3)/(2)/ SCALMODE[1:0];0x02
	{0x24,0x10},//MVALUE[7:0];0x10
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

static int sensor_s_exp(struct v4l2_subdev *sd, unsigned int exp_val)
{
	unsigned char explow,exphigh;
	struct sensor_info *info = to_state(sd);

	vfe_dev_dbg("sensor_set_exposure = %d\n", exp_val);
	if(exp_val>0xffffff)
		exp_val=0xfffff0;
	if(exp_val<16)
		exp_val=16;
	
	exp_val=(exp_val+8)>>4;//rounding to 1
	
	vfe_dev_dbg("sensor_set_exposure real= %d\n", exp_val);
  
    exphigh = (unsigned char) ( (0xff00&exp_val)>>8);
    explow  = (unsigned char) ( (0x00ff&exp_val) );
	
	sensor_write(sd, 0x02, explow);
	sensor_write(sd, 0x01, exphigh);	
	
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
	unsigned int gain_tmp;
	
	if(gain_val<16)
	  gain_val=16;
	
	gain_tmp=((gain_val)*26)>>4;//round to 1/26 step
	
	gainlow=(unsigned char)(gain_tmp&0xff);
	gainhigh=(unsigned char)((gain_tmp>>8)&0xff);
	
	sensor_write(sd, 0x04, gainlow);
	sensor_write(sd, 0x03, gainhigh);
	
	//printk("t8et5 sensor_set_gain = %d, Done!\n", gain_val);
	info->gain = gain_val;
	
	return 0;
}
static int sensor_s_exp_gain(struct v4l2_subdev *sd, struct sensor_exp_gain *exp_gain)
{
	int exp_val, gain_val,shutter,frame_length = 0;  
	unsigned char explow=0,exphigh=0;
	data_type rdval;
	unsigned char gainlow=0,gainhigh=0,gain_tmp;  
	struct sensor_info *info = to_state(sd);	

	//return -EINVAL;
	exp_val = exp_gain->exp_val;
	gain_val = exp_gain->gain_val;

	exp_val=exp_val>>4;//rounding to 1
	shutter = exp_val;

	if(info->exp == exp_val && gain_val == info->gain)
		return 0;
	vfe_dev_dbg("sensor_set_exposure real= %d\n", exp_val);

	exphigh = (unsigned char) ( (0xff00&exp_val)>>8);
	explow  = (unsigned char) ( (0x00ff&exp_val) );

	gain_tmp=((gain_val)*26)>>4;//round to 1/26 step

	gainlow=(unsigned char)(gain_tmp&0xff);
	gainhigh=(unsigned char)((gain_tmp>>8)&0xff);
	vfe_dev_dbg("t8et5_sensor_vts =  %d, shutter = %d,%d\n",t8et5_sensor_vts,shutter,shutter  > t8et5_sensor_vts);

	if(shutter > t8et5_sensor_vts-4)
		frame_length = shutter +4;
	else
		frame_length = t8et5_sensor_vts;
	//set frame length;	
	sensor_write(sd, 0x13, (frame_length & 0xff));
	sensor_write(sd, 0x12, (frame_length >> 8));
	
	sensor_read(sd, 0x00, &rdval);        
	sensor_write(sd, 0x00, rdval | 0x08);

	sensor_write(sd, 0x02, explow);
	sensor_write(sd, 0x01, exphigh); 

	sensor_write(sd, 0x04, gainlow);
	sensor_write(sd, 0x03, gainhigh);
	sensor_write(sd, 0x00, rdval & (0xf7));
	info->gain = gain_val;
	info->exp = exp_val;
	return 0;
}


static int sensor_s_sw_stby(struct v4l2_subdev *sd, int on_off)
{
	int ret;
	data_type rdval;
	
	ret=sensor_read(sd, 0x00, &rdval);
	if(ret!=0)
		return ret;
	
	if(on_off==CSI_GPIO_LOW)//sw stby on
	{
		ret=sensor_write(sd, 0x00, rdval&0x7f);
	}
	else//sw stby off
	{
		ret=sensor_write(sd, 0x00, rdval|0x80);
	}
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
			cci_unlock(sd);        
			ret = sensor_s_sw_stby(sd, CSI_GPIO_HIGH);
			if(ret < 0)
				vfe_dev_err("soft stby off falied!\n");
			usleep_range(10000,12000);
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

	LOG_ERR_RET(sensor_read(sd, 0x3a, &rdval))
	vfe_dev_dbg("sensor read id=0x%x\n",rdval);
	if(rdval != 0x28)
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
	info->width = QSXGA_WIDTH;
	info->height = QSXGA_HEIGHT;
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
		.mbus_code	= V4L2_MBUS_FMT_SGRBG10_1X10,
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
	/* qsxga: 2592*1936 */
	{
		.width      = QSXGA_WIDTH,
		.height     = QSXGA_HEIGHT,
		.hoffset    = (2608-QSXGA_WIDTH)/2,//image cropped from 2608*1960
		.voffset    = (1960-QSXGA_HEIGHT)/2,
		.hts        = 3300,//must over 3254, limited by sensor
		.vts        = 2000,
		.pclk       = 99*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 1,
		.intg_max   = 2000<<4,
		.gain_min   = 1<<4,
		.gain_max   = 8<<4,
		.regs       = sensor_qsxga_regs,
		.regs_size  = ARRAY_SIZE(sensor_qsxga_regs),
		.set_size   = NULL,
	},

	/* 1080P */
	{
		.width			= HD1080_WIDTH,
		.height 		= HD1080_HEIGHT,
		.hoffset	  = (2608-HD1080_WIDTH)/2,//image cropped from 2608*1960
		.voffset	  = (1960-HD1080_HEIGHT)/2,
		.hts        = 3300,//must over 3254, limited by sensor
		.vts        = 2000,
		.pclk       = 99*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 1,
		.intg_max   = 2000<<4,
		.gain_min   = 1<<4,
		.gain_max   = 8<<4,
		.regs       = sensor_qsxga_regs,//sensor_1080p_regs
		.regs_size  = ARRAY_SIZE(sensor_qsxga_regs),//sensor_1080p_regs
		.set_size		= NULL,
	},
	/* SXGA */
	{
		.width			= SXGA_WIDTH,
		.height 		= SXGA_HEIGHT,
		.hoffset	  = (1304-SXGA_WIDTH)/2,//image cropped from 1304*980
		.voffset	  = (980-SXGA_HEIGHT)/2,
		.hts        = 2100,//must > 2048, limited by sensor
		//.hts        = 3300,//must > 3254, limited by sensor
		.vts        = 1000,
		.pclk       = 63*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 1,
		.intg_max   = 1000<<4,
		.gain_min   = 1<<4,
		.gain_max   = 8<<4,
		.regs		    = sensor_sxga_regs,
		.regs_size	= ARRAY_SIZE(sensor_sxga_regs),
		.set_size		= NULL,
	},
	/* 720p */
	{
		.width			= HD720_WIDTH,
		.height 		= HD720_HEIGHT,
		.hoffset    = (1304-HD720_WIDTH)/2,//image cropped from 1304*980
		.voffset    = (980-HD720_HEIGHT)/2,
		.hts        = 2100,//must > 2048, limited by sensor
		//.hts        = 3300,//must > 3254, limited by sensor
		.vts        = 1000,
		.pclk       = 63*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 1,
		.intg_max   = 1000<<4,
		.gain_min   = 1<<4,
		.gain_max   = 8<<4,
		.regs			  = sensor_sxga_regs,//sensor_720p_regs
		.regs_size	= ARRAY_SIZE(sensor_sxga_regs),//sensor_720p_regs
		.set_size		= NULL,
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

	sensor_s_sw_stby(sd, CSI_GPIO_LOW);
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
	t8et5_sensor_vts = wsize->vts;
   
	vfe_dev_print("s_fmt set width = %d, height = %d, t8et5_sensor_vts= %d\n",wsize->width,wsize->height,t8et5_sensor_vts);

	if(info->capture_mode == V4L2_MODE_VIDEO)
	{
	//video
	} else {
	//capture image
	}
	info->gain = 0;
	info->exp = 0;

	sensor_s_sw_stby(sd, CSI_GPIO_HIGH);
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
		return v4l2_ctrl_query_fill(qc, 16, 16*16, 1, 16);
	case V4L2_CID_EXPOSURE:
		return v4l2_ctrl_query_fill(qc, 0, 65535*16, 1, 0);
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
	if (ret < 0) 
	{
		return ret;
	}

	if (ctrl->value < qc.minimum || ctrl->value > qc.maximum) 
	{
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

