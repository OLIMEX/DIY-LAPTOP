/*
 * A V4L2 driver for s5k3h7 cameras.
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
MODULE_DESCRIPTION("A low-level driver for s5k3h7 sensors");
MODULE_LICENSE("GPL");

//for internel driver debug
#define DEV_DBG_EN      1 
#if(DEV_DBG_EN == 1)    
#define vfe_dev_dbg(x,arg...) printk("[s5k3h7]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...) 
#endif
#define vfe_dev_err(x,arg...) printk("[s5k3h7]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[s5k3h7]"x,##arg)

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
#define V4L2_IDENT_SENSOR 0x4e10
int s5k3h7_sensor_vts;

//modified for each device for i2c access format
#define regval_list 		reg_list_w_a16_d16

#define REG_TERM 0xfffe
#define VAL_TERM 0xfe
#define REG_DLY  0xffff

/*
 * Our nominal (default) frame rate.
 */
#ifdef FPGA
#define SENSOR_FRAME_RATE 15
#else
#define SENSOR_FRAME_RATE 30
#endif

/*
 * The s5k3h7 sits on i2c with ID 0x20
 */
#define I2C_ADDR (0x10<<1)
#define SENSOR_NAME "s5k3h7"
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
/*
//$MIPI[Width:3264,Height:2448,Format:Raw10,Lane:4,ErrorCheck:0,PolarityData:0,PolarityClock:0,Buffer:4]
//=====================================================================================
// 3264 x 2448 RAW10 MIPI 4 lanes
// $Rev: 3272 $
// $Date: 2013-03-04 $
// 3H7 basic set file 
// Add LSC M2M TnP
// Add Pink image M2M modify TnP
// Add SHBN Improved TnP
// Add luminance level control TnP
//=====================================================================================

{16, 0x6010,0x0001},	// Reset
{16, 0xffff,3			},	// This delay can be reduced to 3ms, considering the worst case of external clock set to 6Mhz.
				// For optimization sake, the calculation of delay is 18000 cycles of external clock. e.g. 
				// for the case of external clock of 24Mhz, the required delay is 18000/(24*10^6) = 0.75ms
 Start T&P part
 DO NOT DELETE T&P SECTION COMMENTS! They are required to debug T&P related issues.
 https://svn/svn/SVNRoot/System/Software/tcevb/SDK+FW/ISP_3H5_7/Firmware
 SVN Rev: 43608-43608
 ROM Rev: A2
 Signature:
 md5 d27720579d7e81e392672a2e1958dc67 .btp
 md5 127aaa26dd7be401c293993a42f5e1d7 .htp
 md5 b598fd5c751192769e86b29f8956d59d .RegsMap.h
 md5 28fee824a285c1ac6c05c2dd14cba6ab .RegsMap.bin
 md5 08aee70892241325891780836db778d2 .base.RegsMap.h
 md5 8b85eff39783953fbe358970e8f6a9fa .base.RegsMap.bin

{16, 0x6028,0x7000},
{16, 0x602A,0x1750},
{16, 0x6F12,0x10B5},
{16, 0x6F12,0x00F0},
{16, 0x6F12,0xE1FB},
{16, 0x6F12,0x00F0},
{16, 0x6F12,0xE3FB},
{16, 0x6F12,0x10BC},
{16, 0x6F12,0x08BC},
{16, 0x6F12,0x1847},
{16, 0x6F12,0x2DE9},
{16, 0x6F12,0x7040},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0x3867},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x0140},
{16, 0x6F12,0xD6E1},
{16, 0x6F12,0xB010},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0x3057},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x1421},
{16, 0x6F12,0x81E2},
{16, 0x6F12,0x0110},
{16, 0x6F12,0x82E1},
{16, 0x6F12,0x1411},
{16, 0x6F12,0xD5E1},
{16, 0x6F12,0xB020},
{16, 0x6F12,0xC2E1},
{16, 0x6F12,0x0110},
{16, 0x6F12,0xC5E1},
{16, 0x6F12,0xB010},
{16, 0x6F12,0x00EB},
{16, 0x6F12,0xE601},
{16, 0x6F12,0xD6E1},
{16, 0x6F12,0xB000},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0x1827},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x1410},
{16, 0x6F12,0x80E2},
{16, 0x6F12,0x0100},
{16, 0x6F12,0x81E1},
{16, 0x6F12,0x1400},
{16, 0x6F12,0xD5E1},
{16, 0x6F12,0xB010},
{16, 0x6F12,0x80E1},
{16, 0x6F12,0x0100},
{16, 0x6F12,0xC5E1},
{16, 0x6F12,0xB000},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0xF406},
{16, 0x6F12,0xD0E1},
{16, 0x6F12,0xB00C},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0xA011},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0xEC06},
{16, 0x6F12,0x90E5},
{16, 0x6F12,0x0000},
{16, 0x6F12,0xD0E1},
{16, 0x6F12,0xBA39},
{16, 0x6F12,0x53E1},
{16, 0x6F12,0x0100},
{16, 0x6F12,0xD091},
{16, 0x6F12,0xBE09},
{16, 0x6F12,0xD081},
{16, 0x6F12,0xBC09},
{16, 0x6F12,0xC2E1},
{16, 0x6F12,0xB003},
{16, 0x6F12,0xBDE8},
{16, 0x6F12,0x7040},
{16, 0x6F12,0x2FE1},
{16, 0x6F12,0x1EFF},
{16, 0x6F12,0x2DE9},
{16, 0x6F12,0x3840},
{16, 0x6F12,0x10E3},
{16, 0x6F12,0x0100},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x0050},
{16, 0x6F12,0x9F15},
{16, 0x6F12,0xC406},
{16, 0x6F12,0x9015},
{16, 0x6F12,0x2400},
{16, 0x6F12,0x5013},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x000A},
{16, 0x6F12,0x1900},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0xB846},
{16, 0x6F12,0xD4E5},
{16, 0x6F12,0xD700},
{16, 0x6F12,0x50E3},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x001A},
{16, 0x6F12,0x0600},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x0120},
{16, 0x6F12,0x8DE2},
{16, 0x6F12,0x0010},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x2F00},
{16, 0x6F12,0x00EB},
{16, 0x6F12,0xC501},
{16, 0x6F12,0xDDE5},
{16, 0x6F12,0x0000},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0xA001},
{16, 0x6F12,0xC4E5},
{16, 0x6F12,0xD700},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0x8046},
{16, 0x6F12,0x94E5},
{16, 0x6F12,0x0000},
{16, 0x6F12,0xD0E5},
{16, 0x6F12,0x1102},
{16, 0x6F12,0x50E3},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x001A},
{16, 0x6F12,0x0900},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x0120},
{16, 0x6F12,0x8DE2},
{16, 0x6F12,0x0010},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x3700},
{16, 0x6F12,0x00EB},
{16, 0x6F12,0xB901},
{16, 0x6F12,0xDDE5},
{16, 0x6F12,0x0010},
{16, 0x6F12,0x94E5},
{16, 0x6F12,0x0000},
{16, 0x6F12,0xC0E5},
{16, 0x6F12,0x1112},
{16, 0x6F12,0x01E2},
{16, 0x6F12,0xFF00},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0x4816},
{16, 0x6F12,0xC1E1},
{16, 0x6F12,0xBE04},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x0500},
{16, 0x6F12,0xBDE8},
{16, 0x6F12,0x3840},
{16, 0x6F12,0x00EA},
{16, 0x6F12,0xB201},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0x3416},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x0000},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x012C},
{16, 0x6F12,0x81E0},
{16, 0x6F12,0x8030},
{16, 0x6F12,0x83E2},
{16, 0x6F12,0x013C},
{16, 0x6F12,0x80E2},
{16, 0x6F12,0x0100},
{16, 0x6F12,0x50E3},
{16, 0x6F12,0x0400},
{16, 0x6F12,0xC3E1},
{16, 0x6F12,0xBE28},
{16, 0x6F12,0xFFBA},
{16, 0x6F12,0xF9FF},
{16, 0x6F12,0x2FE1},
{16, 0x6F12,0x1EFF},
{16, 0x6F12,0x2DE9},
{16, 0x6F12,0x7040},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0x08C6},
{16, 0x6F12,0xDCE5},
{16, 0x6F12,0x1021},
{16, 0x6F12,0x52E3},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x001A},
{16, 0x6F12,0x0A00},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0x0CE6},
{16, 0x6F12,0x8CE0},
{16, 0x6F12,0x0231},
{16, 0x6F12,0x8EE0},
{16, 0x6F12,0x8250},
{16, 0x6F12,0xD5E1},
{16, 0x6F12,0xB050},
{16, 0x6F12,0x93E5},
{16, 0x6F12,0xD840},
{16, 0x6F12,0x82E2},
{16, 0x6F12,0x0120},
{16, 0x6F12,0x04E0},
{16, 0x6F12,0x9504},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x2444},
{16, 0x6F12,0x52E3},
{16, 0x6F12,0x0400},
{16, 0x6F12,0x83E5},
{16, 0x6F12,0xD840},
{16, 0x6F12,0xFFBA},
{16, 0x6F12,0xF5FF},
{16, 0x6F12,0xBDE8},
{16, 0x6F12,0x7040},
{16, 0x6F12,0x00EA},
{16, 0x6F12,0x9801},
{16, 0x6F12,0x2DE9},
{16, 0x6F12,0x1040},
{16, 0x6F12,0x00EB},
{16, 0x6F12,0x9801},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0xD405},
{16, 0x6F12,0xD0E5},
{16, 0x6F12,0x7310},
{16, 0x6F12,0xBDE8},
{16, 0x6F12,0x1040},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0xCC05},
{16, 0x6F12,0xFFEA},
{16, 0x6F12,0xE6FF},
{16, 0x6F12,0x2DE9},
{16, 0x6F12,0xFF4F},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0xB445},
{16, 0x6F12,0x4DE2},
{16, 0x6F12,0xA4D0},
{16, 0x6F12,0xD4E1},
{16, 0x6F12,0xB20D},
{16, 0x6F12,0xD4E5},
{16, 0x6F12,0x9CA0},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x0150},
{16, 0x6F12,0x8DE5},
{16, 0x6F12,0x5800},
{16, 0x6F12,0xD4E1},
{16, 0x6F12,0xB40D},
{16, 0x6F12,0x5AE3},
{16, 0x6F12,0x1000},
{16, 0x6F12,0xA023},
{16, 0x6F12,0x10A0},
{16, 0x6F12,0x8DE5},
{16, 0x6F12,0x5400},
{16, 0x6F12,0xD4E5},
{16, 0x6F12,0xDB00},
{16, 0x6F12,0xD4E5},
{16, 0x6F12,0xD710},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x2020},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x1500},
{16, 0x6F12,0x81E2},
{16, 0x6F12,0x0310},
{16, 0x6F12,0x01E2},
{16, 0x6F12,0xFF70},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x0010},
{16, 0x6F12,0x8DE5},
{16, 0x6F12,0xA000},
{16, 0x6F12,0xCDE1},
{16, 0x6F12,0xBC07},
{16, 0x6F12,0xCDE1},
{16, 0x6F12,0xBC05},
{16, 0x6F12,0x8DE5},
{16, 0x6F12,0x4C10},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0xB000},
{16, 0x6F12,0x8DE5},
{16, 0x6F12,0x5010},
{16, 0x6F12,0xD0E1},
{16, 0x6F12,0xF600},
{16, 0x6F12,0x8DE5},
{16, 0x6F12,0x4800},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0xB000},
{16, 0x6F12,0xD4E5},
{16, 0x6F12,0xD910},
{16, 0x6F12,0xD0E5},
{16, 0x6F12,0x0800},
{16, 0x6F12,0x80E0},
{16, 0x6F12,0x0100},
{16, 0x6F12,0x8DE5},
{16, 0x6F12,0x4400},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x1500},
{16, 0x6F12,0x80E0},
{16, 0x6F12,0xA00F},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0xC000},
{16, 0x6F12,0x8DE5},
{16, 0x6F12,0x4000},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0x3C15},
{16, 0x6F12,0x8DE2},
{16, 0x6F12,0x2000},
{16, 0x6F12,0x00EB},
{16, 0x6F12,0x6F01},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0x3415},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x1820},
{16, 0x6F12,0x8DE2},
{16, 0x6F12,0x0800},
{16, 0x6F12,0x00EB},
{16, 0x6F12,0x6B01},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0x2825},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x8DE5},
{16, 0x6F12,0x0400},
{16, 0x6F12,0x92E5},
{16, 0x6F12,0x0020},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0xF004},
{16, 0x6F12,0xD2E5},
{16, 0x6F12,0x5921},
{16, 0x6F12,0x90E5},
{16, 0x6F12,0x4010},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0xAC30},
{16, 0x6F12,0x82E0},
{16, 0x6F12,0x8221},
{16, 0x6F12,0x81E0},
{16, 0x6F12,0x8210},
{16, 0x6F12,0x81E0},
{16, 0x6F12,0x8310},
{16, 0x6F12,0xD1E1},
{16, 0x6F12,0xFA30},
{16, 0x6F12,0xD0E1},
{16, 0x6F12,0xBE04},
{16, 0x6F12,0xD1E1},
{16, 0x6F12,0xF210},
{16, 0x6F12,0x60E2},
{16, 0x6F12,0x012C},
{16, 0x6F12,0x02E0},
{16, 0x6F12,0x9302},
{16, 0x6F12,0x20E0},
{16, 0x6F12,0x9120},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x0004},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x4008},
{16, 0x6F12,0x8DE5},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0xC084},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0x5000},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0x5410},
{16, 0x6F12,0x88E0},
{16, 0x6F12,0x8000},
{16, 0x6F12,0xD0E1},
{16, 0x6F12,0xFC0B},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x0150},
{16, 0x6F12,0x00E0},
{16, 0x6F12,0x9100},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x0064},
{16, 0x6F12,0xB0E1},
{16, 0x6F12,0x4668},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0xA000},
{16, 0x6F12,0xA053},
{16, 0x6F12,0x0210},
{16, 0x6F12,0xE043},
{16, 0x6F12,0x0110},
{16, 0x6F12,0x00EB},
{16, 0x6F12,0x4C01},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x0098},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x4998},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x0140},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x05B0},
{16, 0x6F12,0x00EA},
{16, 0x6F12,0x0800},
{16, 0x6F12,0x8DE2},
{16, 0x6F12,0x5C00},
{16, 0x6F12,0x80E0},
{16, 0x6F12,0x8450},
{16, 0x6F12,0x55E1},
{16, 0x6F12,0xF200},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0xA010},
{16, 0x6F12,0x00E0},
{16, 0x6F12,0x9600},
{16, 0x6F12,0x89E0},
{16, 0x6F12,0x8000},
{16, 0x6F12,0x00EB},
{16, 0x6F12,0x4001},
{16, 0x6F12,0x84E2},
{16, 0x6F12,0x0140},
{16, 0x6F12,0xC5E1},
{16, 0x6F12,0xB000},
{16, 0x6F12,0x54E1},
{16, 0x6F12,0x0A00},
{16, 0x6F12,0xFFDA},
{16, 0x6F12,0xF4FF},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x0090},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0x4804},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0x5810},
{16, 0x6F12,0x80E0},
{16, 0x6F12,0x8900},
{16, 0x6F12,0xD0E1},
{16, 0x6F12,0xFE09},
{16, 0x6F12,0x00E0},
{16, 0x6F12,0x9100},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x0064},
{16, 0x6F12,0xB0E1},
{16, 0x6F12,0x4668},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0xA000},
{16, 0x6F12,0xA053},
{16, 0x6F12,0x0210},
{16, 0x6F12,0xE043},
{16, 0x6F12,0x0110},
{16, 0x6F12,0x00EB},
{16, 0x6F12,0x3001},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x0088},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x4888},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x0140},
{16, 0x6F12,0x00EA},
{16, 0x6F12,0x0800},
{16, 0x6F12,0x8DE2},
{16, 0x6F12,0x7C00},
{16, 0x6F12,0x80E0},
{16, 0x6F12,0x8450},
{16, 0x6F12,0x55E1},
{16, 0x6F12,0xF200},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0xA010},
{16, 0x6F12,0x00E0},
{16, 0x6F12,0x9600},
{16, 0x6F12,0x88E0},
{16, 0x6F12,0x8000},
{16, 0x6F12,0x00EB},
{16, 0x6F12,0x2501},
{16, 0x6F12,0x84E2},
{16, 0x6F12,0x0140},
{16, 0x6F12,0xC5E1},
{16, 0x6F12,0xB000},
{16, 0x6F12,0x54E1},
{16, 0x6F12,0x0A00},
{16, 0x6F12,0xFFDA},
{16, 0x6F12,0xF4FF},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x0080},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x0860},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x0850},
{16, 0x6F12,0x00EA},
{16, 0x6F12,0x2300},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x0040},
{16, 0x6F12,0x00EA},
{16, 0x6F12,0x1E00},
{16, 0x6F12,0x45E0},
{16, 0x6F12,0x0400},
{16, 0x6F12,0x8DE2},
{16, 0x6F12,0x7C10},
{16, 0x6F12,0x8DE2},
{16, 0x6F12,0x5C20},
{16, 0x6F12,0x81E0},
{16, 0x6F12,0x8410},
{16, 0x6F12,0x82E0},
{16, 0x6F12,0x8000},
{16, 0x6F12,0xD1E1},
{16, 0x6F12,0xF010},
{16, 0x6F12,0xD0E1},
{16, 0x6F12,0xF000},
{16, 0x6F12,0x0BE0},
{16, 0x6F12,0x9100},
{16, 0x6F12,0x5BE3},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0xA000},
{16, 0x6F12,0xA0A3},
{16, 0x6F12,0x0210},
{16, 0x6F12,0xE0B3},
{16, 0x6F12,0x0110},
{16, 0x6F12,0x00EB},
{16, 0x6F12,0x0E01},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0xA010},
{16, 0x6F12,0x80E0},
{16, 0x6F12,0x0B00},
{16, 0x6F12,0x00EB},
{16, 0x6F12,0x0B01},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0xA410},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x0120},
{16, 0x6F12,0x81E0},
{16, 0x6F12,0x8610},
{16, 0x6F12,0xD1E1},
{16, 0x6F12,0xF010},
{16, 0x6F12,0x00E0},
{16, 0x6F12,0x9100},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x0210},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x1117},
{16, 0x6F12,0x50E3},
{16, 0x6F12,0x0000},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x1227},
{16, 0x6F12,0x62B2},
{16, 0x6F12,0x0020},
{16, 0x6F12,0x80E0},
{16, 0x6F12,0x0200},
{16, 0x6F12,0x00EB},
{16, 0x6F12,0xFF00},
{16, 0x6F12,0x88E0},
{16, 0x6F12,0x0080},
{16, 0x6F12,0x86E2},
{16, 0x6F12,0x0160},
{16, 0x6F12,0x84E2},
{16, 0x6F12,0x0140},
{16, 0x6F12,0x54E1},
{16, 0x6F12,0x0500},
{16, 0x6F12,0xFFDA},
{16, 0x6F12,0xDEFF},
{16, 0x6F12,0x85E2},
{16, 0x6F12,0x0150},
{16, 0x6F12,0x55E1},
{16, 0x6F12,0x0A00},
{16, 0x6F12,0xFFDA},
{16, 0x6F12,0xD9FF},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0x0000},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x021B},
{16, 0x6F12,0x00E0},
{16, 0x6F12,0x9800},
{16, 0x6F12,0x81E0},
{16, 0x6F12,0x4014},
{16, 0x6F12,0x51E3},
{16, 0x6F12,0x020B},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0x4C10},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0xAC20},
{16, 0x6F12,0xA0A1},
{16, 0x6F12,0x4004},
{16, 0x6F12,0x80A2},
{16, 0x6F12,0x020B},
{16, 0x6F12,0x82E0},
{16, 0x6F12,0x0111},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0xA820},
{16, 0x6F12,0xA0B3},
{16, 0x6F12,0x020B},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x0008},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x4008},
{16, 0x6F12,0x82E0},
{16, 0x6F12,0x8110},
{16, 0x6F12,0xC1E1},
{16, 0x6F12,0xB000},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0x0410},
{16, 0x6F12,0x89E2},
{16, 0x6F12,0x0190},
{16, 0x6F12,0x50E1},
{16, 0x6F12,0x0100},
{16, 0x6F12,0xA0D1},
{16, 0x6F12,0x0100},
{16, 0x6F12,0x8DE5},
{16, 0x6F12,0x0400},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0x4C00},
{16, 0x6F12,0x59E3},
{16, 0x6F12,0x0F00},
{16, 0x6F12,0x80E2},
{16, 0x6F12,0x0100},
{16, 0x6F12,0x8DE5},
{16, 0x6F12,0x4C00},
{16, 0x6F12,0xFFBA},
{16, 0x6F12,0xA1FF},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0x5000},
{16, 0x6F12,0x80E2},
{16, 0x6F12,0x0100},
{16, 0x6F12,0x50E3},
{16, 0x6F12,0x0B00},
{16, 0x6F12,0x8DE5},
{16, 0x6F12,0x5000},
{16, 0x6F12,0xFFBA},
{16, 0x6F12,0x7EFF},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0x0400},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0xAC20},
{16, 0x6F12,0x50E3},
{16, 0x6F12,0x020A},
{16, 0x6F12,0xA0C1},
{16, 0x6F12,0x0004},
{16, 0x6F12,0xA0C1},
{16, 0x6F12,0xC01F},
{16, 0x6F12,0x80C0},
{16, 0x6F12,0xA109},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0xA812},
{16, 0x6F12,0xA0D3},
{16, 0x6F12,0x010C},
{16, 0x6F12,0x81E0},
{16, 0x6F12,0x8210},
{16, 0x6F12,0xA0C1},
{16, 0x6F12,0xC006},
{16, 0x6F12,0x8DE5},
{16, 0x6F12,0x9C10},
{16, 0x6F12,0xC1E1},
{16, 0x6F12,0xB000},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0xB000},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0x9C10},
{16, 0x6F12,0xD0E1},
{16, 0x6F12,0xF400},
{16, 0x6F12,0xD1E1},
{16, 0x6F12,0xB010},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x0050},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x0004},
{16, 0x6F12,0x00EB},
{16, 0x6F12,0xC500},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x0088},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0xB000},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x4888},
{16, 0x6F12,0xC0E1},
{16, 0x6F12,0xB480},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0x4800},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x0060},
{16, 0x6F12,0x40E2},
{16, 0x6F12,0x029B},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0xB010},
{16, 0x6F12,0x8DE2},
{16, 0x6F12,0x0800},
{16, 0x6F12,0x80E0},
{16, 0x6F12,0x8600},
{16, 0x6F12,0xD0E1},
{16, 0x6F12,0xF000},
{16, 0x6F12,0xD1E1},
{16, 0x6F12,0xF210},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x0040},
{16, 0x6F12,0x40E0},
{16, 0x6F12,0x0100},
{16, 0x6F12,0x07E0},
{16, 0x6F12,0x9000},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0xB010},
{16, 0x6F12,0x8DE2},
{16, 0x6F12,0x2000},
{16, 0x6F12,0x80E0},
{16, 0x6F12,0x8400},
{16, 0x6F12,0xD0E1},
{16, 0x6F12,0xF000},
{16, 0x6F12,0xD1E1},
{16, 0x6F12,0xF010},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0x20C2},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x0130},
{16, 0x6F12,0x40E0},
{16, 0x6F12,0x0100},
{16, 0x6F12,0x02E0},
{16, 0x6F12,0x9000},
{16, 0x6F12,0xDCE5},
{16, 0x6F12,0xD800},
{16, 0x6F12,0x82E0},
{16, 0x6F12,0x0720},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x1310},
{16, 0x6F12,0x81E0},
{16, 0x6F12,0xA11F},
{16, 0x6F12,0x82E0},
{16, 0x6F12,0xC110},
{16, 0x6F12,0xDCE5},
{16, 0x6F12,0xDA20},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x3110},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x1302},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0x4030},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0x44C0},
{16, 0x6F12,0x23E0},
{16, 0x6F12,0x9931},
{16, 0x6F12,0x80E0},
{16, 0x6F12,0x533C},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0x40C0},
{16, 0x6F12,0x80E0},
{16, 0x6F12,0xA00F},
{16, 0x6F12,0x21E0},
{16, 0x6F12,0x98C1},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0x44C0},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x511C},
{16, 0x6F12,0x01E0},
{16, 0x6F12,0x9301},
{16, 0x6F12,0x81E0},
{16, 0x6F12,0xC000},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x50B2},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0x9C00},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0xA820},
{16, 0x6F12,0xD0E1},
{16, 0x6F12,0xB010},
{16, 0x6F12,0x9DE5},
{16, 0x6F12,0xAC00},
{16, 0x6F12,0x80E0},
{16, 0x6F12,0x0501},
{16, 0x6F12,0x82E0},
{16, 0x6F12,0x80A0},
{16, 0x6F12,0xDAE1},
{16, 0x6F12,0xF000},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x0004},
{16, 0x6F12,0x00EB},
{16, 0x6F12,0x9000},
{16, 0x6F12,0x40E0},
{16, 0x6F12,0x0B00},
{16, 0x6F12,0x84E2},
{16, 0x6F12,0x0140},
{16, 0x6F12,0x54E3},
{16, 0x6F12,0x0F00},
{16, 0x6F12,0x85E2},
{16, 0x6F12,0x0150},
{16, 0x6F12,0xCAE1},
{16, 0x6F12,0xB000},
{16, 0x6F12,0xFFBA},
{16, 0x6F12,0xD3FF},
{16, 0x6F12,0x86E2},
{16, 0x6F12,0x0160},
{16, 0x6F12,0x56E3},
{16, 0x6F12,0x0B00},
{16, 0x6F12,0xFFBA},
{16, 0x6F12,0xC8FF},
{16, 0x6F12,0x8DE2},
{16, 0x6F12,0xB4D0},
{16, 0x6F12,0xBDE8},
{16, 0x6F12,0xF04F},
{16, 0x6F12,0x2FE1},
{16, 0x6F12,0x1EFF},
{16, 0x6F12,0x2DE9},
{16, 0x6F12,0xF041},
{16, 0x6F12,0x00EB},
{16, 0x6F12,0x8400},
{16, 0x6F12,0x50E3},
{16, 0x6F12,0x0000},
{16, 0x6F12,0xBD08},
{16, 0x6F12,0xF041},
{16, 0x6F12,0xA003},
{16, 0x6F12,0x0010},
{16, 0x6F12,0xA003},
{16, 0x6F12,0x3800},
{16, 0x6F12,0x000A},
{16, 0x6F12,0x8100},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0x6C11},
{16, 0x6F12,0xD1E1},
{16, 0x6F12,0xBA01},
{16, 0x6F12,0xD1E1},
{16, 0x6F12,0xBC21},
{16, 0x6F12,0xD1E1},
{16, 0x6F12,0xBE11},
{16, 0x6F12,0x80E1},
{16, 0x6F12,0x0208},
{16, 0x6F12,0x00EB},
{16, 0x6F12,0x7D00},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x0070},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0x5451},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0x5401},
{16, 0x6F12,0xD5E1},
{16, 0x6F12,0xF030},
{16, 0x6F12,0xD0E1},
{16, 0x6F12,0xBAEA},
{16, 0x6F12,0xD0E1},
{16, 0x6F12,0xBCCA},
{16, 0x6F12,0xD5E1},
{16, 0x6F12,0xF220},
{16, 0x6F12,0x00E0},
{16, 0x6F12,0x930C},
{16, 0x6F12,0x42E0},
{16, 0x6F12,0x0360},
{16, 0x6F12,0x02E0},
{16, 0x6F12,0x9E02},
{16, 0x6F12,0x4CE0},
{16, 0x6F12,0x0E40},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x0410},
{16, 0x6F12,0x40E0},
{16, 0x6F12,0x0200},
{16, 0x6F12,0x00EB},
{16, 0x6F12,0x6900},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x0080},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0x2401},
{16, 0x6F12,0xD0E1},
{16, 0x6F12,0xB000},
{16, 0x6F12,0x10E3},
{16, 0x6F12,0x020C},
{16, 0x6F12,0xA011},
{16, 0x6F12,0x0700},
{16, 0x6F12,0x001B},
{16, 0x6F12,0x6B00},
{16, 0x6F12,0x56E3},
{16, 0x6F12,0x0000},
{16, 0x6F12,0xE003},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x000A},
{16, 0x6F12,0x0300},
{16, 0x6F12,0x47E0},
{16, 0x6F12,0x0800},
{16, 0x6F12,0x00E0},
{16, 0x6F12,0x9400},
{16, 0x6F12,0xA0E1},
{16, 0x6F12,0x0610},
{16, 0x6F12,0x00EB},
{16, 0x6F12,0x6200},
{16, 0x6F12,0xC5E1},
{16, 0x6F12,0xB400},
{16, 0x6F12,0xBDE8},
{16, 0x6F12,0xF041},
{16, 0x6F12,0x2FE1},
{16, 0x6F12,0x1EFF},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0xEC10},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0xEC00},
{16, 0x6F12,0x2DE9},
{16, 0x6F12,0x1040},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0xE820},
{16, 0x6F12,0x80E5},
{16, 0x6F12,0x5010},
{16, 0x6F12,0x42E0},
{16, 0x6F12,0x0110},
{16, 0x6F12,0xC0E1},
{16, 0x6F12,0xB415},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0xDC00},
{16, 0x6F12,0x4FE2},
{16, 0x6F12,0xD410},
{16, 0x6F12,0x00EB},
{16, 0x6F12,0x5900},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0xD400},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0xD440},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0x9420},
{16, 0x6F12,0x84E5},
{16, 0x6F12,0x0400},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x0000},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x011C},
{16, 0x6F12,0x82E0},
{16, 0x6F12,0x8030},
{16, 0x6F12,0x80E2},
{16, 0x6F12,0x0100},
{16, 0x6F12,0x50E3},
{16, 0x6F12,0x0400},
{16, 0x6F12,0xC3E1},
{16, 0x6F12,0xB010},
{16, 0x6F12,0xFF3A},
{16, 0x6F12,0xFAFF},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0xB000},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0xB410},
{16, 0x6F12,0x84E5},
{16, 0x6F12,0x5C00},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0xA800},
{16, 0x6F12,0x84E5},
{16, 0x6F12,0x2C00},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0xA800},
{16, 0x6F12,0x00EB},
{16, 0x6F12,0x4700},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0xA400},
{16, 0x6F12,0x4FE2},
{16, 0x6F12,0x711E},
{16, 0x6F12,0x84E5},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0x9C00},
{16, 0x6F12,0x00EB},
{16, 0x6F12,0x4200},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0x2410},
{16, 0x6F12,0xC1E1},
{16, 0x6F12,0xB000},
{16, 0x6F12,0x9FE5},
{16, 0x6F12,0x5C00},
{16, 0x6F12,0xD0E1},
{16, 0x6F12,0xB012},
{16, 0x6F12,0x51E3},
{16, 0x6F12,0x1000},
{16, 0x6F12,0x009A},
{16, 0x6F12,0x0200},
{16, 0x6F12,0xA0E3},
{16, 0x6F12,0x090C},
{16, 0x6F12,0x00EB},
{16, 0x6F12,0x3400},
{16, 0x6F12,0xFFEA},
{16, 0x6F12,0xFEFF},
{16, 0x6F12,0xBDE8},
{16, 0x6F12,0x1040},
{16, 0x6F12,0x2FE1},
{16, 0x6F12,0x1EFF},
{16, 0x6F12,0x0070},
{16, 0x6F12,0xC41F},
{16, 0x6F12,0x00D0},
{16, 0x6F12,0x0061},
{16, 0x6F12,0x0070},
{16, 0x6F12,0x5014},
{16, 0x6F12,0x0070},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x00D0},
{16, 0x6F12,0x00F4},
{16, 0x6F12,0x0070},
{16, 0x6F12,0x7004},
{16, 0x6F12,0x0070},
{16, 0x6F12,0xD005},
{16, 0x6F12,0x0070},
{16, 0x6F12,0xC61F},
{16, 0x6F12,0x0070},
{16, 0x6F12,0x1013},
{16, 0x6F12,0x0070},
{16, 0x6F12,0xB412},
{16, 0x6F12,0x0070},
{16, 0x6F12,0x8C1F},
{16, 0x6F12,0x0070},
{16, 0x6F12,0xAC1F},
{16, 0x6F12,0x0070},
{16, 0x6F12,0x0400},
{16, 0x6F12,0x00D0},
{16, 0x6F12,0x0093},
{16, 0x6F12,0x0070},
{16, 0x6F12,0x8012},
{16, 0x6F12,0x0070},
{16, 0x6F12,0xC00B},
{16, 0x6F12,0x0070},
{16, 0x6F12,0xE012},
{16, 0x6F12,0x0070},
{16, 0x6F12,0xD01F},
{16, 0x6F12,0x0070},
{16, 0x6F12,0x7005},
{16, 0x6F12,0x0070},
{16, 0x6F12,0x902D},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x90A6},
{16, 0x6F12,0x0070},
{16, 0x6F12,0xFC18},
{16, 0x6F12,0x0070},
{16, 0x6F12,0xF804},
{16, 0x6F12,0x0070},
{16, 0x6F12,0x9818},
{16, 0x6F12,0x0070},
{16, 0x6F12,0xE018},
{16, 0x6F12,0x0070},
{16, 0x6F12,0x7018},
{16, 0x6F12,0x0000},
{16, 0x6F12,0xC06A},
{16, 0x6F12,0x0070},
{16, 0x6F12,0xE017},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x781C},
{16, 0x6F12,0x7847},
{16, 0x6F12,0xC046},
{16, 0x6F12,0xFFEA},
{16, 0x6F12,0xB4FF},
{16, 0x6F12,0x7847},
{16, 0x6F12,0xC046},
{16, 0x6F12,0x1FE5},
{16, 0x6F12,0x04F0},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x6CCE},
{16, 0x6F12,0x1FE5},
{16, 0x6F12,0x04F0},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x781C},
{16, 0x6F12,0x1FE5},
{16, 0x6F12,0x04F0},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x54C0},
{16, 0x6F12,0x1FE5},
{16, 0x6F12,0x04F0},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x8448},
{16, 0x6F12,0x1FE5},
{16, 0x6F12,0x04F0},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x146C},
{16, 0x6F12,0x1FE5},
{16, 0x6F12,0x04F0},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x4C7E},
{16, 0x6F12,0x1FE5},
{16, 0x6F12,0x04F0},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x8CDC},
{16, 0x6F12,0x1FE5},
{16, 0x6F12,0x04F0},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x48DD},
{16, 0x6F12,0x1FE5},
{16, 0x6F12,0x04F0},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x7C55},
{16, 0x6F12,0x1FE5},
{16, 0x6F12,0x04F0},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x744C},
{16, 0x6F12,0x1FE5},
{16, 0x6F12,0x04F0},
{16, 0x6F12,0x0000},
{16, 0x6F12,0xE8DE},
{16, 0x6F12,0x1FE5},
{16, 0x6F12,0x04F0},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x4045},
{16, 0x6F12,0x1FE5},
{16, 0x6F12,0x04F0},
{16, 0x6F12,0x0000},
{16, 0x6F12,0xE8CD},
{16, 0x6F12,0x80F9},
{16, 0x6F12,0x00FA},
{16, 0x6F12,0x00FB},
{16, 0x6F12,0x00FC},
{16, 0x6F12,0x00FD},
{16, 0x6F12,0x00FE},
{16, 0x6F12,0x00FF},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x0001},
{16, 0x6F12,0x0002},
{16, 0x6F12,0x0003},
{16, 0x6F12,0x0004},
{16, 0x6F12,0x0005},
{16, 0x6F12,0x0006},
{16, 0x6F12,0x8006},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x00FB},
{16, 0x6F12,0x00FC},
{16, 0x6F12,0x00FD},
{16, 0x6F12,0x00FE},
{16, 0x6F12,0x00FF},
{16, 0x6F12,0x0000},
{16, 0x6F12,0x0001},
{16, 0x6F12,0x0002},
{16, 0x6F12,0x0003},
{16, 0x6F12,0x0004},
{16, 0x6F12,0x0005},
{16, 0x6F12,0x0000},
//
// Parameters Defined in T&P:
//                                                              1610633348 70000000 STRUCT
// smiaRegs_rw                                                    576 70000000 STRUCT
// smiaRegs_ro                                                    432 70000000 STRUCT
// smiaRegs_rd                                                    112 70000000 STRUCT
// smiaRegs                                                       688 70000010 STRUCT
// ContextB                                                       820 70000E30 STRUCT
// smiaRegsB                                                      688 70000E30 STRUCT
// smiaRegsB_rd                                                   112 70000E30 STRUCT
// smiaRegsB_rd_general                                            32 70000E30 STRUCT
// smiaRegsB_rd_general_model_id                                    2 70000E30 SHORT
// smiaRegsB_rd_general_revision_number_major                       1 70000E32 CHAR
// smiaRegsB_rd_general_manufacturer_id                             1 70000E33 CHAR
// smiaRegsB_rd_general_smia_version                                1 70000E34 CHAR
// smiaRegsB_rd_general_frame_count                                 1 70000E35 CHAR
// smiaRegsB_rd_general_pixel_order                                 1 70000E36 CHAR
// smiaRegsB_rd_general_reserved0                                   1 70000E37 CHAR
// smiaRegsB_rd_general_data_pedestal                               2 70000E38 SHORT
// smiaRegsB_rd_general_temperature                                 2 70000E3A SHORT
// smiaRegsB_rd_general_pixel_depth                                 1 70000E3C CHAR
// smiaRegsB_rd_general_reserved2                                   3 70000E3D ARRAY
// smiaRegsB_rd_general_reserved2[0]                                1 70000E3D CHAR
// smiaRegsB_rd_general_reserved2[1]                                1 70000E3E CHAR
// smiaRegsB_rd_general_reserved2[2]                                1 70000E3F CHAR
// smiaRegsB_rd_general_revision_number_minor                       1 70000E40 CHAR
// smiaRegsB_rd_general_additional_specification_version            1 70000E41 CHAR
// smiaRegsB_rd_general_module_date_year                            1 70000E42 CHAR
// smiaRegsB_rd_general_module_date_month                           1 70000E43 CHAR
// smiaRegsB_rd_general_module_date_day                             1 70000E44 CHAR
// smiaRegsB_rd_general_module_date_phase                           1 70000E45 CHAR
// smiaRegsB_rd_general_sensor_model_id                             2 70000E46 SHORT
// smiaRegsB_rd_general_sensor_revision_number                      1 70000E48 CHAR
// smiaRegsB_rd_general_sensor_manufacturer_id                      1 70000E49 CHAR
// smiaRegsB_rd_general_sensor_firmware_version                     1 70000E4A CHAR
// smiaRegsB_rd_general_reserved3                                   1 70000E4B CHAR
// smiaRegsB_rd_general_serial_number_hword                         2 70000E4C SHORT
// smiaRegsB_rd_general_serial_number_lword                         2 70000E4E SHORT
// smiaRegsB_rd_frame_format                                       32 70000E50 STRUCT
// smiaRegsB_rd_frame_format_frame_format_model_type                1 70000E50 CHAR
// smiaRegsB_rd_frame_format_frame_format_model_subtype_col_row     1 70000E51 CHAR
// smiaRegsB_rd_frame_format_frame_format_descriptor_0              2 70000E52 SHORT
// smiaRegsB_rd_frame_format_frame_format_descriptor_1              2 70000E54 SHORT
// smiaRegsB_rd_frame_format_frame_format_descriptor_2              2 70000E56 SHORT
// smiaRegsB_rd_frame_format_frame_format_descriptor_3              2 70000E58 SHORT
// smiaRegsB_rd_frame_format_frame_format_descriptor_4              2 70000E5A SHORT
// smiaRegsB_rd_frame_format_frame_format_descriptor_5              2 70000E5C SHORT
// smiaRegsB_rd_frame_format_frame_format_descriptor_6              2 70000E5E SHORT
// smiaRegsB_rd_frame_format_frame_format_descriptor_7              2 70000E60 SHORT
// smiaRegsB_rd_frame_format_frame_format_descriptor_8              2 70000E62 SHORT
// smiaRegsB_rd_frame_format_frame_format_descriptor_9              2 70000E64 SHORT
// smiaRegsB_rd_frame_format_frame_format_descriptor_10             2 70000E66 SHORT
// smiaRegsB_rd_frame_format_frame_format_descriptor_11             2 70000E68 SHORT
// smiaRegsB_rd_frame_format_frame_format_descriptor_12             2 70000E6A SHORT
// smiaRegsB_rd_frame_format_frame_format_descriptor_13             2 70000E6C SHORT
// smiaRegsB_rd_frame_format_frame_format_descriptor_14             2 70000E6E SHORT
// smiaRegsB_rd_analog_gain                                        32 70000E70 STRUCT
// smiaRegsB_rd_analog_gain_analogue_gain_capabiltiy                2 70000E70 SHORT
// smiaRegsB_rd_analog_gain_reserved                                2 70000E72 SHORT
// smiaRegsB_rd_analog_gain_analogue_gain_code_min                  2 70000E74 SHORT
// smiaRegsB_rd_analog_gain_analogue_gain_code_max                  2 70000E76 SHORT
// smiaRegsB_rd_analog_gain_analogue_gain_code_step                 2 70000E78 SHORT
// smiaRegsB_rd_analog_gain_analogue_gain_type                      2 70000E7A SHORT
// smiaRegsB_rd_analog_gain_analogue_gain_m0                        2 70000E7C SHORT
// smiaRegsB_rd_analog_gain_analogue_gain_c0                        2 70000E7E SHORT
// smiaRegsB_rd_analog_gain_analogue_gain_m1                        2 70000E80 SHORT
// smiaRegsB_rd_analog_gain_analogue_gain_c1                        2 70000E82 SHORT
// smiaRegsB_rd_analog_gain_dummy_align                            12 70000E84 ARRAY
// smiaRegsB_rd_analog_gain_dummy_align[0]                          2 70000E84 SHORT
// smiaRegsB_rd_analog_gain_dummy_align[1]                          2 70000E86 SHORT
// smiaRegsB_rd_analog_gain_dummy_align[2]                          2 70000E88 SHORT
// smiaRegsB_rd_analog_gain_dummy_align[3]                          2 70000E8A SHORT
// smiaRegsB_rd_analog_gain_dummy_align[4]                          2 70000E8C SHORT
// smiaRegsB_rd_analog_gain_dummy_align[5]                          2 70000E8E SHORT
// smiaRegsB_rd_data_format                                        16 70000E90 STRUCT
// smiaRegsB_rd_data_format_data_format_model_type                  1 70000E90 CHAR
// smiaRegsB_rd_data_format_data_format_model_subtype               1 70000E91 CHAR
// smiaRegsB_rd_data_format_data_format_descriptor_0                2 70000E92 SHORT
// smiaRegsB_rd_data_format_data_format_descriptor_1                2 70000E94 SHORT
// smiaRegsB_rd_data_format_data_format_descriptor_2                2 70000E96 SHORT
// smiaRegsB_rd_data_format_data_format_descriptor_3                2 70000E98 SHORT
// smiaRegsB_rd_data_format_data_format_descriptor_4                2 70000E9A SHORT
// smiaRegsB_rd_data_format_data_format_descriptor_5                2 70000E9C SHORT
// smiaRegsB_rd_data_format_data_format_descriptor_6                2 70000E9E SHORT
// smiaRegsB_rw                                                   576 70000EA0 STRUCT
// smiaRegs_rw_analog_gain_mode_dummy_align                         9 D0000121 ARRAY
// smiaRegs_rw_analog_gain_mode_AG_th                               2 D000012A SHORT
// smiaRegs_rw_analog_gain_mode_F430_val                            2 D000012C SHORT
// smiaRegs_rw_analog_gain_mode_F430_default_val                    2 D000012E SHORT
// smiaRegs_ro_edof_cap_uAlphaTempInd                               1 D0001989 CHAR
// smiaRegs_ro_edof_cap_dummy_align                                 6 D000198A ARRAY
// smiaRegs_ro_edof_cap_dummy_align[0]                              1 D000198A CHAR
// smiaRegs_ro_edof_cap_dummy_align[1]                              1 D000198B CHAR
// smiaRegs_ro_edof_cap_dummy_align[2]                              1 D000198C CHAR
// smiaRegs_ro_edof_cap_dummy_align[3]                              1 D000198D CHAR
// smiaRegs_ro_edof_cap_dummy_align[4]                              1 D000198E CHAR
// smiaRegs_ro_edof_cap_dummy_align[5]                              1 D000198F CHAR
// smiaRegsB_rw_analog_gain_mode_dummy_align                        9 D0002121 ARRAY
// smiaRegsB_rw_analog_gain_mode_AG_th                              2 D000212A SHORT
// smiaRegsB_rw_analog_gain_mode_F430_val                           2 D000212C SHORT
// smiaRegsB_rw_analog_gain_mode_F430_default_val                   2 D000212E SHORT
//
// End T&P part
//=====================================================================================
// Base setfile : Rev - 3126 
// Date: 2012-01-05 15:10:35 +0900 (THU, 05 JAN 2012) 
// 3H7 Analog set file BQ Mode
//=====================================================================================
{16, 0x6028,0xD000},
//=====================================================================================
// APS/Analog setting (Date: 2012-01-05 15:10:35 +0900 (THU, 05 JAN 2012)) 
//=====================================================================================
//=====================================================================================
// START OF FW REGISTERS APS/Analog UPDATING
//=====================================================================================
// Offset control
{16, 0x38FA,0x0030},  // gisp_offs_gains_bls_offs_0_
{16, 0x38FC,0x0030},  // gisp_offs_gains_bls_offs_1_

// Sensor XY cordination
{16, 0x32CE,0x0060},    // senHal_usWidthStOfsInit
{16, 0x32D0,0x0024},    // senHal_usHeightStOfsInit

{16, 0x0086,0x01FF},	//#smiaRegs_rd_analog_gain_analogue_gain_code_max        
                                                                  
{16, 0x012A,0x0040},	//#smiaRegs_rw_analog_gain_mode_AG_th            
{16, 0x012C,0x7077},	//#smiaRegs_rw_analog_gain_mode_F430_val         
{16, 0x012E,0x7777},	//#smiaRegs_rw_analog_gain_mode_F430_default_val 

// For 35Mhz BQ Mode
//========================================================
// Setting for MIPI CLK (Don't change)
{16, 0x6218,0xF1D0},	// open all clocks
{16, 0x6214,0xF9F0},	// open all clocks
{16, 0x6226,0x0001},	// open APB clock for I2C transaction
//=====================================================================================
// START OF HW REGISTERS APS/Analog UPDATING
//=====================================================================================
{16, 0xB0C0,0x000C},
{16, 0xF400,0x0BBC}, 
{16, 0xF616,0x0004}, //aig_tmc_gain 
//=====================================================================================
// END OF HW REGISTERS APS/Analog UPDATING
//=====================================================================================
{16, 0x6226,0x0000}, //close APB clock for I2C transaction
{16, 0x6218,0xF9F0}, //close all clocks
//=====================================================================================
// End of APS/Analog setting
//=====================================================================================//=====================================================================================
// START default setting
//=====================================================================================

//=====================================================================================
// End default setting
//=====================================================================================
// Test Name : "fw_8M_280_sclk_BQ_35_mipi.tset"
////////////////////////////////////////////////
//                                            //
//     PLUSARGS for configuration	//
//                                            //
////////////////////////////////////////////////
{16, 0x3338,0x0264}, //senHal_MaxCdsTime 								0264

// set PLL
{16, 0x030E,0x00A2},	// smiaRegs_rw_clocks_secnd_pll_multiplier

{16, 0x311C,0x0BB8},	//#skl_uEndFrCyclesNoCfgDiv4	0BB8		//Increase Blank time on account of process time 
{16, 0x311E,0x0BB8},	//#skl_uEndFrCyclesWithCfgDiv4	0BB8		//Increase Blank time on account of process time 

// Set FPS
{16, 0x0342,0x0EA8},	// smiaRegs_rw_frame_timing_line_length_pck //For MIPI CLK 648Mhz 30.03fps (Default 30fps MIPI_clk 664Mhz 0E50)
{16, 0x0340,0x09B5},	// smiaRegs_rw_frame_timing_frame_length_lines //For MIPI CLK 648MHz 30.03fps (Default 30fps MIPI_clk 664Mhz 09F2)

//034C 0CC8	//smiaRegs_rw_frame_timing_x_output_size			0CC8
//034E 0998	//smiaRegs_rw_frame_timing_y_output_size			0998
//0344 0000	//smiaRegs_rw_frame_timing_x_addr_start			0000
//0346 0000 //smiaRegs_rw_frame_timing_y_addr_start			0000
//0348 0CC7	//smiaRegs_rw_frame_timing_x_addr_end			0CC7
//034A 0997	//smiaRegs_rw_frame_timing_y_addr_end			0997

// Set int.time
{16, 0x0200,0x0618},	// smiaRegs_rw_integration_time_fine_integration_time
{16, 0x0202,0x09A5},	// smiaRegs_rw_integration_time_coarse_integration_time

// Set gain
{16, 0x0204,0x0020},	// X1
////////////////////////////////////////////////
//                                            //
//     End of Parsing Excel File	//
//                                            //
////////////////////////////////////////////////


//CONFIGURATION REGISTERS 

//M2M
{16, 0x31FE,0xC004}, // ash_uDecompressXgrid[0]                        
{16, 0x3200,0xC4F0}, // ash_uDecompressXgrid[1]                        
{16, 0x3202,0xCEC8}, // ash_uDecompressXgrid[2]                        
{16, 0x3204,0xD8A0}, // ash_uDecompressXgrid[3]                        
{16, 0x3206,0xE278}, // ash_uDecompressXgrid[4]                        
{16, 0x3208,0xEC50}, // ash_uDecompressXgrid[5]                        
{16, 0x320A,0xF628}, // ash_uDecompressXgrid[6]                        
{16, 0x320C,0x0000}, // ash_uDecompressXgrid[7]                        
{16, 0x320E,0x09D8}, // ash_uDecompressXgrid[8]                        
{16, 0x3210,0x13B0}, // ash_uDecompressXgrid[9]                        
{16, 0x3212,0x1D88}, // ash_uDecompressXgrid[10]                       
{16, 0x3214,0x2760}, // ash_uDecompressXgrid[11]                       
{16, 0x3216,0x3138}, // ash_uDecompressXgrid[12]                       
{16, 0x3218,0x3B10}, // ash_uDecompressXgrid[13]                       
{16, 0x321A,0x3FFC}, // ash_uDecompressXgrid[14]                       
                           
{16, 0x321C,0xC004}, // ash_uDecompressYgrid[0]     
{16, 0x321E,0xCCD0}, // ash_uDecompressYgrid[1]     
{16, 0x3220,0xD99C}, // ash_uDecompressYgrid[2]     
{16, 0x3222,0xE668}, // ash_uDecompressYgrid[3]     
{16, 0x3224,0xF334}, // ash_uDecompressYgrid[4]     
{16, 0x3226,0x0000}, // ash_uDecompressYgrid[5]     
{16, 0x3228,0x0CCC}, // ash_uDecompressYgrid[6]     
{16, 0x322A,0x1998}, // ash_uDecompressYgrid[7]     
{16, 0x322C,0x2664}, // ash_uDecompressYgrid[8]     
{16, 0x322E,0x3330}, // ash_uDecompressYgrid[9]     
{16, 0x3230,0x3FFC}, // ash_uDecompressYgrid[10]    

{16, 0x3232,0x0100}, // ash_uDecompressWidth  
{16, 0x3234,0x0100}, // ash_uDecompressHeight 

{8,  0x3237,0x00}, // ash_uDecompressScale          // 00 - the value for this register is read from NVM page #0 byte #47 bits [3]-[7] i.e. 5 MSB bits  // other value - e.g. 0E, will be read from this register settings in the set file and ignore the value set in NVM as described above
{8,  0x3238,0x09}, // ash_uDecompressRadiusShifter 
{8,  0x3239,0x09}, // ash_uDecompressParabolaScale 
{8,  0x323A,0x0B}, // ash_uDecompressFinalScale    
{8,  0x3160,0x06},	// ash_GrasCfg  06  // 36  // [5:5]	fegras_gain_clamp   0 _ clamp gain to 0..1023 // _V_// 1 _ clamp_gain to 256..1023// [4:4]	fegras_plus_zero   Adjust final gain by the one or the zero // 0 _ [Output = Input x Gain x Alfa]  // _V_// 1 _ [Output = Input x (1 + Gain x Alfa)]
//BASE Profile parabola start
{8,  0x3161,0x00}, // ash_GrasShifter 00
{16, 0x3164,0x09C4}, // ash_luma_params[0]_tmpr     
{16, 0x3166,0x0100}, // ash_luma_params[0]_alpha[0] 
{16, 0x3168,0x0100}, // ash_luma_params[0]_alpha[1] 
{16, 0x316A,0x0100}, // ash_luma_params[0]_alpha[2] 
{16, 0x316C,0x0100}, // ash_luma_params[0]_alpha[3] 
{16, 0x316E,0x0011}, // ash_luma_params[0]_beta[0]  
{16, 0x3170,0x002F}, // ash_luma_params[0]_beta[1]  
{16, 0x3172,0x0000}, // ash_luma_params[0]_beta[2]  
{16, 0x3174,0x0011}, // ash_luma_params[0]_beta[3]  
{16, 0x3176,0x0A8C}, // ash_luma_params[1]_tmpr     
{16, 0x3178,0x0100}, // ash_luma_params[1]_alpha[0] 
{16, 0x317A,0x0100}, // ash_luma_params[1]_alpha[1] 
{16, 0x317C,0x0100}, // ash_luma_params[1]_alpha[2] 
{16, 0x317E,0x0100}, // ash_luma_params[1]_alpha[3] 
{16, 0x3180,0x0011}, // ash_luma_params[1]_beta[0]  
{16, 0x3182,0x002F}, // ash_luma_params[1]_beta[1]  
{16, 0x3184,0x0000}, // ash_luma_params[1]_beta[2]  
{16, 0x3186,0x0011}, // ash_luma_params[1]_beta[3]  
{16, 0x3188,0x0CE4}, // ash_luma_params[2]_tmpr     
{16, 0x318A,0x0100}, // ash_luma_params[2]_alpha[0] 
{16, 0x318C,0x0100}, // ash_luma_params[2]_alpha[1] 
{16, 0x318E,0x0100}, // ash_luma_params[2]_alpha[2] 
{16, 0x3190,0x0100}, // ash_luma_params[2]_alpha[3] 
{16, 0x3192,0x0011}, // ash_luma_params[2]_beta[0]  
{16, 0x3194,0x002F}, // ash_luma_params[2]_beta[1]  
{16, 0x3196,0x0000}, // ash_luma_params[2]_beta[2]  
{16, 0x3198,0x0011}, // ash_luma_params[2]_beta[3]  
{16, 0x319A,0x1004}, // ash_luma_params[3]_tmpr     
{16, 0x319C,0x0100}, // ash_luma_params[3]_alpha[0] 
{16, 0x319E,0x0100}, // ash_luma_params[3]_alpha[1] 
{16, 0x31A0,0x0100}, // ash_luma_params[3]_alpha[2] 
{16, 0x31A2,0x0100}, // ash_luma_params[3]_alpha[3] 
{16, 0x31A4,0x0011}, // ash_luma_params[3]_beta[0]  
{16, 0x31A6,0x002F}, // ash_luma_params[3]_beta[1]  
{16, 0x31A8,0x0000}, // ash_luma_params[3]_beta[2]  
{16, 0x31AA,0x0011}, // ash_luma_params[3]_beta[3]  
{16, 0x31AC,0x1388}, // ash_luma_params[4]_tmpr     
{16, 0x31AE,0x0100}, // ash_luma_params[4]_alpha[0] 
{16, 0x31B0,0x0100}, // ash_luma_params[4]_alpha[1] 
{16, 0x31B2,0x0100}, // ash_luma_params[4]_alpha[2] 
{16, 0x31B4,0x0100}, // ash_luma_params[4]_alpha[3] 
{16, 0x31B6,0x0011}, // ash_luma_params[4]_beta[0]  
{16, 0x31B8,0x002F}, // ash_luma_params[4]_beta[1]  
{16, 0x31BA,0x0000}, // ash_luma_params[4]_beta[2]  
{16, 0x31BC,0x0011}, // ash_luma_params[4]_beta[3]  
{16, 0x31BE,0x1964}, // ash_luma_params[5]_tmpr     
{16, 0x31C0,0x0100}, // ash_luma_params[5]_alpha[0] 
{16, 0x31C2,0x0100}, // ash_luma_params[5]_alpha[1] 
{16, 0x31C4,0x0100}, // ash_luma_params[5]_alpha[2] 
{16, 0x31C6,0x0100}, // ash_luma_params[5]_alpha[3] 
{16, 0x31C8,0x0011}, // ash_luma_params[5]_beta[0]  
{16, 0x31CA,0x002F}, // ash_luma_params[5]_beta[1]  
{16, 0x31CC,0x0000}, // ash_luma_params[5]_beta[2]  
{16, 0x31CE,0x0011}, // ash_luma_params[5]_beta[3]  
{16, 0x31D0,0x1D4C}, // ash_luma_params[6]_tmpr     
{16, 0x31D2,0x0100}, // ash_luma_params[6]_alpha[0] 
{16, 0x31D4,0x0100}, // ash_luma_params[6]_alpha[1] 
{16, 0x31D6,0x0100}, // ash_luma_params[6]_alpha[2] 
{16, 0x31D8,0x0100}, // ash_luma_params[6]_alpha[3] 
{16, 0x31DA,0x0011}, // ash_luma_params[6]_beta[0]  
{16, 0x31DC,0x002F}, // ash_luma_params[6]_beta[1]  
{16, 0x31DE,0x0000}, // ash_luma_params[6]_beta[2]  
{16, 0x31E0,0x0011}, // ash_luma_params[6]_beta[3]  
{8,  0x3162,0x00},		// ash_bLumaMode 01
//BASE Profile parabola end
{16, 0x301C,0x0100},	// smiaRegs_vendor_gras_nvm_address
{8,  0x301E,0x03},	 	//5 for SRAM test // smiaRegs_vendor_gras_load_from 03
{8,  0x323C,0x00},  	// ash_bSkipNvmGrasOfs 01 // skipping the value set in nvm page 0 address 47
{8,  0x323D,0x01},	 	// ash_uNvmGrasTblOfs 01 // load shading table 1 from nvm
{8,  0x1989,0x04},  	//smiaRegs_ro_edof_cap_uAlphaTempInd 04

{8,  0x0B01,0x00},		// smiaRegs_rw_isp_luminance_correction_level
					// Set shading power value, according to the shading power set at CCKIT and M2M tool when building the base 
					// LSC profile, where 0x80(128dec) stands for 1 (100%), 0x4F stands for 0.62(62%) etc.
					//	When this register is set to 00 in the set file- the value for this register is read from NVM page #0 byte #55 
					// When this register is set to a value other than 00- e.g. 80, the value will be read from this register settings 
					// in the set file and ignore the value set in NVM as described above 

{8,  0x0B00,0x00},	//0x01	// smiaRegs_rw_isp_shading_correction_enable 01

// Streaming on
{8,  0x0100,0x01},	// smiaRegs_rw_general_setup_mode_select
*/
{16, 0x6028,0xD000},
{16, 0x602a,0x0103},
{8, 0x0103,0x01},	// Reset
{16, 0xffff,3			},	//
{8, 0x0103,0x00},	// Reset
{16, 0xffff,3			},	//
// Setting for MIPI CLK (Don't change)
{16, 0x6218,0xF1D0},	// open all clocks
{16, 0x6214,0xF9F0},	// open all clocks
{16, 0x6226,0x0001},	// open APB clock for I2C transaction
{16, 0xB0C0,0x000C},
{16, 0x6226,0x0000},	// close APB clock for I2C transaction
{16, 0x6218,0xF9F0},	// close all clocks
// Offset control
{16, 0x38FA,0x0030},  // gisp_offs_gains_bls_offs_0_
{16, 0x38FC,0x0030},  // gisp_offs_gains_bls_offs_1_
// Sensor XY condination
{16, 0x32CE,0x0060},    // senHal_usWidthStOfsInit
{16, 0x32D0,0x0024},    // senHal_usHeightStOfsInit
//internal correction
{8,  0x0B00,0x00},	//0x01	// smiaRegs_rw_isp_shading_correction_enable 01
{8,  0x0B01,0x80},	//luminance correction
{8,  0x0B04,0x01},	//blc en
{8,  0x0B05,0x01},	//couplet defect pixel correction en

//other
//{8,  0x0101,0x00},    // orientation[1:0]-flip/mirror
//{8,  0x0110,0x00},    // CSI-2
//{8,  0x0111,0x02},    // MIPI
//{16, 0x0112,0x0a0a},    // format-raw10
//{8,  0x0114,0x03},    // 4lane
//{16, 0x0136,0x2100},    // EXTMCLK 24MHz*256
{16, 0xffff,10},
};

//for capture
static struct regval_list sensor_quxga_regs[] = { //quxga
{8,  0x0100,0x00},	//stream off
{8,  0x0104,0x01},	//group on
//====================================================================
//                     |-->dbr_clk_div================>charge_pump_clk
// 6~54    3~6   0.5~1G|              |-->gray_div====>adc_clk
//mclk->pre_div->multi---->vt_sys_div---->vt_pix_div==>sys_clk
//                     |-->op_sys_div---->op_pix_div==>op_clk
//                                    |===============>serial_clk
//====================================================================
// set PLL
{16, 0x0304,0x0006},  //pre_pll_clk_div
{16, 0x0306,0x008a},  //pll_multiplier 15/20/30fps-0x8a/0xb8/0x
{16, 0x0302,0x0001},  //vt_sys_clk_div
{16, 0x0300,0x0004},  //vt_pix_clk_div

{16, 0x030C,0x0006},  //2nd_pre_pll_clk_div
{16, 0x030E,0x00b0},	//2nd_pll_multiplier
{16, 0x030A,0x0001},  //op_sys_clk_div
{16, 0x0308,0x0008},  //op_pix_clk_div
//set output size
{16, 0x0342,0x0E80},	// frame_timing_line_length_pck 
{16, 0x0340,0x09ae},	// frame_timing_frame_length_lines
{16, 0x0344,0x0004},	//x_addr_start
{16, 0x0346,0x0004},  //y_addr_start
{16, 0x0348,0x0CC3},	//x_addr_end	
{16, 0x034A,0x0993},	//y_addr_end	
{16, 0x034C,0x0CC0},	//x_output_size
{16, 0x034E,0x0990},	//y_output_size
//binningcfg
{8, 0x0900,0x00},	//0-dis;1-en
{8, 0x0901,0x11},	//[7:4]/[3:0]-column/row factor
{8, 0x0902,0x01},	//[3:0]:avg/sum(lowlight)/bayer_corrected/userdef
//set subsample
{16, 0x0380,0x0001},	//x_even_inc
{16, 0x0382,0x0001},	//x_odd_inc
{16, 0x0384,0x0001},	//y_even_inc
{16, 0x0386,0x0001},	//y_odd_inc
//set subsample and scaling factor
{16, 0x0400,0x0000},	//0-no;1-h scaling;2-h/v scaling
{16, 0x0404,0x0010},	//ds factor 1/1.5/2/3/4-->16/24/32/48/64
//digital crop
{16, 0x0408,0x0000},	//x offset
{16, 0x040a,0x0000},	//y offset
////fifo cfg
//{16, 0x0700,0x0000},	//
//MIPI
{8,  0x0114,0x03},    // 4Lane
//DPHY
{16, 0x0800,0x0000},	//
// Set int.time
{16, 0x0200,0x0618},	// fine_integration_time
{16, 0x0202,0x09A5},	// coarse_integration_time
// Set gain
{16, 0x0204,0x0020},	// X1
//=================================
{8,  0x0104,0x00},	//group off
{16, 0xffff,10},	// delay
{8,  0x0100,0x01},	//stream on
};

//for video
static struct regval_list sensor_1080p_regs[] = { //1080p: 1920*1080@15fps //84MHz pclk
{8,  0x0100,0x00},	//stream off
{8,  0x0104,0x01},	//group on
//====================================================================
//                     |-->dbr_clk_div================>charge_pump_clk
// 6~54    3~6   0.5~1G|              |-->gray_div====>adc_clk
//mclk->pre_div->multi---->vt_sys_div---->vt_pix_div==>sys_clk
//                     |-->op_sys_div---->op_pix_div==>op_clk
//                                    |===============>serial_clk
//====================================================================
// set PLL
{16, 0x0304,0x0006},  //pre_pll_clk_div
{16, 0x0306,0x00b4},  //pll_multiplier
{16, 0x0302,0x0001},  //vt_sys_clk_div
{16, 0x0300,0x0004},  //vt_pix_clk_div

{16, 0x030C,0x0006},  //2nd_pre_pll_clk_div
{16, 0x030E,0x00b0},	//2nd_pll_multiplier
{16, 0x030A,0x0001},  //op_sys_clk_div
{16, 0x0308,0x0008},  //op_pix_clk_div
//set output size
{16, 0x0342,0x0E80},	// frame_timing_line_length_pck 
{16, 0x0340,0x09c2},	// frame_timing_frame_length_lines
{16, 0x0344,0x0004},	//x_addr_start
{16, 0x0346,0x0004},  //y_addr_start
{16, 0x0348,0x0CC3},	//x_addr_end	
{16, 0x034A,0x0993},	//y_addr_end	
{16, 0x034C,0x0CC0},	//x_output_size
{16, 0x034E,0x0990},	//y_output_size
//binningcfg
{8, 0x0900,0x00},	//0-dis;1-en
{8, 0x0901,0x11},	//[7:4]/[3:0]-column/row factor
{8, 0x0902,0x01},	//[3:0]:avg/sum(lowlight)/bayer_corrected/userdef
//set subsample
{16, 0x0380,0x0001},	//x_even_inc
{16, 0x0382,0x0001},	//x_odd_inc
{16, 0x0384,0x0001},	//y_even_inc
{16, 0x0386,0x0001},	//y_odd_inc
//set subsample and scaling factor
{16, 0x0400,0x0000},	//0-no;1-h scaling;2-h/v scaling
{16, 0x0404,0x001b},	//ds factor 1/1.5/2/3/4-->16/24/32/48/64
//digital crop
{16, 0x0408,0x0000},	//x offset
{16, 0x040a,0x0000},	//y offset
////fifo cfg
//{16, 0x0700,0x0000},	//
//MIPI
{8,  0x0114,0x03},    // 4Lane
//DPHY
{16, 0x0800,0x0000},	//
// Set int.time
{16, 0x0200,0x0618},	// fine_integration_time
{16, 0x0202,0x09A5},	// coarse_integration_time
// Set gain
{16, 0x0204,0x0020},	// X1
//=================================
{8,  0x0104,0x00},	//group off
{16, 0xffff,10},	// delay
{8,  0x0100,0x01},	//stream on
};

static struct regval_list sensor_720p_regs[] = { //720: 1280*720@30fps //84MHz pclk

};

//static struct regval_list sensor_svga_regs[] = { //SVGA: 800*600
//
//  //{REG_TERM,VAL_TERM},
//};

//static struct regval_list sensor_vga_regs[] = { //VGA:  640*480
//  
//  //{REG_TERM,VAL_TERM},
//};

//misc
static struct regval_list sensor_oe_disable_regs[] = {

};

static struct regval_list sensor_oe_enable_regs[] = {

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
 * Low-level register I/O.
 *
 */


/*
 * On most platforms, we'd rather do straight i2c I/O.
 */
//!!!!modified type for each device, be careful of the para type!!!
static int sensor_read_byte(struct v4l2_subdev *sd, unsigned short reg,
    unsigned char *value)
{
	int ret=0;
	int cnt=0;
	
 // struct i2c_client *client = v4l2_get_subdevdata(sd);
  ret = cci_read_a16_d8(sd,reg,value);
  while(ret!=0&&cnt<2)
  {
  	ret = cci_read_a16_d8(sd,reg,value);
  	cnt++;
  }
  if(cnt>0)
  	vfe_dev_dbg("sensor read retry=%d\n",cnt);
  
  return ret;
}
static int sensor_read(struct v4l2_subdev *sd, unsigned short reg,
    unsigned short *value)
{
	int ret=0;
	int cnt=0;
	
 // struct i2c_client *client = v4l2_get_subdevdata(sd);
  ret = cci_read_a16_d16(sd,reg,value);
  while(ret!=0&&cnt<2)
  {
  	ret = cci_read_a16_d16(sd,reg,value);
  	cnt++;
  }
  if(cnt>0)
  	vfe_dev_dbg("sensor read retry=%d\n",cnt);
  
  return ret;
}

static int sensor_write_byte(struct v4l2_subdev *sd, unsigned short reg,
    unsigned char value)
{
	int ret=0;
	int cnt=0;
	
 // struct i2c_client *client = v4l2_get_subdevdata(sd);
  
  ret = cci_write_a16_d8(sd,reg,value);
  while(ret!=0&&cnt<2)
  {
  	ret = cci_write_a16_d8(sd,reg,value);
  	cnt++;
  }
  if(cnt>0)
  	vfe_dev_dbg("sensor write retry=%d\n",cnt);
  
  return ret;
}

static int sensor_write(struct v4l2_subdev *sd, unsigned short reg,
    unsigned short value)
{
	int ret=0;
	int cnt=0;
	
  //struct i2c_client *client = v4l2_get_subdevdata(sd);
  
  ret = cci_write_a16_d16(sd,reg,value);
  while(ret!=0&&cnt<2)
  {
  	ret = cci_write_a16_d16(sd,reg,value);
  	cnt++;
  }
  if(cnt>0)
  	vfe_dev_dbg("sensor write retry=%d\n",cnt);
  
  return ret;
}
static int print_reg(struct v4l2_subdev *sd, unsigned short addr)
{return 0;
  int ret=0;
  unsigned char tmp=0xff;
  sensor_read_byte(sd, addr, &tmp);
  printk("0x%x=0x%x\n", addr, tmp);
  return ret;
}

/*
 * Write a list of register settings;
 */
static int sensor_write_array(struct v4l2_subdev *sd, struct regval_list *regs, int array_size)
{
	int i=0;
	
  if(!regs)
  	return 0;
  	//return -EINVAL;
  
  while(i<array_size)
  {
    if(regs->addr == REG_DLY) {
      msleep(regs->data);
    } 
    else {  
    	//printk("write %d bit 0x%x=0x%x\n",regs->width, regs->addr, regs->data);
    	if(regs->width==16)
        LOG_ERR_RET(sensor_write(sd, regs->addr, regs->data))
      else if(regs->width==8)
        LOG_ERR_RET(sensor_write_byte(sd, regs->addr, regs->data))
    }
    i++;
    regs++;
  }
  return 0;
}

/* 
 * Code for dealing with controls.
 * fill with different sensor module
 * different sensor module has different settings here
 * if not support the follow function ,retrun -EINVAL
 */

/* *********************************************begin of ******************************************** */
/*
static int sensor_g_hflip(struct v4l2_subdev *sd, __s32 *value)
{
  struct sensor_info *info = to_state(sd);
  unsigned char rdval;
    
  LOG_ERR_RET(sensor_read(sd, 0x3821, &rdval))
  
  rdval &= (1<<1);
  rdval >>= 1;
    
  *value = rdval;

  info->hflip = *value;
  return 0;
}

static int sensor_s_hflip(struct v4l2_subdev *sd, int value)
{
  struct sensor_info *info = to_state(sd);
  unsigned char rdval;
  
  if(info->hflip == value)
    return 0;
    
  LOG_ERR_RET(sensor_read(sd, 0x3821, &rdval))
  
  switch (value) {
    case 0:
      rdval &= 0xf9;
      break;
    case 1:
      rdval |= 0x06;
      break;
    default:
      return -EINVAL;
  }
  
  LOG_ERR_RET(sensor_write(sd, 0x3821, rdval))
  
  mdelay(10);
  info->hflip = value;
  return 0;
}

static int sensor_g_vflip(struct v4l2_subdev *sd, __s32 *value)
{
  struct sensor_info *info = to_state(sd);
  unsigned char rdval;
  
  LOG_ERR_RET(sensor_read(sd, 0x3820, &rdval))
  
  rdval &= (1<<1);  
  *value = rdval;
  rdval >>= 1;
  
  info->vflip = *value;
  return 0;
}

static int sensor_s_vflip(struct v4l2_subdev *sd, int value)
{
  struct sensor_info *info = to_state(sd);
  unsigned char rdval;
  
  if(info->vflip == value)
    return 0;
  
  LOG_ERR_RET(sensor_read(sd, 0x3820, &rdval))

  switch (value) {
    case 0:
      rdval &= 0xf9;
      break;
    case 1:
      rdval |= 0x06;
      break;
    default:
      return -EINVAL;
  }

  LOG_ERR_RET(sensor_write(sd, 0x3820, rdval))
  
  mdelay(10);
  info->vflip = value;
  return 0;
}
*/
static int sensor_g_exp(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);
	
	*value = info->exp;
	vfe_dev_dbg("sensor_get_exposure = %d\n", info->exp);
	return 0;
}

static int sensor_s_exp_gain(struct v4l2_subdev *sd, struct sensor_exp_gain *exp_gain)
{//return -1;
  int exp_val, gain_val,frame_length,shutter;
  unsigned char explow=0,expmid=0,exphigh=0;
  unsigned char gainlow=0,gainhigh=0;  
  struct sensor_info *info = to_state(sd);

  exp_val = exp_gain->exp_val;
  gain_val = exp_gain->gain_val;
  
  if(exp_val>0xfffff)
	  exp_val=0xfffff;
  
  gainlow=(unsigned char)((gain_val<<3)&0xff);
  gainhigh=(unsigned char)((gain_val>>5));
  
  exphigh	= (unsigned char) ( (0x0f0000&exp_val)>>16);
  expmid	= (unsigned char) ( (0x00ff00&exp_val)>>8);
  explow	= (unsigned char) ( (0x0000ff&exp_val)	 );
  shutter = exp_val/16;  
  if(shutter  > s5k3h7_sensor_vts- 4)
        frame_length = shutter + 4;
  else
        frame_length = s5k3h7_sensor_vts;
  
  sensor_write(sd, 0x3208, 0x00);//enter group write
  
  sensor_write(sd, 0x3503, 0x03);
  
  sensor_write(sd, 0x380f, (frame_length & 0xff));
  sensor_write(sd, 0x380e, (frame_length >> 8));
  
  sensor_write(sd, 0x3509, gainlow);
  sensor_write(sd, 0x3508, gainhigh);
  
  sensor_write(sd, 0x3502, explow);
  sensor_write(sd, 0x3501, expmid);
  sensor_write(sd, 0x3500, exphigh);	
  sensor_write(sd, 0x3208, 0x10);//end group write
  sensor_write(sd, 0x3208, 0xa0);//init group write
  
  printk("exp_val = %d,gain_val = %d 0x%x 0x%x\t%d\n",exp_val,gain_val,gainhigh,gainlow, frame_length);
  
  info->exp = exp_val;
  info->gain = gain_val;
  return 0;
}

static int sensor_s_exp(struct v4l2_subdev *sd, unsigned int exp_val)
{
	unsigned int exp_coarse;
	unsigned int exp_fine;
	struct sensor_info *info = to_state(sd);

//	vfe_dev_dbg("sensor_set_exposure = %d\n", exp_val);
	if(exp_val>0xffffff)
		exp_val=0xfffff0;
	if(exp_val<16)
		exp_val=16;
	
	if(info->exp == exp_val)
		return 0;
  
	exp_coarse=exp_val>>4;//rounding to 1
	exp_fine=(exp_val-exp_coarse*16)*info->current_wins->hts/16;
	
//	vfe_dev_dbg("sensor_set_exposure real= %d\n", exp_val);
	//sensor_write(sd, 0x0104, 0x01);	
	sensor_write(sd, 0x0200, (unsigned short)exp_fine);//fine integration time
	sensor_write(sd, 0x0202, (unsigned short)exp_coarse);//coarse integration time
	//sensor_write(sd, 0x0104, 0x00);	
	
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
	unsigned short gain;
	
	if(info->gain == gain_val)
		return 0;
	
	gain=gain_val*2;//shift to 1/32 step
	
	LOG_ERR_RET(sensor_write(sd, 0x0204, gain))
	
	//printk("s5k3h7 sensor_set_gain = %d, Done!\n", gain_val);
	info->gain = gain_val;
	
	return 0;
}

static int sensor_s_sw_stby(struct v4l2_subdev *sd, int on_off)
{
	int ret;
	unsigned char rdval;
	
	ret=sensor_read_byte(sd, 0x0100, &rdval);
	if(ret!=0)
		return ret;
	
	if(on_off==CSI_GPIO_LOW)//sw stby on
	{
		ret=sensor_write_byte(sd, 0x0100, rdval&0xfe);
	}
	else//sw stby off
	{
		ret=sensor_write_byte(sd, 0x0100, rdval|0x01);
	}
	return ret;
}

/*
 * Stuff that knows about the sensor.
 */
 
static int sensor_power(struct v4l2_subdev *sd, int on)
{
  int ret;
  
  //insure that clk_disable() and clk_enable() are called in pair 
  //when calling CSI_SUBDEV_STBY_ON/OFF and CSI_SUBDEV_PWR_ON/OFF
  ret = 0;
  switch(on)
  {
    case CSI_SUBDEV_STBY_ON:
      vfe_dev_dbg("CSI_SUBDEV_STBY_ON!\n");
//      //disable io oe
//      vfe_dev_print("disalbe oe!\n");
//      ret = sensor_write_array(sd, sensor_oe_disable_regs, ARRAY_SIZE(sensor_oe_disable_regs));
//      if(ret < 0)
//        vfe_dev_err("disalbe oe falied!\n");
      //software standby on
      ret = sensor_s_sw_stby(sd, CSI_GPIO_LOW);
      if(ret < 0)
        vfe_dev_err("soft stby falied!\n");
      mdelay(10);
      //make sure that no device can access i2c bus during sensor initial or power down
      //when using i2c_lock_adpater function, the following codes must not access i2c bus before calling i2c_unlock_adapter
      cci_lock(sd);    
      //standby on io
      vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
      //remember to unlock i2c adapter, so the device can access the i2c bus again
      cci_unlock(sd);    

      //inactive mclk after stadby in
      vfe_set_mclk(sd,OFF);
      break;
    case CSI_SUBDEV_STBY_OFF:
      vfe_dev_dbg("CSI_SUBDEV_STBY_OFF!\n");
      //make sure that no device can access i2c bus during sensor initial or power down
      //when using i2c_lock_adpater function, the following codes must not access i2c bus before calling i2c_unlock_adapter
      cci_lock(sd);    

      //active mclk before stadby out
      vfe_set_mclk_freq(sd,MCLK);
      vfe_set_mclk(sd,ON);
      mdelay(10);
      //standby off io
      vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
      mdelay(10);
      //remember to unlock i2c adapter, so the device can access the i2c bus again
      cci_unlock(sd);    
      
//      //software standby
      ret = sensor_s_sw_stby(sd, CSI_GPIO_HIGH);
      if(ret < 0)
        vfe_dev_err("soft stby off falied!\n");
      mdelay(10);
//      vfe_dev_print("enable oe!\n");
//      ret = sensor_write_array(sd, sensor_oe_enable_regs);
//      if(ret < 0)
//        vfe_dev_err("enable oe falied!\n");
      break;
    case CSI_SUBDEV_PWR_ON:
      vfe_dev_dbg("CSI_SUBDEV_PWR_ON!\n");
      //make sure that no device can access i2c bus during sensor initial or power down
      //when using i2c_lock_adpater function, the following codes must not access i2c bus before calling i2c_unlock_adapter
      cci_lock(sd);    
      //power on reset
      vfe_gpio_set_status(sd,PWDN,1);//set the gpio to output
      vfe_gpio_set_status(sd,RESET,1);//set the gpio to output
      //power down io
      vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
      //reset on io
      vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
      mdelay(1);
      //active mclk before power on
      vfe_set_mclk_freq(sd,MCLK);
      vfe_set_mclk(sd,ON);
      mdelay(10);
      //power supply
      vfe_gpio_write(sd,POWER_EN,CSI_GPIO_HIGH);
      vfe_set_pmu_channel(sd,IOVDD,ON);
      vfe_set_pmu_channel(sd,AVDD,ON);
      vfe_set_pmu_channel(sd,DVDD,ON);
      vfe_set_pmu_channel(sd,AFVDD,ON);
      //standby off io
      vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
      mdelay(10);
      //reset after power on
      vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);
      mdelay(30);
      //remember to unlock i2c adapter, so the device can access the i2c bus again
      cci_unlock(sd);    

      break;
    case CSI_SUBDEV_PWR_OFF:
      vfe_dev_dbg("CSI_SUBDEV_PWR_OFF!\n");
      //make sure that no device can access i2c bus during sensor initial or power down
      //when using i2c_lock_adpater function, the following codes must not access i2c bus before calling i2c_unlock_adapter
      cci_lock(sd);   
      //inactive mclk before power off
      vfe_set_mclk(sd,OFF);
      //power supply off
      vfe_gpio_write(sd,POWER_EN,CSI_GPIO_LOW);
      vfe_set_pmu_channel(sd,AFVDD,OFF);
      vfe_set_pmu_channel(sd,DVDD,OFF);
      vfe_set_pmu_channel(sd,AVDD,OFF);
      vfe_set_pmu_channel(sd,IOVDD,OFF);  
      //standby and reset io
      mdelay(10);
      vfe_gpio_write(sd,POWER_EN,CSI_GPIO_HIGH);
      vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
      //set the io to hi-z
      vfe_gpio_set_status(sd,RESET,0);//set the gpio to input
      vfe_gpio_set_status(sd,PWDN,0);//set the gpio to input
      //remember to unlock i2c adapter, so the device can access the i2c bus again
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
      mdelay(10);
      break;
    case 1:
      vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
      mdelay(10);
      break;
    default:
      return -EINVAL;
  }
    
  return 0;
}

static int sensor_detect(struct v4l2_subdev *sd)
{
  unsigned short rdval;
  
  LOG_ERR_RET(sensor_read(sd, 0x0000, &rdval))
  if(rdval != 0x3087)
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
//  vfe_dev_dbg("[]cmd=%d\n",cmd);
//  vfe_dev_dbg("[]arg=%0x\n",arg);
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
      ret=0;
//      if((unsigned int *)arg==1)
//        ret=sensor_write(sd, 0x3036, 0x78);
//      else
//        ret=sensor_write(sd, 0x3036, 0x32);
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
		//.mbus_code	= V4L2_MBUS_FMT_SBGGR10_10X1,
		//.mbus_code	= V4L2_MBUS_FMT_SGBRG10_10X1,
		.mbus_code	= V4L2_MBUS_FMT_SGRBG10_10X1,
		//.mbus_code	= V4L2_MBUS_FMT_SRGGB10_10X1,
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
	  /* quxga: 3264*2448 */
	  {
      .width      = QUXGA_WIDTH,
      .height     = QUXGA_HEIGHT,
      .hoffset    = 0,
      .voffset    = 0,
      .hts        = 3712,//must over x, limited by sensor
      .vts        = 2478,
      .pclk       = 184*1000*1000,
      .mipi_bps		= 400*1000*1000,
      .fps_fixed  = 1,
      .bin_factor = 1,
      .intg_min   = 1<<4,
      .intg_max   = 2484<<4,
      .gain_min   = 1<<4,
      .gain_max   = 16<<4,
      .regs       = sensor_quxga_regs,
      .regs_size  = ARRAY_SIZE(sensor_quxga_regs),
      .set_size   = NULL,
    },
    
    /* 1080P */
    {
      .width			= HD1080_WIDTH,
      .height 		= HD1080_HEIGHT,
      .hoffset	  = 0,
      .voffset	  = 0,
//      .hoffset    = (2608-1920)/3,
//      .voffset    = (1960-1080)/3,
      .hts        = 2800,//
      .vts        = 2000,
      .pclk       = 200*1000*1000,
      .mipi_bps		= 600*1000*1000,
      .fps_fixed  = 1,
      .bin_factor = 1,
      .intg_min   = 3<<4,
      .intg_max   = (2000-8)<<4,
      .gain_min   = 1<<4,
      .gain_max   = 8<<4,
      .regs       = sensor_1080p_regs,//sensor_qsxga_regs
      .regs_size  = ARRAY_SIZE(sensor_1080p_regs),//sensor_1080p_regs
      .set_size		= NULL,
    },
	/* UXGA */
//	{
//      .width			= UXGA_WIDTH,
//      .height 		= UXGA_HEIGHT,
//      .hoffset	  = 0,
//      .voffset	  = 0,
//      .hts        = 2800,//limited by sensor
//      .vts        = 1000,
//      .pclk       = 84*1000*1000,
//      .fps_fixed  = 1,
//      .bin_factor = 1,
//      .intg_min   = ,
//      .intg_max   = ,
//      .gain_min   = ,
//      .gain_max   = ,
//      .regs			= sensor_uxga_regs,
//      .regs_size	= ARRAY_SIZE(sensor_uxga_regs),
//      .set_size		= NULL,
//	},
    /* 720p */
    {
      .width      = HD720_WIDTH,
      .height     = HD720_HEIGHT,
      .hoffset    = 0,
      .voffset    = 0,
      .hts        = 2800,//
      .vts        = 1000,
      .pclk       = 84*1000*1000,
      .mipi_bps		= 200*1000*1000,
      .fps_fixed  = 1,
      .bin_factor = 1,
      .intg_min   = 3<<4,
      .intg_max   = (1000-8)<<4,
      .gain_min   = 1<<4,
      .gain_max   = 16<<4,
      .regs			  = sensor_720p_regs,//
      .regs_size	= ARRAY_SIZE(sensor_720p_regs),//
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
  for (wsize = sensor_win_sizes; wsize < sensor_win_sizes + N_WIN_SIZES;
       wsize++)
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
  //pix->bytesperline = pix->width*sensor_formats[index].bpp;
  //pix->sizeimage = pix->height*pix->bytesperline;

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
  
  sensor_write_array(sd, sensor_oe_disable_regs, ARRAY_SIZE(sensor_oe_disable_regs));
  
  LOG_ERR_RET(sensor_write_array(sd, sensor_default_regs, ARRAY_SIZE(sensor_default_regs)) );
  
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
  s5k3h7_sensor_vts = wsize->vts;

  vfe_dev_print("s_fmt set width = %d, height = %d\n",wsize->width,wsize->height);

  if(info->capture_mode == V4L2_MODE_VIDEO)
  {
    //video
   
  } else {
    //capture image

  }
	
	sensor_write_array(sd, sensor_oe_enable_regs, ARRAY_SIZE(sensor_oe_enable_regs));
	
	if(1)
	{
	  unsigned int i=0;
	  for(i=0;i<0x40;i+=2)
	  print_reg(sd,0x300+i);
	}
	
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
  //struct v4l2_fract *tpf = &cp->timeperframe;
  struct sensor_info *info = to_state(sd);
  //unsigned char div;
  
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
		return v4l2_ctrl_query_fill(qc, 1*16, 16*16, 1, 16);
	case V4L2_CID_EXPOSURE:
		return v4l2_ctrl_query_fill(qc, 3*16, 65535*16, 1, 3*16);
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
};

static int sensor_probe(struct i2c_client *client,
      const struct i2c_device_id *id)
{
  struct v4l2_subdev *sd;
  struct sensor_info *info;
//  int ret;

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

