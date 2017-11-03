/*
 * A V4L2 driver for GS5604 cameras.
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


MODULE_AUTHOR("raymonxiu");
MODULE_DESCRIPTION("A low-level driver for GS5604 sensors");
MODULE_LICENSE("GPL");

#define AF_WIN_NEW_COORD
//for internel driver debug
#define DEV_DBG_EN      0
#if(DEV_DBG_EN == 1)    
#define vprintk(x,arg...) printk("[GS5604]"x,##arg)
#else
#define vprintk(x,arg...) 
#endif
#if(DEV_DBG_EN == 1)
#define vfe_dev_dbg(x,arg...) printk("[GS5604]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...)
#endif

#define vfe_dev_err(x,arg...) printk("[GS5604]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[GS5604]"x,##arg)

#define CAP_BDG 0
#if(CAP_BDG == 1)
#define vfe_dev_cap_dbg(x,arg...) printk("[GS5604_CAP_DBG]"x,##arg)
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
#define VREF_POL          V4L2_MBUS_VSYNC_ACTIVE_HIGH
#define HREF_POL          V4L2_MBUS_HSYNC_ACTIVE_HIGH
#define CLK_POL           V4L2_MBUS_PCLK_SAMPLE_RISING
#define V4L2_IDENT_SENSOR 0x5604

#define SENSOR_NAME "gs5604"
#define regval_list reg_list_w_a16_d16

#define REG_TERM 0xfffe
#define VAL_TERM 0xfe
#define REG_DLY  0xffff

#define CONTINUEOUS_AF

//#define QSXGA_HEIGHT 1944
#define AE_CW 1

#define BYTE 0
#define WORD 1

unsigned int night_mode=0;
unsigned int Nfrms=1;
unsigned int cap_manual_gain=0x10;
#define CAP_GAIN_CAL 0//0--auto limit frames;1--manual fixed gain
#define CAP_MULTI_FRAMES
#ifdef CAP_MULTI_FRAMES
#define MAX_FRM_CAP 4
#else
#define MAX_FRM_CAP 1
#endif


/*
 * Our nominal (default) frame rate.
 */
#define SENSOR_FRAME_RATE 30

/*
 * The GS5604 sits on i2c with ID 0x78
 */
#define I2C_ADDR 0x34

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

static unsigned short af_window_xstart;
static unsigned short af_window_ystart;
static unsigned short af_window_width;
static unsigned short af_window_height;

static inline struct sensor_info *to_state(struct v4l2_subdev *sd)
{
	return container_of(sd, struct sensor_info, sd);
}


/*
 * The default register settings
 *
 */

static struct regval_list sensor_default_regs[] = {
	{BYTE,0x5008,0x00},         
	{BYTE,0x0004,0x01}, 
	
#if 1
	{BYTE,0x0007,0x02},         
	{BYTE,0x0008,0x00},                                                          
	{WORD,0x00c2,0x0200},                                                        
	{BYTE,0x00c4,0x10},         
	{BYTE,0x00c5,0x10},         
	{BYTE,0x00c6,0x50},         
	{BYTE,0x00c7,0x51},         
	{BYTE,0x00c8,0x11},         
	{BYTE,0x00c9,0x50},         
	{BYTE,0x00ca,0x51},         
	{BYTE,0x00cc,0x10},         
	{BYTE,0x00cd,0x10},                 	                                                     
	{WORD,0x018c,0x0000},    
	{WORD,0x018e,0x0000},    
	{WORD,0x0190,0x0000},    
	{WORD,0x0192,0x0000},    
	{WORD,0x0194,0x2700},    
	{WORD,0x0196,0x1500},    
	{WORD,0x6a16,0xc002},    
	{WORD,0x6a18,0xc002},    
	{WORD,0x6a1a,0xe001},    
	{WORD,0x6a1c,0xe000},    
	{WORD,0x6a1e,0x2004},    
	{WORD,0x6a20,0xc002},    
	{WORD,0x0016,0x1000},  
#else
	{BYTE,0x0007,0x00},       
	{BYTE,0x0008,0x00},  
	{WORD,0x00c2,0x0200},                                                        	
	{BYTE,0x00C4,0x10},       
	{BYTE,0x00C5,0x10},       
	{BYTE,0x00C6,0x50},       
	{BYTE,0x00C7,0x51},       
	{BYTE,0x00C8,0x30},       
	{BYTE,0x00C9,0x50},       
	{BYTE,0x00CA,0x51},       
	{BYTE,0x00CC,0x10},       
	{BYTE,0x00CD,0x10},       
	{BYTE,0x6A12,0x10},       
	{BYTE,0x6A13,0x10},       
	{BYTE,0x6A14,0x10},       
	{BYTE,0x6A15,0x10},       
	{WORD,0x018C,0x0000},   
	{WORD,0x018E,0x0000},   
	{WORD,0x0190,0x0000},   
	{WORD,0x0192,0x0000},   
	{WORD,0x0194,0x2700},   
	{WORD,0x0196,0x1500},   
	{WORD,0x6A16,0x4004},   
	{WORD,0x6A18,0xC003},   
	{WORD,0x6A1A,0xE001},   
	{WORD,0x6A1C,0xE000},   
	{WORD,0x6A1E,0x2004},   
	{WORD,0x6A20,0xC002},   
	{WORD,0x0016,0x1000},   	
#endif
	
	{BYTE,0x5008,0x00},
	{WORD,0xED00,0x9191},  
	{BYTE,0xED02,0xD7},    
	{BYTE,0xED03,0x69},    
	{BYTE,0xED04,0xD0},    
	{BYTE,0xED05,0x73},    
	{BYTE,0xED06,0x04},    
	{BYTE,0xED07,0xD4},    
	{BYTE,0xED08,0x9B},    
	{BYTE,0xED09,0xF7},    
	{BYTE,0xED0A,0xC0},    
	{BYTE,0xED0B,0x86},    
	{BYTE,0xED0C,0x3C},    
	{BYTE,0xED0D,0xAD},    
	{BYTE,0xED0E,0x09},    
	{BYTE,0xED0F,0x8F},    
	{BYTE,0xED10,0x6B},    
	{BYTE,0xED11,0xC6},    
	{BYTE,0xED12,0x93},    
	{BYTE,0xED13,0x1B},    
	{BYTE,0xED14,0xF9},    
	{BYTE,0xED15,0x24},    
	{BYTE,0xED16,0x07},    
	{BYTE,0xED17,0x41},    
	{BYTE,0xED18,0xCC},    
	{BYTE,0xED19,0x59},    
	{BYTE,0xED1A,0x10},    
	{BYTE,0xED1B,0x75},    
	{BYTE,0xED1C,0x0E},    
	{BYTE,0xED1D,0xB4},    
	{BYTE,0xED1E,0x9C},    
	{BYTE,0xED1F,0xFD},    
	{BYTE,0xED20,0xC8},    
	{BYTE,0xED21,0xC6},    
	{BYTE,0xED22,0x3B},    
	{BYTE,0xED23,0xA0},    
	{BYTE,0xED24,0x69},    
	{BYTE,0xED25,0x8E},    
	{BYTE,0xED26,0x65},    
	{BYTE,0xED27,0x84},    
	{BYTE,0xED28,0xF3},    
	{BYTE,0xED29,0x99},    
	{BYTE,0xED2A,0xE6},    
	{BYTE,0xED2B,0xBC},    
	{BYTE,0xED2C,0x26},    
	{BYTE,0xED2D,0x3C},    
	{BYTE,0xED2E,0xC6},    
	{BYTE,0xED2F,0x09},    
	{BYTE,0xED30,0x90},    
	{BYTE,0xED31,0x72},    
	{BYTE,0xED32,0x12},    
	{BYTE,0xED33,0x34},    
	{BYTE,0xED34,0x9B},    
	{BYTE,0xED35,0xEF},    
	{BYTE,0xED36,0x84},    
	{BYTE,0xED37,0xE6},    
	{BYTE,0xED38,0x38},    
	{BYTE,0xED39,0x7E},    
	{BYTE,0xED3A,0xE9},    
	{BYTE,0xED3B,0xCC},    
	{BYTE,0xED3C,0x59},    
	{BYTE,0xED3D,0x04},    
	{BYTE,0xED3E,0x03},    
	{BYTE,0xED3F,0x16},    
	{BYTE,0xED40,0xBD},    
	{BYTE,0xED41,0x98},    
	{BYTE,0xED42,0x45},    
	{BYTE,0xED43,0x30},    
	{BYTE,0xED44,0x7C},    
	{BYTE,0xED45,0xF1},    
	{BYTE,0xED46,0x4C},    
	{BYTE,0xED47,0x67},    
	{BYTE,0xED48,0x94},    
	{BYTE,0xED49,0xB3},    
	{BYTE,0xED4A,0x9A},    
	{BYTE,0xED4B,0xF0},    
	{BYTE,0xED4C,0x50},    
	{BYTE,0xED4D,0x06},    
	{BYTE,0xED4E,0x37},    
	{BYTE,0xED4F,0x7A},    
	{BYTE,0xED50,0xB9},    
	{BYTE,0xED51,0x4C},    
	{BYTE,0xED52,0x54},    
	{BYTE,0xED53,0xC6},    
	{BYTE,0xED54,0x82},    
	{BYTE,0xED55,0x93},    
	{BYTE,0xED56,0xA2},    
	{BYTE,0xED57,0xB4},    
	{BYTE,0xED58,0x04},    
	{BYTE,0xED59,0x27},    
	{BYTE,0xED5A,0x36},    
	{BYTE,0xED5B,0x21},    
	{BYTE,0xED5C,0x8A},    
	{BYTE,0xED5D,0x53},    
	{BYTE,0xED5E,0xC4},    
	{BYTE,0xED5F,0x72},    
	{BYTE,0xED60,0x17},    
	{BYTE,0xED61,0xCB},    
	{BYTE,0xED62,0x34},    
	{BYTE,0xED63,0x06},    
	{BYTE,0xED64,0x37},    
	{BYTE,0xED65,0x7C},    
	{BYTE,0xED66,0xD9},    
	{BYTE,0xED67,0x8C},    
	{BYTE,0xED68,0x56},    
	{BYTE,0xED69,0xDE},    
	{BYTE,0xED6A,0xF2},    
	{BYTE,0xED6B,0x12},    
	{BYTE,0xED6C,0x9D},    
	{BYTE,0xED6D,0x58},    
	{BYTE,0xED6E,0x64},    
	{BYTE,0xED6F,0x23},    
	{BYTE,0xED70,0x0E},    
	{BYTE,0xED71,0x89},    
	{BYTE,0xED72,0x48},    
	{BYTE,0xED73,0x45},    
	{BYTE,0xED74,0x32},    
	{BYTE,0xED75,0xD2},    
	{BYTE,0xED76,0x12},    
	{BYTE,0xED77,0x9C},    
	{BYTE,0xED78,0x5C},    
	{BYTE,0xED79,0xC5},    
	{BYTE,0xED7A,0x2D},    
	{BYTE,0xED7B,0x75},    
	{BYTE,0xED7C,0xC9},    
	{BYTE,0xED7D,0x4C},    
	{BYTE,0xED7E,0x5B},    
	{BYTE,0xED7F,0x14},    
	{BYTE,0xED80,0xA3},    
	{BYTE,0xED81,0x94},    
	{BYTE,0xED82,0xAE},    
	{BYTE,0xED83,0x88},    
	{BYTE,0xED84,0x44},    
	{BYTE,0xED85,0x25},    
	{BYTE,0xED86,0x09},    
	{BYTE,0xED87,0x59},    
	{BYTE,0xED88,0x08},    
	{BYTE,0xED89,0x40},    
	{BYTE,0xED8A,0x00},    
	{BYTE,0xED8B,0x82},    
	{BYTE,0xED8C,0x10},    
	{BYTE,0xED8D,0x85},    
	{BYTE,0xED8E,0x84},    
	{BYTE,0xED8F,0x04},    
	{BYTE,0xED90,0x25},    
	{BYTE,0xED91,0x48},    
	{BYTE,0xED92,0xE1},    
	{BYTE,0xED93,0x0A},    
	{BYTE,0xED94,0x5A},    
	{BYTE,0xED95,0x10},    
	{BYTE,0xED96,0x03},    
	{BYTE,0xED97,0x97},    
	{BYTE,0xED98,0xC6},    
	{BYTE,0xED99,0x30},    
	{BYTE,0xED9A,0x05},    
	{BYTE,0xED9B,0x2C},    
	{BYTE,0xED9C,0x24},    
	{BYTE,0xED9D,0x69},    
	{BYTE,0xED9E,0xC9},    
	{BYTE,0xED9F,0x42},    
	{BYTE,0xEDA0,0x1A},    
	{BYTE,0xEDA1,0x12},    
	{BYTE,0xEDA2,0x10},    
	{BYTE,0xEDA3,0x81},    
	{BYTE,0xEDA4,0x28},    
	{BYTE,0xEDA5,0x84},    
	{BYTE,0xEDA6,0x21},    
	{BYTE,0xEDA7,0x23},    
	{BYTE,0xEDA8,0x59},    
	{BYTE,0xEDA9,0x89},    
	{BYTE,0xEDAA,0x52},    
	{BYTE,0xEDAB,0xBE},    
	{BYTE,0xEDAC,0xA2},    
	{BYTE,0xEDAD,0x96},    
	{BYTE,0xEDAE,0xC5},    
	{BYTE,0xEDAF,0x08},    
	{BYTE,0xEDB0,0xA6},    
	{BYTE,0xEDB1,0x34},    
	{BYTE,0xEDB2,0x60},    
	{BYTE,0xEDB3,0xC1},    
	{BYTE,0xEDB4,0x4B},    
	{BYTE,0xEDB5,0x4D},    
	{BYTE,0xEDB6,0x84},    
	{BYTE,0xEDB7,0xB2},    
	{BYTE,0xEDB8,0x11},    
	{BYTE,0xEDB9,0x91},    
	{BYTE,0xEDBA,0x4C},    
	{BYTE,0xEDBB,0x04},    
	{BYTE,0xEDBC,0x23},    
	{BYTE,0xEDBD,0x1A},    
	{BYTE,0xEDBE,0x09},    
	{BYTE,0xEDBF,0xC9},    
	{BYTE,0xEDC0,0x4C},    
	{BYTE,0xEDC1,0x82},    
	{BYTE,0xEDC2,0xE2},    
	{BYTE,0xEDC3,0x95},    
	{BYTE,0xEDC4,0xBB},    
	{BYTE,0xEDC5,0xF4},    
	{BYTE,0xEDC6,0x65},    
	{BYTE,0xEDC7,0x34},    
	{BYTE,0xEDC8,0x9C},    
	{BYTE,0xEDC9,0x39},    
	{BYTE,0xEDCA,0x8E},    
	{BYTE,0xEDCB,0x60},    
	{BYTE,0xEDCC,0x48},    
	{BYTE,0xEDCD,0x93},    
	{BYTE,0xEDCE,0x95},    
	{BYTE,0xEDCF,0xB7},    
	{BYTE,0xEDD0,0xFC},    
	{BYTE,0xEDD1,0xE4},    
	{BYTE,0xEDD2,0x29},    
	{BYTE,0xEDD3,0x34},    
	{BYTE,0xEDD4,0x11},    
	{BYTE,0xEDD5,0x8A},    
	{BYTE,0xEDD6,0x4F},    
	{BYTE,0xEDD7,0xA0},    
	{BYTE,0xEDD8,0x72},    
	{BYTE,0xEDD9,0x95},    
	{BYTE,0xEDDA,0xB7},    
	{BYTE,0xEDDB,0x00},    
	{BYTE,0xEDDC,0xA6},    
	{BYTE,0xEDDD,0x34},    
	{BYTE,0xEDDE,0x97},    
	{BYTE,0xEDDF,0x31},    
	{BYTE,0xEDE0,0x0E},    
	{BYTE,0xEDE1,0x6F},    
	{BYTE,0xEDE2,0xE4},    
	{BYTE,0xEDE3,0x93},    
	{BYTE,0xEDE4,0x9A},    
	{BYTE,0xEDE5,0xEC},    
	{BYTE,0xEDE6,0x18},    
	{BYTE,0xEDE7,0x66},    
	{BYTE,0xEDE8,0x35},    
	{BYTE,0xEDE9,0x6F},    
	{BYTE,0xEDEA,0x71},    
	{BYTE,0xEDEB,0x0C},    
	{BYTE,0xEDEC,0x5A},    
	{BYTE,0xEDED,0x0C},    
	{BYTE,0xEDEE,0xE3},    
	{BYTE,0xEDEF,0x96},    
	{BYTE,0xEDF0,0xC7},    
	{BYTE,0xEDF1,0x14},    
	{BYTE,0xEDF2,0x86},    
	{BYTE,0xEDF3,0x35},    
	{BYTE,0xEDF4,0xA8},    
	{BYTE,0xEDF5,0xD9},    
	{BYTE,0xEDF6,0x0E},    
	{BYTE,0xEDF7,0x6E},    
	{BYTE,0xEDF8,0xE2},    
	{BYTE,0xEDF9,0xC3},    
	{BYTE,0xEDFA,0x9D},    
	{BYTE,0xEDFB,0x0E},    
	{BYTE,0xEDFC,0x45},    
	{BYTE,0xEDFD,0xC7},    
	{BYTE,0xEDFE,0x41},    
	{BYTE,0xEDFF,0xB6},    
	{BYTE,0xEE00,0x71},    
	{BYTE,0xEE01,0x8F},    
	{BYTE,0xEE02,0x69},    
	{BYTE,0xEE03,0xB0},    
	{BYTE,0xEE04,0xA3},    
	{BYTE,0xEE05,0x19},    
	{BYTE,0xEE06,0xE6},    
	{BYTE,0xEE07,0x8C},    
	{BYTE,0xEE08,0xE6},    
	{BYTE,0xEE09,0x3A},    
	{BYTE,0xEE0A,0xB5},    
	{BYTE,0xEE0B,0x69},    
	{BYTE,0xEE0C,0x0F},    
	{BYTE,0xEE0D,0x74},    
	{BYTE,0xEE0E,0x20},    
	{BYTE,0xEE0F,0x84},    
	{BYTE,0xEE10,0x1D},    
	{BYTE,0xEE11,0x0E},    
	{BYTE,0xEE12,0x91},    
	{BYTE,0xEE13,0xA7},    
	{BYTE,0xEE14,0x44},    
	{BYTE,0xEE15,0xD4},    
	{BYTE,0xEE16,0x79},    
	{BYTE,0xEE17,0x90},    
	{BYTE,0xEE18,0x6F},    
	{BYTE,0xEE19,0xEC},    
	{BYTE,0xEE1A,0x23},    
	{BYTE,0xEE1B,0x9B},    
	{BYTE,0xEE1C,0xF3},    
	{BYTE,0xEE1D,0xB4},    
	{BYTE,0xEE1E,0x46},    
	{BYTE,0xEE1F,0x3C},    
	{BYTE,0xEE20,0xB0},    
	{BYTE,0xEE21,0x29},    
	{BYTE,0xEE22,0x4F},    
	{BYTE,0xEE23,0x6F},    
	{BYTE,0xEE24,0xEC},    
	{BYTE,0xEE25,0x33},    
	{BYTE,0xEE26,0x1D},    
	{BYTE,0xEE27,0x08},    
	{BYTE,0xEE28,0x7D},    
	{BYTE,0xEE29,0xE7},    
	{BYTE,0xEE2A,0x43},    
	{BYTE,0xEE2B,0x9C},    
	{BYTE,0xEE2C,0xB9},    
	{BYTE,0xEE2D,0x4E},    
	{BYTE,0xEE2E,0x66},    
	{BYTE,0xEE2F,0xA2},    
	{BYTE,0xEE30,0xF3},    
	{BYTE,0xEE31,0x98},    
	{BYTE,0xEE32,0xE0},    
	{BYTE,0xEE33,0x24},    
	{BYTE,0xEE34,0x86},    
	{BYTE,0xEE35,0x36},    
	{BYTE,0xEE36,0x88},    
	{BYTE,0xEE37,0x91},    
	{BYTE,0xEE38,0x8D},    
	{BYTE,0xEE39,0x63},    
	{BYTE,0xEE3A,0x6A},    
	{BYTE,0xEE3B,0x83},    
	{BYTE,0xEE3C,0x19},    
	{BYTE,0xEE3D,0xE1},    
	{BYTE,0xEE3E,0x94},    
	{BYTE,0xEE3F,0x86},    
	{BYTE,0xEE40,0x3A},    
	{BYTE,0xEE41,0xA6},    
	{BYTE,0xEE42,0xB9},    
	{BYTE,0xEE43,0x8E},    
	{BYTE,0xEE44,0x67},    
	{BYTE,0xEE45,0xAE},    
	{BYTE,0xEE46,0xB3},    
	{BYTE,0xEE47,0x99},    
	{BYTE,0xEE48,0xE7},    
	{BYTE,0xEE49,0x28},    
	{BYTE,0xEE4A,0xC6},    
	{BYTE,0xEE4B,0x36},    
	{BYTE,0xEE4C,0x80},    
	{BYTE,0xEE4D,0x31},    
	{BYTE,0xEE4E,0xCD},    
	{BYTE,0xEE4F,0x5E},    
	{BYTE,0xEE50,0x3A},    
	{BYTE,0xEE51,0x53},    
	{BYTE,0xEE52,0x18},    
	{BYTE,0xEE53,0xD4},    
	{BYTE,0xEE54,0x4C},    
	{BYTE,0xEE55,0x26},    
	{BYTE,0xEE56,0x37},    
	{BYTE,0xEE57,0xA8},    
	{BYTE,0xEE58,0x91},    
	{BYTE,0xEE59,0x8E},    
	{BYTE,0xEE5A,0x6A},    
	{BYTE,0xEE5B,0xAC},    
	{BYTE,0xEE5C,0x63},    
	{BYTE,0xEE5D,0x18},    
	{BYTE,0xEE5E,0xDB},    
	{BYTE,0xEE5F,0xEC},    
	{BYTE,0xEE60,0xA5},    
	{BYTE,0xEE61,0x34},    
	{BYTE,0xEE62,0x62},    
	{BYTE,0xEE63,0x19},    
	{BYTE,0xEE64,0xCC},    
	{BYTE,0xEE65,0x54},    
	{BYTE,0xEE66,0xDA},    
	{BYTE,0xEE67,0x02},    
	{BYTE,0xEE68,0x95},    
	{BYTE,0xEE69,0xB3},    
	{BYTE,0xEE6A,0x5C},    
	{BYTE,0xEE6B,0xC5},    
	{BYTE,0xEE6C,0x2D},    
	{BYTE,0xEE6D,0x6A},    
	{BYTE,0xEE6E,0x21},    
	{BYTE,0xEE6F,0xCC},    
	{BYTE,0xEE70,0x61},    
	{BYTE,0xEE71,0x4E},    
	{BYTE,0xEE72,0x33},    
	{BYTE,0xEE73,0x19},    
	{BYTE,0xEE74,0xDA},    
	{BYTE,0xEE75,0xB4},    
	{BYTE,0xEE76,0xC5},    
	{BYTE,0xEE77,0x32},    
	{BYTE,0xEE78,0x5D},    
	{BYTE,0xEE79,0xE9},    
	{BYTE,0xEE7A,0xCB},    
	{BYTE,0xEE7B,0x4F},    
	{BYTE,0xEE7C,0xAC},    
	{BYTE,0xEE7D,0xD2},    
	{BYTE,0xEE7E,0x12},    
	{BYTE,0xEE7F,0x9F},    
	{BYTE,0xEE80,0x98},    
	{BYTE,0xEE81,0x64},    
	{BYTE,0xEE82,0x26},    
	{BYTE,0xEE83,0x30},    
	{BYTE,0xEE84,0xE9},    
	{BYTE,0xEE85,0x09},    
	{BYTE,0xEE86,0x51},    
	{BYTE,0xEE87,0xAA},    
	{BYTE,0xEE88,0x62},    
	{BYTE,0xEE89,0x16},    
	{BYTE,0xEE8A,0xBF},    
	{BYTE,0xEE8B,0xE4},    
	{BYTE,0xEE8C,0x85},    
	{BYTE,0xEE8D,0x32},    
	{BYTE,0xEE8E,0x5B},    
	{BYTE,0xEE8F,0xE9},    
	{BYTE,0xEE90,0x8B},    
	{BYTE,0xEE91,0x50},    
	{BYTE,0xEE92,0xBA},    
	{BYTE,0xEE93,0x32},    
	{BYTE,0xEE94,0x92},    
	{BYTE,0xEE95,0x99},    
	{BYTE,0xEE96,0x44},    
	{BYTE,0xEE97,0x44},    
	{BYTE,0xEE98,0x23},    
	{BYTE,0xEE99,0x0D},    
	{BYTE,0xEE9A,0x91},    
	{BYTE,0xEE9B,0x08},    
	{BYTE,0xEE9C,0x45},    
	{BYTE,0xEE9D,0x30},    
	{BYTE,0xEE9E,0x82},    
	{BYTE,0xEE9F,0x92},    
	{BYTE,0xEEA0,0x98},    
	{BYTE,0xEEA1,0x28},    
	{BYTE,0xEEA2,0x65},    
	{BYTE,0xEEA3,0x2B},    
	{BYTE,0xEEA4,0x63},    
	{BYTE,0xEEA5,0xC9},    
	{BYTE,0xEEA6,0xCB},    
	{BYTE,0xEEA7,0x53},    
	{BYTE,0xEEA8,0xDA},    
	{BYTE,0xEEA9,0x52},    
	{BYTE,0xEEAA,0x93},    
	{BYTE,0xEEAB,0xA5},    
	{BYTE,0xEEAC,0x60},    
	{BYTE,0xEEAD,0x84},    
	{BYTE,0xEEAE,0x24},    
	{BYTE,0xEEAF,0x05},    
	{BYTE,0xEEB0,0x59},    
	{BYTE,0xEEB1,0x08},    
	{BYTE,0xEEB2,0x40},    
	{BYTE,0xEEB3,0x02},    
	{BYTE,0xEEB4,0x82},    
	{BYTE,0xEEB5,0x90},    
	{BYTE,0xEEB6,0x84},    
	{BYTE,0xEEB7,0x74},    
	{BYTE,0xEEB8,0x24},    
	{BYTE,0xEEB9,0x24},    
	{BYTE,0xEEBA,0x3B},    
	{BYTE,0xEEBB,0x41},    
	{BYTE,0xEEBC,0x4A},    
	{BYTE,0xEEBD,0x55},    
	{BYTE,0xEEBE,0xCE},    
	{BYTE,0xEEBF,0xF2},    
	{BYTE,0xEEC0,0x94},    
	{BYTE,0xEEC1,0xB5},    
	{BYTE,0xEEC2,0xD8},    
	{BYTE,0xEEC3,0x24},    
	{BYTE,0xEEC4,0x29},    
	{BYTE,0xEEC5,0x18},    
	{BYTE,0xEEC6,0x19},    
	{BYTE,0xEEC7,0x49},    
	{BYTE,0xEEC8,0x41},    
	{BYTE,0xEEC9,0x14},    
	{BYTE,0xEECA,0x02},    
	{BYTE,0xEECB,0x10},    
	{BYTE,0xEECC,0x80},    
	{BYTE,0xEECD,0x20},    
	{BYTE,0xEECE,0x04},    
	{BYTE,0xEECF,0x21},    
	{BYTE,0xEED0,0x1C},    
	{BYTE,0xEED1,0x01},    
	{BYTE,0xEED2,0x89},    
	{BYTE,0xEED3,0x4E},    
	{BYTE,0xEED4,0x90},    
	{BYTE,0xEED5,0x32},    
	{BYTE,0xEED6,0x95},    
	{BYTE,0xEED7,0xB3},    
	{BYTE,0xEED8,0x7C},    
	{BYTE,0xEED9,0x45},    
	{BYTE,0xEEDA,0x2F},    
	{BYTE,0xEEDB,0x45},    
	{BYTE,0xEEDC,0xD1},    
	{BYTE,0xEEDD,0x4A},    
	{BYTE,0xEEDE,0x49},    
	{BYTE,0xEEDF,0x62},    
	{BYTE,0xEEE0,0x12},    
	{BYTE,0xEEE1,0x11},    
	{BYTE,0xEEE2,0x8C},    
	{BYTE,0xEEE3,0x34},    
	{BYTE,0xEEE4,0xE4},    
	{BYTE,0xEEE5,0x21},    
	{BYTE,0xEEE6,0x13},    
	{BYTE,0xEEE7,0xB1},    
	{BYTE,0xEEE8,0xC8},    
	{BYTE,0xEEE9,0x49},    
	{BYTE,0xEEEA,0x5C},    
	{BYTE,0xEEEB,0x72},    
	{BYTE,0xEEEC,0x94},    
	{BYTE,0xEEED,0xAC},    
	{BYTE,0xEEEE,0x7C},    
	{BYTE,0xEEEF,0x05},    
	{BYTE,0xEEF0,0x2F},    
	{BYTE,0xEEF1,0x77},    
	{BYTE,0xEEF2,0x99},    
	{BYTE,0xEEF3,0xCC},    
	{BYTE,0xEEF4,0x58},    
	{BYTE,0xEEF5,0xF6},    
	{BYTE,0xEEF6,0x32},    
	{BYTE,0xEEF7,0x94},    
	{BYTE,0xEEF8,0xA9},    
	{BYTE,0xEEF9,0xC0},    
	{BYTE,0xEEFA,0x64},    
	{BYTE,0xEEFB,0x27},    
	{BYTE,0xEEFC,0x28},    
	{BYTE,0xEEFD,0x81},    
	{BYTE,0xEEFE,0x09},    
	{BYTE,0xEEFF,0x4C},    
	{BYTE,0xEF00,0x74},    
	{BYTE,0xEF01,0x32},    
	{BYTE,0xEF02,0x14},    
	{BYTE,0xEF03,0xA9},    
	{BYTE,0xEF04,0x88},    
	{BYTE,0xEF05,0x65},    
	{BYTE,0xEF06,0x2F},    
	{BYTE,0xEF07,0x72},    
	{BYTE,0xEF08,0x91},    
	{BYTE,0xEF09,0xCC},    
	{BYTE,0xEF0A,0x64},    
	{BYTE,0xEF0B,0x66},    
	{BYTE,0xEF0C,0x73},    
	{BYTE,0xEF0D,0x18},    
	{BYTE,0xEF0E,0xD1},    
	{BYTE,0xEF0F,0xB0},    
	{BYTE,0xEF10,0x25},    
	{BYTE,0xEF11,0x30},    
	{BYTE,0xEF12,0x5A},    
	{BYTE,0xEF13,0x59},    
	{BYTE,0xEF14,0x0B},    
	{BYTE,0xEF15,0x55},    
	{BYTE,0xEF16,0xCA},    
	{BYTE,0xEF17,0x82},    
	{BYTE,0xEF18,0x95},    
	{BYTE,0xEF19,0xB5},    
	{BYTE,0xEF1A,0xA0},    
	{BYTE,0xEF1B,0x45},    
	{BYTE,0xEF1C,0x30},    
	{BYTE,0xEF1D,0x83},    
	{BYTE,0xEF1E,0x31},    
	{BYTE,0xEF1F,0x0D},    
	{BYTE,0xEF20,0x63},    
	{BYTE,0xEF21,0x68},    
	{BYTE,0xEF22,0xF3},    
	{BYTE,0xEF23,0x9A},    
	{BYTE,0xEF24,0xEA},    
	{BYTE,0xEF25,0xA8},    
	{BYTE,0xEF26,0x86},    
	{BYTE,0xEF27,0x39},    
	{BYTE,0xEF28,0x98},    
	{BYTE,0xEF29,0xA1},    
	{BYTE,0xEF2A,0xCD},    
	{BYTE,0xEF2B,0x62},    
	{BYTE,0xEF2C,0x4A},    
	{BYTE,0xEF2D,0x13},    
	{BYTE,0xEF2E,0x98},    
	{BYTE,0xEF2F,0xCD},    
	{BYTE,0xEF30,0x1C},    
	{BYTE,0xEF31,0x86},    
	{BYTE,0xEF32,0x34},    
	{BYTE,0xEF33,0x92},    
	{BYTE,0xEF34,0xB1},    
	{BYTE,0xEF35,0x0D},    
	{BYTE,0xEF36,0x69},    
	{BYTE,0xEF37,0xA2},    
	{BYTE,0xEF38,0x73},    
	{BYTE,0xEF39,0x1A},    
	{BYTE,0xEF3A,0xEB},    
	{BYTE,0xEF3B,0xC8},    
	{BYTE,0xEF3C,0xC6},    
	{BYTE,0xEF3D,0x3B},    
	{BYTE,0xEF3E,0xA7},    
	{BYTE,0xEF3F,0x89},    
	{BYTE,0xEF40,0x4E},    
	{BYTE,0xEF41,0x66},    
	{BYTE,0xEF42,0x7C},    
	{BYTE,0xEF43,0x13},    
	{BYTE,0xEF44,0x99},    
	{BYTE,0xEF45,0xD9},    
	{BYTE,0xEF46,0x34},    
	{BYTE,0xEF47,0x06},    
	{BYTE,0xEF48,0x36},    
	{BYTE,0xEF49,0x8C},    
	{BYTE,0xEF4A,0xA1},    
	{BYTE,0xEF4B,0xCD},    
	{BYTE,0xEF4C,0x64},    
	{BYTE,0xEF4D,0x80},    
	{BYTE,0xEF4E,0xF3},    
	{BYTE,0xEF4F,0x19},    
	{BYTE,0xEF50,0xEA},    
	{BYTE,0xEF51,0x98},    
	{BYTE,0xEF52,0xC6},    
	{BYTE,0xEF53,0x3B},    
	{WORD,0xED00,0x9191},  
	{BYTE,0xEF54,0xF7},    
	{BYTE,0xEF55,0x89},    
	{BYTE,0xEF56,0x8F},    
	{BYTE,0xEF57,0x77},    
	{BYTE,0xEF58,0xA0},    
	{BYTE,0xEF59,0xC3},    
	{BYTE,0xEF5A,0x1C},    
	{BYTE,0xEF5B,0xE8},    
	{BYTE,0xEF5C,0x7C},    
	{BYTE,0xEF5D,0x67},    
	{BYTE,0xEF5E,0x3E},    
	{BYTE,0xEF5F,0xF3},    
	{BYTE,0xEF60,0x99},    
	{BYTE,0xEF61,0xCF},    
	{BYTE,0xEF62,0x7A},    
	{BYTE,0xEF63,0x9C},    
	{BYTE,0xEF64,0xB3},    
	{BYTE,0xEF65,0x1B},    
	{BYTE,0xEF66,0xD8},    
	{BYTE,0xEF67,0xE8},    
	{BYTE,0xEF68,0xC6},    
	{BYTE,0xEF69,0x39},    
	{BYTE,0xEF6A,0xED},    
	{BYTE,0xEF6B,0x99},    
	{BYTE,0xEF6C,0x0F},    
	{BYTE,0xEF6D,0x73},    
	{BYTE,0xEF6E,0x70},    
	{BYTE,0xEF6F,0xF3},    
	{BYTE,0xEF70,0x98},    
	{BYTE,0xEF71,0xBA},    
	{BYTE,0xEF72,0xB4},    
	{BYTE,0xEF73,0x85},    
	{BYTE,0xEF74,0x2E},    
	{BYTE,0xEF75,0x8D},    
	{BYTE,0xEF76,0xC1},    
	{BYTE,0xEF77,0x4D},    
	{BYTE,0xEF78,0x73},    
	{BYTE,0xEF79,0x4C},    
	{BYTE,0xEF7A,0x93},    
	{BYTE,0xEF7B,0x18},    
	{BYTE,0xEF7C,0xAD},    
	{BYTE,0xEF7D,0xF8},    
	{BYTE,0xEF7E,0x24},    
	{BYTE,0xEF7F,0x26},    
	{BYTE,0xEF80,0x3C},    
	{BYTE,0xEF81,0xC1},    
	{BYTE,0xEF82,0x0A},    
	{BYTE,0xEF83,0x62},    
	{BYTE,0xEF84,0x4A},    
	{BYTE,0xEF85,0xA3},    
	{BYTE,0xEF86,0x98},    
	{BYTE,0xEF87,0xB2},    
	{BYTE,0xEF88,0xD4},    
	{BYTE,0xEF89,0x04},    
	{BYTE,0xEF8A,0x23},    
	{BYTE,0xEF8B,0x0F},    
	{BYTE,0xEF8C,0xB9},    
	{BYTE,0xEF8D,0xC8},    
	{BYTE,0xEF8E,0x4C},    
	{BYTE,0xEF8F,0xC6},    
	{BYTE,0xEF90,0x72},    
	{BYTE,0xEF91,0x98},    
	{BYTE,0xEF92,0xBD},    
	{BYTE,0xEF93,0x50},    
	{BYTE,0xEF94,0xE5},    
	{BYTE,0xEF95,0x24},    
	{BYTE,0xEF96,0x0B},    
	{BYTE,0xEF97,0x01},    
	{BYTE,0xEF98,0x48},    
	{BYTE,0xEF99,0x42},    
	{BYTE,0xEF9A,0x4A},    
	{BYTE,0xEF9B,0x32},    
	{BYTE,0xEF9C,0x15},    
	{BYTE,0xEF9D,0xBC},    
	{BYTE,0xEF9E,0xF4},    
	{BYTE,0xEF9F,0xE5},    
	{BYTE,0xEFA0,0x2A},    
	{BYTE,0xEFA1,0x2A},    
	{BYTE,0xEFA2,0x69},    
	{BYTE,0xEFA3,0x88},    
	{BYTE,0xEFA4,0x40},    
	{BYTE,0xEFA5,0x18},    
	{BYTE,0xEFA6,0x82},    
	{BYTE,0xEFA7,0x12},    
	{BYTE,0xEFA8,0xAB},    
	{BYTE,0xEFA9,0xF0},    
	{BYTE,0xEFAA,0x65},    
	{BYTE,0xEFAB,0x32},    
	{BYTE,0xEFAC,0x6D},    
	{BYTE,0xEFAD,0xE1},    
	{BYTE,0xEFAE,0xC9},    
	{BYTE,0xEFAF,0x47},    
	{BYTE,0xEFB0,0x2C},    
	{BYTE,0xEFB1,0xE2},    
	{BYTE,0xEFB2,0x91},    
	{BYTE,0xEFB3,0x9D},    
	{BYTE,0xEFB4,0xB0},    
	{BYTE,0xEFB5,0x25},    
	{BYTE,0xEFB6,0x32},    
	{BYTE,0xEFB7,0xB3},    
	{BYTE,0xEFB8,0xA9},    
	{BYTE,0xEFB9,0x4C},    
	{BYTE,0xEFBA,0x59},    
	{BYTE,0xEFBB,0x90},    
	{BYTE,0xEFBC,0xC2},    
	{BYTE,0xEFBD,0x13},    
	{BYTE,0xEFBE,0xA4},    
	{BYTE,0xEFBF,0x94},    
	{BYTE,0xEFC0,0xA5},    
	{BYTE,0xEFC1,0x32},    
	{BYTE,0xEFC2,0xB2},    
	{BYTE,0xEFC3,0xE1},    
	{BYTE,0xEFC4,0xCE},    
	{BYTE,0xEFC5,0x71},    
	{BYTE,0xEFC6,0x38},    
	{BYTE,0xEFC7,0x03},    
	{BYTE,0xEFC8,0x18},    
	{BYTE,0xEFC9,0xBC},    
	{BYTE,0xEFCA,0x00},    
	{BYTE,0xEFCB,0x86},    
	{BYTE,0xEFCC,0x33},    
	{BYTE,0xEFCD,0xC8},    
	{BYTE,0xEFCE,0xE1},    
	{BYTE,0xEFCF,0xCE},    
	{BYTE,0xEFD0,0x81},    
	{BYTE,0xEFD1,0xF8},    
	{BYTE,0xEFD2,0xC3},    
	{BYTE,0xEFD3,0x9D},    
	{BYTE,0xEFD4,0xE3},    
	{BYTE,0xEFD5,0xE8},    
	{BYTE,0xEFD6,0xA6},    
	{BYTE,0xEFD7,0x38},    
	{BYTE,0xEFD8,0xDB},    
	{BYTE,0xEFD9,0xE1},    
	{BYTE,0xEFDA,0x8F},    
	{BYTE,0xEFDB,0x81},    
	{BYTE,0xEFDC,0x1E},    
	{BYTE,0xEFDD,0xE4},    
	{BYTE,0xEFDE,0x1F},    
	{BYTE,0xEFDF,0xF3},    
	{BYTE,0xEFE0,0x5C},    
	{BYTE,0xEFE1,0x47},    
	{BYTE,0xEFE2,0x3A},    
	{BYTE,0xEFE3,0xD5},    
	{BYTE,0xEFE4,0x29},    
	{BYTE,0xEFE5,0x8F},    
	{BYTE,0xEFE6,0x7F},    
	{BYTE,0xEFE7,0x12},    
	{BYTE,0xEFE8,0x04},    
	{BYTE,0xEFE9,0x00},    
	{BYTE,0xEFEA,0x00},    
	{BYTE,0xEFEB,0xD0},    
	{BYTE,0xEFEC,0x05},    
	{BYTE,0xEFED,0x00},    
	{WORD,0xED00,0x9191},  
	{BYTE,0xEFEE,0x68},    
	{BYTE,0xEFEF,0xF2},    
	{BYTE,0xEFF0,0x92},    
	{BYTE,0xEFF1,0x91},    
	{BYTE,0xEFF2,0x6A},    
	{BYTE,0xEFF3,0xE4},    
	{BYTE,0xEFF4,0x22},    
	{BYTE,0xEFF5,0x19},    
	{BYTE,0xEFF6,0x19},    
	{BYTE,0xEFF7,0x09},    
	{BYTE,0xEFF8,0x4C},    
	{BYTE,0xEFF9,0x62},    
	{BYTE,0xEFFA,0x12},    
	{BYTE,0xEFFB,0xD3},    
	{BYTE,0xEFFC,0x94},    
	{BYTE,0xEFFD,0x58},    
	{BYTE,0xEFFE,0x24},    
	{BYTE,0xEFFF,0xA1},    
	{BYTE,0xF000,0x01},    
	{BYTE,0xF001,0x41},    
	{BYTE,0xF002,0x88},    
	{BYTE,0xF003,0x45},    
	{BYTE,0xF004,0x56},    
	{BYTE,0xF005,0x02},    
	{BYTE,0xF006,0x13},    
	{BYTE,0xF007,0x8B},    
	{BYTE,0xF008,0x16},    
	{BYTE,0xF009,0x24},    
	{BYTE,0xF00A,0x1D},    
	{BYTE,0xF00B,0xD6},    
	{BYTE,0xF00C,0x80},    
	{BYTE,0xF00D,0x66},    
	{BYTE,0xF00E,0x35},    
	{BYTE,0xF00F,0xD1},    
	{BYTE,0xF010,0x69},    
	{BYTE,0xF011,0x10},    
	{BYTE,0xF012,0x8B},    
	{BYTE,0xF013,0xEC},    
	{BYTE,0xF014,0x83},    
	{BYTE,0xF015,0x9C},    
	{BYTE,0xF016,0xC1},    
	{BYTE,0xF017,0x68},    
	{BYTE,0xF018,0x25},    
	{BYTE,0xF019,0x29},    
	{BYTE,0xF01A,0x58},    
	{BYTE,0xF01B,0x09},    
	{BYTE,0xF01C,0xCC},    
	{BYTE,0xF01D,0x71},    
	{BYTE,0xF01E,0xEA},    
	{BYTE,0xF01F,0xD3},    
	{BYTE,0xF020,0x9C},    
	{BYTE,0xF021,0xC9},    
	{BYTE,0xF022,0x30},    
	{BYTE,0xF023,0x65},    
	{BYTE,0xF024,0x24},    
	{BYTE,0xF025,0x16},    
	{BYTE,0xF026,0x11},    
	{BYTE,0xF027,0x89},    
	{BYTE,0xF028,0x52},    
	{BYTE,0xF029,0x24},    
	{BYTE,0xF02A,0xB3},    
	{BYTE,0xF02B,0x1C},    
	{BYTE,0xF02C,0xDB},    
	{BYTE,0xF02D,0xE8},    
	{BYTE,0xF02E,0xC5},    
	{BYTE,0xF02F,0x26},    
	{BYTE,0xF030,0x0E},    
	{BYTE,0xF031,0x01},    
	{BYTE,0xF032,0x48},    
	{BYTE,0xF033,0x43},    
	{BYTE,0xF034,0x6A},    
	{BYTE,0xF035,0x92},    
	{BYTE,0xF036,0x97},    
	{BYTE,0xF037,0xDA},    
	{BYTE,0xF038,0xE4},    
	{BYTE,0xF039,0xA6},    
	{BYTE,0xF03A,0x2F},    
	{BYTE,0xF03B,0x39},    
	{BYTE,0xF03C,0x81},    
	{BYTE,0xF03D,0x88},    
	{BYTE,0xF03E,0x40},    
	{BYTE,0xF03F,0x1E},    
	{BYTE,0xF040,0x82},    
	{BYTE,0xF041,0x93},    
	{BYTE,0xF042,0xBE},    
	{BYTE,0xF043,0xE0},    
	{BYTE,0xF044,0xE6},    
	{BYTE,0xF045,0x3A},    
	{BYTE,0xF046,0x9B},    
	{BYTE,0xF047,0xA1},    
	{BYTE,0xF048,0x8A},    
	{BYTE,0xF049,0x4A},    
	{BYTE,0xF04A,0x3C},    
	{BYTE,0xF04B,0xA2},    
	{BYTE,0xF04C,0x12},    
	{BYTE,0xF04D,0xAA},    
	{BYTE,0xF04E,0x70},    
	{BYTE,0xF04F,0xC6},    
	{BYTE,0xF050,0x3A},    
	{BYTE,0xF051,0x05},    
	{BYTE,0xF052,0xA2},    
	{BYTE,0xF053,0x8E},    
	{BYTE,0xF054,0x63},    
	{BYTE,0xF055,0xC8},    
	{BYTE,0xF056,0x42},    
	{BYTE,0xF057,0x95},    
	{BYTE,0xF058,0xB2},    
	{BYTE,0xF059,0x40},    
	{BYTE,0xF05A,0xC6},    
	{BYTE,0xF05B,0x3A},    
	{BYTE,0xF05C,0x04},    
	{BYTE,0xF05D,0xEA},    
	{BYTE,0xF05E,0x11},    
	{BYTE,0xF05F,0x87},    
	{BYTE,0xF060,0xBE},    
	{BYTE,0xF061,0x83},    
	{BYTE,0xF062,0x1B},    
	{BYTE,0xF063,0xD6},    
	{BYTE,0xF064,0xE4},    
	{BYTE,0xF065,0x26},    
	{BYTE,0xF066,0x3C},    
	{BYTE,0xF067,0x1F},    
	{BYTE,0xF068,0xF2},    
	{BYTE,0xF069,0x51},    
	{BYTE,0xF06A,0x9D},    
	{BYTE,0xF06B,0xC6},    
	{BYTE,0xF06C,0x94},    
	{BYTE,0xF06D,0xA3},    
	{BYTE,0xF06E,0x0E},    
	{BYTE,0xF06F,0x35},    
	{BYTE,0xF070,0x68},    
	{BYTE,0xF071,0x43},    
	{BYTE,0xF072,0x39},    
	{BYTE,0xF073,0x32},    
	{BYTE,0xF074,0x53},    
	{BYTE,0xF075,0x9D},    
	{BYTE,0xF076,0x04},    
	{BYTE,0xF077,0xB5},    
	{BYTE,0xF078,0x26},    
	{BYTE,0xF079,0x26},    
	{BYTE,0xF07A,0xE1},    
	{BYTE,0xF07B,0x28},    
	{BYTE,0xF07C,0x46},    
	{BYTE,0xF07D,0x36},    
	{BYTE,0xF07E,0x52},    
	{BYTE,0xF07F,0xD2},    
	{BYTE,0xF080,0x9A},    
	{BYTE,0xF081,0xFC},    
	{BYTE,0xF082,0x14},    
	{BYTE,0xF083,0x9C},    
	{BYTE,0xF084,0xDD},    
	{BYTE,0xF085,0xB8},    
	{BYTE,0xF086,0xC6},    
	{BYTE,0xF087,0x34},    
	{BYTE,0xF088,0xC1},    
	{BYTE,0xF089,0xD9},    
	{BYTE,0xF08A,0x8D},    
	{BYTE,0xF08B,0x6B},    
	{BYTE,0xF08C,0x4C},    
	{BYTE,0xF08D,0x43},    
	{BYTE,0xF08E,0x9A},    
	{BYTE,0xF08F,0xD4},    
	{BYTE,0xF090,0xDC},    
	{BYTE,0xF091,0x26},    
	{BYTE,0xF092,0x39},    
	{BYTE,0xF093,0xCB},    
	{BYTE,0xF094,0x01},    
	{BYTE,0xF095,0x8E},    
	{BYTE,0xF096,0x6E},    
	{BYTE,0xF097,0x4C},    
	{BYTE,0xF098,0xA3},    
	{BYTE,0xF099,0x19},    
	{BYTE,0xF09A,0xC9},    
	{BYTE,0xF09B,0x74},    
	{BYTE,0xF09C,0xE6},    
	{BYTE,0xF09D,0x35},    
	{BYTE,0xF09E,0xC9},    
	{BYTE,0xF09F,0x71},    
	{BYTE,0xF0A0,0x8E},    
	{BYTE,0xF0A1,0x68},    
	{BYTE,0xF0A2,0x28},    
	{BYTE,0xF0A3,0x73},    
	{BYTE,0xF0A4,0x97},    
	{BYTE,0xF0A5,0xB2},    
	{BYTE,0xF0A6,0x88},    
	{BYTE,0xF0A7,0x45},    
	{BYTE,0xF0A8,0x2D},    
	{BYTE,0xF0A9,0x80},    
	{BYTE,0xF0AA,0x11},    
	{BYTE,0xF0AB,0x0D},    
	{BYTE,0xF0AC,0x6C},    
	{BYTE,0xF0AD,0x0A},    
	{BYTE,0xF0AE,0x13},    
	{BYTE,0xF0AF,0x97},    
	{BYTE,0xF0B0,0xA7},    
	{BYTE,0xF0B1,0xE8},    
	{BYTE,0xF0B2,0x24},    
	{BYTE,0xF0B3,0x26},    
	{BYTE,0xF0B4,0x3C},    
	{BYTE,0xF0B5,0xA1},    
	{BYTE,0xF0B6,0x0A},    
	{BYTE,0xF0B7,0x5F},    
	{BYTE,0xF0B8,0x24},    
	{BYTE,0xF0B9,0xE3},    
	{BYTE,0xF0BA,0x16},    
	{BYTE,0xF0BB,0xA9},    
	{BYTE,0xF0BC,0xB8},    
	{BYTE,0xF0BD,0x04},    
	{BYTE,0xF0BE,0x23},    
	{BYTE,0xF0BF,0x12},    
	{BYTE,0xF0C0,0xD1},    
	{BYTE,0xF0C1,0x88},    
	{BYTE,0xF0C2,0x4C},    
	{BYTE,0xF0C3,0xB6},    
	{BYTE,0xF0C4,0x82},    
	{BYTE,0xF0C5,0x97},    
	{BYTE,0xF0C6,0xAF},    
	{BYTE,0xF0C7,0x04},    
	{BYTE,0xF0C8,0xE5},    
	{BYTE,0xF0C9,0x23},    
	{BYTE,0xF0CA,0x09},    
	{BYTE,0xF0CB,0x09},    
	{BYTE,0xF0CC,0xC8},    
	{BYTE,0xF0CD,0x42},    
	{BYTE,0xF0CE,0x46},    
	{BYTE,0xF0CF,0x92},    
	{BYTE,0xF0D0,0x94},    
	{BYTE,0xF0D1,0xB3},    
	{BYTE,0xF0D2,0x7C},    
	{BYTE,0xF0D3,0x05},    
	{BYTE,0xF0D4,0x28},    
	{BYTE,0xF0D5,0x1E},    
	{BYTE,0xF0D6,0x39},    
	{BYTE,0xF0D7,0x08},    
	{BYTE,0xF0D8,0x40},    
	{BYTE,0xF0D9,0x12},    
	{BYTE,0xF0DA,0x12},    
	{BYTE,0xF0DB,0x12},    
	{BYTE,0xF0DC,0xA3},    
	{BYTE,0xF0DD,0x90},    
	{BYTE,0xF0DE,0xA5},    
	{BYTE,0xF0DF,0x2D},    
	{BYTE,0xF0E0,0x50},    
	{BYTE,0xF0E1,0x59},    
	{BYTE,0xF0E2,0x09},    
	{BYTE,0xF0E3,0x45},    
	{BYTE,0xF0E4,0x1C},    
	{BYTE,0xF0E5,0x52},    
	{BYTE,0xF0E6,0x91},    
	{BYTE,0xF0E7,0x96},    
	{BYTE,0xF0E8,0x50},    
	{BYTE,0xF0E9,0x25},    
	{BYTE,0xF0EA,0x2E},    
	{BYTE,0xF0EB,0x85},    
	{BYTE,0xF0EC,0x71},    
	{BYTE,0xF0ED,0xCB},    
	{BYTE,0xF0EE,0x52},    
	{BYTE,0xF0EF,0x6A},    
	{BYTE,0xF0F0,0xC2},    
	{BYTE,0xF0F1,0x12},    
	{BYTE,0xF0F2,0x9B},    
	{BYTE,0xF0F3,0x30},    
	{BYTE,0xF0F4,0x45},    
	{BYTE,0xF0F5,0x2E},    
	{BYTE,0xF0F6,0x87},    
	{BYTE,0xF0F7,0x29},    
	{BYTE,0xF0F8,0x4D},    
	{BYTE,0xF0F9,0x65},    
	{BYTE,0xF0FA,0xEE},    
	{BYTE,0xF0FB,0x22},    
	{BYTE,0xF0FC,0x16},    
	{BYTE,0xF0FD,0xAE},    
	{BYTE,0xF0FE,0x88},    
	{BYTE,0xF0FF,0xC5},    
	{BYTE,0xF100,0x2E},    
	{BYTE,0xF101,0x98},    
	{BYTE,0xF102,0x29},    
	{BYTE,0xF103,0x8D},    
	{BYTE,0xF104,0x71},    
	{BYTE,0xF105,0x7A},    
	{BYTE,0xF106,0x73},    
	{BYTE,0xF107,0x1A},    
	{BYTE,0xF108,0xCC},    
	{BYTE,0xF109,0x38},    
	{BYTE,0xF10A,0xE6},    
	{BYTE,0xF10B,0x32},    
	{BYTE,0xF10C,0xA6},    
	{BYTE,0xF10D,0xF1},    
	{BYTE,0xF10E,0x0D},    
	{BYTE,0xF10F,0x71},    
	{BYTE,0xF110,0x96},    
	{BYTE,0xF111,0xD3},    
	{BYTE,0xF112,0x1B},    
	{BYTE,0xF113,0xD5},    
	{BYTE,0xF114,0x84},    
	{BYTE,0xF115,0xA6},    
	{BYTE,0xF116,0x33},    
	{BYTE,0xF117,0x9F},    
	{BYTE,0xF118,0x51},    
	{BYTE,0xF119,0xCD},    
	{BYTE,0xF11A,0x6E},    
	{BYTE,0xF11B,0x8E},    
	{BYTE,0xF11C,0x03},    
	{BYTE,0xF11D,0x00},    
	{BYTE,0xF11E,0xF4},    
	{BYTE,0xF11F,0x53},    
	{BYTE,0xF120,0xCB},    
	{BYTE,0xF121,0x92},    
	{WORD,0x6C32,0x16A8},	 // SHD_INP_TH_HB_H_R2
	{WORD,0x6C34,0x15E0},	 // SHD_INP_TH_HB_L_R2
	{WORD,0x6C36,0x1086},	 // SHD_INP_TH_LB_H_R2
	{WORD,0x6C38,0x0FA0},	 // SHD_INP_TH_LB_L_R2
	{BYTE,0x6C3A,0x58},    // SHD_GAIN_TH_H :
	{BYTE,0x6C3B,0x48},    // SHD_GAIN_TH_L :
	{WORD,0x6C3C,0x0FA0},	 // SHD_INP_TH_HB_H_RB
	{WORD,0x6C3E,0x0ED8},	 // SHD_INP_TH_HB_L_RB
	{WORD,0x6C40,0x0000},	 // SHD_INP_TH_LB_H_RB
	{WORD,0x6C42,0x0000},	 // SHD_INP_TH_LB_L_RB
	{BYTE,0x01BC,0x5D},    // [3]SHD_GAINCTRL_EN [2]SHD_INP_EN [0]SHD_EN
	{WORD,0x6804,0x0E89},	 // NORMR
	{WORD,0x6806,0x0D07},	 // NORMB
	{WORD,0x6808,0x0140},	 // AWBPRER
	{WORD,0x680A,0x023F},	 // AWBPREB
	{BYTE,0x6258,0xB0},    // INJUDGPOS
	{BYTE,0x6259,0xBA},    // OUTJUDGPOS
	{WORD,0x6238,0x0B6C},	 // INIT_CONT_INR :
	{WORD,0x623A,0x187B},	 // INIT_CONT_INB :
	{WORD,0x623C,0x0B6C},	 // INIT_CONT_OUTR :
	{WORD,0x623E,0x187B},	 // INIT_CONT_OUTB :
	{BYTE,0x629C,0x60},    // FRMOUT_RATIO_BLEND1_IN :	80=INIT_CONT
	{BYTE,0x629D,0x80},    // FRMOUT_RATIO_BLEND1_OUT :
	{BYTE,0x629E,0x60},    // FRMOUT_RATIO_BLEND2_IN :	80=Previous Ratio
	{BYTE,0x629F,0x00},    // FRMOUT_RATIO_BLEND2_OUT :
	{BYTE,0x6400,0xAA},    // INFRM_LEFT00 :
	{BYTE,0x6401,0xAA},    // INFRM_LEFT01 :
	{BYTE,0x6402,0xAA},    // INFRM_LEFT02 :
	{BYTE,0x6403,0xAA},    // INFRM_LEFT03 :
	{BYTE,0x6404,0xAA},    // INFRM_LEFT04 :
	{BYTE,0x6405,0xAA},    // INFRM_LEFT05 :
	{BYTE,0x6406,0xAA},    // INFRM_LEFT06 :
	{BYTE,0x6407,0xAA},    // INFRM_LEFT07 :
	{BYTE,0x6408,0xAA},    // INFRM_LEFT08 :
	{BYTE,0x6409,0xAE},    // INFRM_LEFT09 :
	{BYTE,0x640A,0xA0},    // INFRM_LEFT10 :
	{BYTE,0x640B,0x8C},    // INFRM_LEFT11 :
	{BYTE,0x640C,0x72},    // INFRM_LEFT12 :
	{BYTE,0x640D,0x64},    // INFRM_LEFT13 :
	{BYTE,0x640E,0x5A},    // INFRM_LEFT14 :
	{BYTE,0x640F,0x52},    // INFRM_LEFT15 :
	{BYTE,0x6410,0x4B},    // INFRM_LEFT16 :
	{BYTE,0x6411,0x46},    // INFRM_LEFT17 :
	{BYTE,0x6412,0x40},    // INFRM_LEFT18 :
	{BYTE,0x6413,0x3A},    // INFRM_LEFT19 :
	{BYTE,0x6414,0x36},    // INFRM_LEFT20 :
	{BYTE,0x6415,0x34},    // INFRM_LEFT21 :
	{BYTE,0x6416,0x33},    // INFRM_LEFT22 :
	{BYTE,0x6417,0x32},    // INFRM_LEFT23 :
	{BYTE,0x6418,0x31},    // INFRM_LEFT24 :
	{BYTE,0x6419,0x2F},    // INFRM_LEFT25 :
	{BYTE,0x641A,0x2D},    // INFRM_LEFT26 :
	{BYTE,0x641B,0x2A},    // INFRM_LEFT27 :
	{BYTE,0x641C,0x28},    // INFRM_LEFT28 :
	{BYTE,0x641D,0x26},    // INFRM_LEFT29 :
	{BYTE,0x641E,0x23},    // INFRM_LEFT30 :
	{BYTE,0x641F,0x23},    // INFRM_LEFT31 :
	{BYTE,0x6420,0x22},    // INFRM_LEFT32 :
	{BYTE,0x6421,0x1A},    // INFRM_LEFT33 :
	{BYTE,0x6422,0x18},    // INFRM_LEFT34 :
	{BYTE,0x6423,0x17},    // INFRM_LEFT35 :
	{BYTE,0x6424,0x16},    // INFRM_LEFT36 :
	{BYTE,0x6425,0x16},    // INFRM_LEFT37 :
	{BYTE,0x6426,0xAF},    // INFRM_RIGHT00 :
	{BYTE,0x6427,0xAF},    // INFRM_RIGHT01 :
	{BYTE,0x6428,0xAF},    // INFRM_RIGHT02 :
	{BYTE,0x6429,0xAF},    // INFRM_RIGHT03 :
	{BYTE,0x642A,0xAF},    // INFRM_RIGHT04 :
	{BYTE,0x642B,0xAF},    // INFRM_RIGHT05 :
	{BYTE,0x642C,0xAF},    // INFRM_RIGHT06 :
	{BYTE,0x642D,0xAF},    // INFRM_RIGHT07 :
	{BYTE,0x642E,0xAF},    // INFRM_RIGHT08 :
	{BYTE,0x642F,0xAA},    // INFRM_RIGHT09 :
	{BYTE,0x6430,0xB2},    // INFRM_RIGHT10 :
	{BYTE,0x6431,0xB4},    // INFRM_RIGHT11 :
	{BYTE,0x6432,0xB6},    // INFRM_RIGHT12 :
	{BYTE,0x6433,0xB4},    // INFRM_RIGHT13 :
	{BYTE,0x6434,0x9B},    // INFRM_RIGHT14 :
	{BYTE,0x6435,0x82},    // INFRM_RIGHT15 :
	{BYTE,0x6436,0x78},    // INFRM_RIGHT16 :
	{BYTE,0x6437,0x72},    // INFRM_RIGHT17 :
	{BYTE,0x6438,0x69},    // INFRM_RIGHT18 :
	{BYTE,0x6439,0x58},    // INFRM_RIGHT19 :
	{BYTE,0x643A,0x4D},    // INFRM_RIGHT20 :
	{BYTE,0x643B,0x47},    // INFRM_RIGHT21 :
	{BYTE,0x643C,0x44},    // INFRM_RIGHT22 :
	{BYTE,0x643D,0x45},    // INFRM_RIGHT23 :
	{BYTE,0x643E,0x46},    // INFRM_RIGHT24 :
	{BYTE,0x643F,0x4A},    // INFRM_RIGHT25 :
	{BYTE,0x6440,0x46},    // INFRM_RIGHT26 :
	{BYTE,0x6441,0x42},    // INFRM_RIGHT27 :
	{BYTE,0x6442,0x3F},    // INFRM_RIGHT28 :
	{BYTE,0x6443,0x3C},    // INFRM_RIGHT29 :
	{BYTE,0x6444,0x3A},    // INFRM_RIGHT30 :
	{BYTE,0x6445,0x38},    // INFRM_RIGHT31 :
	{BYTE,0x6446,0x37},    // INFRM_RIGHT32 :
	{BYTE,0x6447,0x2E},    // INFRM_RIGHT33 :
	{BYTE,0x6448,0x2D},    // INFRM_RIGHT34 :
	{BYTE,0x6449,0x2C},    // INFRM_RIGHT35 :
	{BYTE,0x644A,0x2C},    // INFRM_RIGHT36 :
	{BYTE,0x644B,0x36},    // INFRM_RIGHT37 :
	{WORD,0x644C,0x1F40},	 // INFRM_TOP :
	{WORD,0x644E,0x0940},	 // INFRM_BOTM :
	{BYTE,0x6450,0x19},    // INFRM_FLTOP :
	{BYTE,0x6451,0x10},    // INFRM_FLBOTM :
	{BYTE,0x6452,0x91},    // INAIM_LEFT00 :
	{BYTE,0x6453,0x91},    // INAIM_LEFT01 :
	{BYTE,0x6454,0x91},    // INAIM_LEFT02 :
	{BYTE,0x6455,0x91},    // INAIM_LEFT03 :
	{BYTE,0x6456,0x91},    // INAIM_LEFT04 :
	{BYTE,0x6457,0x91},    // INAIM_LEFT05 :
	{BYTE,0x6458,0x91},    // INAIM_LEFT06 :
	{BYTE,0x6459,0x91},    // INAIM_LEFT07 :
	{BYTE,0x645A,0x91},    // INAIM_LEFT08 :
	{BYTE,0x645B,0x91},    // INAIM_LEFT09 :
	{BYTE,0x645C,0x91},    // INAIM_LEFT10 :
	{BYTE,0x645D,0x91},    // INAIM_LEFT11 :
	{BYTE,0x645E,0x91},    // INAIM_LEFT12 :
	{BYTE,0x645F,0x66},    // INAIM_LEFT13 :
	{BYTE,0x6460,0x5D},    // INAIM_LEFT14 :
	{BYTE,0x6461,0x55},    // INAIM_LEFT15 :
	{BYTE,0x6462,0x4E},    // INAIM_LEFT16 :
	{BYTE,0x6463,0x47},    // INAIM_LEFT17 :
	{BYTE,0x6464,0x42},    // INAIM_LEFT18 :
	{BYTE,0x6465,0x3C},    // INAIM_LEFT19 :
	{BYTE,0x6466,0x38},    // INAIM_LEFT20 :
	{BYTE,0x6467,0x36},    // INAIM_LEFT21 :
	{BYTE,0x6468,0x35},    // INAIM_LEFT22 :
	{BYTE,0x6469,0x33},    // INAIM_LEFT23 :
	{BYTE,0x646A,0x32},    // INAIM_LEFT24 :
	{BYTE,0x646B,0x30},    // INAIM_LEFT25 :
	{BYTE,0x646C,0x2F},    // INAIM_LEFT26 :
	{BYTE,0x646D,0x2D},    // INAIM_LEFT27 :
	{BYTE,0x646E,0x2C},    // INAIM_LEFT28 :
	{BYTE,0x646F,0x2B},    // INAIM_LEFT29 :
	{BYTE,0x6470,0x2A},    // INAIM_LEFT30 :
	{BYTE,0x6471,0x28},    // INAIM_LEFT31 :
	{BYTE,0x6472,0x26},    // INAIM_LEFT32 :
	{BYTE,0x6473,0x24},    // INAIM_LEFT33 :
	{BYTE,0x6474,0x29},    // INAIM_LEFT34 :
	{BYTE,0x6475,0x28},    // INAIM_LEFT35 :
	{BYTE,0x6476,0x29},    // INAIM_LEFT36 :
	{BYTE,0x6477,0x26},    // INAIM_LEFT37 :
	{BYTE,0x6478,0xFF},    // INAIM_RIGHT00 :
	{BYTE,0x6479,0xFF},    // INAIM_RIGHT01 :
	{BYTE,0x647A,0xFF},    // INAIM_RIGHT02 :
	{BYTE,0x647B,0xFF},    // INAIM_RIGHT03 :
	{BYTE,0x647C,0xFF},    // INAIM_RIGHT04 :
	{BYTE,0x647D,0xFF},    // INAIM_RIGHT05 :
	{BYTE,0x647E,0xFF},    // INAIM_RIGHT06 :
	{BYTE,0x647F,0xFF},    // INAIM_RIGHT07 :
	{BYTE,0x6480,0xFF},    // INAIM_RIGHT08 :
	{BYTE,0x6481,0xFF},    // INAIM_RIGHT09 :
	{BYTE,0x6482,0xD9},    // INAIM_RIGHT10 :
	{BYTE,0x6483,0xB7},    // INAIM_RIGHT11 :
	{BYTE,0x6484,0x96},    // INAIM_RIGHT12 :
	{BYTE,0x6485,0x68},    // INAIM_RIGHT13 :
	{BYTE,0x6486,0x70},    // INAIM_RIGHT14 :
	{BYTE,0x6487,0x72},    // INAIM_RIGHT15 :
	{BYTE,0x6488,0x71},    // INAIM_RIGHT16 :
	{BYTE,0x6489,0x6B},    // INAIM_RIGHT17 :
	{BYTE,0x648A,0x65},    // INAIM_RIGHT18 :
	{BYTE,0x648B,0x56},    // INAIM_RIGHT19 :
	{BYTE,0x648C,0x4D},    // INAIM_RIGHT20 :
	{BYTE,0x648D,0x47},    // INAIM_RIGHT21 :
	{BYTE,0x648E,0x44},    // INAIM_RIGHT22 :
	{BYTE,0x648F,0x45},    // INAIM_RIGHT23 :
	{BYTE,0x6490,0x46},    // INAIM_RIGHT24 :
	{BYTE,0x6491,0x44},    // INAIM_RIGHT25 :
	{BYTE,0x6492,0x41},    // INAIM_RIGHT26 :
	{BYTE,0x6493,0x3E},    // INAIM_RIGHT27 :
	{BYTE,0x6494,0x3B},    // INAIM_RIGHT28 :
	{BYTE,0x6495,0x39},    // INAIM_RIGHT29 :
	{BYTE,0x6496,0x37},    // INAIM_RIGHT30 :
	{BYTE,0x6497,0x34},    // INAIM_RIGHT31 :
	{BYTE,0x6498,0x33},    // INAIM_RIGHT32 :
	{BYTE,0x6499,0x32},    // INAIM_RIGHT33 :
	{BYTE,0x649A,0x31},    // INAIM_RIGHT34 :
	{BYTE,0x649B,0x30},    // INAIM_RIGHT35 :
	{BYTE,0x649C,0x2F},    // INAIM_RIGHT36 :
	{BYTE,0x649D,0x2E},    // INAIM_RIGHT37 :
	{WORD,0x649E,0x1C20},	 // INAIM_TOP :
	{WORD,0x64A0,0x0D90},	 // INAIM_BOTM :
	{BYTE,0x64A2,0x18},    // INAIM_FLTOP :
	{BYTE,0x64A3,0x10},    // INAIM_FLBOTM :
	{BYTE,0x64A4,0xFF},    // OUTFRM_LEFT00 :
	{BYTE,0x64A5,0xFF},    // OUTFRM_LEFT01 :
	{BYTE,0x64A6,0xFF},    // OUTFRM_LEFT02 :
	{BYTE,0x64A7,0xFF},    // OUTFRM_LEFT03 :
	{BYTE,0x64A8,0xFF},    // OUTFRM_LEFT04 :
	{BYTE,0x64A9,0xFF},    // OUTFRM_LEFT05 :
	{BYTE,0x64AA,0xFF},    // OUTFRM_LEFT06 :
	{BYTE,0x64AB,0xFF},    // OUTFRM_LEFT07 :
	{BYTE,0x64AC,0xFF},    // OUTFRM_LEFT08 :
	{BYTE,0x64AD,0xFD},    // OUTFRM_LEFT09 :
	{BYTE,0x64AE,0xCB},    // OUTFRM_LEFT10 :
	{BYTE,0x64AF,0xA9},    // OUTFRM_LEFT11 :
	{BYTE,0x64B0,0x90},    // OUTFRM_LEFT12 :
	{BYTE,0x64B1,0x7D},    // OUTFRM_LEFT13 :
	{BYTE,0x64B2,0x70},    // OUTFRM_LEFT14 :
	{BYTE,0x64B3,0x65},    // OUTFRM_LEFT15 :
	{BYTE,0x64B4,0x5C},    // OUTFRM_LEFT16 :
	{BYTE,0x64B5,0x55},    // OUTFRM_LEFT17 :
	{BYTE,0x64B6,0x4F},    // OUTFRM_LEFT18 :
	{BYTE,0x64B7,0x32},    // OUTFRM_LEFT19 :
	{BYTE,0x64B8,0x4D},    // OUTFRM_LEFT20 :
	{BYTE,0x64B9,0x40},    // OUTFRM_LEFT21 :
	{BYTE,0x64BA,0x2D},    // OUTFRM_LEFT22 :
	{BYTE,0x64BB,0x2B},    // OUTFRM_LEFT23 :
	{BYTE,0x64BC,0x29},    // OUTFRM_LEFT24 :
	{BYTE,0x64BD,0x27},    // OUTFRM_LEFT25 :
	{BYTE,0x64BE,0x25},    // OUTFRM_LEFT26 :
	{BYTE,0x64BF,0x23},    // OUTFRM_LEFT27 :
	{BYTE,0x64C0,0x21},    // OUTFRM_LEFT28 :
	{BYTE,0x64C1,0x1F},    // OUTFRM_LEFT29 :
	{BYTE,0x64C2,0x1D},    // OUTFRM_LEFT30 :
	{BYTE,0x64C3,0x1B},    // OUTFRM_LEFT31 :
	{BYTE,0x64C4,0x1A},    // OUTFRM_LEFT32 :
	{BYTE,0x64C5,0x1A},    // OUTFRM_LEFT33 :
	{BYTE,0x64C6,0x1A},    // OUTFRM_LEFT34 :
	{BYTE,0x64C7,0x28},    // OUTFRM_LEFT35 :
	{BYTE,0x64C8,0x27},    // OUTFRM_LEFT36 :
	{BYTE,0x64C9,0x26},    // OUTFRM_LEFT37 :
	{BYTE,0x64CA,0xFF},    // OUTFRM_RIGHT00 :
	{BYTE,0x64CB,0xFF},    // OUTFRM_RIGHT01 :
	{BYTE,0x64CC,0xFF},    // OUTFRM_RIGHT02 :
	{BYTE,0x64CD,0xFF},    // OUTFRM_RIGHT03 :
	{BYTE,0x64CE,0xFF},    // OUTFRM_RIGHT04 :
	{BYTE,0x64CF,0xFF},    // OUTFRM_RIGHT05 :
	{BYTE,0x64D0,0xFF},    // OUTFRM_RIGHT06 :
	{BYTE,0x64D1,0xFF},    // OUTFRM_RIGHT07 :
	{BYTE,0x64D2,0xFF},    // OUTFRM_RIGHT08 :
	{BYTE,0x64D3,0xFF},    // OUTFRM_RIGHT09 :
	{BYTE,0x64D4,0xD3},    // OUTFRM_RIGHT10 :
	{BYTE,0x64D5,0xB1},    // OUTFRM_RIGHT11 :
	{BYTE,0x64D6,0x98},    // OUTFRM_RIGHT12 :
	{BYTE,0x64D7,0x85},    // OUTFRM_RIGHT13 :
	{BYTE,0x64D8,0x78},    // OUTFRM_RIGHT14 :
	{BYTE,0x64D9,0x6D},    // OUTFRM_RIGHT15 :
	{BYTE,0x64DA,0x64},    // OUTFRM_RIGHT16 :
	{BYTE,0x64DB,0x5D},    // OUTFRM_RIGHT17 :
	{BYTE,0x64DC,0x57},    // OUTFRM_RIGHT18 :
	{BYTE,0x64DD,0x63},    // OUTFRM_RIGHT19 :
	{BYTE,0x64DE,0x5E},    // OUTFRM_RIGHT20 :
	{BYTE,0x64DF,0x5A},    // OUTFRM_RIGHT21 :
	{BYTE,0x64E0,0x56},    // OUTFRM_RIGHT22 :
	{BYTE,0x64E1,0x52},    // OUTFRM_RIGHT23 :
	{BYTE,0x64E2,0x50},    // OUTFRM_RIGHT24 :
	{BYTE,0x64E3,0x4E},    // OUTFRM_RIGHT25 :
	{BYTE,0x64E4,0x4C},    // OUTFRM_RIGHT26 :
	{BYTE,0x64E5,0x4A},    // OUTFRM_RIGHT27 :
	{BYTE,0x64E6,0x48},    // OUTFRM_RIGHT28 :
	{BYTE,0x64E7,0x46},    // OUTFRM_RIGHT29 :
	{BYTE,0x64E8,0x44},    // OUTFRM_RIGHT30 :
	{BYTE,0x64E9,0x43},    // OUTFRM_RIGHT31 :
	{BYTE,0x64EA,0x42},    // OUTFRM_RIGHT32 :
	{BYTE,0x64EB,0x42},    // OUTFRM_RIGHT33 :
	{BYTE,0x64EC,0x42},    // OUTFRM_RIGHT34 :
	{BYTE,0x64ED,0x30},    // OUTFRM_RIGHT35 :
	{BYTE,0x64EE,0x2F},    // OUTFRM_RIGHT36 :
	{BYTE,0x64EF,0x2E},    // OUTFRM_RIGHT37 :
	{WORD,0x64F0,0x1CD2},	 // OUTFRM_TOP :
	{WORD,0x64F2,0x1400},	 // OUTFRM_BOTM :
	{BYTE,0x64F4,0x19},    // OUTFRM_FLTOP :
	{BYTE,0x64F5,0x14},    // OUTFRM_FLBOTM :
	{BYTE,0x64F6,0xFF},    // OUTAIM_LEFT00 :
	{BYTE,0x64F7,0xFF},    // OUTAIM_LEFT01 :
	{BYTE,0x64F8,0xFF},    // OUTAIM_LEFT02 :
	{BYTE,0x64F9,0xFF},    // OUTAIM_LEFT03 :
	{BYTE,0x64FA,0xFF},    // OUTAIM_LEFT04 :
	{BYTE,0x64FB,0xFF},    // OUTAIM_LEFT05 :
	{BYTE,0x64FC,0xFF},    // OUTAIM_LEFT06 :
	{BYTE,0x64FD,0xFF},    // OUTAIM_LEFT07 :
	{BYTE,0x64FE,0xFF},    // OUTAIM_LEFT08 :
	{BYTE,0x64FF,0xFF},    // OUTAIM_LEFT09 :
	{BYTE,0x6500,0x91},    // OUTAIM_LEFT10 :
	{BYTE,0x6501,0x91},    // OUTAIM_LEFT11 :
	{BYTE,0x6502,0x91},    // OUTAIM_LEFT12 :
	{BYTE,0x6503,0x66},    // OUTAIM_LEFT13 :
	{BYTE,0x6504,0x5D},    // OUTAIM_LEFT14 :
	{BYTE,0x6505,0x3C},    // OUTAIM_LEFT15 :
	{BYTE,0x6506,0x3C},    // OUTAIM_LEFT16 :
	{BYTE,0x6507,0x3C},    // OUTAIM_LEFT17 :
	{BYTE,0x6508,0x3A},    // OUTAIM_LEFT18 :
	{BYTE,0x6509,0x39},    // OUTAIM_LEFT19 :
	{BYTE,0x650A,0x40},    // OUTAIM_LEFT20 :
	{BYTE,0x650B,0x46},    // OUTAIM_LEFT21 :
	{BYTE,0x650C,0x42},    // OUTAIM_LEFT22 :
	{BYTE,0x650D,0x40},    // OUTAIM_LEFT23 :
	{BYTE,0x650E,0x3C},    // OUTAIM_LEFT24 :
	{BYTE,0x650F,0x37},    // OUTAIM_LEFT25 :
	{BYTE,0x6510,0x34},    // OUTAIM_LEFT26 :
	{BYTE,0x6511,0x32},    // OUTAIM_LEFT27 :
	{BYTE,0x6512,0x2F},    // OUTAIM_LEFT28 :
	{BYTE,0x6513,0x2E},    // OUTAIM_LEFT29 :
	{BYTE,0x6514,0x2C},    // OUTAIM_LEFT30 :
	{BYTE,0x6515,0x2A},    // OUTAIM_LEFT31 :
	{BYTE,0x6516,0x2D},    // OUTAIM_LEFT32 :
	{BYTE,0x6517,0x2C},    // OUTAIM_LEFT33 :
	{BYTE,0x6518,0x2B},    // OUTAIM_LEFT34 :
	{BYTE,0x6519,0x2A},    // OUTAIM_LEFT35 :
	{BYTE,0x651A,0x29},    // OUTAIM_LEFT36 :
	{BYTE,0x651B,0x28},    // OUTAIM_LEFT37 :
	{BYTE,0x651C,0xFF},    // OUTAIM_RIGHT00 :
	{BYTE,0x651D,0xFF},    // OUTAIM_RIGHT01 :
	{BYTE,0x651E,0xFF},    // OUTAIM_RIGHT02 :
	{BYTE,0x651F,0xFF},    // OUTAIM_RIGHT03 :
	{BYTE,0x6520,0xFF},    // OUTAIM_RIGHT04 :
	{BYTE,0x6521,0xFF},    // OUTAIM_RIGHT05 :
	{BYTE,0x6522,0xFF},    // OUTAIM_RIGHT06 :
	{BYTE,0x6523,0xFF},    // OUTAIM_RIGHT07 :
	{BYTE,0x6524,0xFF},    // OUTAIM_RIGHT08 :
	{BYTE,0x6525,0xFF},    // OUTAIM_RIGHT09 :
	{BYTE,0x6526,0xD9},    // OUTAIM_RIGHT10 :
	{BYTE,0x6527,0xB7},    // OUTAIM_RIGHT11 :
	{BYTE,0x6528,0x96},    // OUTAIM_RIGHT12 :
	{BYTE,0x6529,0x6C},    // OUTAIM_RIGHT13 :
	{BYTE,0x652A,0x64},    // OUTAIM_RIGHT14 :
	{BYTE,0x652B,0x62},    // OUTAIM_RIGHT15 :
	{BYTE,0x652C,0x62},    // OUTAIM_RIGHT16 :
	{BYTE,0x652D,0x61},    // OUTAIM_RIGHT17 :
	{BYTE,0x652E,0x60},    // OUTAIM_RIGHT18 :
	{BYTE,0x652F,0x5E},    // OUTAIM_RIGHT19 :
	{BYTE,0x6530,0x5B},    // OUTAIM_RIGHT20 :
	{BYTE,0x6531,0x4F},    // OUTAIM_RIGHT21 :
	{BYTE,0x6532,0x4B},    // OUTAIM_RIGHT22 :
	{BYTE,0x6533,0x49},    // OUTAIM_RIGHT23 :
	{BYTE,0x6534,0x44},    // OUTAIM_RIGHT24 :
	{BYTE,0x6535,0x3F},    // OUTAIM_RIGHT25 :
	{BYTE,0x6536,0x3D},    // OUTAIM_RIGHT26 :
	{BYTE,0x6537,0x3B},    // OUTAIM_RIGHT27 :
	{BYTE,0x6538,0x3B},    // OUTAIM_RIGHT28 :
	{BYTE,0x6539,0x3A},    // OUTAIM_RIGHT29 :
	{BYTE,0x653A,0x38},    // OUTAIM_RIGHT30 :
	{BYTE,0x653B,0x38},    // OUTAIM_RIGHT31 :
	{BYTE,0x653C,0x33},    // OUTAIM_RIGHT32 :
	{BYTE,0x653D,0x32},    // OUTAIM_RIGHT33 :
	{BYTE,0x653E,0x31},    // OUTAIM_RIGHT34 :
	{BYTE,0x653F,0x30},    // OUTAIM_RIGHT35 :
	{BYTE,0x6540,0x2F},    // OUTAIM_RIGHT36 :
	{BYTE,0x6541,0x2E},    // OUTAIM_RIGHT37 :
	{WORD,0x6542,0x1A56},	 // OUTAIM_TOP :
	{WORD,0x6544,0x16AF},	 // OUTAIM_BOTM :
	{BYTE,0x6546,0x19},    // OUTAIM_FLTOP :
	{BYTE,0x6547,0x17},    // OUTAIM_FLBOTM :
	{BYTE,0x657A,0x7E},    // IN_CTMP_FRM_BG0 :
	{BYTE,0x657B,0x6D},    // IN_CTMP_FRM_BG1 :
	{BYTE,0x657C,0x64},    // IN_CTMP_FRM_BG2 :
	{BYTE,0x657D,0x5B},    // IN_CTMP_FRM_BG3 :
	{BYTE,0x657E,0x55},    // IN_CTMP_FRM_BG4 :
	{BYTE,0x657F,0x4F},    // IN_CTMP_FRM_BG5 :
	{BYTE,0x6580,0x49},    // IN_CTMP_FRM_BG6 :
	{BYTE,0x6581,0x43},    // IN_CTMP_FRM_BG7 :
	{BYTE,0x6582,0x3E},    // IN_CTMP_FRM_BG8 :
	{BYTE,0x6583,0x38},    // IN_CTMP_FRM_BG9 :
	{BYTE,0x6584,0x22},    // IN_CTMP_FRM_BG10 :
	{BYTE,0x6585,0x23},    // IN_CTMP_FRM_RG0 :
	{BYTE,0x6586,0x33},    // IN_CTMP_FRM_RG1 :
	{BYTE,0x6587,0x3F},    // IN_CTMP_FRM_RG2 :
	{BYTE,0x6588,0x53},    // IN_CTMP_FRM_RG3 :
	{BYTE,0x6589,0x63},    // IN_CTMP_FRM_RG4 :
	{BYTE,0x658A,0x76},    // IN_CTMP_FRM_RG5 :
	{BYTE,0x658B,0xC1},    // IN_CTMP_FRM_RG6 :
	{BYTE,0x658C,0x00},    // IN_CTMP_WEIGHT00_01 :
	{BYTE,0x658D,0x00},    // IN_CTMP_WEIGHT02_03 :
	{BYTE,0x658E,0x00},    // IN_CTMP_WEIGHT04_05 :
	{BYTE,0x658F,0x10},    // IN_CTMP_WEIGHT06_07 :
	{BYTE,0x6590,0x00},    // IN_CTMP_WEIGHT08_09 :
	{BYTE,0x6591,0x00},    // IN_CTMP_WEIGHT10_11 :
	{BYTE,0x6592,0x10},    // IN_CTMP_WEIGHT12_13 :
	{BYTE,0x6593,0x03},    // IN_CTMP_WEIGHT14_15 :
	{BYTE,0x6594,0x00},    // IN_CTMP_WEIGHT16_17 :
	{BYTE,0x6595,0x70},    // IN_CTMP_WEIGHT18_19 :
	{BYTE,0x6596,0x07},    // IN_CTMP_WEIGHT20_21 :
	{BYTE,0x6597,0x00},    // IN_CTMP_WEIGHT22_23 :
	{BYTE,0x6598,0x60},    // IN_CTMP_WEIGHT24_25 :
	{BYTE,0x6599,0x37},    // IN_CTMP_WEIGHT26_27 :
	{BYTE,0x659A,0x00},    // IN_CTMP_WEIGHT28_29 :
	{BYTE,0x659B,0x30},    // IN_CTMP_WEIGHT30_31 :
	{BYTE,0x659C,0x37},    // IN_CTMP_WEIGHT32_33 :
	{BYTE,0x659D,0x00},    // IN_CTMP_WEIGHT34_35 :
	{BYTE,0x659E,0x00},    // IN_CTMP_WEIGHT36_37 :
	{BYTE,0x659F,0x33},    // IN_CTMP_WEIGHT38_39 :
	{BYTE,0x65A0,0x01},    // IN_CTMP_WEIGHT40_41 :
	{BYTE,0x65A1,0x00},    // IN_CTMP_WEIGHT42_43 :
	{BYTE,0x65A2,0x31},    // IN_CTMP_WEIGHT44_45 :
	{BYTE,0x65A3,0x01},    // IN_CTMP_WEIGHT46_47 :
	{BYTE,0x65A4,0x00},    // IN_CTMP_WEIGHT48_49 :
	{BYTE,0x65A5,0x10},    // IN_CTMP_WEIGHT50_51 :
	{BYTE,0x65A6,0x01},    // IN_CTMP_WEIGHT52_53 :
	{BYTE,0x65A7,0x00},    // IN_CTMP_WEIGHT54_55 :
	{BYTE,0x65A8,0x00},    // IN_CTMP_WEIGHT56_57 :
	{BYTE,0x65A9,0x00},    // IN_CTMP_WEIGHT58_59 :
	{BYTE,0x65AA,0x7D},    // OUT_CTMP_FRM_BG0 :
	{BYTE,0x65AB,0x74},    // OUT_CTMP_FRM_BG1 :
	{BYTE,0x65AC,0x70},    // OUT_CTMP_FRM_BG2 :
	{BYTE,0x65AD,0x6C},    // OUT_CTMP_FRM_BG3 :
	{BYTE,0x65AE,0x68},    // OUT_CTMP_FRM_BG4 :
	{BYTE,0x65AF,0x64},    // OUT_CTMP_FRM_BG5 :
	{BYTE,0x65B0,0x60},    // OUT_CTMP_FRM_BG6 :
	{BYTE,0x65B1,0x5C},    // OUT_CTMP_FRM_BG7 :
	{BYTE,0x65B2,0x58},    // OUT_CTMP_FRM_BG8 :
	{BYTE,0x65B3,0x54},    // OUT_CTMP_FRM_BG9 :
	{BYTE,0x65B4,0x50},    // OUT_CTMP_FRM_BG10 :
	{BYTE,0x65B5,0x19},    // OUT_CTMP_FRM_RG0 :
	{BYTE,0x65B6,0x27},    // OUT_CTMP_FRM_RG1 :
	{BYTE,0x65B7,0x32},    // OUT_CTMP_FRM_RG2 :
	{BYTE,0x65B8,0x3E},    // OUT_CTMP_FRM_RG3 :
	{BYTE,0x65B9,0x49},    // OUT_CTMP_FRM_RG4 :
	{BYTE,0x65BA,0x54},    // OUT_CTMP_FRM_RG5 :
	{BYTE,0x65BB,0x5E},    // OUT_CTMP_FRM_RG6 :
	{BYTE,0x65BC,0x00},    // OUT_CTMP_WEIGHT00_01 :
	{BYTE,0x65BD,0x00},    // OUT_CTMP_WEIGHT02_03 :
	{BYTE,0x65BE,0x00},    // OUT_CTMP_WEIGHT04_05 :
	{BYTE,0x65BF,0x00},    // OUT_CTMP_WEIGHT06_07 :
	{BYTE,0x65C0,0x00},    // OUT_CTMP_WEIGHT08_09 :
	{BYTE,0x65C1,0x00},    // OUT_CTMP_WEIGHT10_11 :
	{BYTE,0x65C2,0x11},    // OUT_CTMP_WEIGHT12_13 :
	{BYTE,0x65C3,0x11},    // OUT_CTMP_WEIGHT14_15 :
	{BYTE,0x65C4,0x00},    // OUT_CTMP_WEIGHT16_17 :
	{BYTE,0x65C5,0x20},    // OUT_CTMP_WEIGHT18_19 :
	{BYTE,0x65C6,0x22},    // OUT_CTMP_WEIGHT20_21 :
	{BYTE,0x65C7,0x02},    // OUT_CTMP_WEIGHT22_23 :
	{BYTE,0x65C8,0x30},    // OUT_CTMP_WEIGHT24_25 :
	{BYTE,0x65C9,0x33},    // OUT_CTMP_WEIGHT26_27 :
	{BYTE,0x65CA,0x03},    // OUT_CTMP_WEIGHT28_29 :
	{BYTE,0x65CB,0x30},    // OUT_CTMP_WEIGHT30_31 :
	{BYTE,0x65CC,0x77},    // OUT_CTMP_WEIGHT32_33 :
	{BYTE,0x65CD,0x03},    // OUT_CTMP_WEIGHT34_35 :
	{BYTE,0x65CE,0x30},    // OUT_CTMP_WEIGHT36_37 :
	{BYTE,0x65CF,0x77},    // OUT_CTMP_WEIGHT38_39 :
	{BYTE,0x65D0,0x03},    // OUT_CTMP_WEIGHT40_41 :
	{BYTE,0x65D1,0x30},    // OUT_CTMP_WEIGHT42_43 :
	{BYTE,0x65D2,0x33},    // OUT_CTMP_WEIGHT44_45 :
	{BYTE,0x65D3,0x03},    // OUT_CTMP_WEIGHT46_47 :
	{BYTE,0x65D4,0x10},    // OUT_CTMP_WEIGHT48_49 :
	{BYTE,0x65D5,0x11},    // OUT_CTMP_WEIGHT50_51 :
	{BYTE,0x65D6,0x11},    // OUT_CTMP_WEIGHT52_53 :
	{BYTE,0x65D7,0x00},    // OUT_CTMP_WEIGHT54_55 :
	{BYTE,0x65D8,0x00},    // OUT_CTMP_WEIGHT56_57 :
	{BYTE,0x65D9,0x00},    // OUT_CTMP_WEIGHT58_59 :
	{BYTE,0x0180,0x00},    // EV LEVEL
	{BYTE,0x02F0,0x00},    // EVREF_CAP_SN1_2
	{WORD,0x02FC,0x2100},	 // EVREF_TYPE1  2272
	{BYTE,0x0326,0x22},    // SHTCTRLTIME1_TYPE1 :
	{BYTE,0x0327,0x12},    // AGCGAIN1_TYPE1 :
	{BYTE,0x0328,0x53},    // SHTCTRLTIME2_TYPE1 :
	{BYTE,0x0329,0x23},    // AGCGAIN2_TYPE1 :
	{BYTE,0x032A,0x7D},    // SHTCTRLTIME3_TYPE1 :
	{BYTE,0x032B,0x25},    // AGCGAIN3_TYPE1 :
	{BYTE,0x5E00,0x05},    // FLCMODE 0: AUTO 5: Fixed 50Hz
	{BYTE,0x5E30,0x14},    // AESPEED
	{BYTE,0x5E00,0x05},    // Flicker 50HZ
	{WORD,0x7000,0x0000},	 // G0_KNOT_G0 :
	{WORD,0x7002,0x001B},	 // G0_KNOT_G1 :
	{WORD,0x7004,0x001F},	 // G0_KNOT_G2 :
	{WORD,0x7006,0x0035},	 // G0_KNOT_G3 :
	{WORD,0x7008,0x003F},	 // G0_KNOT_G4 :
	{WORD,0x700A,0x0050},	 // G0_KNOT_G5 :
	{WORD,0x700C,0x005A},	 // G0_KNOT_G6 :
	{WORD,0x700E,0x0065},	 // G0_KNOT_G7 :
	{WORD,0x7010,0x006C},	 // G0_KNOT_G8 :
	{WORD,0x7012,0x0074},	 // G0_KNOT_G9 :
	{WORD,0x7014,0x0052},	 // G0_KNOT_G10 :
	{WORD,0x7016,0x0091},	 // G0_KNOT_G11 :
	{WORD,0x7018,0x00B8},	 // G0_KNOT_G12 :
	{WORD,0x701A,0x00D2},	 // G0_KNOT_G13 :
	{WORD,0x701C,0x00E2},	 // G0_KNOT_G14 :
	{WORD,0x701E,0x00F1},	 // G0_KNOT_G15 :
	{WORD,0x7020,0x00FA},	 // G0_KNOT_G16 :
	{WORD,0x7022,0x0104},	 // G0_KNOT_G17 :
	{WORD,0x7024,0x0104},	 // G0_KNOT_G18 :
	{BYTE,0x9211,0x93},    // GAIN_TH_A_TYPE3 :
	{BYTE,0x9212,0xA1},    // GAIN_TH_B_TYPE3 :
	{BYTE,0x9213,0xB4},    // GAIN_TH_C_TYPE3 :
	{BYTE,0x5005,0xDB},    // IHGAIN_OFF
	{BYTE,0x942F,0x2A},    // AP_N_GAIN_POS_A :
	{BYTE,0x9430,0x52},    // AP_N_GAIN_POS_B :
	{BYTE,0x9431,0x44},    // AP_N_GAIN_POS_C1 :
	{BYTE,0x9432,0x44},    // AP_N_GAIN_POS_C2 :
	{BYTE,0x9433,0x29},    // AP_N_GAIN_NEG_A :
	{BYTE,0x9434,0x52},    // AP_N_GAIN_NEG_B :
	{BYTE,0x9435,0x3A},    // AP_N_GAIN_NEG_C1 :
	{BYTE,0x9436,0x3A},    // AP_N_GAIN_NEG_C2 :
	{BYTE,0x9447,0x1C},    // AP_H_GAIN_POS_A :
	{BYTE,0x9448,0x1E},    // AP_H_GAIN_POS_B :
	{BYTE,0x9449,0x3F},    // AP_H_GAIN_POS_C1 :
	{BYTE,0x944A,0x8B},    // AP_H_GAIN_POS_C2 :
	{BYTE,0x944B,0x1C},    // AP_H_GAIN_NEG_A :
	{BYTE,0x944C,0x1D},    // AP_H_GAIN_NEG_B :
	{BYTE,0x944D,0x3E},    // AP_H_GAIN_NEG_C1 :
	{BYTE,0x944E,0x8B},    // AP_H_GAIN_NEG_C2 :
	{BYTE,0x945F,0x0C},    // AP_L_GAIN_POS_A :
	{BYTE,0x9460,0x1D},    // AP_L_GAIN_POS_B :
	{BYTE,0x9461,0x24},    // AP_L_GAIN_POS_C1 :
	{BYTE,0x9462,0x62},    // AP_L_GAIN_POS_C2 :
	{BYTE,0x9463,0x0C},    // AP_L_GAIN_NEG_A :
	{BYTE,0x9464,0x16},    // AP_L_GAIN_NEG_B :
	{BYTE,0x9465,0x1B},    // AP_L_GAIN_NEG_C1 :
	{BYTE,0x9466,0x1B},    // AP_L_GAIN_NEG_C2 :
	{WORD,0x9510,0x00C8},	 // AP_POST_LIM_POS_A :
	{WORD,0x9512,0x00C8},	 // AP_POST_LIM_POS_B :
	{WORD,0x9514,0x00C8},	 // AP_POST_LIM_POS_C1 :
	{WORD,0x9516,0x00C8},	 // AP_POST_LIM_POS_C2 :
	{WORD,0x9518,0x00C8},	 // AP_POST_LIM_NEG_A :
	{WORD,0x951A,0x00C8},	 // AP_POST_LIM_NEG_B :
	{WORD,0x951C,0x00C8},	 // AP_POST_LIM_NEG_C1 :
	{WORD,0x951E,0x00C8},	 // AP_POST_LIM_NEG_C2 :
	{WORD,0x9520,0x0002},	 // AP_POST_CORE_POS_A :
	{WORD,0x9522,0x0002},	 // AP_POST_CORE_POS_B :
	{WORD,0x9524,0x0000},	 // AP_POST_CORE_POS_C1 :
	{WORD,0x9526,0x0000},	 // AP_POST_CORE_POS_C2 :
	{WORD,0x9528,0x0002},	 // AP_POST_CORE_NEG_A :
	{WORD,0x952A,0x0002},	 // AP_POST_CORE_NEG_B :
	{WORD,0x952C,0x0000},	 // AP_POST_CORE_NEG_C1 :
	{WORD,0x952E,0x0000},	 // AP_POST_CORE_NEG_C2 :
	{WORD,0x6E86, 0x0000},	 // IBYHUE1_POS1 :
	{WORD,0x6E88, 0xFFF6},	 // IRYHUE1_POS1 :
	{WORD,0x6E8A, 0xFFF8},	 // IBYHUE2_POS1 :
	{WORD,0x6E8C, 0xFFF7},	 // IRYHUE2_POS1 :
	{WORD,0x6E8E, 0xFFF8},	 // IBYHUE3_POS1 :
	{WORD,0x6E90, 0xFFEE},	 // IRYHUE3_POS1 :
	{WORD,0x6E92, 0x0000},	 // IBYHUE4_POS1 :
	{WORD,0x6E94, 0xFFEC},	 // IRYHUE4_POS1 :
	{WORD,0x6E96, 0x0000},	 // IBYHUE1_POS2 :
	{WORD,0x6E98, 0xFFF6},	 // IRYHUE1_POS2 :
	{WORD,0x6E9A, 0xFFF8},	 // IBYHUE2_POS2 :
	{WORD,0x6E9C, 0xFFF7},	 // IRYHUE2_POS2 :
	{WORD,0x6E9E, 0x0008},	 // IBYHUE3_POS2 :
	{WORD,0x6EA0, 0xFFEE},	 // IRYHUE3_POS2 :
	{WORD,0x6EA2, 0x0000},	 // IBYHUE4_POS2 :
	{WORD,0x6EA4, 0xFFEC},	 // IRYHUE4_POS2 :
	{WORD,0x6EA6, 0x0000},	 // IBYHUE1_POS3 :
	{WORD,0x6EA8, 0xFFF6},	 // IRYHUE1_POS3 :
	{WORD,0x6EAA, 0xFFF8},	 // IBYHUE2_POS3 :
	{WORD,0x6EAC, 0xFFF7},	 // IRYHUE2_POS3 :
	{WORD,0x6EAE, 0xFFF8},	 // IBYHUE3_POS3 :
	{WORD,0x6EB0, 0xFFEE},	 // IRYHUE3_POS3 :
	{WORD,0x6EB2, 0x0000},	 // IBYHUE4_POS3 :
	{WORD,0x6EB4, 0xFFEC},	 // IRYHUE4_POS3 :
	{WORD,0x6EB6, 0x0000},	 // IBYHUE1_POS4 :
	{WORD,0x6EB8, 0xFFF6},	 // IRYHUE1_POS4 :
	{WORD,0x6EBA, 0xFFF8},	 // IBYHUE2_POS4 :
	{WORD,0x6EBC, 0xFFF7},	 // IRYHUE2_POS4 :
	{WORD,0x6EBE, 0xFFF8},	 // IBYHUE3_POS4 :
	{WORD,0x6EC0, 0xFFEE},	 // IRYHUE3_POS4 :
	{WORD,0x6EC2, 0x0000},	 // IBYHUE4_POS4 :
	{WORD,0x6EC4, 0xFFEC},	 // IRYHUE4_POS4 :
	{WORD,0x6EC6, 0x0000},	 // IBYHUE1_POS5 :
	{WORD,0x6EC8, 0xFFF6},	 // IRYHUE1_POS5 :
	{WORD,0x6ECA, 0xFFF8},	 // IBYHUE2_POS5 :
	{WORD,0x6ECC, 0xFFF7},	 // IRYHUE2_POS5 :
	{WORD,0x6ECE, 0xFFF8},	 // IBYHUE3_POS5 :
	{WORD,0x6ED0, 0xFFEE},	 // IRYHUE3_POS5 :
	{WORD,0x6ED2, 0x0000},	 // IBYHUE4_POS5 :
	{WORD,0x6ED4, 0xFFEC},	 // IRYHUE4_POS5 :
	{WORD,0x6ED6, 0x0000},	 // IBYHUE1_POS6 :
	{WORD,0x6ED8, 0xFFF6},	 // IRYHUE1_POS6 :
	{WORD,0x6EDA, 0xFFF8},	 // IBYHUE2_POS6 :
	{WORD,0x6EDC, 0xFFF7},	 // IRYHUE2_POS6 :
	{WORD,0x6EDE, 0xFFF8},	 // IBYHUE3_POS6 :
	{WORD,0x6EE0, 0xFFEE},	 // IRYHUE3_POS6 :
	{WORD,0x6EE2, 0x0000},	 // IBYHUE4_POS6 :
	{WORD,0x6EE4, 0xFFEC},	 // IRYHUE4_POS6 :
	{WORD,0x6EE6, 0x0000},	 // IBYHUE1_POS7 :
	{WORD,0x6EE8, 0xFFF6},	 // IRYHUE1_POS7 :
	{WORD,0x6EEA, 0xFFF8},	 // IBYHUE2_POS7 :
	{WORD,0x6EEC, 0xFFF7},	 // IRYHUE2_POS7 :
	{WORD,0x6EEE, 0xFFF8},	 // IBYHUE3_POS7 :
	{WORD,0x6EF0, 0xFFEE},	 // IRYHUE3_POS7 :
	{WORD,0x6EF2, 0x0000},	 // IBYHUE4_POS7 :
	{WORD,0x6EF4, 0xFFEC},	 // IRYHUE4_POS7 :
	{WORD,0x6EF6, 0xFFF5},	 // IBYHUE1_OUT :
	{WORD,0x6EF8, 0xFFEB},	 // IRYHUE1_OUT :
	{WORD,0x6EFA, 0xFFFD},	 // IBYHUE2_OUT :
	{WORD,0x6EFC, 0xFFEF},	 // IRYHUE2_OUT :
	{WORD,0x6EFE, 0xFFFD},	 // IBYHUE3_OUT :
	{WORD,0x6F00, 0xFFD8},	 // IRYHUE3_OUT :
	{WORD,0x6F02, 0xFFF5},	 // IBYHUE4_OUT :
	{WORD,0x6F04, 0xFFCF},	 // IRYHUE4_OUT :
	{WORD,0x6F06, 0x0000},	 // IBYHUE1_R2_POS4 :
	{WORD,0x6F08, 0xFFF6},	 // IRYHUE1_R2_POS4 :
	{WORD,0x6F0A, 0xFFF8},	 // IBYHUE2_R2_POS4 :
	{WORD,0x6F0C, 0xFFF7},	 // IRYHUE2_R2_POS4 :
	{WORD,0x6F0E, 0xFFF8},	 // IBYHUE3_R2_POS4 :
	{WORD,0x6F10, 0xFFEE},	 // IRYHUE3_R2_POS4 :
	{WORD,0x6F12, 0x0000},	 // IBYHUE4_R2_POS4 :
	{WORD,0x6F14, 0xFFEC},	 // IRYHUE4_R2_POS4 :
	{WORD,0x6F16, 0x0000},	 // IBYHUE1_R2_POS5 :
	{WORD,0x6F18, 0xFFF6},	 // IRYHUE1_R2_POS5 :
	{WORD,0x6F1A, 0xFFF8},	 // IBYHUE2_R2_POS5 :
	{WORD,0x6F1C, 0xFFF7},	 // IRYHUE2_R2_POS5 :
	{WORD,0x6F1E, 0xFFF8},	 // IBYHUE3_R2_POS5 :
	{WORD,0x6F20, 0xFFEE},	 // IRYHUE3_R2_POS5 :
	{WORD,0x6F22, 0x0000},	 // IBYHUE4_R2_POS5 :
	{WORD,0x6F24, 0xFFEC},	 // IRYHUE4_R2_POS5 :
	{BYTE,0x6F26, 0x4E},    // IRYGAIN1_POS1 :
	{BYTE,0x6F27, 0x50},    // IBYGAIN1_POS1 :
	{BYTE,0x6F28, 0x4E},    // IRYGAIN2_POS1 :
	{BYTE,0x6F29, 0x5A},    // IBYGAIN2_POS1 :
	{BYTE,0x6F2A, 0x50},    // IRYGAIN3_POS1 :
	{BYTE,0x6F2B, 0x5A},    // IBYGAIN3_POS1 :
	{BYTE,0x6F2C, 0x50},    // IRYGAIN4_POS1 :
	{BYTE,0x6F2D, 0x50},    // IBYGAIN4_POS1 :
	{BYTE,0x6F2E, 0x4E},    // IRYGAIN1_POS2 :
	{BYTE,0x6F2F, 0x50},    // IBYGAIN1_POS2 :
	{BYTE,0x6F30, 0x4E},    // IRYGAIN2_POS2 :
	{BYTE,0x6F31, 0x5A},    // IBYGAIN2_POS2 :
	{BYTE,0x6F32, 0x50},    // IRYGAIN3_POS2 :
	{BYTE,0x6F33, 0x5A},    // IBYGAIN3_POS2 :
	{BYTE,0x6F34, 0x50},    // IRYGAIN4_POS2 :
	{BYTE,0x6F35, 0x50},    // IBYGAIN4_POS2 :
	{BYTE,0x6F36, 0x4E},    // IRYGAIN1_POS3 :
	{BYTE,0x6F37, 0x50},    // IBYGAIN1_POS3 :
	{BYTE,0x6F38, 0x4E},    // IRYGAIN2_POS3 :
	{BYTE,0x6F39, 0x5A},    // IBYGAIN2_POS3 :
	{BYTE,0x6F3A, 0x50},    // IRYGAIN3_POS3 :
	{BYTE,0x6F3B, 0x5A},    // IBYGAIN3_POS3 :
	{BYTE,0x6F3C, 0x50},    // IRYGAIN4_POS3 :
	{BYTE,0x6F3D, 0x50},    // IBYGAIN4_POS3 :
	{BYTE,0x6F3E, 0x4E},    // IRYGAIN1_POS4 :
	{BYTE,0x6F3F, 0x50},    // IBYGAIN1_POS4 :
	{BYTE,0x6F40, 0x4E},    // IRYGAIN2_POS4 :
	{BYTE,0x6F41, 0x5A},    // IBYGAIN2_POS4 :
	{BYTE,0x6F42, 0x50},    // IRYGAIN3_POS4 :
	{BYTE,0x6F43, 0x5A},    // IBYGAIN3_POS4 :
	{BYTE,0x6F44, 0x50},    // IRYGAIN4_POS4 :
	{BYTE,0x6F45, 0x50},    // IBYGAIN4_POS4 :
	{BYTE,0x6F46, 0x4E},    // IRYGAIN1_POS5 :
	{BYTE,0x6F47, 0x50},    // IBYGAIN1_POS5 :
	{BYTE,0x6F48, 0x4E},    // IRYGAIN2_POS5 :
	{BYTE,0x6F49, 0x5A},    // IBYGAIN2_POS5 :
	{BYTE,0x6F4A, 0x50},    // IRYGAIN3_POS5 :
	{BYTE,0x6F4B, 0x5A},    // IBYGAIN3_POS5 :
	{BYTE,0x6F4C, 0x50},    // IRYGAIN4_POS5 :
	{BYTE,0x6F4D, 0x50},    // IBYGAIN4_POS5 :
	{BYTE,0x6F4E, 0x4E},    // IRYGAIN1_POS6 :
	{BYTE,0x6F4F, 0x50},    // IBYGAIN1_POS6 :
	{BYTE,0x6F50, 0x4E},    // IRYGAIN2_POS6 :
	{BYTE,0x6F51, 0x5A},    // IBYGAIN2_POS6 :
	{BYTE,0x6F52, 0x50},    // IRYGAIN3_POS6 :
	{BYTE,0x6F53, 0x5A},    // IBYGAIN3_POS6 :
	{BYTE,0x6F54, 0x50},    // IRYGAIN4_POS6 :
	{BYTE,0x6F55, 0x50},    // IBYGAIN4_POS6 :
	{BYTE,0x6F56, 0x4E},    // IRYGAIN1_POS7 :
	{BYTE,0x6F57, 0x50},    // IBYGAIN1_POS7 :
	{BYTE,0x6F58, 0x4E},    // IRYGAIN2_POS7 :
	{BYTE,0x6F59, 0x5A},    // IBYGAIN2_POS7 :
	{BYTE,0x6F5A, 0x50},    // IRYGAIN3_POS7 :
	{BYTE,0x6F5B, 0x5A},    // IBYGAIN3_POS7 :
	{BYTE,0x6F5C, 0x50},    // IRYGAIN4_POS7 :
	{BYTE,0x6F5D, 0x50},    // IBYGAIN4_POS7 :
	{BYTE,0x6F5E, 0x78},    // IRYGAIN1_OUT :
	{BYTE,0x6F5F, 0x4E},    // IBYGAIN1_OUT :
	{BYTE,0x6F60, 0x78},    // IRYGAIN2_OUT :
	{BYTE,0x6F61, 0x5F},    // IBYGAIN2_OUT :
	{BYTE,0x6F62, 0x78},    // IRYGAIN3_OUT :
	{BYTE,0x6F63, 0x5F},    // IBYGAIN3_OUT :
	{BYTE,0x6F64, 0x78},    // IRYGAIN4_OUT :
	{BYTE,0x6F65, 0x4E},    // IBYGAIN4_OUT :
	{BYTE,0x6F66, 0x4E},    // IRYGAIN1_R2_POS4 :
	{BYTE,0x6F67, 0x50},    // IBYGAIN1_R2_POS4 :
	{BYTE,0x6F68, 0x4E},    // IRYGAIN2_R2_POS4 :
	{BYTE,0x6F69, 0x5A},    // IBYGAIN2_R2_POS4 :
	{BYTE,0x6F6A, 0x50},    // IRYGAIN3_R2_POS4 :
	{BYTE,0x6F6B, 0x5A},    // IBYGAIN3_R2_POS4 :
	{BYTE,0x6F6C, 0x50},    // IRYGAIN4_R2_POS4 :
	{BYTE,0x6F6D, 0x50},    // IBYGAIN4_R2_POS4 :
	{BYTE,0x6F6E, 0x4E},    // IRYGAIN1_R2_POS5 :
	{BYTE,0x6F6F, 0x50},    // IBYGAIN1_R2_POS5 :
	{BYTE,0x6F70, 0x4E},    // IRYGAIN2_R2_POS5 :
	{BYTE,0x6F71, 0x5A},    // IBYGAIN2_R2_POS5 :
	{BYTE,0x6F72, 0x50},    // IRYGAIN3_R2_POS5 :
	{BYTE,0x6F73, 0x5A},    // IBYGAIN3_R2_POS5 :
	{BYTE,0x6F74, 0x50},    // IRYGAIN4_R2_POS5 :
	{BYTE,0x6F75, 0x50},    // IBYGAIN4_R2_POS5 :
	{BYTE,0x7638,0x41},    // MC3_RDEF0_POS1 :
	{BYTE,0x7639,0x46},    // MC3_RDEF1_POS1 :
	{BYTE,0x763A,0x46},    // MC3_RDEF2_POS1 :
	{BYTE,0x763B,0x71},    // MC3_RDEF3_POS1 :
	{BYTE,0x763C,0x41},    // MC3_RDEF0_POS2 :
	{BYTE,0x763D,0x46},    // MC3_RDEF1_POS2 :
	{BYTE,0x763E,0x46},    // MC3_RDEF2_POS2 :
	{BYTE,0x763F,0x71},    // MC3_RDEF3_POS2 :
	{BYTE,0x7640,0x3C},    // MC3_RDEF0_POS3 :
	{BYTE,0x7641,0x46},    // MC3_RDEF1_POS3 :
	{BYTE,0x7642,0x46},    // MC3_RDEF2_POS3 :
	{BYTE,0x7643,0x71},    // MC3_RDEF3_POS3 :
	{BYTE,0x7644,0x46},    // MC3_RDEF0_POS4 :
	{BYTE,0x7645,0x46},    // MC3_RDEF1_POS4 :
	{BYTE,0x7646,0x46},    // MC3_RDEF2_POS4 :
	{BYTE,0x7647,0x71},    // MC3_RDEF3_POS4 :
	{BYTE,0x7648,0x46},    // MC3_RDEF0_POS5 :
	{BYTE,0x7649,0x46},    // MC3_RDEF1_POS5 :
	{BYTE,0x764A,0x46},    // MC3_RDEF2_POS5 :
	{BYTE,0x764B,0x71},    // MC3_RDEF3_POS5 :
	{BYTE,0x764C,0x46},    // MC3_RDEF0_POS6 :
	{BYTE,0x764D,0x46},    // MC3_RDEF1_POS6 :
	{BYTE,0x764E,0x46},    // MC3_RDEF2_POS6 :
	{BYTE,0x764F,0x71},    // MC3_RDEF3_POS6 :
	{BYTE,0x7650,0x46},    // MC3_RDEF0_POS7 :
	{BYTE,0x7651,0x46},    // MC3_RDEF1_POS7 :
	{BYTE,0x7652,0x46},    // MC3_RDEF2_POS7 :
	{BYTE,0x7653,0x71},    // MC3_RDEF3_POS7 :
	{BYTE,0x7654,0x2D},    // MC3_RDEF0_OUT :
	{BYTE,0x7655,0x2D},    // MC3_RDEF1_OUT :
	{BYTE,0x7656,0x62},    // MC3_RDEF2_OUT :
	{BYTE,0x7657,0x54},    // MC3_RDEF3_OUT :
	{BYTE,0x7658,0x46},    // MC3_RDEF0_R2_POS4 :
	{BYTE,0x7659,0x32},    // MC3_RDEF1_R2_POS4 :
	{BYTE,0x765A,0x46},    // MC3_RDEF2_R2_POS4 :
	{BYTE,0x765B,0x71},    // MC3_RDEF3_R2_POS4 :
	{BYTE,0x765C,0x46},    // MC3_RDEF0_R2_POS5 :
	{BYTE,0x765D,0x32},    // MC3_RDEF1_R2_POS5 :
	{BYTE,0x765E,0x46},    // MC3_RDEF2_R2_POS5 :
	{BYTE,0x765F,0x71},    // MC3_RDEF3_R2_POS5 :
	{WORD,0x7660,0xFFBA},	 // MC3_X0DEF0_POS1 :
	{WORD,0x7662,0xFFBA},	 // MC3_Y0DEF0_POS1 :
	{WORD,0x7664,0xFFE2},	 // MC3_X0DEF1_POS1 :
	{WORD,0x7666,0x0039},	 // MC3_Y0DEF1_POS1 :
	{WORD,0x7668,0xFFD3},	 // MC3_X0DEF2_POS1 :
	{WORD,0x766A,0xFFF6},	 // MC3_Y0DEF2_POS1 :
	{WORD,0x766C,0x003B},	 // MC3_X0DEF3_POS1 :
	{WORD,0x766E,0xFFBB},	 // MC3_Y0DEF3_POS1 :
	{WORD,0x7670,0xFFBA},	 // MC3_X0DEF0_POS2 :
	{WORD,0x7672,0xFFBA},	 // MC3_Y0DEF0_POS2 :
	{WORD,0x7674,0xFFE2},	 // MC3_X0DEF1_POS2 :
	{WORD,0x7676,0x0039},	 // MC3_Y0DEF1_POS2 :
	{WORD,0x7678,0xFFD3},	 // MC3_X0DEF2_POS2 :
	{WORD,0x767A,0xFFF6},	 // MC3_Y0DEF2_POS2 :
	{WORD,0x767C,0x003B},	 // MC3_X0DEF3_POS2 :
	{WORD,0x767E,0xFFBB},	 // MC3_Y0DEF3_POS2 :
	{WORD,0x7680,0xFFCE},	 // MC3_X0DEF0_POS3 :
	{WORD,0x7682,0xFFBA},	 // MC3_Y0DEF0_POS3 :
	{WORD,0x7684,0xFFE2},	 // MC3_X0DEF1_POS3 :
	{WORD,0x7686,0x0039},	 // MC3_Y0DEF1_POS3 :
	{WORD,0x7688,0xFFD3},	 // MC3_X0DEF2_POS3 :
	{WORD,0x768A,0xFFF6},	 // MC3_Y0DEF2_POS3 :
	{WORD,0x768C,0x003B},	 // MC3_X0DEF3_POS3 :
	{WORD,0x768E,0xFFBB},	 // MC3_Y0DEF3_POS3 :
	{WORD,0x7690,0xFFCE},	 // MC3_X0DEF0_POS4 :
	{WORD,0x7692,0xFFC9},	 // MC3_Y0DEF0_POS4 :
	{WORD,0x7694,0xFFE2},	 // MC3_X0DEF1_POS4 :
	{WORD,0x7696,0x0039},	 // MC3_Y0DEF1_POS4 :
	{WORD,0x7698,0xFFD3},	 // MC3_X0DEF2_POS4 :
	{WORD,0x769A,0xFFF6},	 // MC3_Y0DEF2_POS4 :
	{WORD,0x769C,0x003B},	 // MC3_X0DEF3_POS4 :
	{WORD,0x769E,0xFFBB},	 // MC3_Y0DEF3_POS4 :
	{WORD,0x76A0,0xFFCE},	 // MC3_X0DEF0_POS5 :
	{WORD,0x76A2,0xFFC9},	 // MC3_Y0DEF0_POS5 :
	{WORD,0x76A4,0xFFE2},	 // MC3_X0DEF1_POS5 :
	{WORD,0x76A6,0x0039},	 // MC3_Y0DEF1_POS5 :
	{WORD,0x76A8,0xFFD3},	 // MC3_X0DEF2_POS5 :
	{WORD,0x76AA,0xFFF6},	 // MC3_Y0DEF2_POS5 :
	{WORD,0x76AC,0x003B},	 // MC3_X0DEF3_POS5 :
	{WORD,0x76AE,0xFFBB},	 // MC3_Y0DEF3_POS5 :
	{WORD,0x76B0,0xFFCE},	 // MC3_X0DEF0_POS6 :
	{WORD,0x76B2,0xFFC9},	 // MC3_Y0DEF0_POS6 :
	{WORD,0x76B4,0xFFE2},	 // MC3_X0DEF1_POS6 :
	{WORD,0x76B6,0x0039},	 // MC3_Y0DEF1_POS6 :
	{WORD,0x76B8,0xFFD3},	 // MC3_X0DEF2_POS6 :
	{WORD,0x76BA,0xFFF6},	 // MC3_Y0DEF2_POS6 :
	{WORD,0x76BC,0x003B},	 // MC3_X0DEF3_POS6 :
	{WORD,0x76BE,0xFFBB},	 // MC3_Y0DEF3_POS6 :
	{WORD,0x76C0,0xFFCE},	 // MC3_X0DEF0_POS7 :
	{WORD,0x76C2,0xFFC9},	 // MC3_Y0DEF0_POS7 :
	{WORD,0x76C4,0xFFE2},	 // MC3_X0DEF1_POS7 :
	{WORD,0x76C6,0x0039},	 // MC3_Y0DEF1_POS7 :
	{WORD,0x76C8,0xFFD3},	 // MC3_X0DEF2_POS7 :
	{WORD,0x76CA,0xFFF6},	 // MC3_Y0DEF2_POS7 :
	{WORD,0x76CC,0x003B},	 // MC3_X0DEF3_POS7 :
	{WORD,0x76CE,0xFFBB},	 // MC3_Y0DEF3_POS7 :
	{WORD,0x76D0,0xFF7E},	 // MC3_X0DEF0_OUT :
	{WORD,0x76D2,0xFFE2},	 // MC3_Y0DEF0_OUT :
	{WORD,0x76D4,0x003C},	 // MC3_X0DEF1_OUT :
	{WORD,0x76D6,0xFFEC},	 // MC3_Y0DEF1_OUT :
	{WORD,0x76D8,0xFFD0},	 // MC3_X0DEF2_OUT :
	{WORD,0x76DA,0x0037},	 // MC3_Y0DEF2_OUT :
	{WORD,0x76DC,0xFFC4},	 // MC3_X0DEF3_OUT :
	{WORD,0x76DE,0xFFEC},	 // MC3_Y0DEF3_OUT :
	{WORD,0x76E0,0xFFCE},	 // MC3_X0DEF0_R2_POS4 :
	{WORD,0x76E2,0xFFC9},	 // MC3_Y0DEF0_R2_POS4 :
	{WORD,0x76E4,0xFFD0},	 // MC3_X0DEF1_R2_POS4 :
	{WORD,0x76E6,0x0037},	 // MC3_Y0DEF1_R2_POS4 :
	{WORD,0x76E8,0xFFD3},	 // MC3_X0DEF2_R2_POS4 :
	{WORD,0x76EA,0xFFF6},	 // MC3_Y0DEF2_R2_POS4 :
	{WORD,0x76EC,0x003B},	 // MC3_X0DEF3_R2_POS4 :
	{WORD,0x76EE,0xFFBB},	 // MC3_Y0DEF3_R2_POS4 :
	{WORD,0x76F0,0xFFCE},	 // MC3_X0DEF0_R2_POS5 :
	{WORD,0x76F2,0xFFC9},	 // MC3_Y0DEF0_R2_POS5 :
	{WORD,0x76F4,0xFFD0},	 // MC3_X0DEF1_R2_POS5 :
	{WORD,0x76F6,0x0037},	 // MC3_Y0DEF1_R2_POS5 :
	{WORD,0x76F8,0xFFD3},	 // MC3_X0DEF2_R2_POS5 :
	{WORD,0x76FA,0xFFF6},	 // MC3_Y0DEF2_R2_POS5 :
	{WORD,0x76FC,0x003B},	 // MC3_X0DEF3_R2_POS5 :
	{WORD,0x76FE,0xFFBB},	 // MC3_Y0DEF3_R2_POS5 :
	{WORD,0x7700,0x0019},	 // MC3_PXDEF0_POS1 :
	{WORD,0x7702,0xFF66},	 // MC3_PYDEF0_POS1 :
	{WORD,0x7704,0x0009},	 // MC3_PXDEF1_POS1 :
	{WORD,0x7706,0x000A},	 // MC3_PYDEF1_POS1 :
	{WORD,0x7708,0xFFCC},	 // MC3_PXDEF2_POS1 :
	{WORD,0x770A,0xFFCC},	 // MC3_PYDEF2_POS1 :
	{WORD,0x770C,0xFFD7},	 // MC3_PXDEF3_POS1 :
	{WORD,0x770E,0x0068},	 // MC3_PYDEF3_POS1 :
	{WORD,0x7710,0x0000},	 // MC3_PXDEF0_POS2 :
	{WORD,0x7712,0xFF66},	 // MC3_PYDEF0_POS2 :
	{WORD,0x7714,0x0009},	 // MC3_PXDEF1_POS2 :
	{WORD,0x7716,0x000A},	 // MC3_PYDEF1_POS2 :
	{WORD,0x7718,0xFFCC},	 // MC3_PXDEF2_POS2 :
	{WORD,0x771A,0xFFCC},	 // MC3_PYDEF2_POS2 :
	{WORD,0x771C,0xFFD7},	 // MC3_PXDEF3_POS2 :
	{WORD,0x771E,0x0068},	 // MC3_PYDEF3_POS2 :
	{WORD,0x7720,0x0000},	 // MC3_PXDEF0_POS3 :
	{WORD,0x7722,0xFF80},	 // MC3_PYDEF0_POS3 :
	{WORD,0x7724,0x0009},	 // MC3_PXDEF1_POS3 :
	{WORD,0x7726,0x000A},	 // MC3_PYDEF1_POS3 :
	{WORD,0x7728,0xFFE6},	 // MC3_PXDEF2_POS3 :
	{WORD,0x772A,0xFFCC},	 // MC3_PYDEF2_POS3 :
	{WORD,0x772C,0xFFD7},	 // MC3_PXDEF3_POS3 :
	{WORD,0x772E,0x0068},	 // MC3_PYDEF3_POS3 :
	{WORD,0x7730,0x0000},	 // MC3_PXDEF0_POS4 :
	{WORD,0x7732,0xFFCC},	 // MC3_PYDEF0_POS4 :
	{WORD,0x7734,0x0009},	 // MC3_PXDEF1_POS4 :
	{WORD,0x7736,0x000A},	 // MC3_PYDEF1_POS4 :
	{WORD,0x7738,0xFFCC},	 // MC3_PXDEF2_POS4 :
	{WORD,0x773A,0xFFCC},	 // MC3_PYDEF2_POS4 :
	{WORD,0x773C,0xFFD7},	 // MC3_PXDEF3_POS4 :
	{WORD,0x773E,0x0068},	 // MC3_PYDEF3_POS4 :
	{WORD,0x7740,0x0000},	 // MC3_PXDEF0_POS5 :
	{WORD,0x7742,0xFFCC},	 // MC3_PYDEF0_POS5 :
	{WORD,0x7744,0x0009},	 // MC3_PXDEF1_POS5 :
	{WORD,0x7746,0x000A},	 // MC3_PYDEF1_POS5 :
	{WORD,0x7748,0xFFCC},	 // MC3_PXDEF2_POS5 :
	{WORD,0x774A,0xFFCC},	 // MC3_PYDEF2_POS5 :
	{WORD,0x774C,0xFFD7},	 // MC3_PXDEF3_POS5 :
	{WORD,0x774E,0x0068},	 // MC3_PYDEF3_POS5 :
	{WORD,0x7750,0xFFB3},	 // MC3_PXDEF0_POS6 :
	{WORD,0x7752,0x0000},	 // MC3_PYDEF0_POS6 :
	{WORD,0x7754,0x0009},	 // MC3_PXDEF1_POS6 :
	{WORD,0x7756,0x000A},	 // MC3_PYDEF1_POS6 :
	{WORD,0x7758,0xFFE6},	 // MC3_PXDEF2_POS6 :
	{WORD,0x775A,0xFFCC},	 // MC3_PYDEF2_POS6 :
	{WORD,0x775C,0xFFD7},	 // MC3_PXDEF3_POS6 :
	{WORD,0x775E,0x0068},	 // MC3_PYDEF3_POS6 :
	{WORD,0x7760,0xFFB3},	 // MC3_PXDEF0_POS7 :
	{WORD,0x7762,0x0000},	 // MC3_PYDEF0_POS7 :
	{WORD,0x7764,0x0009},	 // MC3_PXDEF1_POS7 :
	{WORD,0x7766,0x000A},	 // MC3_PYDEF1_POS7 :
	{WORD,0x7768,0xFFE6},	 // MC3_PXDEF2_POS7 :
	{WORD,0x776A,0xFFCC},	 // MC3_PYDEF2_POS7 :
	{WORD,0x776C,0xFFD7},	 // MC3_PXDEF3_POS7 :
	{WORD,0x776E,0x0068},	 // MC3_PYDEF3_POS7 :
	{WORD,0x7770,0x0019},	 // MC3_PXDEF0_OUT :
	{WORD,0x7772,0xFFE6},	 // MC3_PYDEF0_OUT :
	{WORD,0x7774,0xFF99},	 // MC3_PXDEF1_OUT :
	{WORD,0x7776,0xFFB3},	 // MC3_PYDEF1_OUT :
	{WORD,0x7778,0x001E},	 // MC3_PXDEF2_OUT :
	{WORD,0x777A,0x0000},	 // MC3_PYDEF2_OUT :
	{WORD,0x777C,0xFFE1},	 // MC3_PXDEF3_OUT :
	{WORD,0x777E,0xFFEB},	 // MC3_PYDEF3_OUT :
	{WORD,0x7780,0x0000},	 // MC3_PXDEF0_R2_POS4 :
	{WORD,0x7782,0xFFCC},	 // MC3_PYDEF0_R2_POS4 :
	{WORD,0x7784,0x0000},	 // MC3_PXDEF1_R2_POS4 :
	{WORD,0x7786,0x0000},	 // MC3_PYDEF1_R2_POS4 :
	{WORD,0x7788,0xFFCC},	 // MC3_PXDEF2_R2_POS4 :
	{WORD,0x778A,0xFFCC},	 // MC3_PYDEF2_R2_POS4 :
	{WORD,0x778C,0xFFD7},	 // MC3_PXDEF3_R2_POS4 :
	{WORD,0x778E,0x0068},	 // MC3_PYDEF3_R2_POS4 :
	{WORD,0x7790,0x0000},	 // MC3_PXDEF0_R2_POS5 :
	{WORD,0x7792,0xFFCC},	 // MC3_PYDEF0_R2_POS5 :
	{WORD,0x7794,0x0000},	 // MC3_PXDEF1_R2_POS5 :
	{WORD,0x7796,0x0000},	 // MC3_PYDEF1_R2_POS5 :
	{WORD,0x7798,0xFFCC},	 // MC3_PXDEF2_R2_POS5 :
	{WORD,0x779A,0xFFCC},	 // MC3_PYDEF2_R2_POS5 :
	{WORD,0x779C,0xFFD7},	 // MC3_PXDEF3_R2_POS5 :
	{WORD,0x779E,0x0068},	 // MC3_PYDEF3_R2_POS5 :
	{BYTE,0x981A,0x0E},    // CS_SLP_YC_L_A :
	{BYTE,0x01C7,0x80},    // UICONTRAST
	{BYTE,0x01C6,0x00},    // UIBrightness
	{BYTE,0x03A1,0x20},    // UISHARPNESS POS//Setting Updated
	{BYTE,0x03A4,0x20},    // UISHARPNESS NEGS, 0082, 01, 8  //UPDATED
	{BYTE,0x039E,0x80},    // UISATURATION

	//preview resolution	1280*960
    {BYTE,0x0089,0x00},			  
    {BYTE,0x008c,0x03},			  
    {WORD,0x0090,0x0500},	  
    {WORD,0x0096,0x03c0},	   
    {BYTE,0x0086,0x02},				
    {BYTE,0x0083,0x01},				
    {WORD,0x00DE,0x1169},		   							  
    {WORD,0x6A9E,0x15c0},		   
    {BYTE,0x00AF,0x11},					
    {BYTE,0x0082,0x01},
    {BYTE,0x0006,0x16},
};                                                           
   
/***2592X1944 QSXGA***/
static struct regval_list sensor_qsxga_regs[] = {
	{BYTE,0x0089,0x00},				    
	{WORD,0x0090,0x0a20},
	{WORD,0x0096,0x0798},		
	{BYTE,0x0086,0x03},				 
	{BYTE,0x0083,0x00},				 					
	{BYTE,0x0082,0x01},  //UPDATED
};

/***2048*1536 QXGA***/
static struct regval_list sensor_qxga_regs[] = {
	{BYTE,0x0089,0x00},				 
	{WORD,0x0090,0x0800},		
	{WORD,0x0096,0x0600},		
	{BYTE,0x0086,0x03},				
	{BYTE,0x0083,0x00},				 
	{WORD,0x00DE,0x1169},										   
	{WORD,0x6A9E,0x15c0},			
	{BYTE,0x00AF,0x11},					
	{BYTE,0x0082,0x01},	//UPDATED	
};                                      

/***1600X1200 UXGA***/
static struct regval_list sensor_uxga_regs[] =  {
	{BYTE,0x0089,0x00},		 
	{WORD,0x0090,0x0640},		
	{WORD,0x0096,0x04b0},		 
	{BYTE,0x0086,0x03},				 
	{BYTE,0x0083,0x00},				 
	{WORD,0x00DE,0x1169},									   
	{WORD,0x6A9E,0x15c0},			
	{BYTE,0x00AF,0x11},					 
	{BYTE,0x0082,0x01},  //UPDATED	
};

/***1280X1024 SXGA***/
static struct regval_list sensor_sxga_regs[] =  {
	{BYTE,0x0089,0x00},				   
	{WORD,0x0090,0x0500},		
	{WORD,0x0096,0x0400},		
	{BYTE,0x0086,0x02},				 
	{BYTE,0x0083,0x00},				
	{WORD,0x00DE,0x1169},									   
	{WORD,0x6A9E,0x15c0},			
	{BYTE,0x00AF,0x11},					
	{BYTE,0x0082,0x01},	//UPDATED
};

/***1024X768 XGA***/
static struct regval_list sensor_xga_regs[] =  {
	{BYTE,0x0089,0x00},				 
	{WORD,0x0090,0x0400},		
	{WORD,0x0096,0x0300},		
	{BYTE,0x0086,0x03},				 
	{BYTE,0x0083,0x01},				 
	{WORD,0x00DE,0x1169},										   
	{WORD,0x6A9E,0x15c0},			
	{BYTE,0x00AF,0x11},					 
	{BYTE,0x0082,0x01},  //UPDATED	
};

/***800X600 SVGA***/
static struct regval_list sensor_svga_regs[] =  {
	{BYTE,0x0089,0x00}, 			 
	{WORD,0x0090,0x0320},		
	{WORD,0x0096,0x0258},		
	{BYTE,0x0086,0x03}, 			 
	{BYTE,0x0083,0x01}, 			 
	{WORD,0x00DE,0x1169},										   
	{WORD,0x6A9E,0x15c0},			
	{BYTE,0x00AF,0x11}, 				 
	{BYTE,0x0082,0x01},  //UPDATED	
};

/*** 640X480 VGA***/
static struct regval_list sensor_vga_regs[] =  {
	{BYTE,0x0089,0x00},				
	{WORD,0x0090,0x0280},		
	{WORD,0x0096,0x01e0}, 		
	{BYTE,0x0086,0x03},				 
	{BYTE,0x0083,0x01},				 
	{WORD,0x00DE,0x1169}, 										   
	{WORD,0x6A9E,0x15c0}, 			
	{BYTE,0x00AF,0x11},					
	{BYTE,0x0082,0x01},  //UPDATED	
};

//for video
/***  720p 1280x720***/
static struct regval_list sensor_720p_regs[] =  {
	{BYTE,0x0089,0x00},				  
	{WORD,0x0090,0x0500},		
	{WORD,0x0096,0x02d0},		
	{BYTE,0x0086,0x03},				
	{BYTE,0x0083,0x01},				
	{WORD,0x00DE,0x1169},										   
	{WORD,0x6A9E,0x15c0},			
	{BYTE,0x00AF,0x11},					 
	{BYTE,0x0082,0x01},	//UPDATED	
};

/*** 1080p 1920x1080***/
static struct regval_list sensor_1080p_regs[] =  {
	{BYTE,0x0089,0x00},				  
	{WORD,0x0090,0x0780},		
	{WORD,0x0096,0x0438},		
	{BYTE,0x0086,0x03},				
	{BYTE,0x0083,0x00},				
	{WORD,0x00DE,0x1169},										   
	{WORD,0x6A9E,0x15c0},			
	{BYTE,0x00AF,0x11},					 
	{BYTE,0x0082,0x01},	//UPDATED
};

static struct regval_list sensor_fmt_yuv422_yuyv[] = {
	//{WORD,0x00de,0x1161},  //YUYV
};

static struct regval_list sensor_fmt_yuv422_yvyu[] = {
	//{WORD,0x00de,0x1165},  //YVYU
};

static struct regval_list sensor_fmt_yuv422_vyuy[] = {
	//{WORD,0x00de,0x116d},  //VYUY
};

static struct regval_list sensor_fmt_yuv422_uyvy[] = {
	//{WORD,0x00de,0x1169},  //UYVY
};

static int sensor_read_value8(struct v4l2_subdev *sd, unsigned short reg,
    unsigned char *value)
{
	int ret=0;
	int cnt=0;
	
	ret = cci_read_a16_d8(sd,reg,value);
	while(ret!=0&&cnt<2)
	{
  		ret = cci_read_a16_d8(sd,reg,value);
		cnt++;
	}
	if(cnt>0)
		vprintk("sensor read retry=%d\n",cnt);
	return ret;
}
static int sensor_read_value16(struct v4l2_subdev *sd, unsigned short reg,
    unsigned short *value)
{
	int ret=0;
	int cnt=0;
	ret = cci_read_a16_d16(sd,reg,value);
	while(ret!=0&&cnt<2)
	{
  		ret = cci_read_a16_d16(sd,reg,value);
		cnt++;
	}
	if(cnt>0)
		vprintk("sensor read retry=%d\n",cnt);
	return ret;
}
static int sensor_write_value8(struct v4l2_subdev *sd, unsigned short reg,
    unsigned char value)
{
	int ret=0;
	int cnt=0;
	
	if(reg == REG_DLY) {
		msleep(value);
		return 0;
	} 
  	ret = cci_write_a16_d8(sd,reg,value);
	while(ret!=0&&cnt<2)
	{
  		ret = cci_write_a16_d8(sd,reg,value);
		cnt++;
	}
	if(cnt>0)
  	vprintk("sensor write retry=%d\n",cnt);
	return ret;
}

static int sensor_write_value16(struct v4l2_subdev *sd, unsigned short reg,
    unsigned short value)
{
	int ret=0;
	int cnt=0;
	unsigned short tem;
	unsigned char tem1,tem2, tem11,tem22;
	
	tem=0x00ff&value;
	tem1=tem;       //  ��

	tem=0xff00&value;
	tem=tem>>8;
	tem2=tem;   // ��
	
	if(tem2==0)
	{
		tem=(value<<8);
	}
	else
	{
		tem=tem1;
		tem=(tem<<8)|tem2;
	}
   
	value=tem;
	
  	ret = cci_write_a16_d16(sd,reg,value);
	while(ret!=0&&cnt<2)
	{
		ret = cci_write_a16_d16(sd,reg,value);
		cnt++;
	}
	if(cnt>0)
		vprintk("sensor write retry=%d\n",cnt);
	return ret;
}

/*
 * Write a list of register settings;
 */
static int sensor_write_array(struct v4l2_subdev *sd, struct regval_list *regs, int array_size)
{
	int i=0,j=0;

	if(!regs)
  	return -EINVAL;
	printk("[gs5604]enter sensor_write_array----------------------------!!!!!! \n");

	while(i<array_size)
	{
		if(regs->addr == REG_DLY) 
		{
			msleep(regs->data);
		} 
		else 
		{
			if(regs[j].width == BYTE)
				LOG_ERR_RET(sensor_write_value8(sd, regs->addr, regs->data))
			else if(regs[j].width == WORD)
				LOG_ERR_RET(sensor_write_value16(sd, regs->addr, regs->data))
		}
		i++;
		regs++;
	}
	printk("[gs5604]exit sensor_write_array----------------------------!!!!!! \n");
	return 0;
}

static int sensor_download_af_fw(struct v4l2_subdev *sd)
{
   
}

static int sensor_g_single_af(struct v4l2_subdev *sd)
{                                                                   
	unsigned char AF_state=0;
	unsigned char val=0;
	int ret;
	struct sensor_info *info = to_state(sd);

	ret = V4L2_AUTO_FOCUS_STATUS_IDLE;
	msleep(5);
	sensor_read_value8(sd, 0x8b8a, &AF_state);
	msleep(1);
	switch (AF_state)
	{
		case 0x08:
			ret = V4L2_AUTO_FOCUS_STATUS_REACHED;	//single
			break;
		default:
			ret = V4L2_AUTO_FOCUS_STATUS_BUSY; 
			break;
	}		
	if((AF_state==0xff)||(AF_state==0x8b))		
	{
		sensor_write_value8(sd, 0x00b2, 0X03);
		sensor_write_value8(sd, 0x00b3, 0X03);
		sensor_write_value8(sd, 0x00b4, 0X03);
		sensor_write_value8(sd, 0x8b8a, 0X03);
		msleep(5);
		sensor_write_value8(sd, 0x00b2, 0X00);
		sensor_write_value8(sd, 0x00b3, 0X00);
		sensor_write_value8(sd, 0x00b4, 0X00);
	}			
	return	ret;
}

static int sensor_g_contin_af(struct v4l2_subdev *sd)
{
	unsigned char AF_state=0;
	unsigned char val=0;
	int ret;
	struct sensor_info *info = to_state(sd);

	ret = V4L2_AUTO_FOCUS_STATUS_IDLE;
	msleep(5);
	sensor_read_value8(sd, 0x8b8a, &AF_state);
	msleep(1);
	switch (AF_state)
	{
		case 0x0f:
			ret = V4L2_AUTO_FOCUS_STATUS_REACHED;	//constant
			break;
		default:
			ret = V4L2_AUTO_FOCUS_STATUS_BUSY; 
			break;
	}	
	if((AF_state==0xff)||(AF_state==0x8b))		
	{
		sensor_write_value8(sd, 0x00b2, 0X03);
		sensor_write_value8(sd, 0x00b3, 0X03);
		sensor_write_value8(sd, 0x00b4, 0X03);
		sensor_write_value8(sd, 0x8b8a, 0X03);
		msleep(5);
		sensor_write_value8(sd, 0x00b2, 0X01);
		sensor_write_value8(sd, 0x00b3, 0X01);
		sensor_write_value8(sd, 0x00b4, 0X01);
	}
	return	ret;	
}

static int sensor_g_af_status(struct v4l2_subdev *sd)
{
	int ret=0;
	struct sensor_info *info = to_state(sd);
	
	if(info->auto_focus==1)
		ret = sensor_g_contin_af(sd);
	else
		ret = sensor_g_single_af(sd);
	return ret;
}

static int sensor_g_3a_lock(struct v4l2_subdev *sd)
{
	struct sensor_info *info = to_state(sd);
	return ( (info->auto_focus==0)?V4L2_LOCK_FOCUS:~V4L2_LOCK_FOCUS |
           (info->autowb==0)?V4L2_LOCK_WHITE_BALANCE:~V4L2_LOCK_WHITE_BALANCE |
           (~V4L2_LOCK_EXPOSURE));
}

static int sensor_s_init_af(struct v4l2_subdev *sd)
{
	struct sensor_info *info = to_state(sd);
	sensor_write_value8(sd,0x5008,0x00);
	sensor_write_value8(sd,0x000B,0x01); //AF_EXT : AF driver start                                   
    sensor_write_value16(sd,0x6666, 0x0000);    // AF_AREA_LOW_TYPE1                                   
    sensor_write_value16(sd,0x6668, 0x0340);    // AF_AREA_HIGH_TYPE1(AF_SEARCH_AREA_HIGH) = 600       
    sensor_write_value16(sd,0x8B4C, 0x0340);    // AF_SEARCH_CORNER_HIGH =500                          
    sensor_write_value16(sd,0x6656, 0x0000);    // AF_OVERSRCH_AREA_HIGH = CORNER_HIGH - AREA_HIGH     
    sensor_write_value16(sd,0x6622, 0x0004);    // AF_CAF_PARAM_WOBBLE_STEP                            
    sensor_write_value16(sd,0x6624, 0x0010);    // AF_CAF_CLIMB_STEP                                 
    sensor_write_value16(sd,0x665A, 0x0190);    // AF_LENSPOS_ON_AFNG   
    sensor_write_value8(sd,0x0082,0x01);  
	info->af_first_flag=0;
	return 0;
}

static int sensor_s_single_af(struct v4l2_subdev *sd)
{
	struct sensor_info *info = to_state(sd);
	info->focus_status = 0; //idle  
	vfe_dev_dbg("enter sensor_s_single_af\n");
	sensor_write_value8(sd,0x5000,0x00); //AF unlock
	sensor_write_value16(sd, 0x6a50, af_window_xstart);//af_xcoordinate
	sensor_write_value16(sd, 0x6a52, af_window_ystart);//af_ycoordinate
	sensor_write_value16(sd, 0x6a54, af_window_width);  //af_width
	sensor_write_value16(sd, 0x6a56, af_window_height);  // af_height
	msleep(10);
	sensor_write_value8(sd, 0x00b2, 0x00); //preview single af enable
	sensor_write_value8(sd, 0x00b3, 0x00); //half release single af enable
	sensor_write_value8(sd, 0x00b4, 0x00); //movie single af enable
	msleep(50);
	info->focus_status = 1; //busy
	info->auto_focus=0;
	vfe_dev_dbg("exit sensor_s_single_af\n");
	return 0;
}

static int sensor_s_continueous_af(struct v4l2_subdev *sd, int value)
{
	struct sensor_info *info = to_state(sd);
	vfe_dev_print("sensor_s_continueous_af[0x%x]\n",value);

	if(value==1)
	{
		vfe_dev_dbg("enter sensor_s_continueous_af\n");
		sensor_write_value8(sd,0x5000,0x00); //AF unlock
		sensor_write_value8(sd, 0x00b2, 0X01);
		sensor_write_value8(sd, 0x00b3, 0X01);
		sensor_write_value8(sd, 0x00b4, 0X01);
		info->auto_focus=1;
		msleep(50);
	}
	else
	{
		info->auto_focus=0;
	}
	return 0;
}

static int sensor_s_pause_af(struct v4l2_subdev *sd)
{ 
	vfe_dev_dbg("enter sensor_s_pause_af\n");
    char status,af_state;
	struct sensor_info *info = to_state(sd);
	sensor_write_value8(sd,0x5000,0x08); //AF lock
	vfe_dev_dbg("exit sensor_s_pause_af\n");
	return 0;
}

static int sensor_s_release_af(struct v4l2_subdev *sd)
{
	struct sensor_info *info = to_state(sd);
	sensor_write_value8(sd, 0x00b2, 0X03);
	sensor_write_value8(sd, 0x00b3, 0X03);
	sensor_write_value8(sd, 0x00b4, 0X03);
	return 0;
}
static int sensor_s_relaunch_af_zone(struct v4l2_subdev *sd)
{
	vfe_dev_dbg("enter sensor_s_relaunch_af_zone\n");
	struct sensor_info *info = to_state(sd);
	sensor_write_value8(sd,0x5000,0x00);//AF unlock
	sensor_write_value8(sd,0x00b1,0X01);//restart
	usleep_range(5000,6000);
	vfe_dev_dbg("eixt sensor_s_relaunch_af_zone\n");
	return 0;
}

static int sensor_s_af_zone(struct v4l2_subdev *sd,struct v4l2_win_coordinate * win_c)
{
	vfe_dev_dbg("enter sensor_s_af_zone\n");
	struct sensor_info *info = to_state(sd);
	int ret;
	int x1,y1,x2,y2;
	int tem_x1,tem_y1,tem_x2,tem_y2;
	int xc,yc,xstart,ystart,af_width,af_height;
	int prv_x,prv_y,prv_x1,prv_y1,prv_x2,prv_y2;
	int tem_width,temp_heigh;	
    int reg_x,reg_y,reg_width,reg_heigh;

	prv_x=(int)info->width;
	prv_y=(int)info->height;
	x1=win_c->x1;
	y1=win_c->y1;
	x2=win_c->x2;
	y2=win_c->y2;

	tem_x1=x1+1000;
	tem_y1=y1+1000;
	tem_x2=x2+1000;
	tem_y2=y2+1000;

	prv_x1 = tem_x1*2592/2000;
	prv_y1 = tem_y1*1944/2000;
	prv_x2 = tem_x2*2592/2000;
	prv_y2 = tem_y2*1944/2000;

	xstart = prv_x1 + 49;
	ystart = prv_y1 + 4;
	af_width = abs(prv_x2 - prv_x1);
	af_height = abs(prv_y2 - prv_y1);

	if(af_width < 350) af_width = 350;
    if(af_width > 2559) af_width = 2559;
    if(af_height < 350) af_height = 350;
    if(af_height > 1939) af_height = 1939;
    if((xstart+af_width) > 2559) xstart = 2559 - af_width;
    if((ystart+af_height) > 1939) ystart = 1939 - af_height;

	af_window_xstart = xstart;	
	af_window_ystart = ystart;
	af_window_width = af_width;
	af_window_height = af_height; 	
	printk("[gs5604]af zone input af_window_xstart=%d,af_window_ystart=%d, af_window_width=%d,af_window_height=%d \n",xstart,ystart,af_width,af_height);
	vfe_dev_dbg("exit sensor_s_af_zone\n");
	return 0;
}


static int sensor_s_3a_lock(struct v4l2_subdev *sd, int value)
{
  int ret;
  value=!((value&V4L2_LOCK_FOCUS)>>2);
  if(value==0)
  	ret=sensor_s_pause_af(sd);
  else
    ret=sensor_s_relaunch_af_zone(sd);  
  return ret;
}

#if 1
static int sensor_s_sharpness_auto(struct v4l2_subdev *sd)
{
	return  0;
}
#endif

static int sensor_s_sharpness_value(struct v4l2_subdev *sd, unsigned char value)
{
	return  0;
}

#if 1
static int sensor_s_denoise_auto(struct v4l2_subdev *sd)
{
	return  0;
}
#endif

static int sensor_s_denoise_value(struct v4l2_subdev *sd, unsigned char value)
{
	return  0;
}

/* *********************************************begin of ******************************************** */
static int sensor_g_hflip(struct v4l2_subdev *sd, __s32 *value)
{
	return  0;
}

static int sensor_s_hflip(struct v4l2_subdev *sd, int value)
{
	return  0;
}

static int sensor_g_vflip(struct v4l2_subdev *sd, __s32 *value)
{
	return  0;
}

static int sensor_s_vflip(struct v4l2_subdev *sd, int value)
{
	return  0;
}

static int sensor_g_autogain(struct v4l2_subdev *sd, __s32 *value)
{
	return  0;
}

static int sensor_s_autogain(struct v4l2_subdev *sd, int value)
{
	return  0;
}

static int sensor_g_autoexp(struct v4l2_subdev *sd, __s32 *value)
{
	return  0;
}

static int sensor_s_autoexp(struct v4l2_subdev *sd,
    enum v4l2_exposure_auto_type value)
{
	return  0;
}

static int sensor_g_autowb(struct v4l2_subdev *sd, int *value)
{
	return  0;
}

static int sensor_s_autowb(struct v4l2_subdev *sd, int value)
{
	struct sensor_info *info = to_state(sd);
	unsigned char rdval;	

	if(info->autowb == value)
    return 0;

	switch(value) {
		case 0:
			sensor_write_value8(sd,0x0282,  0x20);
			break;
		case 1: 
			sensor_write_value8(sd,0x0282,  0x02);
			break;
		default:
			break;
	}
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

static int sensor_g_band_filter(struct v4l2_subdev *sd, 
    __s32 *value)
{
	return  0;
}

static int sensor_s_band_filter(struct v4l2_subdev *sd, 
    enum v4l2_power_line_frequency value)
{
	return  0;
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

	switch (value)
	{
		case -4:
			sensor_write_value8(sd,0x0180,  0x86);
			break;
		case -3:
			sensor_write_value8(sd,0x0180,  0x86);
			break;
		case -2: 
			sensor_write_value8(sd,0x0180,  0x84);  //83
			break;
		case -1:
			sensor_write_value8(sd,0x0180,  0x82);
			break;
		case  0 :
			sensor_write_value8(sd,0x0180,  0x00);
			break;      			
		case  1:
			sensor_write_value8(sd,0x0180,  0x02);
			break;
		case  2:
			sensor_write_value8(sd,0x0180,  0x03);
			break;
		case  3: 
			sensor_write_value8(sd,0x0180,  0x04);
			break;
		case  4:
			sensor_write_value8(sd,0x0180,  0x06);
			break;
		default:
			break;
	}
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
	switch (value)
	{
		case 0:
			break;
		case 1:
			sensor_write_value8(sd,0x0282,	0x20);
			break;
		case 2: 
			sensor_write_value8(sd,0x0282,  0x28);
			break;
		case 3:
			break;
		case  4:
			sensor_write_value8(sd,0x0282,  0x27);
			break;
		case 5:
			break;
		case 6:
			sensor_write_value8(sd,0x0282,  0x25);
			break;
		case 7: 
			break;
		case 8:
			sensor_write_value8(sd,0x0282,  0x26);
			break;
		default:
			break;
	}
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
	vprintk("sensor_s_flash_mode[0x%d]!\n",value);

	info->flash_mode = value;
	return 0;
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
		case CSI_SUBDEV_PWR_ON:
		case CSI_SUBDEV_STBY_OFF:
			vprintk("CSI_SUBDEV_PWR_ON!\n");
			cci_lock(sd);    
			vfe_gpio_write(sd,POWER_EN,CSI_GPIO_HIGH);
			vfe_set_pmu_channel(sd,DVDD,ON);
			vfe_set_pmu_channel(sd,IOVDD,ON);
			vfe_set_pmu_channel(sd,AVDD,ON);
			vfe_set_pmu_channel(sd,AFVDD,ON);
			vfe_set_mclk_freq(sd,MCLK);
			vfe_set_mclk(sd,ON);
			usleep_range(10000,12000);
			vfe_gpio_set_status(sd,PWDN,1);//set the gpio to output
			vfe_gpio_set_status(sd,RESET,1);//set the gpio to output
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);
			usleep_range(30000,31000);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			msleep(50);
			usleep_range(10000,12000);    
			cci_unlock(sd);        
			break;
		case CSI_SUBDEV_PWR_OFF:
		case CSI_SUBDEV_STBY_ON: 
			vprintk("CSI_SUBDEV_PWR_OFF!\n");
			cci_lock(sd);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
			msleep(20);
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			vfe_gpio_set_status(sd,RESET,0);//set the gpio to input
			vfe_gpio_set_status(sd,PWDN,0);//set the gpio to input
			vfe_set_mclk(sd,OFF);
			vfe_gpio_write(sd,POWER_EN,CSI_GPIO_LOW);
			vfe_set_pmu_channel(sd,AFVDD,OFF);
			vfe_set_pmu_channel(sd,DVDD,OFF);
			vfe_set_pmu_channel(sd,AVDD,OFF);
			vfe_set_pmu_channel(sd,IOVDD,OFF);  
			msleep(20);
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
	unsigned  short rdval;
	unsigned  short val;

	sensor_read_value16(sd, 0x0000, &val); 
	vprintk("gs5604  senso =%x\n",val);
	if(val != 0x1760)
		vfe_dev_err("gs5604  sensor_read err at sensor_detect!\n");

	LOG_ERR_RET(sensor_read_value16(sd, 0x0000, &rdval))
	if(rdval != 0x1760)
		return -ENODEV;
	return 0;
}

static void gs5604_wait_status(struct v4l2_subdev *sd)
{
    unsigned int temp1,temp2;
    printk("[gs5604]enter gs5604_wait_status function:\n ");
	do
	{
    	sensor_read_value8(sd, 0x000E, &temp1);
		temp2 = temp1 & 0x01 ;
    	printk("[gs5604]gs5604_wait_status while1!\r\n");		    	
    }while(!temp2);
	sensor_write_value8(sd,0x0012,0x01);
	do
	{
    	sensor_read_value8(sd, 0x000E, &temp1);
		temp2 = temp1 & 0x01 ;
    	printk("[gs5604]gs5604_wait_status while2!\r\n");	
    }while(temp2);
    printk("[gs5604]exit gs5604_wait_status function:\n ");
}


static void sensor_init_gs5604(struct v4l2_subdev *sd)
{
}

static int sensor_init(struct v4l2_subdev *sd, u32 val)
{
	int ret;
	unsigned char value;
	struct sensor_info *info = to_state(sd);

	vprintk("sensor_init 0x%x\n",val);

	/*Make sure it is a target sensor*/
	ret = sensor_detect(sd);
	if (ret) {
		vfe_dev_err("chip found is not an target chip.\n");
		return ret;
	}

	vfe_get_standby_mode(sd,&info->stby_mode);
	printk("[gs5604]sensor_init: stby_mode = %x\n",info->stby_mode);

	if((info->stby_mode == HW_STBY || info->stby_mode == SW_STBY) \
      && info->init_first_flag == 0) {
		vfe_dev_print("stby_mode and init_first_flag = 0\n");
		return 0;
	} 

	info->focus_status = 0;
	info->auto_focus = 1;
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

	msleep(50);
	
	if(ret < 0) {
		vfe_dev_err("write sensor_default_regs error\n");
		return ret;
	}

	sensor_s_band_filter(sd, V4L2_CID_POWER_LINE_FREQUENCY_50HZ);

	if(info->stby_mode == 0)
		info->init_first_flag = 0;

	info->preview_first_flag = 1;

	night_mode=0;
	Nfrms = MAX_FRM_CAP;

	return 0;
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
//  {
//    .desc   = "Raw RGB Bayer",
//    .mbus_code  = V4L2_MBUS_FMT_SBGGR8_1X8,
//    .regs     = sensor_fmt_raw,
//    .regs_size = ARRAY_SIZE(sensor_fmt_raw),
//    .bpp    = 1
//  },
};
#define N_FMTS ARRAY_SIZE(sensor_formats)
/*
 * Then there is the issue of window sizes.  Try to capture the info here.
 */
static struct sensor_win_size sensor_win_sizes[] = {
/* qsxga: 2592*1936 */
	{
		.width		= QSXGA_WIDTH,
		.height 	= QSXGA_HEIGHT,
		.hoffset	= 0,
		.voffset	= 0,
		.regs		= sensor_qsxga_regs,
		.regs_size	= ARRAY_SIZE(sensor_qsxga_regs),
		.set_size	= NULL,
	},
/* qxga: 2048*1536 */
	{
		.width		= QXGA_WIDTH,
		.height 	= QXGA_HEIGHT,
		.hoffset	= 0,
		.voffset	= 0,
		.regs		= sensor_qxga_regs,
		.regs_size	= ARRAY_SIZE(sensor_qxga_regs),
		.set_size	= NULL,
	},
/* 1080P */
	{
		.width		= HD1080_WIDTH,
		.height 	= HD1080_HEIGHT,
		.hoffset	= 0,
		.voffset	= 0,
		.regs		= sensor_1080p_regs,
		.regs_size	= ARRAY_SIZE(sensor_1080p_regs),
		.set_size	= NULL,
	},
/* UXGA */
	{
		.width		= UXGA_WIDTH,
		.height 	= UXGA_HEIGHT,
		.hoffset	= 0,
		.voffset	= 0,
		.regs		= sensor_uxga_regs,
		.regs_size	= ARRAY_SIZE(sensor_uxga_regs),
		.set_size	= NULL,
	},

/* SXGA */
	{
		.width		= SXGA_WIDTH,
		.height 	= SXGA_HEIGHT,
		.hoffset	= 0,
		.voffset	= 0,
		.regs		= sensor_sxga_regs,
		.regs_size	= ARRAY_SIZE(sensor_sxga_regs),
		.set_size	= NULL,
	},

/* 720p */
	{
		.width		= HD720_WIDTH,
		.height 	= HD720_HEIGHT,
		.hoffset	= 0,
		.voffset	= 0,
		.regs		= sensor_720p_regs,
		.regs_size	= ARRAY_SIZE(sensor_720p_regs),
		.set_size	= NULL,
	},
/* XGA */
	{
		.width		= XGA_WIDTH,
		.height 	= XGA_HEIGHT,
		.hoffset	= 0,
		.voffset	= 0,
		.regs		= sensor_xga_regs,
		.regs_size	= ARRAY_SIZE(sensor_xga_regs),
		.set_size	= NULL,
	},
/* SVGA */
	{
		.width		= SVGA_WIDTH,
		.height 	= SVGA_HEIGHT,
		.hoffset	= 0,
		.voffset	= 0,
		.regs		= sensor_svga_regs,
		.regs_size	= ARRAY_SIZE(sensor_svga_regs),
		.set_size	= NULL,
	},
/* VGA */
	{
		.width		= VGA_WIDTH,
		.height 	= VGA_HEIGHT,
		.hoffset	= 0,
		.voffset	= 0,
		.regs		= sensor_vga_regs,
		.regs_size	= ARRAY_SIZE(sensor_vga_regs),
		.set_size	= NULL,
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

static int sensor_s_fmt(struct v4l2_subdev *sd, 
             struct v4l2_mbus_framefmt *fmt)
{
	int ret;
	struct sensor_format_struct *sensor_fmt;
	struct sensor_win_size *wsize;
	struct sensor_info *info = to_state(sd);

	vprintk("sensor_s_fmt\n");

	ret = sensor_try_fmt_internal(sd, fmt, &sensor_fmt, &wsize);
	if (ret)
		return ret;

	if(info->capture_mode == V4L2_MODE_VIDEO)
	{

	}
	else if(info->capture_mode == V4L2_MODE_IMAGE)
	{
		sensor_s_pause_af(sd);
		ret = sensor_s_autoexp(sd,V4L2_EXPOSURE_MANUAL);
		if (ret < 0)
			vfe_dev_err("sensor_s_autoexp off err when capturing image!\n");
		ret = sensor_s_autogain(sd,0);
		if (ret < 0)
			vfe_dev_err("sensor_s_autogain off err when capturing image!\n");
		if (wsize->width > SVGA_WIDTH) {
		}
	}

	/**************** start set resolution ******************/
	if(wsize->height==480)  //   640*480
	{
		sensor_write_value8(sd,0x5008,0x00);
		sensor_write_value8(sd,0x0089,0x00); 
	   	sensor_write_value16(sd,0x0090,0x0280);
	    sensor_write_value16(sd,0x0096,0x01e0);
		sensor_write_value8(sd,0x0086,0x02);
	    sensor_write_value8(sd,0x0083,0x01);
	    sensor_write_value8(sd,0x0082,0x01);
		printk("[gs5604] sensor_s_fmt: set resolution 640*480\n");
		msleep(100);
	}
	if(wsize->height==960)  //   1280*960
	{
		sensor_write_value8(sd,0x5008,0x00);
		sensor_write_value8(sd,0x0089,0x00);
	    sensor_write_value16(sd,0x0090,0x0500);
	    sensor_write_value16(sd,0x0096,0x03c0);
		sensor_write_value8(sd,0x0086,0x02);
	    sensor_write_value8(sd,0x0083,0x01); 
	    sensor_write_value8(sd,0x0082,0x01);
		printk("[gs5604] sensor_s_fmt   960\n");
		msleep(100);
	}
	if(wsize->height==1200)
	{
		sensor_write_value8(sd,0x5008,0x00);
		sensor_write_value8(sd,0x0089,0x00 );				  // OUTFMT_CAP(YUV)   
		sensor_write_value16(sd,0x0090,0x0640);		// HSIZE_CAP(2560)) 2592 
		sensor_write_value16(sd,0x0096,0x04b0);	// VSIZE_CAP(1920)	1944 
		sensor_write_value8(sd,0x0086,0x02 );				 // FPSTYPE_CAP(15fps)
		sensor_write_value8(sd,0x0083,0x00 );			 // SENSMODE_CAP(Full) 
		sensor_write_value8(sd,0x0082,0x01);  //UPDATED	
		printk("[gs5604] sensor_s_fmt  1200\n");
		msleep(200);
	}
	if(wsize->height==1936)  //   2592*1936
	{
		sensor_write_value8(sd,0x5008,0x00);
		sensor_write_value8(sd,0x0089,0x00);
		sensor_write_value16(sd,0x0090,0x0a20);
		sensor_write_value16(sd,0x0096,0x0790); //1936
		sensor_write_value8(sd,0x0086,0x02);
		sensor_write_value8(sd,0x0083,0x00);
		sensor_write_value8(sd,0x0082,0x01);
		printk("[gs5604] sensor_s_fmt 55 1936\n");
		msleep(200);

	}
	/**************** end set resolution ******************/

	if (wsize->set_size)
		LOG_ERR_RET(wsize->set_size(sd))

	sensor_s_hflip(sd,info->hflip);
	sensor_s_vflip(sd,info->vflip);

	if(info->capture_mode == V4L2_MODE_VIDEO ||
		info->capture_mode == V4L2_MODE_PREVIEW)
	{
		if (info->wb == V4L2_WHITE_BALANCE_AUTO) {
			ret = sensor_s_autowb(sd,1); //unlock wb
			if (ret < 0)
				vfe_dev_err("sensor_s_autowb on err when capturing image!\n");
		}
		if(info->low_speed == 1) {
			if(info->preview_first_flag == 1) {
				info->preview_first_flag = 0;
				msleep(600);
			}
			else {
				msleep(200);
			}
		}
		if( (info->width!=QSXGA_WIDTH)&&(info->preview_first_flag != 1) )
		{
			ret = sensor_s_relaunch_af_zone(sd);
			if (ret < 0) {
				vfe_dev_err("sensor_s_relaunch_af_zone err !\n");
				return ret;
			}
			msleep(50);			
			ret =  sensor_write_value8(sd, 0x00b2, 0x00); // sensor_s_single_af
			if (ret < 0) {
				vfe_dev_err("sensor_s_single_af err !\n");
				return ret;
			}
			if(info->auto_focus==1)
				sensor_s_continueous_af(sd,1);
			msleep(100);
		} 
		else
			msleep(150);
		sensor_s_continueous_af(sd,1);
	} 
 	//sensor_s_continueous_af(sd,1);
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
  
	vprintk("sensor_s_parm\n");
  
	if (parms->type != V4L2_BUF_TYPE_VIDEO_CAPTURE){
		vprintk("parms->type!=V4L2_BUF_TYPE_VIDEO_CAPTURE\n");
		return -EINVAL;
	}
  
	if (info->tpf.numerator == 0){
		vprintk("info->tpf.numerator == 0\n");
		return -EINVAL;
	}
    
	info->capture_mode = cp->capturemode;
  
	if (info->capture_mode == V4L2_MODE_IMAGE) {
		vprintk("capture mode is not video mode,can not set frame rate!\n");
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
  
	vprintk("set frame rate %d\n",tpf->denominator/tpf->numerator);
  
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
	case V4L2_CID_3A_LOCK:
		return v4l2_ctrl_query_fill(qc, 0, V4L2_LOCK_FOCUS, 1, 0);
//	case V4L2_CID_AUTO_FOCUS_RANGE:
//		return v4l2_ctrl_query_fill(qc, 0, 0, 0, 0);//only auto
	case V4L2_CID_AUTO_FOCUS_INIT:
	case V4L2_CID_AUTO_FOCUS_RELEASE:
	case V4L2_CID_AUTO_FOCUS_START:
	case V4L2_CID_AUTO_FOCUS_STOP:
	case V4L2_CID_AUTO_FOCUS_STATUS:
		return v4l2_ctrl_query_fill(qc, 0, 0, 0, 0);
	case V4L2_CID_FOCUS_AUTO:
		return v4l2_ctrl_query_fill(qc, 0, 1, 1, 0);
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
	case V4L2_CID_POWER_LINE_FREQUENCY:
		return sensor_g_band_filter(sd, &ctrl->value);
	case V4L2_CID_3A_LOCK:
		return sensor_g_3a_lock(sd);
//	case V4L2_CID_AUTO_FOCUS_RANGE:
//		ctrl->value=0;//only auto
//		return 0;
//	case V4L2_CID_AUTO_FOCUS_INIT:
//	case V4L2_CID_AUTO_FOCUS_RELEASE:
//	case V4L2_CID_AUTO_FOCUS_START:
//	case V4L2_CID_AUTO_FOCUS_STOP:
	case V4L2_CID_AUTO_FOCUS_STATUS:
		return sensor_g_af_status(sd);
//	case V4L2_CID_FOCUS_AUTO:
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
		return sensor_s_flash_mode(sd,(enum v4l2_flash_led_mode) ctrl->value);
	case V4L2_CID_POWER_LINE_FREQUENCY:
		return sensor_s_band_filter(sd, (enum v4l2_power_line_frequency) ctrl->value);

	case V4L2_CID_3A_LOCK:
		return sensor_s_3a_lock(sd, ctrl->value);
//	case V4L2_CID_AUTO_FOCUS_RANGE:
//		return 0;
	case V4L2_CID_AUTO_FOCUS_INIT:
		return sensor_s_init_af(sd);
	case V4L2_CID_AUTO_FOCUS_RELEASE:
		return sensor_s_release_af(sd);
	case V4L2_CID_AUTO_FOCUS_START:
		return sensor_s_single_af(sd);
	case V4L2_CID_AUTO_FOCUS_STOP:
		return sensor_s_pause_af(sd);
//	case V4L2_CID_AUTO_FOCUS_STATUS:
	case V4L2_CID_FOCUS_AUTO:
		return sensor_s_continueous_af(sd, ctrl->value);
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
	//.addr_width = CCI_BITS_16,
	//.data_width = CCI_BITS_16,
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

