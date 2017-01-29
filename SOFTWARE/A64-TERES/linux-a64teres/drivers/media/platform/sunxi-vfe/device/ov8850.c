/*
 * A V4L2 driver for OV8850 cameras.
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
MODULE_DESCRIPTION("A low-level driver for OV8850 sensors");
MODULE_LICENSE("GPL");

//for internel driver debug
#define DEV_DBG_EN      1 
#if(DEV_DBG_EN == 1)    
#define vfe_dev_dbg(x,arg...) printk("[OV8850]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...) 
#endif
#define vfe_dev_err(x,arg...) printk("[OV8850]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[OV8850]"x,##arg)

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
#define V4L2_IDENT_SENSOR 0x8850
int ov8850_sensor_vts;



/*
 * Our nominal (default) frame rate.
 */

#define SENSOR_FRAME_RATE 30


/*
 * The ov8850 sits on i2c with ID 0x6c
 */
#define I2C_ADDR 0x20
#define SENSOR_NAME "ov8850"
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
	// Slave_ID=0x20//
	{0x0103, 0x01},//  ; software reset
	{0x0102, 0x01},//  ; 
	{0x3002, 0x08},//  ; STROBE output enable
	{0x3004, 0x00},// 
	{0x3005, 0x00},//  ; strobe, PWM input
	{0x3011, 0x41},//  ; MIPI 4 lane
	{0x3012, 0x08},// 
	{0x3014, 0x4a},// 
	{0x3015, 0x0a},// ` ; MIPI 4 lane enable
	{0x3021, 0x00},// 
	{0x3022, 0x02},// 
	{0x3081, 0x02},// 
	{0x3083, 0x01},// 
	{0x3092, 0x00},//  ; PLL2 divs
	{0x3093, 0x00},//  ; PLL2 seld5
	{0x309a, 0x00},//  ; PLL3 divs
	{0x309b, 0x00},//  ; PLL3 div
	{0x309c, 0x00},//  ; PLL3 multiplier
	{0x30b3, 0x62},//  ; PLL1 multiplier
	{0x30b4, 0x03},//  ; PLL1 prediv
	{0x30b5, 0x04},//  ; PLL1 op_pix_div
	
	{0x30b6, 0x01},//  ; PLL1 op_sys_div
	{0x3104, 0xa1},// 
	{0x3106, 0x01},// 
	{0x3503, 0x07},//  ; AGC manual, AEC manual
	{0x350a, 0x00},//  ; Gain H
	{0x350b, 0x38},//  ; Gain L
	{0x3602, 0x70},//  ; analog control
	{0x3620, 0x64},//  ;
	{0x3622, 0x0f},//  ;
	{0x3623, 0x68},//  ;
	{0x3625, 0x40},//  ;
	{0x3631, 0x83},//  ;
	{0x3633, 0x34},//  ;
	{0x3634, 0x03},//  ;
	{0x364c, 0x00},//  ;
	{0x364d, 0x00},//  ;
	{0x364e, 0x00},//  ;
	{0x364f, 0x00},//  ;
	{0x3660, 0x80},//  ;
	{0x3662, 0x10},//  ;
	{0x3665, 0x00},//  ;
	{0x3666, 0x00},//  ;
	{0x366f, 0x20},//  ; analog control
	{0x3703, 0x2e},//  ; sensor control
	{0x3732, 0x05},//  ;
	{0x373a, 0x51},//  ;
	{0x373d, 0x22},//  ;
	{0x3754, 0xc0},//  ;
	{0x3756, 0x2a},//  ;
	{0x3759, 0x0f},//  ;
	{0x376b, 0x44},//  ; sensor control
	{0x3795, 0x00},//  ; PSRAM control
	{0x379c, 0x0c},//  ; PSRAM control
	{0x3810, 0x00},//  ; ISP x offset H
	
	{0x3811, 0x04},//  ; ISP x offset L
	{0x3812, 0x00},//  ; ISP y offset H
	{0x3813, 0x04},//  ; ISP y offset L
	{0x3820, 0x10},//  ; bin off, flip off
	{0x3821, 0x0e},//  ; bin off, mirror on
	{0x3826, 0x00},// 
	{0x4000, 0x10},// ; enable module DCBLC autoload 
	{0x4002, 0xc5},//  ; BLC level trigger on
	{0x4005, 0x18},//  ; BLC level trigger
	{0x4006, 0x20},// 
	{0x4007, 0x90},//
	{0x4008, 0x20},// 
	{0x4009, 0x10},// 
	{0x404f, 0xA0},//  ; BLC stable range
	{0x4100, 0x1d},// 
	{0x4101, 0x23},// 
	{0x4102, 0x04},// ; solve blc issue 
	{0x4104, 0x5c},// 
	{0x4109, 0x03},// 
	{0x4300, 0xff},//  ; data max H
	{0x4301, 0x00},//  ; data min H
	{0x4315, 0x00},//  ; Vsync delay
	{0x4512, 0x01},//  ; vertical binning average
	{0x4837, 0x08},//  ; MIPI global timing
	{0x4a00, 0xaa},//  ; LVDS control
	{0x4a03, 0x01},//  ; LVDS control
	{0x4a05, 0x08},//  ; LVDS control
	{0x4d00, 0x04},//  ; temperature monitor
	{0x4d01, 0x52},//  ;
	{0x4d02, 0xfe},//  ;
	{0x4d03, 0x05},//  ;
	{0x4d04, 0xff},//  ;
	{0x4d05, 0xff},//  ; temperature monitor
	{0x5000, 0x06},//  ; DPC on
	
	{0x5001, 0x01},//  ; MWB on
	{0x5002, 0x88},//  ; scale on
	{0x5013, 0x80},//
	{0x5041, 0x04},//  ; average enable
	{0x5043, 0x48},// 
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
	{0x5b00, 0x00},//
	{0x5b01, 0xf7},//
	{0x5b03, 0x19},//
	{0x5e00, 0x00},//  ; test pattern off
	{0x5e10, 0x1c},// 
	{0x3500, 0x00},// ; AEC[18:16]
	{0x3501, 0x9C},//  ; AEC[15:8]
	{0x3502, 0x20},//  ; AEC[7:0]
	{0x3090, 0x03},//  ; PLL2 prediv
	{0x3091, 0x11},//  ; PLL2 multiplier
	{0x3094, 0x01},//  ; PLL2 mult2
	{0x3098, 0x02},//  ; PLL3 prediv
	{0x3099, 0x16},//  ; PLL3 multiplier
	{0x3624, 0x00},//  ; analog control
	
	{0x3680, 0xb0},//  ; analog control
	{0x3702, 0x6e},//  ; sensor control
	{0x3704, 0x55},//  ; sensor control
	{0x3708, 0xe3},//  ; sensor control
	{0x3709, 0xc3},//  ; sensor control
	{0x371f, 0x0d},//  ; sensor control
	{0x3739, 0x80},//  ; sensor control
	{0x373c, 0x24},//  ; sensor control
	{0x3781, 0xc8},//  ; PSRAM control
	{0x3786, 0x08},//  ; PSRAM control
	{0x3796, 0x43},//  ; PSRAM control
	{0x3800, 0x00},//  ; x start H
	{0x3801, 0x04},//  ; x start L
	{0x3802, 0x00},//  ; y start H
	{0x3803, 0x0c},//  ; y start L
	{0x3804, 0x0c},//  ; x end H
	{0x3805, 0xcb},//  ; x end L
	{0x3806, 0x09},//  ; y end H
	{0x3807, 0xa3},//  ; y end L
	{0x3808, 0x0c},//  ; x output size H
	{0x3809, 0xc0},//  ; x output size L
	{0x380a, 0x09},//  ; y output size H
	{0x380b, 0x90},//  ; y output size L
	{0x380c, 0x0e},//  ; HTS H
	{0x380d, 0x17},//  ; HTS L
	{0x380e, 0x09},//  ; VTS H
	{0x380f, 0xD0},//  ; VTS L
	{0x3814, 0x11},//  ; x inc
	{0x3815, 0x11},//  ; y inc
	{0x3820, 0x10},// ; bin off, flip off
	{0x3821, 0x0e},// ; bin off, mirror on
	{0x3a04, 0x09},//
	{0x3a05, 0xcc},//
	{0x4001, 0x06},// ; BLC start line 
	
	{0x4004, 0x04},//  ; number of black lines
};

//for capture                                                                         
static struct regval_list sensor_quxga_regs[] = {
	//@@3264_2448_2lane_2.5fps_110Mbps/lane
	{0x0100, 0x00},// ; sleep									   
						  
	{0x3500, 0x00},// ; AEC[18:16]								   
	{0x3501, 0x9A},//  ; AEC[15:8]									
	{0x3502, 0x60},//  ; AEC[7:0]									
	{0x3011, 0x21},// ; 2 lane MIPI 							   
	{0x3015, 0xca},// ; 2 lane enable							   
	{0x3090, 0x03},// ; PLL2 prediv 								
	{0x3091, 0x22},//  ; PLL2 multiplier							
	{0x3093, 0x02},// ; PLL2 seld5								   
	{0x3094, 0x00},//  ; PLL2 mult2 								
	{0x3098, 0x03},//  ; PLL3 prediv								
	{0x3099, 0x1e},// ; PLL3 multiplier 						   
	{0x30b3, 0x51},//  ; PLL1 multiplier							
	{0x3624, 0x04},//  ; analog control 							
	{0x3680, 0xe0},//  ; analog control 							
	{0x3702, 0xf3},//  ; sensor control 							
	{0x3704, 0x71},//  ; sensor control 							
	{0x3708, 0xe3},//  ; sensor control 							
	{0x3709, 0xc3},//  ; sensor control 							
	{0x371f, 0x0c},//  ; sensor control 							
	{0x3739, 0x30},//  ; sensor control 							
	{0x373c, 0x20},//  ; sensor control 							
	{0x3781, 0x0c},//  ; PSRAM control								
	{0x3786, 0x16},//  ; PSRAM control								
	{0x3796, 0x64},//  ; PSRAM control								
	{0x3800, 0x00},//  ; x start H									
	{0x3801, 0x04},//  ; x start L									
	{0x3802, 0x00},//  ; y start H									
	{0x3803, 0x0c},//  ; y start L									
	{0x3804, 0x0c},//  ; x end H									
	{0x3805, 0xcb},//  ; x end L									
	{0x3806, 0x09},//  ; y end H									
	{0x3807, 0xa3},//  ; y ens L									
	{0x3808, 0x0c},//  ; x output size H							
	
	{0x3809, 0xc0},//  ; x output size L							
	{0x380a, 0x09},//  ; y output size H							
	{0x380b, 0x90},//  ; y output size L							
	{0x380c, 0x0e},//  ; HTS H										
	{0x380d, 0x17},//  ; HTS L										
	{0x380e, 0x09},//  ; VTS H										
	{0x380f, 0xd0},//  ; VTS L										
	{0x3814, 0x11},//  ; x inc										
	{0x3815, 0x11},//  ; y inc										
	{0x3820, 0x10},// ; bin off, flip off						   
	{0x3821, 0x0e},// ; bin off, mirror on						   
	{0x3a04, 0x09},//											   
	{0x3a05, 0xb0},//											   
	{0x4001, 0x02},//	   ; BLC start line 										
	{0x4004, 0x08},// ; number of black line							 
	{0x4005, 0x1a},// ; BLC trigger every frame, for single capture ; for single capture
	{0x4837, 0x0c},//  ; MIPI global timing 						
	{0x0100, 0x01},// ; wake up from sleep 
};


//for video
static struct regval_list sensor_1080p_regs[] = { 
	//@@1920_1080_2Lane_5fps_120Mbps/lane
	{0x0100, 0x00},// ; sleep									   
											 
	{0x3500, 0x00},// ; AEC[18:16]								   
	{0x3501, 0x7c},//  ; AEC[15:8]									
	{0x3502, 0x00},//  ; AEC[7:0]								   
	{0x3011, 0x21},// ; 2 lane MIPI 							   
	{0x3015, 0xca},// ; 2 lane enable								
	{0x3090, 0x03},// ; PLL2 prediv 								
	{0x3091, 0x22},//  ; PLL2 multiplier							
	{0x3093, 0x02},// ; PLL2 seld5								   
	{0x3094, 0x00},//  ; PLL2 mult2 								
	{0x3098, 0x03},//  ; PLL3 prediv								
	{0x3099, 0x1e},// ; PLL3 multiplier 						   
	{0x30b3, 0x51},//  ; PLL1 multiplier							
	
	
	
	{0x3624, 0x04},//  ; analog control 							
	{0x3680, 0xe0},//  ; analog control 							
	{0x3702, 0xf3},//  ; sensor control 							
	{0x3704, 0x71},//  ; sensor control 							
	{0x3708, 0xe3},//  ; sensor control 							
	{0x3709, 0xc3},//  ; sensor control 							
	{0x371f, 0x0c},//  ; sensor control 							
	{0x3739, 0x30},//  ; sensor control 							
	{0x373c, 0x20},//  ; sensor control 							
	{0x3781, 0x0c},//  ; PSRAM control								
	{0x3786, 0x16},//  ; PSRAM control								
	{0x3796, 0x64},//  ; PSRAM control								
	{0x3800, 0x00},//  ; x start H									
	{0x3801, 0x0c},//  ; x start L									
	{0x3802, 0x01},//  ; y start H									
	{0x3803, 0x40},//  ; y start L									
	{0x3804, 0x0c},//  ; x end H									
	{0x3805, 0xd3},//  ; x end L									
	{0x3806, 0x08},//  ; y end H									
	{0x3807, 0x73},//  ; y ens L									
	{0x3808, 0x07},//  ; x output size H							
	{0x3809, 0x80},//  ; x output size L							
	{0x380a, 0x04},//  ; y output size H							
	{0x380b, 0x38},//  ; y output size L							
	{0x380c, 0x0e},//  ; HTS H										
	{0x380d, 0x17},//  ; HTS L										
	{0x380e, 0x04},//  ; VTS H										
	{0x380f, 0xe8},//  ; VTS L										
	{0x3814, 0x11},//  ; x inc										
	{0x3815, 0x11},//  ; y inc										
	{0x3820, 0x10},// ; bin off, flip off						   
	{0x3821, 0x0e},// ; bin off, mirror on						   
	{0x3a04, 0x07},//											   
	{0x3a05, 0xc8},//											   
	
	
	
	{0x4001, 0x02},//	   ; BLC start line 										
	{0x4004, 0x08},// ; number of black line							 
	{0x4005, 0x18},// ; BLC trigger every frame, for single capture
	{0x4837, 0x0c},//  ; MIPI global timing 						
	{0x0100, 0x01},// ; wake up from sleep
};

static struct regval_list sensor_sxga_regs[] = { 
};

static struct regval_list sensor_720p_regs[] = {
	//@@1280_720_2Lane_60fps_456Mbps/lane
	{0x0100, 0x00},// ; sleep									   
											 
	{0x3500, 0x00},// ; AEC[18:16]								   
	{0x3501, 0x3d},//  ; AEC[15:8]									
	{0x3502, 0x80},//  ; AEC[7:0]									
	{0x3011, 0x21},// ; 2 lane MIPI 							   
	{0x3015, 0xca},// ; 2 lane enable							   
	{0x3090, 0x03},// ; PLL2 prediv 								
	{0x3091, 0x22},//  ; PLL2 multiplier							
	{0x3093, 0x02},// ; PLL2 seld5								   
	{0x3094, 0x00},//  ; PLL2 mult2 								
	{0x3098, 0x03},//  ; PLL3 prediv								
	{0x3099, 0x1e},// ; PLL3 multiplier 						   
	{0x30b3, 0x51},//  ; PLL1 multiplier							
	{0x3624, 0x04},//  ; analog control 							
	{0x3680, 0xe0},//  ; analog control 							
	{0x3702, 0xf3},//  ; sensor control 							
	{0x3704, 0x71},//  ; sensor control 							
	{0x3708, 0xe6},//  ; sensor control 							
	{0x3709, 0xc3},//  ; sensor control 							
	{0x371f, 0x0c},//  ; sensor control 							
	{0x3739, 0x30},//  ; sensor control 							
	{0x373c, 0x20},//  ; sensor control 							
	{0x3781, 0x0c},//  ; PSRAM control								
	{0x3786, 0x16},//  ; PSRAM control								
	{0x3796, 0x64},//  ; PSRAM control								
	{0x3800, 0x00},//  ; x start H									
	
	{0x3801, 0x28},//  ; x start L									
	{0x3802, 0x01},//  ; y start H									
	{0x3803, 0x4c},//  ; y start L									
	{0x3804, 0x0c},//  ; x end H									
	{0x3805, 0xb7},//  ; x end L									
	{0x3806, 0x08},//  ; y end H									
	{0x3807, 0x63},//  ; y ens L									
	{0x3808, 0x05},//  ; x output size H							
	{0x3809, 0x00},//  ; x output size L							
	{0x380a, 0x02},//  ; y output size H							
	{0x380b, 0xd0},//  ; y output size L							
	{0x380c, 0x0e},//  ; HTS H										
	{0x380d, 0x17},//  ; HTS L										
	{0x380e, 0x04},//  ; VTS H										
	{0x380f, 0xe8},//  ; VTS L										
	{0x3814, 0x31},//  ; x inc										
	{0x3815, 0x31},//  ; y inc										
	{0x3820, 0x11},// ; bin on, flip off						  
	{0x3821, 0x0f},// ; bin on, mirror on						  
	{0x3a04, 0x03},//											   
	{0x3a05, 0xe2},//											   
	{0x4001, 0x02},//	   ; BLC start line 										
	{0x4004, 0x04},// ; number of black line							 
	{0x4005, 0x18},// ; BLC trigger every frame, for single capture
	{0x4837, 0x0c},//  ; MIPI global timing 						
	{0x0100, 0x01},// ; wake up from sleep
};


static struct regval_list sensor_vga_regs[] = { //VGA:  640*480
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
  
	gainlow=(unsigned char)(gain_val&0xff);
	gainhigh=(unsigned char)((gain_val>>8)&0x3);
  
	exphigh	= (unsigned char) ( (0x0f0000&exp_val)>>16);
	expmid	= (unsigned char) ( (0x00ff00&exp_val)>>8);
	explow	= (unsigned char) ( (0x0000ff&exp_val)	 );
	shutter = exp_val/16;  
	if(shutter  > ov8850_sensor_vts- 4)
	    frame_length = shutter + 4;
	else
	    frame_length = ov8850_sensor_vts;

	//printk("exp_val = %d,gain_val = %d\n",exp_val,gain_val);
	sensor_write(sd, 0x3208, 0x00);//enter group write

	sensor_write(sd, 0x3503, 0x07);
  
	sensor_write(sd, 0x380f, (frame_length & 0xff));
	sensor_write(sd, 0x380e, (frame_length >> 8));

	sensor_write(sd, 0x350b, gainlow);
	sensor_write(sd, 0x350a, gainhigh);
	
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
	printk("8850 sensor_set_exp = %d, Done!\n", exp_val);
	
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
	
	sensor_write(sd, 0x3503, 0x17);
	sensor_write(sd, 0x350b, gainlow);
	sensor_write(sd, 0x350a, gainhigh);
	sensor_write(sd, 0x3208, 0x10);//end group write
	sensor_write(sd, 0x3208, 0xa0);//init group write
	
	printk("8850 sensor_set_gain = %d, Done!\n", gain_val);
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
			mdelay(10);
			cci_unlock(sd);        
			break;
		case CSI_SUBDEV_PWR_ON:
			vfe_dev_dbg("CSI_SUBDEV_PWR_ON!\n");
			cci_lock(sd);
			vfe_gpio_set_status(sd,PWDN,1);//set the gpio to output
			vfe_gpio_set_status(sd,RESET,1);//set the gpio to output
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			mdelay(1);
			vfe_set_mclk_freq(sd,MCLK);
			vfe_set_mclk(sd,ON);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,POWER_EN,CSI_GPIO_HIGH);
			vfe_set_pmu_channel(sd,IOVDD,ON);
			vfe_set_pmu_channel(sd,AVDD,ON);
			vfe_set_pmu_channel(sd,DVDD,ON);
			vfe_set_pmu_channel(sd,AFVDD,ON);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			mdelay(10);
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);
			mdelay(30);
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
			mdelay(10);
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
	if(rdval != 0x88)
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
		.hts        = 3607,
		.vts        = 2512,
		.pclk       = 136*1000*1000,
		.mipi_bps	  = 648*1000*1000,
		.fps_fixed  = 2,
		.bin_factor = 1,
		.intg_min   = 16,
		.intg_max   = (2512-4)<<4,
		.gain_min   = 1<<4,
		.gain_max   = 12<<4,
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
		.hts		  = 3607,
		.vts		  = 1256,
		.pclk       = 136*1000*1000,
		.mipi_bps   = 648*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 1<<4,
		.intg_max   = (1868-4)<<4,
		.gain_min   = 1<<4,
		.gain_max   = 12<<4,
		.regs       = sensor_1080p_regs,//
		.regs_size  = ARRAY_SIZE(sensor_1080p_regs),//
		.set_size		= NULL,
	},

	/* SXGA */
	{
		.width	  = SXGA_WIDTH,
		.height 	  = SXGA_HEIGHT,
		.hoffset	  = 176,
		.voffset	  = 132,
		.hts        = 3516,
		.vts        = 1264,
		.pclk       = 133*1000*1000,
		.mipi_bps   = 720*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 1,
		.intg_max   = (1264-4)<<4,
		.gain_min   = 1<<4,
		.gain_max   = 12<<4,
		.regs		    = sensor_sxga_regs,
		.regs_size	= ARRAY_SIZE(sensor_sxga_regs),
		.set_size		= NULL,
	},

	/* 720p */
	{
		.width      = HD720_WIDTH,
		.height     = HD720_HEIGHT,
		.hoffset    = 0,
		.voffset    = 0,
		.hts		  = 3516,
		.vts		  = 1264,
		.pclk       = 136*1000*1000,
		.mipi_bps	  = 648*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 16,
		.intg_max   = (928-4)<<4,
		.gain_min   = 1<<4,
		.gain_max   = 12<<4,
		.regs		  = sensor_720p_regs,//
		.regs_size  = ARRAY_SIZE(sensor_720p_regs),//
		.set_size	  = NULL,
	},

	/* VGA */
	{
		.width	  = VGA_WIDTH,
		.height 	  = VGA_HEIGHT,
		.hoffset	  = 0,
		.voffset	  = 0,
		.hts        = 3504,//limited by sensor
		.vts        = 634,
		.pclk       = 67*1000*1000,//67
		.mipi_bps   = 240*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 16,
		.intg_max   = (634-4)<<4,
		.gain_min   = 1<<4,
		.gain_max   = 12<<4,
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
	ov8850_sensor_vts = wsize->vts;
   
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
		return v4l2_ctrl_query_fill(qc, 1*16, 32*16, 1, 16);
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

