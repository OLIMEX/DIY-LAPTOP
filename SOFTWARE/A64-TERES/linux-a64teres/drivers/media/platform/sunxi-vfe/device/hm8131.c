/*
 * A V4L2 driver for HM8131 cameras.
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
MODULE_DESCRIPTION("A low-level driver for HM8131 sensors");
MODULE_LICENSE("GPL");

//for internel driver debug
#define DEV_DBG_EN      1 
#if(DEV_DBG_EN == 1)    
#define vfe_dev_dbg(x,arg...) printk("[HM8131]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...) 
#endif
#define vfe_dev_err(x,arg...) printk("[HM8131]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[HM8131]"x,##arg)

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
#define V4L2_IDENT_SENSOR 0x8131
int hm8131_sensor_vts;



/*
 * Our nominal (default) frame rate.
 */

#define SENSOR_FRAME_RATE 15


/*
 * The HM8131 sits on i2c with ID 0x6c
 */
#define I2C_ADDR 0x68
#define SENSOR_NAME "hm8131"
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
};

//for capture                                                                         
static struct regval_list sensor_quxga_regs[] = {
	//@@3264_2448_2lane
	{0x0100, 0x00},
	{REG_DLY, 0x32},// very important!! Without this, it fails!
	
	{0x0103, 0x00},
	
	{0x0100, 0x02},
	
	{0x0303, 0x02}, //PLL adc1 - enable PLL -> was 002A
	{0x0305, 0x0C}, //PLL N, mclk 24mhz
	{0x0307, 0x44}, //PLL M, pclk_raw=68mhz
	{0x030D, 0x0C}, //PLL N,
	{0x030F, 0x5E}, //PLL M, pkt_clk=94mhz
	

	{0x414A, 0x02},
	{0x4147, 0x03},
	{0x4144, 0x01},
	{0x4145, 0x31},
	{0x4146, 0x40},
	{0x4149, 0x5C},
	{0x4260, 0x00},
	{0x4261, 0x00},
	{0x4270, 0x18},
	{0x4271, 0x6D},
	{0x4272, 0x22},
	{0x427D, 0x00},
	{0x427E, 0x03},
	{0x427F, 0x00},
	{0x4380, 0xA0},
	{0x4381, 0x7B},
	{0x4383, 0x58},
	{0x4387, 0x37},
	{0x4386, 0x01},
	{0x4382, 0x00},
	{0x4388, 0x9F},
	{0x438A, 0x80},
	{0x438C, 0x1F},
	{0x4384, 0x20},
	{0x438B, 0x00},
	{0x4385, 0xA6},
	{0x438F, 0x00},
	{0x438D, 0xA0},
	{0x4B44, 0x00},
	{0x4B46, 0x4E},
	{0x4B47, 0x3F},
	{0x44D0, 0x00},
	{0x44D1, 0x00},
	{0x44D2, 0x00},
	{0x44D3, 0x00},
	{0x44D4, 0x00},
	{0x44D5, 0x00},
	{0x4B07, 0xF0},
	{0x4131, 0x01},
	
	{0x4274, 0x33}, //[5] mask out bad frame due to mode (flip/mirror) change
	{0x4002, 0x23}, //output BPC
	{0x4B18, 0x28}, //[7:0] FULL_TRIGGER_TIME
	
	{0x4B08, 0xA0}, //VSIZE (2464)
	{0x4B09, 0x09}, //
	{0x4B0A, 0xD0}, //HSIZE (3280)
	{0x4B0B, 0x0C}, //
	{0x0111, 0x00}, //00:2lane, 01:4lane (org 4B19)
	{0x4B20, 0x9E}, //clock always on(9E) / clock always on while sending packet(BE)
	{0x4B0E, 0x03},
	{0x4B42, 0x05},
	
	{0x4B03, 0x0E}, //hs_zero_width
	{0x4B04, 0x05}, //hs_trail_width
	
	{0x4B06, 0x06}, //clk_trail_width
	
	{0x4B3F, 0x18}, //[3:0] mipi_req_dly
	
	{0x4024, 0x40}, //enabled MIPI -> was 0024
	
	{0x0101, 0x03}, //flip+mirror
	
	{0x0100, 0x01},
};


//for video

static struct regval_list sensor_sxga_regs[] = {
	//MIPI=720Mbps, SysClk=144Mhz,Dac Clock=360Mhz.
	{0x0100, 0x00},
	{REG_DLY, 0x32},// very important!! Without this, it fails!
	{0x0103, 0x00},
	{0x0100, 0x02},
	
	{0x0303, 0x02}, //PLL adc1 - enable PLL -> was 002A
	{0x0305, 0x0C}, //PLL N, mclk 24mhz
	{0x0307, 0x46}, //PLL M, pclk_raw=68mhz
	{0x030D, 0x0C}, //PLL N,
	{0x030F, 0x5E}, //PLL M, pkt_clk=94mhz
	
	{0x0309, 0x02},
	{0x400D, 0x04},
	{0x0383, 0x03},
	{0x0387, 0x03},
	{0x0390, 0x01},
	
	{0x414A, 0x02},
	{0x4147, 0x03},
	{0x4144, 0x01},
	{0x4145, 0x31},
	{0x4146, 0x40},
	{0x4149, 0x5C},
	{0x4260, 0x00},
	{0x4261, 0x00},
	{0x4270, 0x18},
	{0x4271, 0x6D},
	{0x4272, 0x22},
	{0x427D, 0x00},
	{0x427E, 0x03},
	{0x427F, 0x00},
	{0x4380, 0xA0},
	{0x4381, 0x7B},
	{0x4383, 0x58},
	{0x4387, 0x37},
	{0x4386, 0x01},
	{0x4382, 0x00},
	{0x4388, 0x9F},
	{0x438A, 0x80},
	{0x438C, 0x1F},
	{0x4384, 0x20},
	{0x438B, 0x00},
	{0x4385, 0xA6},
	{0x438F, 0x00},
	{0x438D, 0xA0},
	{0x4B44, 0x00},
	{0x4B46, 0x4E},
	{0x4B47, 0x3F},
	{0x44D0, 0x00},
	{0x44D1, 0x00},
	{0x44D2, 0x00},
	{0x44D3, 0x00},
	{0x44D4, 0x00},
	{0x44D5, 0x00},
	{0x4B07, 0xF0},
	{0x4131, 0x01},
	
	{0x4274, 0x33}, //[5] mask out bad frame due to mode (flip/mirror) change
	{0x4002, 0x23}, //output BPC
	{0x4B18, 0x08}, //[7:0] FULL_TRIGGER_TIME
	
	{0x0383, 0x03},// x odd increment -> was 01
	{0x0387, 0x03},// y odd increment -> was 01
	{0x0390, 0x01},// binning mode -> was 00   1:binning 2:summing
	{0x0344, 0x00},// Image size X start Hb
	{0x0345, 0x08},// Image size X start Lb
	{0x0346, 0x00},// Image size Y star Hb
	{0x0347, 0x08},// Image size Y star Lb
	{0x0348, 0x0C},// Image size X end Hb
	{0x0349, 0xC5},// Image size X end Lb
	{0x034A, 0x09},// Image size Y end Hb
	{0x034B, 0x95},// Image size Y end Lb
	{0x0340, 0x05},// frame length lines Hb
	{0x0341, 0x1A},// frame length lines Lb
	{0x0342, 0x03},// smia line length Hb (=0x427A)
	{0x0343, 0x70},// smia line length Lb (=0x427B)
	
	{0x4B08, 0xC8},// VSIZE 
	{0x4B09, 0x04},// 
	{0x4B0A, 0x60},// HSIZE 
	{0x4B0B, 0x06},// 
	{0x034C, 0x06},// HSIZE 
	{0x034D, 0x60},// HSIZE 
	{0x034E, 0x04},// VSIZE 
	{0x034F, 0xC8},// VSIZE	
	
	{0x0111, 0x00}, //00:2lane, 01:4lane
	{0x4B20, 0x9E}, //clock always on(9E) / clock always on while sending packet(BE)
	{0x4B0E, 0x01},
	{0x4B42, 0x02},
	
	{0x4B02, 0x02}, //lpx_width
	
	{0x4B03, 0x05}, //hs_zero_width
	{0x4B04, 0x02}, //hs_trail_width
	
	{0x4B05, 0x0C}, //clk_zero_width
	
	{0x4B06, 0x03}, //clk_trail_width
	
	{0x4B0F, 0x07}, //clk_back_porch
	{0x4B39, 0x02}, //clk_width_exit
	
	{0x4B3F, 0x08}, //[3:0] mipi_req_dly
	
	{0x4B42, 0x02}, //HS_PREPARE_WIDTH
	{0x4B43, 0x02}, //CLK_PREPARE_WIDTH
	
	{0x4024, 0x40}, //enabled MIPI -> was 0024
	
	{0x0101, 0x03}, //flip+mirror
	
	{0x0340, 0x05},
	{0x0341, 0x1A},
	
	{0x0100, 0x01},
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
	unsigned char explow,expmid;
	struct sensor_info *info = to_state(sd);

	vfe_dev_dbg("sensor_set_exposure = %d\n", exp_val>>4);
	if(exp_val>0xfffff)
		exp_val=0xfffff;

    expmid  = (unsigned char) ( (0x0ff000&exp_val)>>12);
    explow  = (unsigned char) ( (0x000ff0&exp_val)>>4);
	
	sensor_write(sd, 0x0104, 0x01);
	sensor_write(sd, 0x0202, expmid);
	sensor_write(sd, 0x0203, explow);
	sensor_write(sd, 0x0104, 0x00);
	
//	printk("8131 sensor_set_exp = %d %d %d, Done!\n", exp_val, expmid, explow);
	
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
	
	if(gain_val<1*16)
		gain_val=16;
	if(gain_val>64*16-1)
		gain_val=64*16-1;
	vfe_dev_dbg("sensor_set_gain = %d\n", gain_val);

	gainlow=(unsigned char)(gain_val - 16);
	
	sensor_write(sd, 0x0104, 0x01);
	sensor_write(sd, 0x0205, gainlow);
	sensor_write(sd, 0x0104, 0x00);
	info->gain = gain_val;
	
	return 0;
}

static int sensor_s_exp_gain(struct v4l2_subdev *sd, struct sensor_exp_gain *exp_gain)
{
	int exp_val, gain_val,frame_length,shutter;
	unsigned char explow=0,expmid=0;//,exphigh=0;
	unsigned char gainlow=0;//,gainhigh=0;  
	struct sensor_info *info = to_state(sd);

	exp_val = exp_gain->exp_val;
	gain_val = exp_gain->gain_val;

	if((info->exp == exp_val)&&(info->gain == gain_val))
		return 0;
  
	if(gain_val<1*16)
		gain_val=16;
	if(gain_val>64*16-1)
		gain_val=64*16-1;
  
	if(exp_val>0xfffff)
		exp_val=0xfffff;
  
	shutter = exp_val/16;
	if(shutter  > hm8131_sensor_vts - 4)
		frame_length = shutter + 4;
	else
		frame_length = hm8131_sensor_vts;
  
	gainlow = (unsigned char)(gain_val - 16);
	expmid  = (unsigned char)((0x0ff000&exp_val)>>12);
	explow  = (unsigned char)((0x000ff0&exp_val)>>4);

	sensor_write(sd, 0x0104, 0x01);
	sensor_write(sd, 0x0202, expmid);
	sensor_write(sd, 0x0203, explow);
	sensor_write(sd, 0x0205, gainlow);
	sensor_write(sd, 0x0104, 0x00);

	info->exp = exp_val;
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
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			cci_unlock(sd);  
			vfe_set_mclk(sd,OFF);
			break;
		case CSI_SUBDEV_STBY_OFF:
			vfe_dev_dbg("CSI_SUBDEV_STBY_OFF!\n");
			cci_lock(sd);    
			vfe_set_mclk_freq(sd,MCLK);
			vfe_set_mclk(sd,ON);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);
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
			vfe_gpio_write(sd,POWER_EN,CSI_GPIO_LOW);
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
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
  
	LOG_ERR_RET(sensor_read(sd, 0x0000, &rdval))
	if(rdval != 0x81)
		return -ENODEV;
  	
	LOG_ERR_RET(sensor_read(sd, 0x0001, &rdval))
	if(rdval != 0x31)
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
		.mbus_code	= V4L2_MBUS_FMT_SRGGB10_10X1, //V4L2_MBUS_FMT_SBGGR10_10X1,//
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
	/* quxga: 3264*2448 */
	{
		.width      = QUXGA_WIDTH,//3280,
		.height     = QUXGA_HEIGHT,//2464,
		.hoffset    = 0,
		.voffset    = 0,
		.hts        = 3578,
		.vts        = 2534,
		.pclk       = 136*1000*1000,
		.mipi_bps	  = 720*1000*1000,
		.fps_fixed  = 2,
		.bin_factor = 1,
		.intg_min   = 16,
		.intg_max   = 2534<<4,
		.gain_min   = 1<<4,
		.gain_max   = 10<<4,
		.regs       = sensor_quxga_regs,
		.regs_size  = ARRAY_SIZE(sensor_quxga_regs),
		.set_size   = NULL,
	},
#endif
#if 1
	/* SXGA */
	{
		.width	  = SXGA_WIDTH,
		.height 	  = SXGA_HEIGHT,
		.hoffset	  = 176,
		.voffset	  = 132,
		.hts        = 3573,
		.vts        = 1306,
		.pclk 	  = 140*1000*1000,
		.mipi_bps   = 720*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 1,
		.intg_max   = 1306<<4,
		.gain_min   = 1<<4,
		.gain_max   = 12<<4,
		.regs		  = sensor_sxga_regs,
		.regs_size  = ARRAY_SIZE(sensor_sxga_regs),
		.set_size	  = NULL,
	},
	/* 720p */
	{
		.width      = HD720_WIDTH,
		.height     = HD720_HEIGHT,
		.hoffset    = 176,
		.voffset    = 252,
		.hts		  = 3573,
		.vts		  = 1306,
		.pclk 	  = 140*1000*1000,
		.mipi_bps   = 720*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 16,
		.intg_max   = 1306<<4,
		.gain_min   = 1<<4,
		.gain_max   = 12<<4,
		.regs		  = sensor_sxga_regs,//
		.regs_size  = ARRAY_SIZE(sensor_sxga_regs),//
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
	hm8131_sensor_vts = wsize->vts;
   
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

