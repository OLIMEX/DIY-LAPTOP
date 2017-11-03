/*
 * A V4L2 driver for ov4689 cameras.
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
MODULE_DESCRIPTION("A low-level driver for ov4689 sensors");
MODULE_LICENSE("GPL");

//for internel driver debug
#define DEV_DBG_EN      1 
#if(DEV_DBG_EN == 1)    
#define vfe_dev_dbg(x,arg...) printk("[ov4689]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...) 
#endif
#define vfe_dev_err(x,arg...) printk("[ov4689]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[ov4689]"x,##arg)

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
#define V4L2_IDENT_SENSOR 0x4689


/*
 * Our nominal (default) frame rate.
 */

#define SENSOR_FRAME_RATE 30


/*
 * The ov4689 sits on i2c with ID 0x42
 */
#define I2C_ADDR 0x42
#define  SENSOR_NAME "ov4689"
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
	{0x0103, 0x01},// ; software reset
	{REG_DLY,0x05},
	{0x3638, 0x00},// ; ADC & analog 
	{0x0300, 0x00},// ; PLL1 prediv 
	{0x0302, 0x1c},// ; PLL1 divm 
	{0x0304, 0x03},// ; PLL1 div mipi 
	{0x030b, 0x00},// ; PLL2 pre div 
	{0x030d, 0x1e},// ; PLL2 multiplier 
	{0x030e, 0x04},// ; PLL2 divs 
	{0x030f, 0x01},// ; PLL2 divsp 
	{0x0312, 0x01},// ; PLL2 divdac 
	{0x031e, 0x00},// ; Debug mode 
	{0x3000, 0x20},// ; FSIN output 
	{0x3002, 0x00},// ; Vsync input, HREF input, FREX input, GPIO0 input 
	{0x3018, 0x32},// ; MIPI 2 lane, Reset MIPI PHY when sleep 
	{0x3019, 0x0c},//
	{0x3020, 0x93},// ; Clock switch to PLL clock, Debug mode 
	{0x3021, 0x03},// ; Sleep latch, software standby at line blank 
	{0x3022, 0x01},// ; LVDS disable, Enable power down MIPI when sleep 
	{0x3031, 0x0a},// ; MIPI 10-bit mode 
	{0x303f, 0x0c},
	{0x3305, 0xf1},// ; ASRAM 
	{0x3307, 0x04},// ; ASRAM 
	{0x3309, 0x29},// ; ASRAM 
	{0x3500, 0x00},// ; Long exposure HH 
	{0x3501, 0x60},// ; Long exposure H 
	{0x3502, 0x00},// ; Long exposure L 
	{0x3503, 0x04},// ; Gain delay 1 frame, use sensor gain, exposure delay 1 frame 
	{0x3504, 0x00},// ; debug mode 
	{0x3505, 0x00},// ; debug mode 
	{0x3506, 0x00},// ; debug mode 
	{0x3507, 0x00},// ; Long gain HH 
	{0x3508, 0x00},// ; Long gain H 
	{0x3509, 0x80},// ; Long gain L 
	{0x350a, 0x00},// ; Middle exposure HH 
	{0x350b, 0x00},// ; Middle exposure H 
	{0x350c, 0x00},// ; Middle exposure L 
	{0x350d, 0x00},// ; Middle gain HH 
	{0x350e, 0x00},// ; Middle gain H 
	{0x350f, 0x80},// ; Middle gain L 
	{0x3510, 0x00},// ; Short exposure HH 
	{0x3511, 0x00},// ; Short exposure H 
	{0x3512, 0x00},// ; Short exposure L 
	{0x3513, 0x00},// ; Short gain HH 
	{0x3514, 0x00},// ; Short gain H 
	{0x3515, 0x80},// ; Short gain L 
	{0x3516, 0x00},// ; 4th exposure HH 
	{0x3517, 0x00},// ; 4th exposure H 
	{0x3518, 0x00},// ; 4th exposure L 
	{0x3519, 0x00},// ; 4th gain HH 
	{0x351a, 0x00},// ; 4th gain H 
	{0x351b, 0x80},// ; 4th gian L 
	{0x351c, 0x00},// ; 5th exposure HH 
	{0x351d, 0x00},// ; 5th exposure H 
	{0x351e, 0x00},// ; 5th exposure L 
	{0x351f, 0x00 },//; 5th gain HH 
	{0x3520, 0x00 },//; 5th gain H 
	{0x3521, 0x80 },//; 5th gain L 
	{0x3522, 0x08 },//; Middle digital fraction gain H 
	{0x3524, 0x08 },//; Short digital fraction gain H 
	{0x3526, 0x08 },//; 4th digital fraction gain H 
	{0x3528, 0x08 },//; 5th digital framction gain H 
	{0x352a, 0x08 },//; Long digital fraction gain H 
	{0x3602, 0x00 },//; ADC & analog 
	{0x3604, 0x02 },//; 
	{0x3605, 0x00 },//; 
	{0x3606, 0x00 },//; 
	{0x3607, 0x00 },//; 
	{0x3609, 0x12 },//; 
	{0x360a, 0x40 },//; 
	{0x360c, 0x08 },//; 
	{0x360f, 0xe5 },//; 
	{0x3608, 0x8f },//; 
	{0x3611, 0x00 },//; 
	{0x3613, 0xf7 },//; 
	{0x3616, 0x58 },//; 
	{0x3619, 0x99 },//; 
	{0x361b, 0x60 },//; 
	{0x361c, 0x7a },//; 
	{0x361e, 0x79 },//; 
	{0x361f, 0x02 },//; 
	{0x3632, 0x00 },//; 
	{0x3633, 0x10 },//; 
	{0x3634, 0x10 },//; 
	{0x3635, 0x10 },//; 
	{0x3636, 0x15 },//; 
	{0x3646, 0x86 },//; 
	{0x364a, 0x0b },//; ADC & analog 
	{0x3700, 0x17 },//; Sensor control 
	{0x3701, 0x22 },//; 
	{0x3703, 0x10 },//; 
	{0x370a, 0x37 },//; 
	{0x3705, 0x00 },//; 
	{0x3706, 0x63 },//; 
	{0x3709, 0x3c },//; 
	{0x370b, 0x01 },//; 
	{0x370c, 0x30 },//; 
	{0x3710, 0x24 },//; 
	{0x3711, 0x0c },//; 
	{0x3716, 0x00 },//; 
	{0x3720, 0x28 },//; 
	{0x3729, 0x7b },//; 
	{0x372a, 0x84 },//; 
	{0x372b, 0xbd },//; 
	{0x372c, 0xbc },//; 
	{0x372e, 0x52 },//; 
	{0x373c, 0x0e },//; 
	{0x373e, 0x33 },//; 
	{0x3743, 0x10 },//; 
	{0x3744, 0x88 },//; 
	{0x3745, 0xc0 },//  important!!!
	{0x374a, 0x43 },//; 
	{0x374c, 0x00 },//; 
	{0x374e, 0x23 },//; 
	{0x3751, 0x7b },//; 
	{0x3752, 0x84 },//; 
	{0x3753, 0xbd },//; 
	{0x3754, 0xbc },//; 
	{0x3756, 0x52 },//; 
	{0x375c, 0x00 },//; 
	{0x3760, 0x00 },//; 
	{0x3761, 0x00 },//; 
	{0x3762, 0x00 },//; 
	{0x3763, 0x00 },//; 
	{0x3764, 0x00 },//; 
	{0x3767, 0x04 },//; 
	{0x3768, 0x04 },//; 
	{0x3769, 0x08 },//; 
	{0x376a, 0x08 },//; 
	{0x376b, 0x20 },//; 
	{0x376c, 0x00 },//; 
	{0x376d, 0x00 },//; 
	{0x376e, 0x00 },//; 
	{0x3773, 0x00 },//; 
	{0x3774, 0x51 },//; 
	{0x3776, 0xbd },//; 
	{0x3777, 0xbd },//; 
	{0x3781, 0x18 },//; 
	{0x3783, 0x25 },//; Sensor control 
	{0x3798, 0x1b },// 
	{0x3800, 0x00 },//; H crop start H 
	{0x3801, 0x08 },//; H crop start L 
	{0x3802, 0x00 },//; V crop start H 
	{0x3803, 0x04 },//; V crop start L 
	{0x3804, 0x0a },//; H crop end H 
	{0x3805, 0x97 },//; H crop end L 
	{0x3806, 0x05 },//; V crop end H 
	{0x3807, 0xfb },//; V crop end L 
	{0x3808, 0x0a },//; H output size H 
	{0x3809, 0x80 },//; H output size L 
	{0x380a, 0x05 },//; V output size H 
	{0x380b, 0xf0 },//; V output size L 
	{0x380c, 0x0a },//; HTS H 
	{0x380d, 0x10 },//; HTS L 
	{0x380e, 0x06 },//; VTS H 
	{0x380f, 0x12 },//; VTS L 
	{0x3810, 0x00 },//; H win off H 
	{0x3811, 0x08 },//; H win off L 
	{0x3812, 0x00 },//; V win off H 
	{0x3813, 0x04 },//; V win off L 
	{0x3814, 0x01 },//; H inc odd 
	{0x3815, 0x01 },//; H inc even 
	{0x3819, 0x01 },//; Vsync end L 
	{0x3820, 0x00 },//; flip off, bin off 
	{0x3821, 0x06 },//; mirror on, bin off 
	{0x3829, 0x00 },//; HDR lite off 
	{0x382a, 0x01 },//; vertical subsample odd increase number 
	{0x382b, 0x01 },//; vertical subsample even increase number 
	{0x382d, 0x7f },//; black column end address 
	{0x3830, 0x04 },//; blc use num/2 
	{0x3836, 0x01 },//; r zline use num/2 
	{0x3837, 0x00},//
	{0x3841, 0x02 },//; r_rcnt_fix on 
	{0x3846, 0x08 },//; fcnt_trig_rst_en on 
	{0x3847, 0x07 },//; debug mode 
	{0x3d85, 0x36 },//; OTP bist enable, OTP BIST compare with zero, OTP power up load data on, OTP power up load setting on, OTP write register load setting off 
	{0x3d8c, 0x71 },//; OTP start address H 
	{0x3d8d, 0xcb },//; OTP start address L 
	{0x3f0a, 0x00 },//; PSRAM 
	{0x4000, 0x71 },//; out of range trig off, format chg on, gain chg on, exp chg on, manual trig off, no freeze, always trig off, debug mode
	{0x4001, 0x40 },//; debug mode 
	{0x4002, 0x04 },//; debug mode 
	{0x4003, 0x14 },//; black line number 
	{0x400e, 0x00 },//; offset for BLC bypass 
	{0x4011, 0x00 },//; offset man same off, offset man off, black line output off, 
	{0x401a, 0x00 },//; debug mode 
	{0x401b, 0x00 },//; debug mode 
	{0x401c, 0x00 },//; debug mode 
	{0x401d, 0x00 },//; debug mode 
	{0x401f, 0x00 },//; debug mode 
	{0x4020, 0x00 },//; Anchor left start H 
	{0x4021, 0x10 },//; Anchor left start L 
	{0x4022, 0x07 },//; Anchor left end H 
	{0x4023, 0xcf },//; Anchor left end L 
	{0x4024, 0x09 },//; Anchor right start H 
	{0x4025, 0x60 },//; Andhor right start L 
	{0x4026, 0x09 },//; Anchor right end H 
	{0x4027, 0x6f },//; Anchor right end L 
	{0x4028, 0x00 },//; top Zline start 
	{0x4029, 0x02 },//; top Zline number 
	{0x402a, 0x06 },//; top blk line start 
	{0x402b, 0x04 },//; to blk line number 
	{0x402c, 0x02 },//; bot Zline start 
	{0x402d, 0x02 },//; bot Zline number 
	{0x402e, 0x0e },//; bot blk line start 
	{0x402f, 0x04 },//; bot blk line number 
	{0x4302, 0xff },//; clipping max H 
	{0x4303, 0xff },//; clipping max L 
	{0x4304, 0x00 },//; clipping min H 
	{0x4305, 0x00 },//; clipping min L 
	{0x4306, 0x00 },//; vfifo pix swap off, dpcm off, vfifo first line is blue line 
	{0x4308, 0x02 },//; debug mode, embeded off 
	{0x4500, 0x6c },//; ADC sync control 
	{0x4501, 0xc4 },//; 
	{0x4502, 0x40 },//; 
	{0x4503, 0x02 },//; ADC sync control 
	{0x4601, 0x04 },//; V fifo read start 
	{0x4800, 0x04 },//; MIPI always high speed off, Clock lane gate off, line short packet off, 
	{0x4813, 0x08 },//; Select HDR VC 
	{0x481f, 0x40 },//; MIPI clock prepare min 
	{0x4829, 0x78 },//; MIPI HS exit min 
	{0x4837, 0x18 },//; MIPI global timing 
	{0x4b00, 0x2a },//
	{0x4b0d, 0x00 },//
	{0x4d00, 0x04 },//; tpm slope H 
	{0x4d01, 0x42 },//; tpm slope L 
	{0x4d02, 0xd1 },//; tpm offset HH 
	{0x4d03, 0x93 },//; tpm offset H 
	{0x4d04, 0xf5 },//; tpm offset M 
	{0x4d05, 0xc1 },//; tpm offset L 
	{0x5000, 0xf3 },
	{0x5001, 0x11 },//; ISP EOF select, ISP SOF off, BLC on 
	{0x5004, 0x00 },//; debug mode 
	{0x500a, 0x00 },//; debug mode 
	{0x500b, 0x00 },//; debug mode 
	{0x5032, 0x00 },//; debug mode 
	{0x5040, 0x00 },//; test mode off 
	{0x5050, 0x0c },//; debug mode 
	{0x5500, 0x00 },//; OTP DPC start H 
	{0x5501, 0x10 },//; OTP DPC start L 
	{0x5502, 0x01 },//; OTP DPC end H 
	{0x5503, 0x0f },//; OTP DPC end L 
	{0x8000, 0x00 },//; test mode 
	{0x8001, 0x00 },//; 
	{0x8002, 0x00 },//; 
	{0x8003, 0x00 },//; 
	{0x8004, 0x00 },//; 
	{0x8005, 0x00 },//; 
	{0x8006, 0x00 },//; 
	{0x8007, 0x00 },//; 
	{0x8008, 0x00 },//; test mode 
	{0x3638, 0x00 },//; ADC & analog 
	{0x3105, 0x31 },//; SCCB control, debug mode 
	{0x301a, 0xf9 },//; enable emb clock, enable strobe clock, enable timing control clock, mipi-phy manual reset, reset timing control block 
	{0x3508, 0x07 },//; Long gain H 
	{0x484b, 0x05 },//; line start after fifo_st, sclock start after SOF, frame start after SOF 
	{0x4805, 0x03 },//; MIPI control 
	{0x3601, 0x01 },//; ADC & analog 
	{0x0100, 0x01 },//; wake up from sleep 
	{REG_DLY, 0x02},// delay 2ms here 
	{0x3105, 0x11 },//; SCCB control, debug mode 
	{0x301a, 0xf1 },//; disable mipi-phy reset 
	{0x4805, 0x00 },//; MIPI control 
	{0x301a, 0xf0 },//; enable emb clock, enable strobe clock, disable mipi-phy reset, enable timing control clock, 
	{0x3208, 0x00 },//; group hold start, group bank 0 
	{0x302a, 0x00 },//; delay? 
	{0x302a, 0x00 },//; 
	{0x302a, 0x00 },//; 
	{0x302a, 0x00 },//; 
	{0x302a, 0x00 },//; 
	{0x3601, 0x00 },//; ADC & analog 
	{0x3638, 0x00 },//; ADC & analog 
	{0x3208, 0x10 },//; group hold end, group select 0 
	{0x3208, 0xa0 },//; group delay launch, group select 0 
	
	//MWB gain
	{0x500c,0x08},
	{0x500d,0x88},
	{0x500e,0x04},
	{0x500f,0x00},
	{0x5010,0x06},
	{0x5011,0x3d},
		
	{0x0100, 0x01 },//; wake up from sleep 
};

//for capture                                                                         
static struct regval_list sensor_quxga_regs[] = {
	// Full Resolution 2688x1520 30fps  
	{0x0100,0x00}, // sleep
	{REG_DLY,0x32},
	{0x3501, 0x60},// ; long exposure H                        
	{0x3632, 0x00},// ; ADC & Analog                           
	{0x376b, 0x20},// ; Sensor control                         
	{0x3800, 0x00},// ; H crop start H                         
	{0x3801, 0x08},// ; H crop start L                         
	{0x3803, 0x04},// ; V crop start L                         
	{0x3804, 0x0a},// ; H crop end H                           
	{0x3805, 0x97},// ; H crop end L                           
	{0x3807, 0xfb},// ; V crop end L                           
	{0x3808, 0x0a},// ; H output size H                        
	{0x3809, 0x80},// ; H output size L                        
	{0x380a, 0x05},// ; V output size H                        
	{0x380b, 0xf0},// ; V output size L                        
	{0x380d, 0x10},// ; HTS L                                  
	{0x380e, 0x06},// ; VTS H                                  
	{0x380f, 0x12},// ; VTS L                                  
	{0x3811, 0x08},// ; H win off L                            
	{0x3813, 0x04},// ; V win off L                            
	{0x3814, 0x01},// ; H inc odd                              
	{0x3820, 0x06},// ; flip off, bin off
	{0x3821, 0x00},// ; mirror on, bin off
	{0x382a, 0x01},// ; vertical subsample odd increase number 
	{0x3830, 0x04},// ; blc use num/2                          
	{0x3836, 0x01},// ; r zline use num/2                      
	{0x4001, 0x40},// ; debug mode                             
	{0x4022, 0x07},// ; Anchor left end H                      
	{0x4023, 0xcf},// ; Anchor left end L                      
	{0x4024, 0x09},// ; Anchor right start H                   
	{0x4025, 0x60},// ; Andhor right start L                   
	{0x4026, 0x09},// ; Anchor right end H                     
	{0x4027, 0x6f},// ; Anchor right end L                     
	{0x4502, 0x40},// ; ADC sync control                       
	{0x4601, 0x04},// ; V fifo read start  
	{0x0100, 0x01}, // wake up
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
int ov4689_sensor_vts ;
static int sensor_s_exp_gain(struct v4l2_subdev *sd, struct sensor_exp_gain *exp_gain)
{
	int exp_val, gain_val,frame_length,shutter;
	unsigned char explow=0,expmid=0,exphigh=0;
	unsigned char gainlow=0,gainhigh=0; 
	struct sensor_info *info = to_state(sd);
	exp_val = exp_gain->exp_val;
	gain_val = exp_gain->gain_val;

//	if((info->exp == exp_val)&&(info->gain == gain_val))
//		return 0;
	if(gain_val>16*16-1)
		gain_val=16*16-1;

	if(exp_val>0xfffff)
		exp_val=0xfffff;
//	printk("exp_val = %d,gain_val = %d\n",exp_val,gain_val);

	gain_val *=8;
	gain_val = gain_val & 0x7ff;

	if (gain_val<2*16*8)
	{
		gainhigh=0;
		gainlow = gain_val;
	}
	else if (2*16*8<=gain_val && gain_val <4*16*8)
	{
		gainhigh = 1;
		gainlow = gain_val/2-8;
	}
	else if (4*16*8<= gain_val && gain_val <8*16*8)
	{
		gainhigh = 3;
		gainlow = gain_val/4-12;
	}
	else 
	{
		gainhigh = 7;
		gainlow= gain_val/8-8;
	}
	    
	exp_val >>=4;

	exphigh = (unsigned char) ( (0xffff&exp_val)>>12);  
	expmid  = (unsigned char) ( (0xfff&exp_val)>>4);  
	explow  = (unsigned char) ( (0x0f&exp_val)<<4   );
	shutter = exp_val;  
	if(shutter  > ov4689_sensor_vts- 4)
	    frame_length = shutter + 4;
	else
	    frame_length = ov4689_sensor_vts;
   
	sensor_write(sd, 0x380f, (frame_length & 0xff));
	sensor_write(sd, 0x380e, (frame_length >> 8));

	sensor_write(sd, 0x3509, /*0x0f);//*/gainlow);
	sensor_write(sd, 0x3508, /*0x07);//*/gainhigh);

	sensor_write(sd, 0x3502, /*0x00);//*/explow);
	sensor_write(sd, 0x3501, /*0xa0);//*/expmid);
	sensor_write(sd, 0x3500, /*0x01);//*/exphigh);

	info->exp = exp_val;
	info->gain = gain_val;
	return 0;
}

static int sensor_s_exp(struct v4l2_subdev *sd, unsigned int exp_val)
{  
	unsigned char explow,expmid,exphigh;
	unsigned int tmp;
	data_type tmp1,tmp2,tmp3;
	struct sensor_info *info = to_state(sd);
	
	if(exp_val>0xfffff)
		exp_val=0xfffff;
       
	exp_val >>=4;
	exphigh = (unsigned char) ( (0xffff&exp_val)>>12);  
	expmid  = (unsigned char) ( (0xfff&exp_val)>>4);  
	explow  = (unsigned char) ( (0x0f&exp_val)<<4   );
	
	
	sensor_write(sd, 0x3208,0x00);

	sensor_write(sd, 0x3502, /*0x00);//*/explow);
	sensor_write(sd, 0x3501, /*0xa0);//*/expmid);
	sensor_write(sd, 0x3500, /*0x01);//*/exphigh);

	sensor_write(sd, 0x3208,0x10);
	sensor_write(sd, 0x3208,0xe0);

	sensor_read(sd,0x3502,&tmp1);
	sensor_read(sd,0x3501,&tmp2);
	sensor_read(sd,0x3500,&tmp3);
	tmp = (tmp1>>4)+(tmp2<<4)+(tmp3<<12);
	
//	printk("ov4689 sensor_set_exp = %d, Done!\n", tmp);
	
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
	unsigned int tmp;
	data_type tmp1,tmp2;
	
	if(gain_val<1*16)
		gain_val=16;
	if(gain_val>64*16-1)
		gain_val=64*16-1;
//  printk("transfer gain =%d\n",gain_val);
	gain_val *=8;
	gain_val = gain_val & 0x7ff;
	 
	if (gain_val<2*16*8)
	{
		gainhigh=0;
		gainlow = gain_val;
	}
	else if (2*16*8<=gain_val && gain_val <4*16*8)
	{
		gainhigh = 1;
		gainlow = gain_val/2-8;
	}
	else if (4*16*8<= gain_val && gain_val <8*16*8)
	{
		gainhigh = 3;
		gainlow = gain_val/4-12;
	}
	else 
	{
		gainhigh = 7;
		gainlow= gain_val/8-8;
	}

	sensor_write(sd, 0x3208,0x11);
	sensor_write(sd, 0x3509, /*0x0f);//*/gainlow);
	sensor_write(sd, 0x3508, /*0x07);//*/gainhigh);

	sensor_write(sd, 0x3208,0x11);
	sensor_write(sd, 0x3208,0xe1);
	sensor_read(sd,0x3509,&tmp1);
	sensor_read(sd,0x3508,&tmp2);
	if (tmp2 == 7)
	    tmp = (tmp1+8)*8;
	else if (tmp2 == 3)
	    tmp = (tmp1+12)*4;
	else if (tmp2 ==1)
		tmp = (tmp1+8)*2;
	else if (tmp2 == 0)
	    tmp = tmp1;
	
//	printk("ov4689 sensor_set_gain = %d, Done!\n", tmp);
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
			vfe_gpio_set_status(sd,POWER_EN,1);//set the gpio to output
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			vfe_set_pmu_channel(sd,IOVDD,ON);
			usleep_range(10000,12000);

			vfe_set_pmu_channel(sd,AVDD,ON);
			usleep_range(5000,6000);
			vfe_gpio_write(sd,POWER_EN,CSI_GPIO_HIGH);
			vfe_set_pmu_channel(sd,DVDD,ON);
			vfe_set_pmu_channel(sd,AFVDD,ON);
			usleep_range(5000,6000);
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			usleep_range(10000,12000);
			vfe_set_mclk_freq(sd,MCLK);
			vfe_set_mclk(sd,ON);
			usleep_range(10000,12000);
			cci_unlock(sd);    
			break;
		case CSI_SUBDEV_PWR_OFF:
			vfe_dev_dbg("CSI_SUBDEV_PWR_OFF!\n");
			cci_lock(sd);   
			vfe_set_mclk(sd,OFF);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
			usleep_range(10000,12000);
			vfe_set_pmu_channel(sd,AFVDD,OFF);
			vfe_set_pmu_channel(sd,DVDD,OFF);
			vfe_gpio_write(sd,POWER_EN,CSI_GPIO_LOW);
			usleep_range(5000,6000);
			vfe_set_pmu_channel(sd,AVDD,OFF);

			usleep_range(5000,6000);
			vfe_set_pmu_channel(sd,IOVDD,OFF);  
			usleep_range(10000,12000);
			vfe_gpio_set_status(sd,RESET,0);//set the gpio to input
			vfe_gpio_set_status(sd,PWDN,0);//set the gpio to input
			vfe_gpio_set_status(sd,POWER_EN,0);//set the gpio to input
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
	if(rdval != 0x46)
		return -ENODEV;
  	
	LOG_ERR_RET(sensor_read(sd, 0x300b, &rdval))
	if(rdval != 0x88)
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
	info->width = 2688;
	info->height = 1520;
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
	/* 2688*1520 */
	{
		.width      = 2688,//QUXGA_WIDTH,//3280,
		.height     = 1520,//QUXGA_HEIGHT,//2464,
		.hoffset    = 0,
		.voffset    = 0,
		.hts        = 2576,
		.vts        = 1554,//2480,
		.pclk       = 120*1000*1000,
		.mipi_bps	  = 720*1000*1000,
		.fps_fixed  = 2,
		.bin_factor = 1,
		.intg_min   = 16,
		.intg_max   =(1554-4)<<4,
		.gain_min   = 1<<4,
		.gain_max   = (12<<4)-1,
		.regs       = sensor_quxga_regs,
		.regs_size  = ARRAY_SIZE(sensor_quxga_regs),
		.set_size   = NULL,
	},
		/* 2688*1520 */
	{
		.width      = 2304,//QUXGA_WIDTH,//3280,
		.height     = 1296,//QUXGA_HEIGHT,//2464,
		.hoffset    = 0,
		.voffset    = 0,
		.hts        = 2576,
		.vts        = 1554,//2480,
		.pclk       = 120*1000*1000,
		.mipi_bps	  = 720*1000*1000,
		.fps_fixed  = 2,
		.bin_factor = 1,
		.intg_min   = 16,
		.intg_max   =(1554-4)<<4,
		.gain_min   = 1<<4,
		.gain_max   = (12<<4)-1,
		.width_input	  = 2688,
		.height_input 	  = 1520,
		.regs       = sensor_quxga_regs,
		.regs_size  = ARRAY_SIZE(sensor_quxga_regs),
		.set_size   = NULL,
	},
	{
		.width      = HD1080_WIDTH,
		.height     = HD1080_HEIGHT,
		.hoffset    = 0,
		.voffset    = 0,
		.hts        = 2576,
		.vts        = 1554,//2480,
		.pclk       = 120*1000*1000,
		.mipi_bps	  = 720*1000*1000,
		.fps_fixed  = 2,
		.bin_factor = 1,
		.intg_min   = 16,
		.intg_max   =(1554-4)<<4,
		.gain_min   = 1<<4,
		.gain_max   =  (12<<4)-1,
		.width_input	  = 2688,
		.height_input 	  = 1520,
		.regs       = sensor_quxga_regs,
		.regs_size  = ARRAY_SIZE(sensor_quxga_regs),
		.set_size   = NULL,
	},
	{
		.width      = HD720_WIDTH,
		.height     = HD720_HEIGHT,
		.hoffset    = 0,
		.voffset    = 0,
		.hts        = 2576,
		.vts        = 1554,//2480,
		.pclk       = 120*1000*1000,
		.mipi_bps	  = 720*1000*1000,
		.fps_fixed  = 2,
		.bin_factor = 1,
		.intg_min   = 16,
		.intg_max   =(1554-4)<<4,
		.gain_min   = 1<<4,
		.gain_max   = (12<<4)-1,
		.width_input	  = 2688,
		.height_input 	  = 1520,
		.regs       = sensor_quxga_regs,
		.regs_size  = ARRAY_SIZE(sensor_quxga_regs),
		.set_size   = NULL,
	},
	{
		.width      = VGA_WIDTH,
		.height     = VGA_HEIGHT,
		.hoffset    = 320,
		.voffset    = 0,
		.hts        = 2576,
		.vts        = 1554,//2480,
		.pclk       = 120*1000*1000,
		.mipi_bps	  = 720*1000*1000,
		.fps_fixed  = 2,
		.bin_factor = 1,
		.intg_min   = 16,
		.intg_max   =(1554-4)<<4,
		.gain_min   = 1<<4,
		.gain_max   = (12<<4)-1,
		.width_input	  = 2048,
		.height_input 	  = 1520,
		.regs       = sensor_quxga_regs,
		.regs_size  = ARRAY_SIZE(sensor_quxga_regs),
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
	ov4689_sensor_vts = wsize->vts;

	vfe_dev_print("s_fmt set width = %d, height = %d\n",wsize->width,wsize->height);
   
	if(info->capture_mode == V4L2_MODE_VIDEO)
	{
	//video
	} else {
	//capture image
	}

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
		return v4l2_ctrl_query_fill(qc, 1*16, 128*16-1, 1, 16);
	case V4L2_CID_EXPOSURE:
		return v4l2_ctrl_query_fill(qc, 0, 65536*16, 1, 0);
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

