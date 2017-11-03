/*
 * A V4L2 driver for superpix sp2508 cameras.
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
MODULE_DESCRIPTION("A low-level driver for superpix sp2508 sensors");
MODULE_LICENSE("GPL");

//for internel driver debug
#define DEV_DBG_EN      0
#if(DEV_DBG_EN == 1)
#define vfe_dev_dbg(x,arg...) printk("[sp2508]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...)
#endif
#define vfe_dev_err(x,arg...) printk("[sp2508]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[sp2508]"x,##arg)

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
//#define CLK_POL           V4L2_MBUS_PCLK_SAMPLE_FALLING

#define V4L2_IDENT_SENSOR 0x2508


/*
 * Our nominal (default) frame rate.
 */



#define I2C_ADDR (0x78)
#define  SENSOR_NAME "sp2508"
static struct v4l2_subdev *glb_sd;
static int sensor_s_exp_gain(struct v4l2_subdev *sd, struct sensor_exp_gain * exp_gain);

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
	
	{0xfd,0x00},//
	{0x1b,0x00},//
	{0x1c,0x00},//
	{0x1e,0x95},//
	{0x35,0x20},// ;pll bias
	{0x2f,0x10},//3b/*08*/},// ;pll clk 60M
	{0x34,0x01},//01},
	{0xfd,0x01},//
	{0x03,0x02},// ;exp time, 3 base
	{0x04,0x2b},//
	{0x06,0x08},// ;vblank
	{0x24,0x60},// ;pga gain 6x
	{0x01,0x01},// ;enable reg write
	{0x2b,0xc5},// ;readout vref
	{0x2e,0x20},// ;dclk delay
	{0x79,0x42},// ;p39 p40
	{0x85,0x0f},// ;p51
	{0x09,0x03},// ;hblank
	{0x0a,0x00},//
	{0x1e,0x82},//
	{0x21,0xef},// ;pcp tx 4.05v
	{0x25,0xf2},// ;reg dac 2.7v, enable bl_en,vbl 1.4v
	{0x26,0x00},// ;vref2 1v, disable ramp driver
	{0x2a,0xea},// ;bypass dac res, adc range 0.745, vreg counter 0.9
	{0x2c,0xf0},// ;high 8bit, pldo 2.7v
	{0x8a,0x55},// ;pixel bias 1uA
	{0x8b,0x55},// 
	{0x19,0xf3},// ;icom1 1.7u, icom2 0.6u 
	{0x3c,0x02},//
	{0x0e,0x04},// ;4 sample
	{0x0f,0x04},//
	{0x11,0x60},// ;rst num
	{0xd0,0x00},// ;disable boost
	{0x55,0x20},//
	{0x58,0x4d},//
	{0x5d,0x15},//
	{0x5e,0x05},//
	{0x64,0x40},//
	{0x65,0x00},//
	{0x66,0x66},//
	{0x67,0x00},//
	{0x68,0x68},//
	{0x72,0x70},//
	{0x75,0x60},//
	{0xfb,0x25},//
	{0xfd,0x02},// ;raw data digital gain
	{0x00,0x84},//
	{0x01,0x84},//
	{0x03,0x84},//
	{0x04,0x84},//



};
static struct regval_list sensor_uxga_regs[] = { //UXGA: 1600*1200
	{0xfd,0x00},
	{0x1b,0x00},
	{0x1c,0x00},
	{0x30,0x05},
	{0x1e,0x55},
	{0xfd,0x01},
	{0x31,0x00},
	{0x0d,0x00},//add

	{0x1e,0x8b},
	{0x01,0x01},
};

static struct regval_list sensor_720p_regs[] = { //1280*720
	{0xfd,0x00}, 
	{0x1b,0x00}, 
	{0x1c,0x00}, 
	{0x30,0x05}, 
	{0xfd,0x01}, 
	{0x31,0x10}, 
	{0x01,0x01},
};

static struct regval_list sensor_sxga_regs[] = { //SXGA: 1280*960

};

static struct regval_list sensor_fmt_raw[] = {

};




static int sensor_s_exp_gain(struct v4l2_subdev *sd, struct sensor_exp_gain *exp_gain)
{
	int exp_val, gain_val;  
	unsigned char explow=0,expmid=0;//,exphigh=0,vts_diff_low,vts_diff_high;
	unsigned char gainlow=0;//,gainhigh=0;  
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
	expmid  = (unsigned char) ( (0x0ff000&exp_val)>>12);
	explow  = (unsigned char) ( (0x000ff0&exp_val)>>4);


	sensor_write(sd, 0xfd, 0x01);

	sensor_write(sd, 0x24, gainlow*2);	

	sensor_write(sd, 0x03, expmid);
	sensor_write(sd, 0x04, explow);
	sensor_write(sd, 0x01, 0x01);
	printk("exp=%d,gain=%d\n",exp_val,gainlow*2);
	info->exp = exp_val;
	info->gain = gain_val;
	return 0;
}

static int sensor_g_exp(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);

	*value = info->exp;
	vfe_dev_dbg("sensor_get_exposure = %d\n", info->exp);
	return 0;
}

static int sensor_s_exp(struct v4l2_subdev *sd, unsigned int exp_val)
{
	unsigned char expmid=0,explow=0;
	struct sensor_info *info = to_state(sd);
	sensor_write(sd, 0xfd, 0x01);
	expmid  = (unsigned char) ( (0x0ff000&exp_val)>>12);
	explow  = (unsigned char) ( (0x000ff0&exp_val)>>4);
	sensor_write(sd, 0x03, expmid);
	sensor_write(sd, 0x04, explow);
	sensor_write(sd, 0x01, 0x01);
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
	unsigned char gainlow;
	
	gainlow=(unsigned char)(gain_val&0xff);
	sensor_write(sd, 0xfd, 0x01);//enter group write
	sensor_write(sd, 0x24, gainlow*2);	
	sensor_write(sd, 0x01, 0x01);//end group write
	printk("sp2508 sensor gain value is %d\n",gain_val);
	return 0;
}

static int sensor_s_sw_stby(struct v4l2_subdev *sd, int on_off)
{
	int ret;
	data_type rdval;

	ret=sensor_read(sd, 0xfc, &rdval);
	if(ret!=0)
		return ret;

	if(on_off==CSI_GPIO_HIGH)//sw stby on
	{
		ret=sensor_write(sd, 0xfc, rdval | 0x01);
		sensor_write(sd, 0xf2, 0x00);

	}
	else//sw stby off
	{
	    sensor_write(sd, 0xfc, rdval & 0xfe);
		ret=sensor_write(sd, 0xf2, 0xff);
	}
	return ret;
	//return 0;
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
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			usleep_range(30000,31000);
			vfe_set_mclk(sd,OFF);
			usleep_range(10000,12000);
			break;
		case CSI_SUBDEV_STBY_OFF:
			vfe_dev_dbg("CSI_SUBDEV_STBY_OFF\n");
			vfe_set_mclk_freq(sd,MCLK);
			vfe_set_mclk(sd,ON);
			usleep_range(30000,31000);
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);
			usleep_range(30000,31000);
			break;
		case CSI_SUBDEV_PWR_ON:
			vfe_dev_dbg("CSI_SUBDEV_PWR_ON\n");
			vfe_gpio_set_status(sd,PWDN,1);//set the gpio to output
			vfe_gpio_set_status(sd,RESET,1);//set the gpio to output
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			usleep_range(1000,1200);
			vfe_set_mclk_freq(sd,MCLK);
			vfe_set_mclk(sd,ON);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,POWER_EN,CSI_GPIO_HIGH);
			vfe_set_pmu_channel(sd,AVDD,ON);
			vfe_set_pmu_channel(sd,IOVDD,ON);
			vfe_set_pmu_channel(sd,DVDD,ON);
			vfe_set_pmu_channel(sd,AFVDD,ON);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
			usleep_range(10000,12000);
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
	data_type rdval;

	LOG_ERR_RET(sensor_read(sd, 0x02, &rdval))
	//  vfe_dev_dbg("0x0000=0x%x\n",rdval);
	if(rdval != 0x25)
		return -ENODEV;
	LOG_ERR_RET(sensor_read(sd, 0x03, &rdval))
	//  vfe_dev_dbg("0x0001=0x%x\n",rdval);
	if(rdval != 0x08)
		return -ENODEV;

  return 0;
}

static int sensor_init(struct v4l2_subdev *sd, u32 val)
{
	int ret;
	struct sensor_info *info = to_state(sd);

	vfe_dev_dbg("sensor_init\n");

	/*Make sure it is a target sensor*/
//	ret = sensor_detect(sd);
//	if (ret) {
//		vfe_dev_err("chip found is not an target chip.\n");
//		return ret;
//	}

	vfe_get_standby_mode(sd,&info->stby_mode);

	if((info->stby_mode == HW_STBY || info->stby_mode == SW_STBY) \
			&& info->init_first_flag == 0) {
		vfe_dev_print("stby_mode and init_first_flag = 0\n");
		return 0;
	}

	info->focus_status = 0;
	info->low_speed = 0;
	info->width = UXGA_WIDTH;
	info->height = UXGA_HEIGHT;
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
			sensor_s_exp_gain(sd, (struct sensor_exp_gain *)arg);
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
		.mbus_code	= V4L2_MBUS_FMT_SBGGR8_1X8,
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
	/* UXGA */
	{
		.width	  = UXGA_WIDTH,
		.height 	  = UXGA_HEIGHT,
		.hoffset	  = 0,
		.voffset	  = 0,
		.hts = 3205,
		.vts = 1224,
		.pclk       = 82*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 1<<4,
		.intg_max   = 1224<<4,
		.gain_min   = 1<<4,
		.gain_max   = (7<<4),
		.regs			= sensor_uxga_regs,
		.regs_size	= ARRAY_SIZE(sensor_uxga_regs),
		.set_size		= NULL,
	},
	{
		.width      = HD720_WIDTH,
		.height     = HD720_HEIGHT,
		.hoffset    = 0,
		.voffset    = 0,
		.hts        = 3300,//1646,//1616,
		.vts        = 736,//754,//764,
		.pclk       = 60*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 1<<4,
		.intg_max   = 736<<4,
		.gain_min   = 1<<4,
		.gain_max   = (8<<4)-1,
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

	vfe_dev_print("s_fmt set width = %d, height = %d\n",wsize->width,wsize->height);
	usleep_range(500000,600000);
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
		return v4l2_ctrl_query_fill(qc, 1*16, 32*16, 1, 1*16);
	case V4L2_CID_EXPOSURE:
		return v4l2_ctrl_query_fill(qc, 0, 8192*16, 1, 0);
	case V4L2_CID_FRAME_RATE:
		return v4l2_ctrl_query_fill(qc, 15, 120, 1, 120);
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

