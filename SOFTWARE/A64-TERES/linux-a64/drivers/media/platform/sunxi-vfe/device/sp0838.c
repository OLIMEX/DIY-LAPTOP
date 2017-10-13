/*
 * A V4L2 driver for Superpix SP0838 cameras.
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

MODULE_AUTHOR("EricYuan");
MODULE_DESCRIPTION("A low-level driver for Superpix SP0838 sensors");
MODULE_LICENSE("GPL");



//for internel driver debug
#define DEV_DBG_EN   		0
#if(DEV_DBG_EN == 1)		
#define vfe_dev_dbg(x,arg...) printk("[CSI_DEBUG][SP0838]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...) 
#endif
#define vfe_dev_err(x,arg...) printk("[CSI_ERR][SP0838]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[CSI][SP0838]"x,##arg)

#define LOG_ERR_RET(x)  { \
                          int ret;  \
                          ret = x; \
                          if(ret < 0) {\
                            vfe_dev_err("error at %s\n",__func__);  \
                            return ret; \
                          } \
                        }

//define module timing
#define MCLK (24*1000*1000)
#define VREF_POL          V4L2_MBUS_VSYNC_ACTIVE_HIGH
#define HREF_POL          V4L2_MBUS_HSYNC_ACTIVE_HIGH
#define CLK_POL           V4L2_MBUS_PCLK_SAMPLE_RISING
#define V4L2_IDENT_SENSOR 0x0838


#define AWB_EN   (1<<4)
#define AWB_DIS	(~(1<<4))
#define AE_EN   (1<<0)
#define AE_DIS	(~(1<<0))



#define VGA_WIDTH		640
#define VGA_HEIGHT		480
#define QVGA_WIDTH		320
#define QVGA_HEIGHT	240
#define CIF_WIDTH		352
#define CIF_HEIGHT		288
#define QCIF_WIDTH		176
#define	QCIF_HEIGHT	144

/*
 * Our nominal (default) frame rate.
 */
#define SENSOR_FRAME_RATE 10

/*
 * The sp0838 sits on i2c with ID 0x30
 */
#define I2C_ADDR 0x30
#define SENSOR_NAME "sp0838"
//HEQ
#define  Pre_Value_P0_0xdd  0x70
#define  Pre_Value_P0_0xde  0x90

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

static struct regval_list sensor_default_regs[] = 
{
	//[Sensor]
	{0xfd,0x00}, //P0
	{0x1B,0x02},
	{0x27,0xe8},
	{0x28,0x0B},
	{0x32,0x00},
	{0x22,0xc0},
	{0x26,0x10}, 
	{0x31,0x10},   //Upside/mirr/Pclk inv/sub
	{0x5f,0x11},   //Bayer order
	{0xfd,0x01},   //P1
	{0x25,0x1a},   //Awb start
	{0x26,0xfb}, 
	{0x28,0x75}, 
	{0x29,0x4e},
	
	{0xfd,0x00},   
	{0xe7,0x03}, 
	{0xe7,0x00}, 
	{0xfd,0x01},
	
	{0x31,0x60},//64
	{0x32,0x18},
	{0x4d,0xdc},
	{0x4e,0x53},
	{0x41,0x8c},
	{0x42,0x57},
	{0x55,0xff},
	{0x56,0x00},
	{0x59,0x82},
	{0x5a,0x00},
	{0x5d,0xff},
	{0x5e,0x6f},
	{0x57,0xff},
	{0x58,0x00},
	{0x5b,0xff},
	{0x5c,0xa8},
	{0x5f,0x75},
	{0x60,0x00},
	{0x2d,0x00},
	{0x2e,0x00},
	{0x2f,0x00},
	{0x30,0x00},
	{0x33,0x00},
	{0x34,0x00},
	{0x37,0x00},
	{0x38,0x00},  //awb end
	{0xfd,0x00},  //P0
	{0x33,0x6f},  //LSC BPC EN
	{0x51,0x3f},  //BPC debug start
	{0x52,0x09},  
	{0x53,0x00},  
	{0x54,0x00},
	{0x55,0x10},  //BPC debug end
	{0x4f,0x08},  //blueedge
	{0x50,0x08},
	{0x57,0x10},  //Raw filter debut start
	{0x58,0x10},
	{0x59,0x10},
	{0x56,0x71},
	{0x5a,0x02},
	{0x5b,0x05},
	{0x5c,0x28},  //Raw filter debut end 
	{0x65,0x04},  //Sharpness debug start
	{0x66,0x01},
	{0x67,0x03},
	{0x68,0x45},
	{0x69,0x7f},
	{0x6a,0x01},
	{0x6b,0x06},//zyy20130709
	{0x6c,0x01},
	{0x6d,0x03},  //Edge gain normal
	{0x6e,0x43},  //Edge gain normal
	{0x6f,0x7f},
	{0x70,0x01},
	{0x71,0x08},  //����ֵ           
	{0x72,0x01},  //��������ֵ         
	{0x73,0x03},  //��Ե��������ֵ     
	{0x74,0x43},  //��Ե��������ֵ     
	{0x75,0x7f},  //ʹ��λ             
	{0x76,0x01},  //Sharpness debug end
	{0xcb,0x07},  //HEQ&Saturation debug start 
	{0xcc,0x04},
	{0xce,0xff},
	{0xcf,0x10},
	{0xd0,0x20},
	{0xd1,0x00},
	{0xd2,0x1c},
	{0xd3,0x16},
	{0xd4,0x00},
	{0xd6,0x1c},
	{0xd7,0x16},
	{0xdd,0x70},  //Contrast
	{0xde,0x94},  //HEQ&Saturation debug end
	{0x7f,0xd7},  //Color Correction start
	{0x80,0xbc},                          
	{0x81,0xed},                          
	{0x82,0xd7},                          
	{0x83,0xd4},                          
	{0x84,0xd6},                          
	{0x85,0xff},                          
	{0x86,0x89},                          
	{0x87,0xf8},                          
	{0x88,0x3c},                          
	{0x89,0x33},                          
	{0x8a,0x0f},  //Color Correction end 
	{0x8b,0x00},  //gamma start
	{0x8c,0x1a},               
	{0x8d,0x29},               
	{0x8e,0x41},               
	{0x8f,0x62},               
	{0x90,0x7c},               
	{0x91,0x90},               
	{0x92,0xa2},               
	{0x93,0xaf},               
	{0x94,0xbc},               
	{0x95,0xc5},               
	{0x96,0xcd},               
	{0x97,0xd5},               
	{0x98,0xdd},               
	{0x99,0xe5},               
	{0x9a,0xed},               
	{0x9b,0xf5},               
	{0xfd,0x01},  //P1         
	{0x8d,0xfd},               
	{0x8e,0xff},  //gamma end  
	{0xfd,0x00},  //P0
	{0xca,0xcf},
	{0xd8,0x48},  //UV outdoor
	{0xd9,0x48},  //UV indoor 
	{0xda,0x44},  //UV dummy//zyy
	{0xdb,0x40},  //UV lowlight
	{0xb9,0x00},  //Ygamma start
	{0xba,0x04},
	{0xbb,0x08},
	{0xbc,0x10},
	{0xbd,0x20},
	{0xbe,0x30},
	{0xbf,0x40},
	{0xc0,0x50},
	{0xc1,0x60},
	{0xc2,0x70},
	{0xc3,0x80},
	{0xc4,0x90},
	{0xc5,0xA0},
	{0xc6,0xB0},
	{0xc7,0xC0},
	{0xc8,0xD0},
	{0xc9,0xE0},
	{0xfd,0x01},  //P1
	{0x89,0xf0},
	{0x8a,0xff},  //Ygamma end
	{0xfd,0x00},  //P0
	{0xe8,0x30},  //AEdebug start
	{0xe9,0x30},
	{0xea,0x40},  //Alc Window sel
	{0xf4,0x1b},  //outdoor mode sel
	{0xf5,0x80},
	{0xf7,0x78},  //AE target
	{0xf8,0x63},  
	{0xf9,0x68},  //AE target
	{0xfa,0x53},
	{0xfd,0x01},  //P1
	{0x09,0x31},  //AE Step 3.0
	{0x0a,0x85},
	{0x0b,0x0b},  //AE Step 3.0
	{0x14,0x20},
	{0x15,0x0f},
	
	//caprure preview daylight 24M 50hz 20-8FPS maxgain:0x70	
	{0xfd,0x00},
	{0x05,0x00},
	{0x06,0x00},
	{0x09,0x01},
	{0x0a,0x76},
	{0xf0,0x62},
	{0xf1,0x00},
	{0xf2,0x5f},
	{0xf5,0x78},
	{0xfd,0x01},
	{0x00,0xb2},
	{0x0f,0x60},
	{0x16,0x60},
	{0x17,0xa2},
	{0x18,0xaa},
	{0x1b,0x60},
	{0x1c,0xaa},
	{0xb4,0x20},
	{0xb5,0x3a},
	{0xb6,0x5e},
	{0xb9,0x40},
	{0xba,0x4f},
	{0xbb,0x47},
	{0xbc,0x45},
	{0xbd,0x43},
	{0xbe,0x42},
	{0xbf,0x42},
	{0xc0,0x42},
	{0xc1,0x41},
	{0xc2,0x41},
	{0xc3,0x41},
	{0xc4,0x41},
	{0xc5,0x70},
	{0xc6,0x41},
	{0xca,0x70},
	{0xcb,0xc },
	
	{0xfd,0x00},  //P0
	{0x32,0x15},  //Auto_mode set
	{0x34,0x66},  //Isp_mode set
	{0x35,0x00},  //out format
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
	//sp0838_reg_WB_auto    
	{0xfd, 0x01},                                                          
	{0x28, 0x75},		                                                       
	{0x29, 0x4e},
	{0xfd, 0x00},  // AUTO 3000K~7000K                                     
	{0x32, 0x15}, 
	{0xfd, 0x00},
};

static struct regval_list sensor_wb_incandescence_regs[] = {
	//bai re guang
	{0xfd, 0x00},                                    
	{0x32, 0x05},                                                          
	{0xfd, 0x01},                                                          
	{0x28, 0x41},		                                                       
	{0x29, 0x71},		                                                       
	{0xfd, 0x00},
};

static struct regval_list sensor_wb_fluorescent_regs[] = {
	//ri guang deng
	{0xfd, 0x00},                                  
	{0x32, 0x05},                                                          
	{0xfd, 0x01},                                                          
	{0x28, 0x5a},		                                                       
	{0x29, 0x62},		                                                       
	{0xfd, 0x00},
};

static struct regval_list sensor_wb_tungsten_regs[] = {
	//wu si deng
	{0xfd, 0x00},                                  
	{0x32, 0x05},                                                          
	{0xfd, 0x01},                                                          
	{0x28, 0x57},		                                                       
	{0x29, 0x66},		                                                       
	{0xfd, 0x00},
};

static struct regval_list sensor_wb_horizon[] = { 
//null
};

static struct regval_list sensor_wb_daylight_regs[] = {
	//tai yang guang
	{0xfd, 0x00},                                  
	{0x32, 0x05},                                                          
	{0xfd, 0x01},                                                          
	{0x28, 0x6b},		                                                       
	{0x29, 0x4b},		                                                       
	{0xfd, 0x00},
};

static struct regval_list sensor_wb_flash[] = { 
//null
};


static struct regval_list sensor_wb_cloud_regs[] = {
	{0xfd, 0x00},                                  
	{0x32, 0x05},                                                          
	{0xfd, 0x01},                                                          
	{0x28, 0x71},		                                                       
	{0x29, 0x41},		                                                       
	{0xfd, 0x00},
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
	{0xfd, 0x00},
	{0x62, 0x00},
	{0x63, 0x80},
	{0x64, 0x80},
	{0xfd, 0x00}
};

static struct regval_list sensor_colorfx_bw_regs[] = {
	{0xfd, 0x00},
	{0x62, 0x40},
	{0x63, 0x80},
	{0x64, 0x80},
	{0xfd, 0x00}
};

static struct regval_list sensor_colorfx_sepia_regs[] = {
	{0xfd, 0x00},
	{0x62, 0x20},
	{0x63, 0xc0},
	{0x64, 0x20},
	{0xfd, 0x00}
};

static struct regval_list sensor_colorfx_negative_regs[] = {
	{0xfd, 0x00},
	{0x62, 0x10},
	{0x63, 0x80},
	{0x64, 0x80},
	{0xfd, 0x00}
};

static struct regval_list sensor_colorfx_emboss_regs[] = {
	//null
};

static struct regval_list sensor_colorfx_sketch_regs[] = {
	//null
};

static struct regval_list sensor_colorfx_sky_blue_regs[] = {
	{0xfd, 0x00},
	{0x62, 0x20},
	{0x63, 0x20},
	{0x64, 0xf0},
	{0xfd, 0x00}
};

static struct regval_list sensor_colorfx_grass_green_regs[] = {
	{0xfd, 0x00},
	{0x62, 0x20},
	{0x63, 0x20},
	{0x64, 0x20},
	{0xfd, 0x00}
};

static struct regval_list sensor_colorfx_skin_whiten_regs[] = {
	//null
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
	{0xfd, 0x00},
	{0xdc, 0xc0},
};

static struct regval_list sensor_brightness_neg3_regs[] = {
	{0xfd, 0x00},
	{0xdc, 0xd0},
};

static struct regval_list sensor_brightness_neg2_regs[] = {
	{0xfd, 0x00},
	{0xdc, 0xe0},
};

static struct regval_list sensor_brightness_neg1_regs[] = {
	{0xfd, 0x00},
	{0xdc, 0xf0},
};

static struct regval_list sensor_brightness_zero_regs[] = {
	{0xfd, 0x00},
	{0xdc, 0x00},
};

static struct regval_list sensor_brightness_pos1_regs[] = {
	{0xfd, 0x00},
	{0xdc, 0x10},
};

static struct regval_list sensor_brightness_pos2_regs[] = {
	{0xfd, 0x00},
	{0xdc, 0x20},
};

static struct regval_list sensor_brightness_pos3_regs[] = {
	{0xfd, 0x00},
	{0xdc, 0x30},
};

static struct regval_list sensor_brightness_pos4_regs[] = {
	{0xfd, 0x00},
	{0xdc, 0x40},
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
	{0xfd, 0x00},
	{0xdd,  Pre_Value_P0_0xdd-0x40},	//level -4
	{0xde,  Pre_Value_P0_0xde-0x40}
};

static struct regval_list sensor_contrast_neg3_regs[] = {
	{0xfd, 0x00},
	{0xdd,  Pre_Value_P0_0xdd-0x30},	//level -3
	{0xde,  Pre_Value_P0_0xde-0x30}
};

static struct regval_list sensor_contrast_neg2_regs[] = {
	{0xfd, 0x00},
	{0xdd,  Pre_Value_P0_0xdd-0x20},	//level -2
	{0xde,  Pre_Value_P0_0xde-0x20}
};

static struct regval_list sensor_contrast_neg1_regs[] = {
	{0xfd, 0x00},
	{0xdd,  Pre_Value_P0_0xdd-0x10},	//level -1
	{0xde,  Pre_Value_P0_0xde-0x10}
};

static struct regval_list sensor_contrast_zero_regs[] = {
	{0xfd, 0x00},
	{0xdd,  Pre_Value_P0_0xdd},		//level 0
	{0xde,  Pre_Value_P0_0xde},
};

static struct regval_list sensor_contrast_pos1_regs[] = {
	{0xfd, 0x00},
	{0xdd,  Pre_Value_P0_0xdd+0x10},	//level +1
	{0xde,  Pre_Value_P0_0xde+0x10}
};

static struct regval_list sensor_contrast_pos2_regs[] = {
	{0xfd, 0x00},
	{0xdd,  Pre_Value_P0_0xdd+0x20},	//level +2
	{0xde,  Pre_Value_P0_0xde+0x20}
};

static struct regval_list sensor_contrast_pos3_regs[] = {
	{0xfd, 0x00},
	{0xdd,  Pre_Value_P0_0xdd+0x30},	//level +3
	{0xde,  Pre_Value_P0_0xde+0x30}
};

static struct regval_list sensor_contrast_pos4_regs[] = {
	{0xfd, 0x00},
	{0xdd,  Pre_Value_P0_0xdd+0x40},	//level +4
	{0xde,  Pre_Value_P0_0xde+0x40}
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
	//null
};

static struct regval_list sensor_saturation_neg3_regs[] = {
	//null

};

static struct regval_list sensor_saturation_neg2_regs[] = {
	//null

};

static struct regval_list sensor_saturation_neg1_regs[] = {
	//null

};

static struct regval_list sensor_saturation_zero_regs[] = {
	//null
};

static struct regval_list sensor_saturation_pos1_regs[] = {
	//null
};

static struct regval_list sensor_saturation_pos2_regs[] = {
	//null
};

static struct regval_list sensor_saturation_pos3_regs[] = {
	//null
};

static struct regval_list sensor_saturation_pos4_regs[] = {
	//null
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
	{0xfd, 0x00},
	{0xdc, 0xc0},
};

static struct regval_list sensor_ev_neg3_regs[] = {
	{0xfd, 0x00},
	{0xdc, 0xd0},
};

static struct regval_list sensor_ev_neg2_regs[] = {
	{0xfd, 0x00},
	{0xdc, 0xe0},
};

static struct regval_list sensor_ev_neg1_regs[] = {
	{0xfd, 0x00},
	{0xdc, 0xf0},
};

static struct regval_list sensor_ev_zero_regs[] = {
	{0xfd, 0x00},
	{0xdc, 0x00},
};

static struct regval_list sensor_ev_pos1_regs[] = {
	{0xfd, 0x00},
	{0xdc, 0x10},
};

static struct regval_list sensor_ev_pos2_regs[] = {
	{0xfd, 0x00},
	{0xdc, 0x20},
};

static struct regval_list sensor_ev_pos3_regs[] = {
	{0xfd, 0x00},
	{0xdc, 0x30},
};

static struct regval_list sensor_ev_pos4_regs[] = {
	{0xfd, 0x00},
	{0xdc, 0x40},
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
	{0xfd, 0x00},	//YCbYCr
	{0x35, 0x40}
};

static struct regval_list sensor_fmt_yuv422_yvyu[] = {
	{0xfd, 0x00},	//YCrYCb
	{0x35, 0x40}
};

static struct regval_list sensor_fmt_yuv422_vyuy[] = {
	{0xfd, 0x00},	//CrYCbY
	{0x35, 0x01}
};

static struct regval_list sensor_fmt_yuv422_uyvy[] = {
	{0xfd, 0x00},	//CbYCrY
	{0x35, 0x00}
};

static struct regval_list sensor_fmt_raw[] = {
	{0xfd, 0x00},	//raw
	{0x35, 0x20}
};





static int sensor_g_hflip(struct v4l2_subdev *sd, __s32 *value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	data_type val;
	
	ret = sensor_write(sd, 0xfd, 0x00);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_g_hflip!\n");
		return ret;
	}
	
	ret = sensor_read(sd, 0x31, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_g_hflip!\n");
		return ret;
	}
	
	val &= (1<<5);
	val = val>>5;		//0x31 bit5 is mirror
		
	*value = val;

	info->hflip = *value;
	return 0;
}

static int sensor_s_hflip(struct v4l2_subdev *sd, int value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	data_type val;
	
    msleep(100);
	ret = sensor_write(sd, 0xfd, 0x00);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_hflip!\n");
		return ret;
	}
	
    msleep(100);
	ret = sensor_read(sd, 0x31, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_s_hflip!\n");
		return ret;
	}
	
    msleep(100);
	switch (value) {
		case 0:
			val &= 0xdf;
			break;
		case 1:
			val |= 0x20;
			break;
		default:
			return -EINVAL;
	}
	ret = sensor_write(sd, 0x31, val);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_hflip!\n");
		return ret;
	}
	
	msleep(10);
	
	info->hflip = value;
	return 0;
}

static int sensor_g_vflip(struct v4l2_subdev *sd, __s32 *value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	data_type val;
	
	ret = sensor_write(sd, 0xfd, 0x00);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_g_vflip!\n");
		return ret;
	}
	
	ret = sensor_read(sd, 0x31, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_g_vflip!\n");
		return ret;
	}
	
	val &= (1<<6);
	val = val>>6;		//0x31 bit6 is upsidedown
		
	*value = val;

	info->vflip = *value;
	return 0;
}

static int sensor_s_vflip(struct v4l2_subdev *sd, int value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	data_type val;
	
    msleep(100);
	ret = sensor_write(sd, 0xfd, 0x00);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_vflip!\n");
		return ret;
	}
	
    msleep(100);
	ret = sensor_read(sd, 0x31, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_s_vflip!\n");
		return ret;
	}
	
    msleep(100);
	switch (value) {
		case 0:
			val &= 0xbf;
			break;
		case 1:
			val |= 0x40;
			break;
		default:
			return -EINVAL;
	}
	ret = sensor_write(sd, 0x31, val);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_vflip!\n");
		return ret;
	}
	
	msleep(10);
	
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
	
	ret = sensor_write(sd, 0xfd, 0x00);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_g_autoexp!\n");
		return ret;
	}
	
	ret = sensor_read(sd, 0x32, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_g_autoexp!\n");
		return ret;
	}

	val &= 0x01;
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
	
	ret = sensor_write(sd, 0xfd, 0x00);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_autoexp!\n");
		return ret;
	}
	
	ret = sensor_read(sd, 0x32, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_s_autoexp!\n");
		return ret;
	}

	switch (value) {
		case V4L2_EXPOSURE_AUTO:
		  val |= AE_EN;
			break;
		case V4L2_EXPOSURE_MANUAL:
			val &= AE_DIS;
			break;
		case V4L2_EXPOSURE_SHUTTER_PRIORITY:
			return -EINVAL;    
		case V4L2_EXPOSURE_APERTURE_PRIORITY:
			return -EINVAL;
		default:
			return -EINVAL;
	}
		
	ret = sensor_write(sd, 0x32, val);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_autoexp!\n");
		return ret;
	}
	
	msleep(10);
	
	info->autoexp = value;
	return 0;
}

static int sensor_g_autowb(struct v4l2_subdev *sd, int *value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	data_type val;
	
	ret = sensor_write(sd, 0xfd, 0x00);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_g_autowb!\n");
		return ret;
	}
	
	ret = sensor_read(sd, 0x32, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_g_autowb!\n");
		return ret;
	}

	val &= (1<<4);
	val = val>>4;		//0x22 bit1 is awb enable
		
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
	
	ret = sensor_write(sd, 0xfd, 0x00);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_autowb!\n");
		return ret;
	}
	ret = sensor_read(sd, 0x32, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_s_autowb!\n");
		return ret;
	}

	switch(value) {
	case 0:
		val &= AWB_DIS;
		break;
	case 1:
		val |= AWB_EN;
		break;
	default:
		break;
	}	
	ret = sensor_write(sd, 0x32, val);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_autowb!\n");
		return ret;
	}
	
	msleep(10);
	
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
			usleep_range(30000,31000);
			vfe_set_mclk(sd,OFF);
			break;
		case CSI_SUBDEV_STBY_OFF:
			vfe_dev_dbg("CSI_SUBDEV_STBY_OFF\n");
			vfe_set_mclk_freq(sd,MCLK);
			vfe_set_mclk(sd,ON);
			usleep_range(30000,31000);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
			usleep_range(10000,12000);
			break;
		case CSI_SUBDEV_PWR_ON:
			vfe_dev_dbg("CSI_SUBDEV_PWR_ON\n");
			vfe_gpio_set_status(sd,PWDN,1);//set the gpio to output
			vfe_gpio_set_status(sd,RESET,1);//set the gpio to output
			usleep_range(10000,12000);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			vfe_gpio_write(sd,POWER_EN,CSI_GPIO_HIGH);
			vfe_set_pmu_channel(sd,AVDD,ON);
			vfe_set_pmu_channel(sd,IOVDD,ON);
			vfe_set_pmu_channel(sd,DVDD,ON);
			vfe_set_pmu_channel(sd,AFVDD,ON);
			usleep_range(20000,22000);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
			usleep_range(10000,12000);
			vfe_set_mclk_freq(sd,MCLK);
			vfe_set_mclk(sd,ON);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			usleep_range(30000,31000);
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);
			usleep_range(30000,31000);
			break;
		case CSI_SUBDEV_PWR_OFF:
			vfe_dev_dbg("CSI_SUBDEV_PWR_OFF\n");
			usleep_range(10000,12000);
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			usleep_range(10000,12000);
			vfe_set_mclk(sd,OFF);
			vfe_gpio_write(sd,POWER_EN,CSI_GPIO_LOW);
			vfe_set_pmu_channel(sd,AFVDD,OFF);
			vfe_set_pmu_channel(sd,DVDD,OFF);
			vfe_set_pmu_channel(sd,AVDD,OFF);
			vfe_set_pmu_channel(sd,IOVDD,OFF);  
			usleep_range(10000,12000);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			usleep_range(10000,12000);
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

	ret = sensor_write(sd, 0xfd, 0x00);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_detect!\n");
		return ret;
	}

	ret = sensor_read(sd, 0x02, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_detect!\n");
		return ret;
	}

	if(val != 0x27)
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


static struct sensor_win_size sensor_win_sizes[] = {
	/* VGA */
	{
		.width      = VGA_WIDTH,
		.height     = VGA_HEIGHT,
		.hoffset    = 0,
		.voffset    = 0,
		.regs       = NULL,
		.regs_size  = 0,
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
	if(client)
	{
		client->addr=0x30>>1;
	}

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

