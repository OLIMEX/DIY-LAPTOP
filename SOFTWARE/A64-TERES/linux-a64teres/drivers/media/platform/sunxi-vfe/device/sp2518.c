	/*
 * A V4L2 driver for Superpix  sp2518 cameras.
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
MODULE_DESCRIPTION("A low-level driver for Superpix  SP2518 sensors");
MODULE_LICENSE("GPL");



//for internel driver debug
#define DEV_DBG_EN   		0 
#if(DEV_DBG_EN == 1)		
#define vfe_dev_dbg(x,arg...) printk("[CSI_DEBUG][SP2518]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...) 
#endif

#define vfe_dev_err(x,arg...) printk("[CSI_ERR][SP2518]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[CSI][SP2518]"x,##arg)

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
#define V4L2_IDENT_SENSOR  0x2518


/*
 * Our nominal (default) frame rate.
 */
#define SENSOR_FRAME_RATE  6 //30

/*
 * The sp2518 sits on i2c with ID 0x60
 */
#define I2C_ADDR      0x60   
#define SENSOR_NAME "sp2518"
//HEQ
#define  SP2518_P0_0xdd  0x74
#define  SP2518_P0_0xde  0x9c
//sat
#define  SP2518_P0_0xd4  0x60
#define  SP2518_P0_0xd9  0x60
//auto lum
#define SP2518_NORMAL_Y0ffset  0x14
#define SP2518_LOWLIGHT_Y0ffset  0x28

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
	{0xfd,0x00},
	{0x0e,0x01},
	{0x0f,0x2f},
	{0x10,0x2e},
	{0x11,0x00},
	{0x12,0x4f},  //0x2f 	modify by sp_yjp,20120507
	{0x14,0x40},// 0x20 zch 20120920
	{0x16,0x02},
	{0x17,0x10},
	{0x1a,0x1f},		//0x1e	modify by sp_yjp,20120507
	{0x1e,0x81},
	{0x21,0x00},//0x08	//0x0f 	modify by sp_yjp,20120507
	{0x22,0x1b},//0x19
	{0x25,0x10},
	{0x26,0x25},
	{0x27,0x6d},
	{0x2c,0x23},		//0x45 	modify by sp_yjp,20120507
	{0x2d,0x75},
	{0x2e,0x38},//18
	{0x31,0x10},//mirror upside down
	{0x44,0x03},
	{0x6f,0x00},
	{0xa0,0x04},
	{0x5f,0x01},
	{0x32,0x00},
	{0xfd,0x01},
	{0x2c,0x00},
	{0x2d,0x00},
	{0xfd,0x00},
	{0xfb,0x83},
	{0xf4,0x09},			
	
	//Pregain	 
	{0xfd,0x01},
	{0xc6,0x90},
	{0xc7,0x90}, 
	{0xc8,0x90},
	{0xc9,0x90},
	
	//black level
	{0xfd,0x00},
	{0x65,0x0a},
	{0x66,0x0f},
	{0x67,0x0f},
	{0x68,0x0e},
	//bpc
	{0x46,0xff},

	//rpc		  
	
	{0xfd,0x00},
	{0xe0,0x6c}, 
	{0xe1,0x54}, 
	{0xe2,0x48}, 
	{0xe3,0x40},
	{0xe4,0x40},
	{0xe5,0x3e},
	{0xe6,0x3e},
	{0xe8,0x3a},
	{0xe9,0x3a},
	{0xea,0x3a},
	{0xeb,0x38},
	{0xf5,0x38},
	{0xf6,0x38},
	{0xfd,0x01},
	{0x94,0xa0},//f8
	{0x95,0x28},
	{0x9c,0x6c},
	{0x9d,0x38}, 

  
	//SP2518 UXGA 24M 2  1 50Hz 8.5582-8.5fps
	//ae setting
	{0xfd,0x00},
	{0x03,0x02},
	{0x04,0x9a},
	{0x05,0x00},
	{0x06,0x00},
	{0x07,0x00},
	{0x08,0x00},
	{0x09,0x00},
	{0x0a,0x9b},
	{0x2f,0x00},
	{0x30,0x04},
	{0xf0,0x6f},
	{0xf1,0x00},
	{0xfd,0x01},
	{0x90,0x0b},
	{0x92,0x01},
	{0x98,0x6f},
	{0x99,0x00},
	{0x9a,0x01},
	{0x9b,0x00},
	//Status
	{0xfd,0x01},
	{0xce,0xc5},
	{0xcf,0x04},
	{0xd0,0xc5},
	{0xd1,0x04},
	{0xd7,0x6b},
	{0xd8,0x00},
	{0xd9,0x6f},
	{0xda,0x00},
	{0xfd,0x00},

	{0xfd,0x01},
	{0xca,0x30},//mean dummy2low
	{0xcb,0x50},//mean low2dummy
	{0xcc,0xc0},//rpc low
	{0xcd,0xc0},//rpc dummy
	{0xd5,0x80},//mean normal2dummy
	{0xd6,0x90},//mean dummy2normal
	{0xfd,0x00},

	//lens shading for ˴̩979C-171A\181A
	{0xfd,0x00}, 
	{0xa1,0x20},
	{0xa2,0x20},
	{0xa3,0x20},
	{0xa4,0xff},
	{0xa5,0x80},
	{0xa6,0x80},
	{0xfd,0x01},
	{0x64,0x1e},//28
	{0x65,0x1c},//25
	{0x66,0x1c},//
	{0x67,0x16},//25
	{0x68,0x1c},//25
	{0x69,0x1c},//29
	{0x6a,0x1a},//28
	{0x6b,0x16},//20
	{0x6c,0x1a},//22
	{0x6d,0x1a},//22
	{0x6e,0x1a},//22
	{0x6f,0x16},//
	{0xb8,0x04},//0a
	{0xb9,0x13},//0a
	{0xba,0x00},//23
	{0xbb,0x03},//14
	{0xbc,0x03},//08
	{0xbd,0x11},//08
	{0xbe,0x00},//12
	{0xbf,0x02},//00
	{0xc0,0x04},//05
	{0xc1,0x0e},//05
	{0xc2,0x00},//18
	{0xc3,0x05},//08 
	//raw filter  
	{0xfd,0x01},
	{0xde,0x0f},//whl130730 test  0x0f
	{0xfd,0x00},
				 
	{0x57,0x08},//raw_dif_thr
	{0x58,0x08},//a
	{0x56,0x08},//10
	{0x59,0x10},
	//R\B
	{0x5a,0xa0},// c0raw_rb_fac_outdoor
	{0xc4,0x80},//60raw_rb_fac_indoor
	{0x43,0x40},//40raw_rb_fac_dummy
	{0xad,0x20},//raw_rb_fac_low
	
	{0x4f,0xa0},//raw_gf_fac_outdoor
	{0xc3,0x80},//a0 sp_pxj 20140620//60raw_gf_fac_indoor
	{0x3f,0x40},//40raw_gf_fac_dummy
	{0x42,0x20},//raw_gf_fac_low
	{0xc2,0x15},
				
	{0xb6,0xa0},//80 sp_pxj 20140620//c0raw_gflt_fac_outdoor
	{0xb7,0x60},//80 sp_pxj 20140620//60raw_gflt_fac_normal
	{0xb8,0x30},//40raw_gflt_fac_dummy
	{0xb9,0x20},//raw_gflt_fac_low

	{0xfd,0x01},
	{0x50,0x0c},//c sp_pxj 20140620//10raw_grgb_thr
	{0x51,0x0c},//c sp_pxj 20140620//10
	{0x52,0x10},//10 sp_pxj 20140620
	{0x53,0x10},//10 sp_pxj 20140620
	{0xfd,0x00},	
	
	// awb1 	  
	{0xfd,0x01},
	{0x11,0x10},
	{0x12,0x1f},
	{0x16,0x1c},
	{0x18,0x00},
	{0x19,0x00},
	{0x1b,0x96},
	{0x1a,0x9a},//95
	{0x1e,0x2f},
	{0x1f,0x29},
	{0x20,0xff},
	{0x22,0xff},
	{0x28,0xce},
	{0x29,0x8a},
	{0xfd,0x00},
	{0xe7,0x03},
	{0xe7,0x00},
	{0xfd,0x01},
	{0x2a,0xf0},
	{0x2b,0x10},
	{0x2e,0x04},
	{0x2f,0x18},
	{0x21,0x60},
	{0x23,0x60},
	{0x8b,0xab},
	{0x8f,0x12},
	
	//awb2		  
	{0xfd,0x01},
	{0x1a,0x80},
	{0x1b,0x80}, 
	{0x43,0x80},
	//outdoor
	{0x00,0xd4},
	{0x01,0xb0},
	{0x02,0x90},
	{0x03,0x78},
	//d65
	{0x35,0xd6},//d6;b0
	{0x36,0xf0},//f0;d1;e9
	{0x37,0x7a},//8a;70
	{0x38,0x9a},//dc;9a;af
	//indoor
	{0x39,0xab},
	{0x3a,0xca},
	{0x3b,0xa3},
	{0x3c,0xc1},
	//f 		  
	{0x31,0x82},//7d
	{0x32,0xa5},//a0
	{0x33,0xd6},//d2
	{0x34,0xec},//e8
	//cwf		  
	{0x3d,0xa5},//88 a7
	{0x3e,0xc2},//bb be
	{0x3f,0xa7},//ad b3
	{0x40,0xc5},//d0 c5
				 
	//Color Correction
	{0xfd,0x01},			
	{0x1c,0xc0},	
	{0x1d,0x95},	

	{0xa0,0xa7},//b8 
	{0xa1,0xda},//d5
	{0xa2,0x00},//f2
	{0xa3,0x06},//e8
	{0xa4,0xb0},//95
	{0xa5,0xc7},//03
	{0xa6,0x00},//f2
	{0xa7,0xce},//c4
	{0xa8,0xb2},//ca
	{0xa9,0x0c},//3    //03
	{0xaa,0x30},//03
	{0xab,0x0c},//0f
	
	{0xac,0xc0},//b8 
	{0xad,0xc0},//d5
	{0xae,0x00},//f2
	{0xaf,0xf2},//e8
	{0xb0,0xa0},//95
	{0xb1,0xe8},//03
	{0xb2,0x00},//f2
	{0xb3,0xe7},//c4
	{0xb4,0x99},//ca
	{0xb5,0x0c},//  3c
	{0xb6,0x33},//03
	{0xb7,0x0c},//0f
				 
	//Saturation  
	{0xfd,0x00},
	{0xbf,0x01},
	{0xbe,0x99},
	{0xc0,0xb0},
	{0xc1,0xf0},
	{0xd3,0x74},
	{0xd4,SP2518_P0_0xd4},
	{0xd6,0x5c},
	{0xd7,0x58},
	{0xd8,0x74},
	{0xd9,SP2518_P0_0xd9},
	{0xda,0x5c},
	{0xdb,0x58},
	//uv_dif	  
	{0xfd,0x00},
	{0xf3,0x03},
	{0xb0,0x00},
	{0xb1,0x23}, 
	
	//gamma1 outdoor 
	{0xfd,0x00},
	{0x8b,0x00},
	{0x8c,0x0a},
	{0x8d,0x13},
	{0x8e,0x25},
	{0x8f,0x43},
	{0x90,0x5d},
	{0x91,0x74},
	{0x92,0x88},
	{0x93,0x9a},
	{0x94,0xa9},
	{0x95,0xb5},
	{0x96,0xc0},
	{0x97,0xcb},
	{0x98,0xd3},
	{0x99,0xdd},
	{0x9a,0xe5},
	{0x9b,0xea},
	{0xfd,0x01},
	{0x8d,0xf0},
	{0x8e,0xf4},
	//gamma2
	{0xfd,0x00},
	{0x78,0x0 },
	{0x79,0xa },
	{0x7a,0x13},
	{0x7b,0x25},
	{0x7c,0x43},
	{0x7d,0x5d},
	{0x7e,0x74},
	{0x7f,0x88},
	{0x80,0x9a},
	{0x81,0xa9},
	{0x82,0xb5},
	{0x83,0xc0},
	{0x84,0xca},
	{0x85,0xd4},
	{0x86,0xdd},
	{0x87,0xe6},
	{0x88,0xef},
	{0x89,0xf7},
	{0x8a,0xff},
	//gamma_ae
	{0xfd,0x01},
	{0x96,0x46},
	{0x97,0x14},
	{0x9f,0x06},
				 
	//HEQ		  
	{0xfd,0x00},
	{0xdd,SP2518_P0_0xdd},
	{0xde,SP2518_P0_0xde},
	{0xdf,0x80},
	
	//Ytarget	   //+++  
	{0xfd,0x00},     
	{0xec,0x88},//70 sp_pxj 20140620//6a 
	{0xed,0x9e},//86 sp_pxj 20140620//7c 
	{0xee,0x88},//70 sp_pxj 20140620//65 
	{0xef,0x9e},//86 sp_pxj 20140620//78 
	{0xf7,0x98},//80 sp_pxj 20140620//78 
	{0xf8,0x8c},//74 sp_pxj 20140620//6e 
	{0xf9,0x98},//80 sp_pxj 20140620//74 
	{0xfa,0x8c},//74 sp_pxj 20140620//6a 
			   
	//sharpen	  
	{0xfd,0x01},
	{0xdf,0x0f},
	{0xe5,0x10},
	{0xe7,0x0c},//10 sp_pxj 20140620
	{0xe8,0x18},//20 sp_pxj 20140620
	{0xec,0x20},//20 sp_pxj 20140620
	{0xe9,0x14},//20 sp_pxj 20140620
	{0xed,0x14},//20 sp_pxj 20140620
	{0xea,0x0c},//10 sp_pxj 20140620
	{0xef,0x10},//10 sp_pxj 20140620
	{0xeb,0x0c},//10 sp_pxj 20140620
	{0xf0,0x10},//10 sp_pxj 20140620
	
	//gw		  
	{0xfd,0x01},
	{0x70,0x76},
	{0x7b,0x40},
	{0x81,0x30},
	
	
	//Y_offset	 
	{0xfd,0x00},
	{0xb2,SP2518_NORMAL_Y0ffset},
	{0xb3,0x1f},
	{0xb4,0x30},
	{0xb5,0x50},
	
	//CNR		  
	{0xfd,0x00},
	{0x5b,0x20},
	{0x61,0x80},
	{0x77,0x80},	//0x20	0x40 0x80//modify by sp_yjp,20120507
	{0xca,0x80},
	
	{0xab,0x00},
	{0xac,0x02},
	{0xae,0x08},
	{0xaf,0x20},
				 
	{0xfd,0x00},
	{0x32,0x0d},
	{0x33,0xcf},	//0xef	//modify by sp_yjp,20120507
	{0x34,0x7f},//0x3f kuang	0x3c //modify by sp_yjp,20120507	
	{0x35,0x00},
	
	{0x1b,0x12},//0x02 bit[4:3]ds_data
	{0xe7,0x03},
	{0xe7,0x00},
	//add xg
	{0xfd,0x01},
	{0x0e,0x00},
	{0x0f,0x00},
	
	{0xfd,0x00},
	{0x2f,0x00},	
	{0xfd,0x01}, 
	
	{0x06,0x00}, 
	{0x07,0x40}, 
	{0x08,0x00}, 
	{0x09,0x40}, 
	
	{0x0a,0x02},	//600
	{0x0b,0x58}, 
	{0x0c,0x03}, 	//800
	{0x0d,0x20}, 
	
	{0x0e,0x01},
	{0x0f,0x01},
	{0xfd,0x00},
};

static struct regval_list sensor_uxga_regs[] = {
	//Resoltion Setting : 1600*1200
	{0xfd,0x00},

	{0x47,0x00},
	{0x48,0x00},
	{0x49,0x04},	
	{0x4a,0xb0},

	{0x4b,0x00},
	{0x4c,0x00},
	{0x4d,0x06},
	{0x4e,0x40},

	{0xfd,0x01}, 	
	{0x0e,0x00},	//resize disable
	{0x0f,0x00},	//add xg
	{0xfd,0x00}, 
	{0x31,0x10},	//720P_sel disable	
	{0x2f,0x00},
};

static struct regval_list sensor_720p_regs[] = {
	//Resoltion Setting : 1600*1200
	{0xfd,0x00},
	
	{0x47,0x00},
	{0x48,0x00},
	{0x49,0x04},	
	{0x4a,0xb0},
	
	{0x4b,0x00},
	{0x4c,0x00},
	{0x4d,0x06},
	{0x4e,0x40},
	
	{0xfd,0x01}, 	
	{0x0e,0x00},	//resize disable
	{0x0f,0x00},	//add xg
	{0xfd,0x00}, 
	{0x31,0x10},	//720P_sel disable	
	{0x2f,0x00},
};

static struct regval_list sensor_svga_regs[] = {
//Resolution Setting : 800*600
	{0xfd,0x00},
	{0x47,0x00},
	{0x48,0x00},
	{0x49,0x04},	
	{0x4a,0xb0},
	
	{0x4b,0x00},
	{0x4c,0x00},
	{0x4d,0x06},
	{0x4e,0x40},
	

	{0xfd,0x01}, 	
	{0x06,0x00}, 
	{0x07,0x40}, 
	{0x08,0x00}, 
	{0x09,0x40}, 	
	{0x0a,0x02},	//600
	{0x0b,0x58}, 
	{0x0c,0x03}, 	//800
	{0x0d,0x20}, 	
	{0x0e,0x01},
	{0x0f,0x01},

	{0xfd,0x00},
	{0x2f,0x08},
	{0x31,0x10},	//720P_sel enable

};

static struct regval_list sensor_vga_regs[] = {
//Resolution Setting : 640*480
	{0xfd,0x00},

	{0x47,0x00},
	{0x48,0x00},
	{0x49,0x04},	
	{0x4a,0xb0},

	{0x4b,0x00},
	{0x4c,0x00},
	{0x4d,0x06},
	{0x4e,0x40},

	{0xfd,0x01},	
	{0x06,0x00}, 
	{0x07,0x50}, 
	{0x08,0x00}, 
	{0x09,0x50}, 	
	{0x0a,0x01},	//480
	{0x0b,0xe0}, 
	{0x0c,0x02}, 	//640
	{0x0d,0x80}, 	
	{0x0e,0x01},
	{0x0f,0x01},

	{0xfd,0x00},
	{0x2f,0x08},
	{0x31,0x10},	//720P_sel enable
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
	{0xfd,0x01},
	{0x28,0xce},
	{0x29,0x8a},
	{0xfd,0x00},
	{0x32,0x0d},
};


static struct regval_list sensor_wb_incandescence_regs[] = {
	//bai re guang
	{0xfd,0x00},
	{0x32,0x05},
	{0xfd,0x01},
	{0x28,0x7b},
	{0x29,0xd3},
	{0xfd,0x00},
};

static struct regval_list sensor_wb_fluorescent_regs[] = {
	//ri guang deng
	{0xfd,0x00},
	{0x32,0x05},
	{0xfd,0x01},
	{0x28,0xb4},
	{0x29,0xc4},
	{0xfd,0x00},
};

static struct regval_list sensor_wb_tungsten_regs[] = {
	//wu si deng
	{0xfd,0x00},
	{0x32,0x05},
	{0xfd,0x01},
	{0x28,0xae},
	{0x29,0xcc},
	{0xfd,0x00},
};

static struct regval_list sensor_wb_horizon[] = { 
//null
};

static struct regval_list sensor_wb_daylight_regs[] = {
	//tai yang guang
	{0xfd,0x00},
	{0x32,0x05},
	{0xfd,0x01},
	{0x28,0xc1},
	{0x29,0x88},
	{0xfd,0x00},
};

static struct regval_list sensor_wb_flash[] = { 
//null
};

static struct regval_list sensor_wb_cloud_regs[] = {
	{0xfd,0x00},
	{0x32,0x05},
	{0xfd,0x01},
	{0x28,0xe2},
	{0x29,0x82},
	{0xfd,0x00},
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
	{0xfd,0x00},
	{0x62,0x00},
	{0x63,0x80},
	{0x64,0x80},
};

static struct regval_list sensor_colorfx_bw_regs[] = {
	{0xfd,0x00},
	{0x62,0x20},
	{0x63,0x80},
	{0x64,0x80},
};

static struct regval_list sensor_colorfx_sepia_regs[] = {
	{0xfd,0x00},
	{0x62,0x10},
	{0x63,0xb0},
	{0x64,0x40},
};

static struct regval_list sensor_colorfx_negative_regs[] = {
	{0xfd,0x00},
	{0x62,0x04},
	{0x63,0x80},
	{0x64,0x80},
};

static struct regval_list sensor_colorfx_emboss_regs[] = {
	{0xfd,0x00},
	{0x62,0x01},
	{0x63,0x80},
	{0x64,0x80},
};

static struct regval_list sensor_colorfx_sketch_regs[] = {
	{0xfd,0x00},
	{0x62,0x40},
	{0x63,0x80},
	{0x64,0x80},
};

static struct regval_list sensor_colorfx_sky_blue_regs[] = {
	{0xfd,0x00},
	{0x62,0x10},
	{0x63,0x80},
	{0x64,0xb0},
};

static struct regval_list sensor_colorfx_grass_green_regs[] = {
	{0xfd,0x00},
	{0x62,0x10},
	{0x63,0x50},
	{0x64,0x50},
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
	{0xfd,0x00},
	{0xdc,0xc0},
};

static struct regval_list sensor_brightness_neg3_regs[] = {
	{0xfd,0x00},
	{0xdc,0xd0},
};

static struct regval_list sensor_brightness_neg2_regs[] = {
	{0xfd,0x00},
	{0xdc,0xe0},
};

static struct regval_list sensor_brightness_neg1_regs[] = {
	{0xfd,0x00},
	{0xdc,0xf0},
};

static struct regval_list sensor_brightness_zero_regs[] = {
	{0xfd,0x00},
	{0xdc,0x00},
};

static struct regval_list sensor_brightness_pos1_regs[] = {
	{0xfd,0x00},
	{0xdc,0x10},
};

static struct regval_list sensor_brightness_pos2_regs[] = {
	{0xfd,0x00},
	{0xdc,0x20},
};

static struct regval_list sensor_brightness_pos3_regs[] = {
	{0xfd,0x00},
	{0xdc,0x30},
};

static struct regval_list sensor_brightness_pos4_regs[] = {
	{0xfd,0x00},
	{0xdc,0x40},
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
	{0xfd,0x00},
	{0xde},{SP2518_P0_0xde-0x40},
};

static struct regval_list sensor_contrast_neg3_regs[] = {
	{0xfd,0x00},
	{0xde},{SP2518_P0_0xde-0x30},
};

static struct regval_list sensor_contrast_neg2_regs[] = {
	{0xfd,0x00},
	{0xde},{SP2518_P0_0xde-0x20},
};

static struct regval_list sensor_contrast_neg1_regs[] = {
	{0xfd,0x00},
	{0xde},{SP2518_P0_0xde-0x10},
};

static struct regval_list sensor_contrast_zero_regs[] = {
	{0xfd,0x00},
	{0xde},{SP2518_P0_0xde},
};

static struct regval_list sensor_contrast_pos1_regs[] = {
	{0xfd,0x00},
	{0xde},{SP2518_P0_0xde+0x10},
};

static struct regval_list sensor_contrast_pos2_regs[] = {
	{0xfd,0x00},
	{0xde},{SP2518_P0_0xde+0x20},
};

static struct regval_list sensor_contrast_pos3_regs[] = {
	{0xfd,0x00},
	{0xde},{SP2518_P0_0xde+0x30},
};

static struct regval_list sensor_contrast_pos4_regs[] = {
	{0xfd,0x00},
	{0xde},{SP2518_P0_0xde+0x40},
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
	{0xfd,0x00},
	{0xd4},{SP2518_P0_0xd4-0x10},
	{0xd9},{SP2518_P0_0xd9-0x10},
};

static struct regval_list sensor_saturation_neg3_regs[] = {
	{0xfd,0x00},
	{0xd4},{SP2518_P0_0xd4-0x20},
	{0xd9},{SP2518_P0_0xd9-0x20},
};

static struct regval_list sensor_saturation_neg2_regs[] = {
	{0xfd,0x00},
	{0xd4},{SP2518_P0_0xd4-0x30},
	{0xd9},{SP2518_P0_0xd9-0x30},
};

static struct regval_list sensor_saturation_neg1_regs[] = {
	{0xfd,0x00},
	{0xd4},{SP2518_P0_0xd4-0x40},
	{0xd9},{SP2518_P0_0xd9-0x40},
};

static struct regval_list sensor_saturation_zero_regs[] = {
	{0xfd,0x00},
	{0xd4},{SP2518_P0_0xd4},
	{0xd9},{SP2518_P0_0xd9},
};

static struct regval_list sensor_saturation_pos1_regs[] = {
	{0xfd,0x00},
	{0xd4},{SP2518_P0_0xd4+0x10},
	{0xd9},{SP2518_P0_0xd9+0x10},
};

static struct regval_list sensor_saturation_pos2_regs[] = {
	{0xfd,0x00},
	{0xd4},{SP2518_P0_0xd4+0x20},
	{0xd9},{SP2518_P0_0xd9+0x20},
};

static struct regval_list sensor_saturation_pos3_regs[] = {
	{0xfd,0x00},
	{0xd4},{SP2518_P0_0xd4+0x30},
	{0xd9},{SP2518_P0_0xd9+0x30},
};

static struct regval_list sensor_saturation_pos4_regs[] = {
	{0xfd,0x00},
	{0xd4},{SP2518_P0_0xd4+0x40},
	{0xd9},{SP2518_P0_0xd9+0x40},
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
	{0xfd,0x00},
	{0xdc,0xc0},
};

static struct regval_list sensor_ev_neg3_regs[] = {
	{0xfd,0x00},
	{0xdc,0xd0},
};

static struct regval_list sensor_ev_neg2_regs[] = {
	{0xfd,0x00},
	{0xdc,0xe0},
};

static struct regval_list sensor_ev_neg1_regs[] = {
	{0xfd,0x00},
	{0xdc,0xf0},
};                     

static struct regval_list sensor_ev_zero_regs[] = {
	{0xfd,0x00},
	{0xdc,0x00},
};

static struct regval_list sensor_ev_pos1_regs[] = {
	{0xfd,0x00},
	{0xdc,0x10},
};

static struct regval_list sensor_ev_pos2_regs[] = {
	{0xfd,0x00},
	{0xdc,0x20},
};

static struct regval_list sensor_ev_pos3_regs[] = {
	{0xfd,0x00},
	{0xdc,0x30},
};

static struct regval_list sensor_ev_pos4_regs[] = {
	{0xfd,0x00},
	{0xdc,0x40},
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
	//YCbYCr
	{0xfd,0x00},
	{0x35,0x40},
};


static struct regval_list sensor_fmt_yuv422_yvyu[] = {
	//YCrYCb
	{0xfd,0x00},
	{0x35,0x41},
};

static struct regval_list sensor_fmt_yuv422_vyuy[] = {
	//CrYCbY
	{0xfd,0x00},
	{0x35,0x01},
};

static struct regval_list sensor_fmt_yuv422_uyvy[] = {
	//CbYCrY
	{0xfd,0x00},
	{0x35,0x00},
};

static int sensor_s_hflip_vflip(struct v4l2_subdev *sd, int h_value,int v_value)
{
	struct sensor_info *info = to_state(sd);
	data_type rdval;
	LOG_ERR_RET(sensor_write(sd, 0xfd, 0x00)) //page 0
	LOG_ERR_RET(sensor_read(sd, 0x31, &rdval))
	switch (h_value) {
	case 0:
		rdval &= 0xdf;
		break;
	case 1:
		rdval |= 0x20;
		break;
	default:
		return -EINVAL;
	}
  
	switch (v_value) {
		case 0:
			rdval &= 0xbf;
			break;
		case 1:
			rdval |= 0x40;
			break;
		default:
			return -EINVAL;
	}
	LOG_ERR_RET(sensor_write(sd, 0x31, rdval))

	usleep_range(10000,12000);
	info->hflip = h_value;
	info->vflip = v_value;
	return 0;
}

static int sensor_g_hflip(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);
	data_type rdval;
  
	LOG_ERR_RET(sensor_write(sd, 0xfd, 0x00)) //page 0
	LOG_ERR_RET(sensor_read(sd, 0x31, &rdval))
  
	rdval &= (1<<5);  
	*value = (rdval>>5);
  
	info->vflip = *value;
	return 0;
}

static int sensor_s_hflip(struct v4l2_subdev *sd, int value)
{
	struct sensor_info *info = to_state(sd);
	data_type rdval;
      
	LOG_ERR_RET(sensor_write(sd, 0xfd, 0x00)) //page 0 
	LOG_ERR_RET(sensor_read(sd, 0x31, &rdval))

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

	LOG_ERR_RET(sensor_write(sd, 0x31, rdval))

	usleep_range(10000,12000);
	info->hflip = value;
	return 0;
}

static int sensor_g_vflip(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);
	data_type rdval;
  
	LOG_ERR_RET(sensor_write(sd, 0xfd, 0x00)) //page 0
	LOG_ERR_RET(sensor_read(sd, 0x31, &rdval))
  
	rdval &= (1<<6);  
	*value = (rdval>>6);
  
	info->vflip = *value;
	return 0;
}

static int sensor_s_vflip(struct v4l2_subdev *sd, int value)
{
	struct sensor_info *info = to_state(sd);
	data_type rdval;
  
	LOG_ERR_RET(sensor_write(sd, 0xfd, 0x00)) //page 0
	LOG_ERR_RET(sensor_read(sd, 0x31, &rdval))

	switch (value) {
		case 0:
			rdval &= 0xbf;
			break;
		case 1:
			rdval |= 0x40;
			break;
		default:
			return -EINVAL;
	}

	LOG_ERR_RET(sensor_write(sd, 0x31, rdval))

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
	return 0;
}

static int sensor_s_autoexp(struct v4l2_subdev *sd,
		enum v4l2_exposure_auto_type value)
{
	return 0;
}

static int sensor_g_autowb(struct v4l2_subdev *sd, int *value)
{
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
	
	ret = sensor_read(sd, 0x32, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_s_autowb!\n");
		return ret;
	}

	switch(value) {
	case 0:
		val &= 0xef;
		break;
	case 1:
		val |= 0x10;
		break;
	default:
		break;
	}	
	ret = sensor_write(sd, 0x32, val);
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
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			msleep(100);
			vfe_set_mclk(sd,OFF);
			usleep_range(10000,12000);
			break;
		case CSI_SUBDEV_STBY_OFF:
			vfe_dev_dbg("CSI_SUBDEV_STBY_OFF\n");
			vfe_set_mclk_freq(sd,MCLK);
			vfe_set_mclk(sd,ON);
			usleep_range(30000,31000);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
			msleep(100);
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
			vfe_set_pmu_channel(sd,AVDD,ON);
			usleep_range(10000,12000);
			vfe_set_pmu_channel(sd,IOVDD,ON);
			usleep_range(10000,12000);
			vfe_set_pmu_channel(sd,DVDD,ON);
			usleep_range(10000,12000);
			vfe_set_pmu_channel(sd,AFVDD,ON);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
			usleep_range(5000,10000);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);
			usleep_range(30000,31000);
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			usleep_range(30000,31000);
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);
			usleep_range(30000,31000);
			break;
		case CSI_SUBDEV_PWR_OFF:
			vfe_dev_dbg("CSI_SUBDEV_PWR_OFF\n");
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			usleep_range(10000,12000);				
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,POWER_EN,CSI_GPIO_LOW);
			vfe_set_pmu_channel(sd,AFVDD,OFF);					
			usleep_range(10000,12000);  
			vfe_set_pmu_channel(sd,DVDD,OFF);
			usleep_range(10000,12000);
			vfe_set_pmu_channel(sd,AVDD,OFF);
			usleep_range(10000,12000);
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
	int ret;
	data_type val;
	
	ret = sensor_read(sd, 0x02, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_detect!\n");
		return ret;
	}

	if(val != 0x53)
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
	return -EINVAL;
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


static struct sensor_win_size 
sensor_win_sizes[] = {
	/* UXGA */
	{
		.width      = UXGA_WIDTH,
		.height     = UXGA_HEIGHT,
		.hoffset    = 0,
		.voffset    = 0,
		.regs       = sensor_uxga_regs,
		.regs_size  = ARRAY_SIZE(sensor_uxga_regs),
		.set_size   = NULL,
	},
	/* 720p */
	{
		.width      = HD720_WIDTH,
		.height     = HD720_HEIGHT,
		.hoffset    = 0,
		.voffset    = 0,
		.regs				= sensor_720p_regs,
		.regs_size	= ARRAY_SIZE(sensor_720p_regs),
		.set_size   = NULL,
	},
	/* SVGA */
	{
		.width      = SVGA_WIDTH,
		.height     = SVGA_HEIGHT,
		.hoffset    = 0,
		.voffset    = 0,
		.regs       = sensor_svga_regs,
		.regs_size  = ARRAY_SIZE(sensor_svga_regs),
		.set_size   = NULL,
	},
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
	sensor_s_hflip_vflip(sd,info->hflip,info->vflip);	
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
	} else {
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

	//client->addr=0x60>>1;

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
