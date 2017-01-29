/*
 * A V4L2 driver for OV5647 cameras.
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
MODULE_DESCRIPTION("A low-level driver for OV5647 sensors");
MODULE_LICENSE("GPL");

//for internel driver debug
#define DEV_DBG_EN      0 
#if(DEV_DBG_EN == 1)    
#define vfe_dev_dbg(x,arg...) printk("[OV5647]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...) 
#endif
#define vfe_dev_err(x,arg...) printk("[OV5647]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[OV5647]"x,##arg)

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
#define VREF_POL          V4L2_MBUS_VSYNC_ACTIVE_LOW
#define HREF_POL          V4L2_MBUS_HSYNC_ACTIVE_HIGH
#define CLK_POL           V4L2_MBUS_PCLK_SAMPLE_RISING
#define V4L2_IDENT_SENSOR 0x5647


/*
 * Our nominal (default) frame rate.
 */
#ifdef FPGA
#define SENSOR_FRAME_RATE 15
#else
#define SENSOR_FRAME_RATE 30
#endif

/*
 * The ov5647 sits on i2c with ID 0x6c
 */
#define I2C_ADDR 0x6c
#define  SENSOR_NAME "ov5647"
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

static int ov5647_sensor_vts;
/*
 * The default register settings
 *
 */


static struct regval_list sensor_default_regs[] = {
	//Slave_ID=0x6c;
	{0x0100,0x00}, //
	{0x0103,0x01}, //
	//delay(5ms)
	{REG_DLY,0x05}, //must delay
	{0x3013,0x08}, //bit3=1 bypass internal dvdd ldo
	{0x5000,0x03}, // ;No lenc,WBC on
	{0x5001,0x00}, // ;manual AWB
	//{0x5002,0x41}, //
	//{0x5003,0x08}, //
	//{0x5005,0x31}, //
	//{0x5025,0x00}, //awb src sel
	//{0x5046,0x09}, //

	//{0x5180,0x08}, //
	//{0x5186,0x04}, //
	//{0x5187,0x00}, //
	//{0x5188,0x04}, //
	//{0x5189,0x00}, //
	//{0x518a,0x04}, //
	//{0x518b,0x00}, //


	{0x5a00,0x08}, //
	//  {0x3000,0xff}, //
	//  {0x3001,0xff}, //
	//  {0x3002,0xff}, //
	{0x3a18,0x01}, //
	{0x3a19,0xe0}, //
	{0x3c01,0x80}, //
	{0x3b07,0x0c}, //
	{0x3630,0x2e}, //
	{0x3632,0xe2}, //
	{0x3633,0x23}, //
	{0x3634,0x44}, //
	{0x3620,0x64}, //
	{0x3621,0xe0}, //
	{0x3600,0x37}, //
	{0x3704,0xa0}, //
	{0x3703,0x5a}, //
	{0x3715,0x78}, //
	{0x3717,0x01}, //
	{0x3731,0x02}, //
	{0x370b,0x60}, //
	{0x3705,0x1a}, //
	{0x3f05,0x02}, //
	{0x3f06,0x10}, //
	{0x3f01,0x0a}, //
	{0x3a08,0x00}, //
	{0x3a0a,0x00}, //
	{0x3a0f,0x58}, //
	{0x3a10,0x50}, //
	{0x3a1b,0x58}, //
	{0x3a1e,0x50}, //
	{0x3a11,0x60}, //
	{0x3a1f,0x28}, //
	{0x0100,0x01}, //
	{0x4000,0x89}, //0x89
	{0x4001,0x02}, //
	{0x4002,0xc5},
	{0x4004,0x06}, //
	{0x4005,0x1a}, //
	// manual AWB,manual AE,close Lenc,open WBC
	{0x3503,0x13}, // ;manual AE 13
	{0x3501,0x10}, //
	{0x3502,0x80}, //
	{0x350a,0x00}, //
	{0x350b,0x7f}, //
	{0x350c,0x00}, // vts diff manual vts set to 0
	{0x350d,0x00}, // vts diff manual vts set to 0
	{0x3011,0x22}, // bit[6:5] drive strength
};

//for capture                                                                         
static struct regval_list sensor_qsxga_regs[] = { //qsxga: 2592*1936@15fps
	{0x3035,0x11}, //clk
#ifdef QSXGA_12FPS
	{0x3036,0x50}, //clk
#else
	{0x3036,0x64}, //clk
#endif
	{0x303c,0x11}, //clk
	{0x3820,0x00}, //vbin
	{0x3821,0x06}, //hbin
	{0x3612,0x4b}, //
	{0x3618,0x04}, //
	{0x3708,0x24}, //
	{0x3709,0x12}, //
	{0x370c,0x00}, //
	{0x380c,0x0a}, //[4:0]hts high
	{0x380d,0x96}, //[7:0]hts low
	{0x380e,0x07}, //[4:0]vts high
	{0x380f,0xb0}, //[7:0]vts low
	{0x3814,0x11}, //h subsample inc
	{0x3815,0x11}, //v subsample inc
	{0x3808,0x0a}, //[4:0]dvp h out high 
	{0x3809,0x20}, //[7:0]dvp h out low  
	{0x380a,0x07}, //[4:0]dvp v out high 
	{0x380b,0x98}, //[7:0]dvp v out low  
	{0x3800,0x00}, //[4:0]dvp h start
	{0x3801,0x0c}, //[7:0]dvp h start
	{0x3802,0x00}, //[4:0]dvp v start
	{0x3803,0x04}, //[7:0]dvp v start
	{0x3804,0x0a}, //[4:0]dvp h end
	{0x3805,0x33}, //[7:0]dvp h end
	{0x3806,0x07}, //[4:0]dvp v end
	{0x3807,0xa3}, //[7:0]dvp v end
	//  {0x3a08,0x01}, //
	//  {0x3a09,0x27}, //
	//  {0x3a0a,0x00}, //
	//  {0x3a0b,0xf6}, //
	//  {0x3a0d,0x08}, //
	//  {0x3a0e,0x06}, //
};

static struct regval_list sensor_sxga_regs[] = { //SXGA: 1280*960@30fps
#ifdef FPGA_VER
	{0x3035,0x21}, //clk   
#else
	{0x3035,0x11}, //clk                
#endif
	{0x3036,0x46}, //clk                
	{0x303c,0x11}, //clk                
	{0x3820,0x41}, //vbin               
	{0x3821,0x07}, //hbin               
	{0x3612,0x49}, //                   
	{0x3618,0x00}, //                   
	{0x3708,0x22}, //                   
	{0x3709,0x52}, //                   
	{0x370c,0x03}, //                   
	{0x380c,0x07}, //[4:0]hts high      
	{0x380d,0x68}, //[7:0]hts low       
	{0x380e,0x03}, //[4:0]vts high      
	{0x380f,0xd8}, //[7:0]vts low       
	{0x3814,0x31}, //h subsample inc    
	{0x3815,0x31}, //v subsample inc    
	{0x3808,0x05}, //[4:0]dvp h out high
	{0x3809,0x00}, //[7:0]dvp h out low 
	{0x380a,0x03}, //[4:0]dvp v out high
	{0x380b,0xc0}, //[7:0]dvp v out low 
	{0x3800,0x00}, //[4:0]dvp h start   
	{0x3801,0x18}, //[7:0]dvp h start   
	{0x3802,0x00}, //[4:0]dvp v start   
	{0x3803,0x0e}, //[7:0]dvp v start   
	{0x3804,0x0a}, //[4:0]dvp h end     
	{0x3805,0x27}, //[7:0]dvp h end     
	{0x3806,0x07}, //[4:0]dvp v end     
	{0x3807,0x95}, //[7:0]dvp v end     
	//  {0x3a08,0x01}, //
	//  {0x3a09,0x27}, //
	//  {0x3a0a,0x00}, //
	//  {0x3a0b,0xf6}, //
	//  {0x3a0d,0x04}, //
	//  {0x3a0e,0x03}, //
};

//for video
static struct regval_list sensor_1080p_regs[] = { //1080: 1920*1080@30fps
	{0x3035,0x11}, //clk                
	{0x3036,0x50}, //clk                
	{0x303c,0x11}, //clk                
	{0x3820,0x00}, //vbin               
	{0x3821,0x06}, //hbin               
	{0x3612,0x4b}, //                   
	{0x3618,0x04}, //                   
	{0x3708,0x24}, //                   
	{0x3709,0x12}, //                   
	{0x370c,0x00}, //                   
	{0x380c,0x09}, //[4:0]hts high      
	{0x380d,0x4e}, //[7:0]hts low       
	{0x380e,0x04}, //[4:0]vts high      
	{0x380f,0x60}, //[7:0]vts low       
	{0x3814,0x11}, //h subsample inc    
	{0x3815,0x11}, //v subsample inc    
	{0x3808,0x07}, //[4:0]dvp h out high
	{0x3809,0x80}, //[7:0]dvp h out low 
	{0x380a,0x04}, //[4:0]dvp v out high
	{0x380b,0x38}, //[7:0]dvp v out low 
	{0x3800,0x01}, //[4:0]dvp h start   
	{0x3801,0x5c}, //[7:0]dvp h start   
	{0x3802,0x01}, //[4:0]dvp v start   
	{0x3803,0xb2}, //[7:0]dvp v start   
	{0x3804,0x08}, //[4:0]dvp h end     
	{0x3805,0xe3}, //[7:0]dvp h end     
	{0x3806,0x05}, //[4:0]dvp v end     
	{0x3807,0xf1}, //[7:0]dvp v end     
	//  {0x3a08,0x01}, //
	//  {0x3a09,0x4b}, //
	//  {0x3a0a,0x01}, //
	//  {0x3a0b,0x14}, //
	//  {0x3a0d,0x04}, //
	//  {0x3a0e,0x03}, //
};


static struct regval_list sensor_720p_regs[] = { //720: 1280*720@30fps
	{0x3035,0x21}, //clk                
	{0x3036,0x69}, //clk                
	{0x303c,0x12}, //clk                
	{0x3820,0x41}, //vbin               
	{0x3821,0x07}, //hbin               
	{0x3612,0x49}, //                   
	{0x3618,0x00}, //                   
	{0x3708,0x22}, //                   
	{0x3709,0x52}, //                   
	{0x370c,0x03}, //                   
	{0x380c,0x06}, //[4:0]hts high      
	{0x380d,0xd6}, //[7:0]hts low       
	{0x380e,0x03}, //[4:0]vts high      
	{0x380f,0x20}, //[7:0]vts low       
	{0x3814,0x31}, //h subsample inc    
	{0x3815,0x31}, //v subsample inc    
	{0x3808,0x05}, //[4:0]dvp h out high
	{0x3809,0x00}, //[7:0]dvp h out low 
	{0x380a,0x02}, //[4:0]dvp v out high
	{0x380b,0xd0}, //[7:0]dvp v out low 
	{0x3800,0x00}, //[4:0]dvp h start   
	{0x3801,0x18}, //[7:0]dvp h start   
	{0x3802,0x00}, //[4:0]dvp v start   
	{0x3803,0xf8}, //[7:0]dvp v start   
	{0x3804,0x0a}, //[4:0]dvp h end     
	{0x3805,0x27}, //[7:0]dvp h end     
	{0x3806,0x06}, //[4:0]dvp v end     
	{0x3807,0xa7}, //[7:0]dvp v end     
	//  {0x3a08,0x00}, //
	//  {0x3a09,0xdf}, //
	//  {0x3a0a,0x00}, //
	//  {0x3a0b,0xba}, //
	//  {0x3a0d,0x04}, //
	//  {0x3a0e,0x03}, //
};


//misc
static struct regval_list sensor_oe_disable_regs[] = {
	{0x3000,0x00},
	{0x3001,0x00},
	{0x3002,0x00},
};

static struct regval_list sensor_oe_enable_regs[] = {
	{0x3000,0x0f},
	{0x3001,0xff},
	{0x3002,0xe4},
};


/*
 * Here we'll try to encapsulate the changes for just the output
 * video format.
 * 
 */

static struct regval_list sensor_fmt_raw[] = {
};

static int sensor_s_fps(struct v4l2_subdev *sd)
{
	return 0;
}

static int sensor_g_exp(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);
  
	*value = info->exp;
	vfe_dev_dbg("sensor_get_exposure = %d\n", info->exp);
	return 0;
}
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
	//printk("sensor_vts = %d\n",sensor_vts);

	if(shutter  > ov5647_sensor_vts- 4)
	    frame_length = shutter + 4;
	else
	    frame_length = ov5647_sensor_vts;

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

	if(exp_val>0xfffff)
		exp_val=0xfffff;
	
//	if(info->exp == exp_val && exp_val <= (1968)*16)
//		return 0;
  
	sensor_write(sd, 0x3208, 0x00);//enter group write
	sensor_write(sd, 0x3503, 0x13);
	//sensor_write(sd, 0x350d, 0x0);
	//sensor_write(sd, 0x350c, 0x0);
    exphigh = (unsigned char) ( (0x0f0000&exp_val)>>16);
    expmid  = (unsigned char) ( (0x00ff00&exp_val)>>8);
    explow  = (unsigned char) ( (0x0000ff&exp_val)   );
	
	//sensor_write(sd, 0x3208, 0x00);//enter group write
	sensor_write(sd, 0x3502, explow);
	sensor_write(sd, 0x3501, expmid);
	sensor_write(sd, 0x3500, exphigh);	
	sensor_write(sd, 0x3208, 0x10);//end group write
	sensor_write(sd, 0x3208, 0xa0);//init group write
	//printk("5647 sensor_set_exp = %d, Done!\n", exp_val);
	
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
	
//	if(info->gain == gain_val)
//		return 0;
		
	gainlow=(unsigned char)(gain_val&0xff);
	gainhigh=(unsigned char)((gain_val>>8)&0x3);
	
	sensor_write(sd, 0x3208, 0x00);//enter group write
	
	sensor_write(sd, 0x3503, 0x13);
	sensor_write(sd, 0x350b, gainlow);
	sensor_write(sd, 0x350a, gainhigh);
	sensor_write(sd, 0x3208, 0x10);//end group write
	sensor_write(sd, 0x3208, 0xa0);//init group write
	
	//printk("5647 sensor_set_gain = %d, Done!\n", gain_val);
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
	
	if(on_off==CSI_GPIO_HIGH)//sw stby on
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
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);

			vfe_set_mclk_freq(sd,MCLK);
			vfe_set_mclk(sd,ON);
			usleep_range(10000,12000);
			vfe_set_pmu_channel(sd,IOVDD,ON);
			usleep_range(5000,6000);

			vfe_set_pmu_channel(sd,AVDD,ON);
			usleep_range(5000,6000);
			vfe_set_pmu_channel(sd,DVDD,ON);
			vfe_set_pmu_channel(sd,AFVDD,ON);
			usleep_range(10000,12000);

			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);

			cci_unlock(sd);    
			break;
		case CSI_SUBDEV_PWR_OFF:
			vfe_dev_dbg("CSI_SUBDEV_PWR_OFF!\n");
			cci_lock(sd);   

			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			usleep_range(10000,12000);
			vfe_set_pmu_channel(sd,DVDD,OFF);
			usleep_range(5000,6000);
			vfe_set_pmu_channel(sd,AVDD,OFF);
			usleep_range(5000,6000);
			vfe_set_pmu_channel(sd,IOVDD,OFF);  
			vfe_set_pmu_channel(sd,AFVDD,OFF);
			vfe_set_mclk(sd,OFF);
			usleep_range(10000,12000);
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
	if(rdval != 0x56)
		return -ENODEV;
 
	LOG_ERR_RET(sensor_read(sd, 0x300b, &rdval))
	if(rdval != 0x47)
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
		.mbus_code	= V4L2_MBUS_FMT_SBGGR10_1X10,
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
		.hoffset    = 0,
		.voffset    = 0,
		.hts        = 2710,
		.vts        = 1968,
#ifdef QSXGA_12FPS
		.pclk       = 64*1000*1000,
#else
		.pclk       = 80*1000*1000,
#endif
		.fps_fixed  = 2,
		.bin_factor = 1,
		.intg_min   = 1,
		//.intg_max   = 2000<<4,
		.intg_max   = (1968/1)<<4,
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
		.hoffset	  = 0,
		.voffset	  = 0,
		.hts        = 2382,
		.vts        = 1120,
		.pclk       = 64*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 1<<4,
		.intg_max   = 1120<<4,
		.gain_min   = 1<<4,
		.gain_max   = 10<<4,
		.regs       = sensor_1080p_regs,//
		.regs_size  = ARRAY_SIZE(sensor_1080p_regs),//
		.set_size		= NULL,
	},
	/* SXGA */
	{
		.width			= SXGA_WIDTH,
		.height 		= SXGA_HEIGHT,
		.hoffset	  = 0,
		.voffset	  = 0,
		.hts        = 1896,
		.vts        = 984,
		.pclk       = 56*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 1,
		.intg_max   = 984<<4,
		.gain_min   = 1<<4,
		.gain_max   = 10<<4,
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
		.hts        = 1750,
		.vts        = 800,
		.pclk       = 42*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 1,
		.intg_max   = 800<<4,
		.gain_min   = 1<<4,
		.gain_max   = 10<<4,
		.regs			  = sensor_720p_regs,//
		.regs_size	= ARRAY_SIZE(sensor_720p_regs),//
		.set_size		= NULL,
	},
	/* VGA */
	{
		.width	  = VGA_WIDTH,
		.height 	  = VGA_HEIGHT,
		.hoffset	  = 320,
		.voffset	  = 240,
		.hts        = 1896,
		.vts        = 984,
		.pclk       = 56*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 1,
		.intg_max   = 984<<4,
		.gain_min   = 1<<4,
		.gain_max   = 10<<4,
		.regs         = sensor_sxga_regs,
		.regs_size    = ARRAY_SIZE(sensor_sxga_regs),
		.set_size     = NULL,
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
    
	sensor_write_array(sd, sensor_oe_disable_regs, ARRAY_SIZE(sensor_oe_disable_regs));

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
	ov5647_sensor_vts = wsize->vts;
   
	vfe_dev_print("s_fmt set width = %d, height = %d\n",wsize->width,wsize->height);

	if(info->capture_mode == V4L2_MODE_VIDEO)
	{
	//video
	} else {
	//capture image
	}
	
	sensor_write_array(sd, sensor_oe_enable_regs, ARRAY_SIZE(sensor_oe_enable_regs));
	
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
		return v4l2_ctrl_query_fill(qc, 1*16, 64*16-1, 1, 1*16);
	case V4L2_CID_EXPOSURE:
		return v4l2_ctrl_query_fill(qc, 0, 65535*16, 1, 0);
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
	case V4L2_CID_FRAME_RATE:
		return sensor_s_fps(sd);
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

