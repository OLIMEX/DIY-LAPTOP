/*
 * A V4L2 driver for ar0330_mipi Raw cameras.
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
MODULE_DESCRIPTION("A low-level driver for Aptina ar0330_mipi Raw sensors");
MODULE_LICENSE("GPL");

//for internel driver debug
#define DEV_DBG_EN      0 
#if(DEV_DBG_EN == 1)    
#define vfe_dev_dbg(x,arg...) printk("[ar0330_mipi Raw]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...) 
#endif
#define vfe_dev_err(x,arg...) printk("[ar0330_mipi Raw]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[ar0330_mipi Raw]"x,##arg)

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
#define V4L2_IDENT_SENSOR  0x0330

/*
 *Our nominal (default) frame rate.
 */

#define SENSOR_FRAME_RATE 30


/*
 * The ar0330_mipi i2c address
 */
#define I2C_ADDR 0x20
#define SENSOR_NAME "ar0330_mipi"

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

static struct regval_list sensor_default_regs[] = 
{
    {0x301a,0x0059},	//Reset Sensor
    {REG_DLY,0x0064},
    {0x31AE,0x0202},  //Output Interface Configured to 2lane MIPI
    {0x301A,0x0058},//Disable Streaming
    {REG_DLY,0x0032},
    {0x3064,0x1802},
    {0x3078,0x0001},  //Marker to say that 'Defaults' have been run
    {0x31e0,0x0003},

    //Toggle Flash on Each Frame
    {0x3046,0x4038},  // Enable Flash Pin
    {0x3048,0x8480},  // Flash Pulse Length
    {0x31E0,0x0203},  //OTPM V5
    {0x3ED2,0x0146},
    {0x3EDA,0x88BC},
    {0x3EDC,0xAA63},
    {0x305E,0x00A0},

    //PLL_settings 588Mbps 98Mhz
    //STATE = Master Clock,98000000
    {0x302A,0x0006},     //VT_PIX_CLK_DIV = 6
    {0x302C,0x0002},     //VT_SYS_CLK_DIV = 2
    {0x302E,0x0002},     //PRE_PLL_CLK_DIV = 2
    {0x3030,0x0031},     //PLL_MULTIPLIER = 49
    {0x3036,0x000C},     //OP_PIX_CLK_DIV = 12
    {0x3038,0x0001},     //OP_SYS_CLK_DIV = 1
    {0x31AC,0x0C0C},     //DATA_FORMAT_BITS

    //MIPI Port Timing continuous mode
    {0x31B0,0x002d},
    {0x31B2,0x0012},
    {0x31B4,0x3b44},
    {0x31B6,0x314d},
    {0x31B8,0x2089},
    {0x31BA,0x0206},
    {0x31BC,0x8005},
    {0x31BE,0x2003},

    //Timing_settings
    {0x3002, 0x0078},  //Y_ADDR_START = 120
    {0x3004, 0x0006},  //X_ADDR_START = 6
    {0x3006, 0x0587},  //Y_ADDR_END = 1415
    {0x3008, 0x0905},  //X_ADDR_END = 2309
    {0x300A, 0x051c},  //FRAME_LENGTH_LINES = 1308
    {0x300C, 0x04E0},  //LINE_LENGTH_PCK = 1248
    {0x3012, 0x051b},  //COARSE_INTEGRATION_TIME = 1307
    {0x3014, 0x0000},  //FINE_INTEGRATION_TIME = 0
    {0x30A2, 0x0001},  //X_ODD_INC = 1
    {0x30A6, 0x0001},  //Y_ODD_INC = 1

    {0x3040,0x0000}, //READ_MODE = 0
    {0x3042,0x0000}, //EXTRA_DELAY = 0
    {0x30BA,0x002C}, //DIGITAL_CTRL = 44
    {0x3070,0x0000},

    {0x301A,0x025C}, //Enable Streaming
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
 * Code for dealing with controls.
 * fill with different sensor module
 * different sensor module has different settings here
 * if not support the follow function ,retrun -EINVAL
 */
static int sensor_g_exp(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);
	
	*value = info->exp;
	vfe_dev_dbg("sensor_get_exposure = %d\n", info->exp);
	return 0;
}

static int sensor_s_exp(struct v4l2_subdev *sd, unsigned int exp_val)
{
    struct sensor_info *info = to_state(sd);

    vfe_dev_dbg("sensor_set_exposure = %d\n", exp_val);
    if(exp_val>0xffffff)
        exp_val=0xfffff0;
    if(exp_val<16)
        exp_val=16;
  
    exp_val=(exp_val)>>4;//rounding to 1
    
    sensor_write(sd, 0x3012,exp_val);//coarse integration time
  
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
	unsigned short dig_gain = 0x80;	// 1 times digital gain

    if (gain_val < 16)
        gain_val = 16;

    if (16<= gain_val*100 && gain_val*100 < (103*16) )
        sensor_write(sd,0x3060,0x0000);
    else if ((103*16) <= gain_val*100 && gain_val*100 < (107*16))
        sensor_write(sd,0x3060,0x0001);
    else if ((107*16) <= gain_val*100 && gain_val*100 < (110*16))
        sensor_write(sd,0x3060,0x0002);
    else if ((110*16) <= gain_val*100 && gain_val*100 < (114*16))
        sensor_write(sd,0x3060,0x0003);
    else if ((114*16) <= gain_val*100 && gain_val*100 < (119*16))
        sensor_write(sd,0x3060,0x0004);
    else if ((119*16) <= gain_val*100 && gain_val*100 < (123*16))
        sensor_write(sd,0x3060,0x0005);
    else if ((123*16) <= gain_val*100 && gain_val*100 < (128*16))
        sensor_write(sd,0x3060,0x0006);
    else if ((128*16) <= gain_val*100 && gain_val*100 < (133*16))
        sensor_write(sd,0x3060,0x0007);
    else if ((133*16) <= gain_val*100 && gain_val*100 < (139*16))
        sensor_write(sd,0x3060,0x0008);
    else if ((139*16) <= gain_val*100 && gain_val*100 < (145*16))
        sensor_write(sd,0x3060,0x0009);
    else if ((145*16) <= gain_val*100 && gain_val*100 < (152*16))
        sensor_write(sd,0x3060,0x000a);
    else if ((152*16) <= gain_val*100 && gain_val*100 < (160*16))
        sensor_write(sd,0x3060,0x000b);
    else if ((160*16) <= gain_val*100 && gain_val*100 < (168*16))
        sensor_write(sd,0x3060,0x000c);
    else if ((168*16) <= gain_val*100 && gain_val*100 < (178*16))
        sensor_write(sd,0x3060,0x000d);
    else if ((178*16) <= gain_val*100 && gain_val*100 < (188*16))
        sensor_write(sd,0x3060,0x000e);
    else if ((188*16) <= gain_val*100 && gain_val*100 < (200*16))
        sensor_write(sd,0x3060,0x000f);
    else if ((200*16) <= gain_val*100 && gain_val*100 < (213*16))
	{
        sensor_write(sd,0x3060,0x0010);
		dig_gain = gain_val*12800/(200*16);
	}
    else if ((213*16) <= gain_val*100 && gain_val*100 < (229*16))
    {
        sensor_write(sd,0x3060,0x0012);
		dig_gain = gain_val*12800/(213*16);
    }
    else if ((229*16) <= gain_val*100 && gain_val*100 < (246*16))
    {
        sensor_write(sd,0x3060,0x0014);
		dig_gain = gain_val*12800/(229*16);
    }
    else if ((246*16) <= gain_val*100 && gain_val*100 < (267*16))
	{
		sensor_write(sd,0x3060,0x0016);
		dig_gain = gain_val*12800/(246*16);
	}
    else if ((267*16) <= gain_val*100 && gain_val*100 < (291*16))
	{
		sensor_write(sd,0x3060,0x0018);
		dig_gain = gain_val*12800/(267*16);
	}
    else if ((291*16) <= gain_val*100 && gain_val*100 < (320*16))
	{
		sensor_write(sd,0x3060,0x001a);
		dig_gain = gain_val*12800/(291*16);
	}
    else if ((320*16) <= gain_val*100 && gain_val*100 < (356*16))
	{
		sensor_write(sd,0x3060,0x001c);
		dig_gain = gain_val*12800/(320*16);
	}
    else if ((356*16) <= gain_val*100 && gain_val*100 < (400*16))
	{
		sensor_write(sd,0x3060,0x001e);
		dig_gain = gain_val*12800/(356*16);
	}
    else if ((400*16) <= gain_val*100 && gain_val*100 < (457*16))
	{
		sensor_write(sd,0x3060,0x0020);
		dig_gain = gain_val*12800/(400*16);
	}
    else if ((457*16) <= gain_val*100 && gain_val*100 < (533*16))
	{
		sensor_write(sd,0x3060,0x0024);
		dig_gain = gain_val*12800/(457*16);
	}
    else if ((533*16) <= gain_val*100 && gain_val*100 < (640*16))
	{
		sensor_write(sd,0x3060,0x0028);
		dig_gain = gain_val*12800/(533*16);
	}
    else if ((640*16) <= gain_val*100 && gain_val*100 < (800*16))
	{
		sensor_write(sd,0x3060,0x002c);
		dig_gain = gain_val*12800/(640*16);
	}
    else if ((800*16) <= gain_val*100 )
	{
		sensor_write(sd,0x3060,0x0030);
		dig_gain = gain_val*12800/(800*16);
	}

	sensor_write(sd, 0x305e, dig_gain);
    
    info->gain = gain_val;
    return 0;
}

static int ar0330_sensor_vts;
static int sensor_s_exp_gain(struct v4l2_subdev *sd, struct sensor_exp_gain *exp_gain)
{
    int exp_val, gain_val,shutter,frame_length;  
    struct sensor_info *info = to_state(sd);
    exp_val = exp_gain->exp_val;
    gain_val = exp_gain->gain_val;
    if(gain_val<1*16)
        gain_val=16;
    if(gain_val>64*16-1)
        gain_val=64*16-1;
    if(exp_val>0xfffff)
        exp_val=0xfffff;

    shutter = exp_val/16;
    if(shutter > ar0330_sensor_vts - 4)
    	frame_length = shutter + 4;
    else
    	frame_length = ar0330_sensor_vts;

    printk("norm exp_val = %d,gain_val = %d\n",exp_val,gain_val);
    sensor_s_exp(sd,exp_val);
    sensor_s_gain(sd,gain_val);
    info->exp = exp_val;
    info->gain = gain_val;
    return 0;
}
static int sensor_s_sw_stby(struct v4l2_subdev *sd, int on_off)
{
    int ret ;
    data_type rdtmp;
    ret = sensor_read(sd,0x301a,&rdtmp);
    if (ret!=0)
        return ret;
    if (on_off == 1)
        sensor_write(sd,0x301a,(rdtmp & 0xfff8));
    else 
        sensor_write(sd,0x301a,rdtmp );
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
			ret = sensor_s_sw_stby(sd, CSI_GPIO_HIGH);
			if(ret < 0)
				vfe_dev_err("soft stby falied!\n");
			usleep_range(10000,12000);
			cci_lock(sd);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			cci_unlock(sd);
			vfe_set_mclk(sd,OFF);
			break;
		case CSI_SUBDEV_STBY_OFF:
			vfe_dev_dbg("CSI_SUBDEV_STBY_OFF!\n");
			cci_lock(sd);
			vfe_set_mclk_freq(sd,MCLK);
			vfe_set_mclk(sd,ON);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
			usleep_range(10000,12000);
			ret = sensor_s_sw_stby(sd, CSI_GPIO_LOW);
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
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);  
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			vfe_gpio_write(sd,POWER_EN,CSI_GPIO_LOW);
			usleep_range(1000,1200);
			vfe_set_pmu_channel(sd,AVDD,ON);	
			vfe_gpio_write(sd,POWER_EN,CSI_GPIO_HIGH);
			vfe_set_pmu_channel(sd,DVDD,ON);
			vfe_set_pmu_channel(sd,AFVDD,ON);
			usleep_range(1000,1200);
			vfe_set_pmu_channel(sd,IOVDD,ON);
			vfe_set_mclk_freq(sd,MCLK);
			vfe_set_mclk(sd,ON);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW); 
			usleep_range(10000,12000); 
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			usleep_range(20000,22000);
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);
			cci_lock(sd);
			vfe_set_mclk_freq(sd,MCLK);
			vfe_set_mclk(sd,ON);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
			usleep_range(10000,12000);
			ret = sensor_s_sw_stby(sd, CSI_GPIO_LOW);
			if(ret < 0)
				vfe_dev_err("soft stby off falied!\n");
			cci_unlock(sd);
			break;
		case CSI_SUBDEV_PWR_OFF:
			vfe_dev_dbg("CSI_SUBDEV_PWR_OFF!\n");
			cci_lock(sd);
			vfe_gpio_set_status(sd,PWDN,1);//set the gpio to output
			vfe_gpio_set_status(sd,RESET,1);//set the gpio to output
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);  
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			vfe_set_mclk(sd,OFF);
			vfe_set_pmu_channel(sd,AFVDD,OFF);
			vfe_set_pmu_channel(sd,DVDD,OFF);
			vfe_gpio_write(sd,POWER_EN,CSI_GPIO_LOW);
			vfe_set_pmu_channel(sd,AVDD,OFF);
			vfe_set_pmu_channel(sd,IOVDD,OFF);
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
    unsigned short rdval = 0;
    LOG_ERR_RET(sensor_read(sd, 0x3000, &rdval))
    if(rdval != 0x2604)
    {
        printk(KERN_DEBUG"*********sensor error,read id is %x.\n",rdval);
        return -ENODEV;
    }
    else
    {
        printk(KERN_DEBUG"*********find ar0330_mipi raw data camera sensor now.\n");
        return 0;
    }
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
    info->width = HD1080_WIDTH;
    info->height = HD1080_HEIGHT;
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
		.desc		= "Raw RGB Bayer",
		.mbus_code = V4L2_MBUS_FMT_SGRBG12_12X1,
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
    /* qsxga: 2304*1296 */  
    {
        .width      = 2304,
        .height     = 1296,
        .hoffset    = 0,
        .voffset    = 0,
        .hts        = 1248,
        .vts        = 1308,
        .pclk       = 49*1000*1000,
        .mipi_bps	= (588*1000*1000),
        .fps_fixed  = 1,
        .bin_factor = 1,
        .intg_min   = 1<<4,
        .intg_max   = 1308<<4,//
        .gain_min   = 1<<4,
        .gain_max   = 64<<4,
        .regs       = sensor_default_regs,
        .regs_size  = ARRAY_SIZE(sensor_default_regs),
        .set_size   = NULL,
    },
    
    /* 1080P */
    {
        .width	    = HD1080_WIDTH,
        .height 	= HD1080_HEIGHT,
        .hoffset    = 0,
        .voffset    = 0,
        .hts        = 1248,
        .vts        = 1308,
        .pclk       = 49*1000*1000,
        .mipi_bps	= (588*1000*1000)/1,
        .fps_fixed  = 1,
        .bin_factor = 1,
        .intg_min   = 1<<4,
        .intg_max   = 1308<<4,//
        .gain_min   = 1<<4,
        .gain_max   = 64<<4,
        .width_input = 2304,
		.height_input = 1296,
        .regs       = sensor_default_regs,
        .regs_size  = ARRAY_SIZE(sensor_default_regs),
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

    LOG_ERR_RET(sensor_write_array(sd, sensor_fmt->regs, sensor_fmt->regs_size))
    ret = 0;
    if (wsize->regs)
        LOG_ERR_RET(sensor_write_array(sd, wsize->regs, wsize->regs_size))
    if (wsize->set_size)
        LOG_ERR_RET(wsize->set_size(sd))

    info->fmt = sensor_fmt;
    info->width = wsize->width;
    info->height = wsize->height;
    ar0330_sensor_vts = wsize->vts;
    // show_regs_array(sd,sensor_1080p_regs);

    vfe_dev_print("s_fmt set width = %d, height = %d\n",wsize->width,wsize->height);
    if(info->capture_mode == V4L2_MODE_VIDEO)
    {
        //video
    } else {
        //capture image
    }
    //sensor_write_array(sd, sensor_oe_enable_regs, ARRAY_SIZE(sensor_oe_enable_regs));
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
    	return v4l2_ctrl_query_fill(qc, 1*16, 16*9-1, 1, 16);
    case V4L2_CID_EXPOSURE:
    	return v4l2_ctrl_query_fill(qc, 1, 65536*16, 1, 1);
    case V4L2_CID_FRAME_RATE:
    	return v4l2_ctrl_query_fill(qc, 15, 120, 1, 30);
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
    .data_width = CCI_BITS_16,
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
