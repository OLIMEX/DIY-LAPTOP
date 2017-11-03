/*
 * A V4L2 driver for s5k5e2_mipi cameras.
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
MODULE_DESCRIPTION("A low-level driver for s5k5e2yx sensors");
MODULE_LICENSE("GPL");

//for internel driver debug
#define DEV_DBG_EN      1
#if(DEV_DBG_EN == 1)    
#define vfe_dev_dbg(x,arg...) printk("[s5k5e2yx]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...) 
#endif
#define vfe_dev_err(x,arg...) printk("[s5k5e2yx]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[s5k5e2yx]"x,##arg)

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
#define V4L2_IDENT_SENSOR 0x5e20


/*
 * Our nominal (default) frame rate.
 */
#ifdef FPGA
#define SENSOR_FRAME_RATE 15
#else
#define SENSOR_FRAME_RATE 30
#endif

/*
 * The s5k5e2_mipi sits on i2c with ID 0x20
 */
#define I2C_ADDR (0x20)
#define SENSOR_NAME "s5k5e2yx"
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
	//	/ +++++++++++++++++++++++++++//                                                               
	// Reset for operation                                                                         
	{0x0100,0x00}, //stream off

	//Analog Timing Tuning (0817)
	{0x3000,0x04},    // ct_ld_start
	{0x3002,0x03},    // ct_sl_start
	{0x3003,0x04},    // ct_sl_margin
	{0x3004,0x02},    // ct_rx_start
	{0x3005,0x00},    // ct_rx_margin (MSB)
	{0x3006,0x10},    // ct_rx_margin (LSB)
	{0x3007,0x03},    // ct_tx_start
	{0x3008,0x55},    // ct_tx_width
	{0x3039,0x00},    // cintc1_margin
	{0x303A,0x00},    // cintc2_margin
	{0x303B,0x00},    // offs_sh
	{0x3009,0x05},    // ct_srx_margin
	{0x300A,0x55},    // ct_stx_width
	{0x300B,0x38},    // ct_dstx_width
	{0x300C,0x10},    // ct_stx2dstx
	{0x3012,0x05},    // ct_cds_start
	{0x3013,0x00},    // ct_s1s_start
	{0x3014,0x22},    // ct_s1s_end
	{0x300E,0x79},    // ct_s3_width
	{0x3010,0x68},    // ct_s4_width
	{0x3019,0x03},    // ct_s4d_start
	{0x301A,0x00},    // ct_pbr_start
	{0x301B,0x06},    // ct_pbr_width
	{0x301C,0x00},    // ct_pbs_start
	{0x301D,0x22},    // ct_pbs_width
	{0x301E,0x00},    // ct_pbr_ob_start
	{0x301F,0x10},    // ct_pbr_ob_width
	{0x3020,0x00},    // ct_pbs_ob_start
	{0x3021,0x00},    // ct_pbs_ob_width
	{0x3022,0x0A},    // ct_cds_lim_start
	{0x3023,0x1E},    // ct_crs_start
	{0x3024,0x00},    // ct_lp_hblk_cds_start (MSB)
	{0x3025,0x00},    // ct_lp_hblk_cds_start (LSB)
	{0x3026,0x00},    // ct_lp_hblk_cds_end (MSB)
	{0x3027,0x00},    // ct_lp_hblk_cds_end (LSB)
	{0x3028,0x1A},    // ct_rmp_off_start
	{0x3015,0x00},    // ct_rmp_rst_start (MSB)
	{0x3016,0x84},    // ct_rmp_rst_start (LSB)
	{0x3017,0x00},    // ct_rmp_sig_start (MSB)
	{0x3018,0xA0},    // ct_rmp_sig_start (LSB)
	{0x302B,0x10},    // ct_cnt_margin
	{0x302C,0x0A},    // ct_rmp_per
	{0x302D,0x06},    // ct_cnt_ms_margin1
	{0x302E,0x05},    // ct_cnt_ms_margin2
	{0x302F,0x0E},    // rst_mx
	{0x3030,0x2F},    // sig_mx
	{0x3031,0x08},    // ct_latch_start
	{0x3032,0x05},    // ct_latch_width
	{0x3033,0x09},    // ct_hold_start
	{0x3034,0x05},    // ct_hold_width
	{0x3035,0x00},    // ct_lp_hblk_dbs_start (MSB)
	{0x3036,0x00},    // ct_lp_hblk_dbs_start (LSB)
	{0x3037,0x00},    // ct_lp_hblk_dbs_end (MSB)
	{0x3038,0x00},    // ct_lp_hblk_dbs_end (LSB)
	{0x3088,0x06},    // ct_lat_lsb_offset_start1
	{0x308A,0x08},    // ct_lat_lsb_offset_end1
	{0x308C,0x05},    // ct_lat_lsb_offset_start2
	{0x308E,0x07},    // ct_lat_lsb_offset_end2
	{0x3090,0x06},    // ct_conv_en_offset_start1
	{0x3092,0x08},    // ct_conv_en_offset_end1
	{0x3094,0x05},    // ct_conv_en_offset_start2
	{0x3096,0x21},    // ct_conv_en_offset_end2

	//CDS
	{0x3099,0x0E},  // cds_option ([3]:crs switch disable, s3,s4 strengthx16)
	{0x3070,0x10},  // comp1_bias (default:77)
	{0x3085,0x11},  // comp1_bias (gain1~4)
	{0x3086,0x01},  // comp1_bias (gain4~8) modified 8/13

	{0x306B,0x10},

	//RMP
	{0x3064,0x00}, // Multiple sampling(gainx8,x16)
	{0x3062,0x08}, // off_rst
	   
	//DBR
	{0x3061,0x11},  // dbr_tune_rd (default :08, 0E 3.02V)	3.1V
	{0x307B,0x20},  // dbr_tune_rgsl (default :08)

	//Bias sampling
	{0x3068,0x00}, // RMP BP bias sampling
	{0x3074,0x00}, // Pixel bias sampling [2]:Default L
	{0x307D,0x00}, // VREF sampling [1]
	{0x3045,0x01}, // ct_opt_l1_start
	{0x3046,0x05}, // ct_opt_l1_width
	{0x3047,0x78},

	//Smart PLA
	{0x307F,0xB1}, //RDV_OPTION[5:4], RG default high
	{0x3098,0x01}, //CDS_OPTION[16] SPLA-II enable
	{0x305C,0xF6}, //lob_extension[6]

	{0x3063,0x27}, // ADC_SAT 490mV --> 610mV 
	{0x320C,0x07}, // ADC_MAX (def:076Ch --> 0700h)
	{0x320D,0x00},                                                                       
	{0x3400,0x01}, // GAS bypass
	{0x3235,0x49}, // L/F-ADLC on
	{0x3233,0x00}, // D-pedestal L/F ADLC off (1FC0h)
	{0x3234,0x00},
	{0x3300,0x0D}, //BPC bypass
	{0x0204,0x00}, //Analog gain x16
	{0x0205,0x20},
	//{0x0114,0x01},
	// streaming ON
	{0x0100,0x01}, 
};

                                                                         
static struct regval_list sensor_qsxga_regs[] = { //2576*1936 pclk=89.5MHz//2560x1920
#if 1
	{0x0100,0x00}, //stream off
	//{0x0301,0x01},
	{0x0301,0x01},
	{REG_DLY,0x22},

	// Size Setting
	{0x0305,0x05}, //PLLP (def:5)pre_pll_clk
	{0x0306,0x00},
	{0x0307,0xB3}, //PLLM (def:CCh 204d --> B3h 179d)pll_multiple
	{0x3C1F,0x00}, //PLLS pll_scaler
 
	{0x0820,0x03}, // requested link bit rate mbps : (def:3D3h 979d --> 35Bh 859d)
	{0x0821,0x5B}, 
	{0x3C1C,0x58}, //dbr_div 7:4 sys_clk_divider 3:0 divider for DBLR

	// Size Setting
	{0x0340,0x07}, // frame_length_lines : def. 1962d (7C2 --> 7A6 Mimnimum 22 lines)1959
	{0x0341,0xA7},
	{0x0342,0x0B}, // line_length_pck : def. 2900d 
	{0x0343,0x54},

	{0x0900,0x00},
	{0x0901,0x11},

	{0x3c33,0x00},

	{0x0344,0x00},	//x_addr_start		 :8 							 
	{0x0345,0x08},												   
	{0x0346,0x00},	//y_addr_start		 :248							 
	{0x0347,0x08},													 
	{0x0348,0x0A},	//x_addr_end		 :2567			 
	{0x0349,0x07},									   
	{0x034A,0x07},	//y_addr_end		 :1687			
	{0x034B,0x87},													 
	{0x034C,0x0A},	//x_output size 	 :2560			
	{0x034D,0x00},									 
	{0x034E,0x07},	//y_output size 	 :1440			
	{0x034F,0x80},		   

	{0x0387,0x01},
	
	//Integration time	
	{0x0202,0x02},	// coarse integration
	{0x0203,0x00},
	//{0x0200,0x0A},	// fine integration (AA8h --> AC4h)
	{0x0200,0x03},
	{0x0201,0x14},
	// streaming ON
	{0x0100,0x01}, 
#endif

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
static int s5k5e2_sensor_vts ;

static int sensor_s_exp_gain(struct v4l2_subdev *sd, struct sensor_exp_gain *exp_gain)
{
	int exp_val, gain_val,shutter,frame_length;

	unsigned char explow,exphigh;
	unsigned char gainlow=0;
	unsigned char gainhigh=0;
	struct sensor_info *info = to_state(sd);
	exp_val = exp_gain->exp_val;
	gain_val = exp_gain->gain_val;

	exp_val=exp_val>>4;//rounding to 1
	shutter = exp_val;
	gain_val=gain_val*2;//shift to 1/32 step

	exphigh = (unsigned char) ( (0xff00&exp_val)>>8);
	explow  = (unsigned char) ( (0x00ff&exp_val) );

	gainlow=(unsigned char)(gain_val&0xff);
	gainhigh=(unsigned char)((gain_val>>8)&0xff);
	if(shutter  > s5k5e2_sensor_vts-4)
		frame_length = shutter+4;
	else
		frame_length = s5k5e2_sensor_vts;
		
	vfe_dev_dbg("frame_length = %d,%d,%d\n",frame_length,shutter,s5k5e2_sensor_vts);
    sensor_write(sd,0x0104,0x01);
	sensor_write(sd, 0x0341,( (frame_length) & 0xff));
	sensor_write(sd, 0x0340,((frame_length) >> 8));
	sensor_write(sd, 0x0203, explow);
	sensor_write(sd, 0x0202, exphigh);
	sensor_write(sd, 0x0205, gainlow);
	sensor_write(sd, 0x0204, gainhigh);
	
    sensor_write(sd,0x0104,0x00);

	//vfe_dev_dbg("s5k5e2 sensor_set_gain = %d, Done!\n", gain_val);

	info->gain = gain_val;
	info->exp = exp_val;
	return 0;
}
static int sensor_s_exp(struct v4l2_subdev *sd, unsigned int exp_val)
{
	unsigned char explow,exphigh;
	data_type tmp1,tmp2;
	unsigned short tmp = 0;
	struct sensor_info *info = to_state(sd);

	//vfe_dev_dbg("sensor_set_exposure = %d\n", exp_val);
	if(exp_val>0xffffff)
		exp_val=0xfffff0;
	if(exp_val<16)
		exp_val=16;
	
	exp_val=(exp_val+8)>>4;//rounding to 1
	
	//printk("sensor_set_exposure real= %d\n", exp_val);
  
    exphigh = (unsigned char) ( (0xff00&exp_val)>>8);
    explow  = (unsigned char) ( (0x00ff&exp_val) );

	
    sensor_write(sd,0x0104,0x01);
	sensor_write(sd, 0x0203, explow);//coarse integration time
	sensor_write(sd, 0x0202, exphigh);	
	
    sensor_write(sd,0x0104,0x00);
	sensor_read(sd,0x0203,&tmp1);
	sensor_read(sd,0x0202,&tmp2);
	tmp = ((tmp2<<8)|tmp1);
	printk("readout shutter =%d\n",tmp);
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
	gain_val=gain_val*2;//step 32
	
	gainlow=(unsigned char)(gain_val&0xff);
	gainhigh=(unsigned char)((gain_val>>8)&0xff);
	
    sensor_write(sd,0x0104,0x01);
	sensor_write(sd, 0x0205, gainlow);
	sensor_write(sd, 0x0204, gainhigh);
	
    sensor_write(sd,0x0104,0x00);
//	sensor_write(sd, 0xb7, anagmax);
	
	printk("s5k5e2yx set_gain = %d\n", gain_val);
	info->gain = gain_val;
	
	return 0;
}

static int sensor_s_sw_stby(struct v4l2_subdev *sd, int on_off)
{
	int ret;
	data_type rdval;
	
	ret=sensor_read(sd, 0x0100, &rdval);
	printk("sensor read 0x0100 value is 0x%x",rdval);
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
            ret = sensor_s_sw_stby(sd, 1);
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
            cci_unlock(sd);        
            ret = sensor_s_sw_stby(sd, 0);
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
            vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
            vfe_set_pmu_channel(sd,DVDD,OFF);
            vfe_set_pmu_channel(sd,AVDD,OFF);			
            vfe_set_pmu_channel(sd,IOVDD,OFF);
			vfe_set_pmu_channel(sd,AFVDD,OFF);
            //vfe_gpio_set_status(sd,RESET,0);//set the gpio to input
            //vfe_gpio_set_status(sd,PWDN,0);//set the gpio to input
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
  
	sensor_read(sd, 0x0000, &rdval);
	printk("0x0000=0x%x\n",rdval);
	if(rdval != 0x5e)
		return -ENODEV;
	sensor_read(sd, 0x0001, &rdval);
	printk("0x0001=0x%x\n",rdval);
	if(rdval != 0x20)
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
		.mbus_code	= V4L2_MBUS_FMT_SGRBG10_10X1,
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
	/* qsxga: 2576*1936 */
	{
		.width      = 2560,//QSXGA_WIDTH,
		.height     = 1920,//QSXGA_HEIGHT,
		.hoffset    = 0,//(2576-2560)/2,//(2608-2592)/2,
		.voffset    = 0,//(1936-1440)/2,//(1960-1936)/2,
		.hts        = 2900,//must over 2738, limited by sensor
		.vts        = 1958,
		.pclk 	  = 170*1000*1000,//(89*1000*1000+500*1000),
		.mipi_bps 	= (859*1000*1000+600*1000)/1,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 3<<4,
		.intg_max   = (1959-8)<<4,
		.gain_min   = 1<<4,
		.gain_max   = 16<<4,
		.regs       = sensor_qsxga_regs,
		.regs_size  = ARRAY_SIZE(sensor_qsxga_regs),
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
	s5k5e2_sensor_vts = wsize->vts;
	vfe_dev_print("s_fmt set width = %d, height = %d\n",wsize->width,wsize->height);
	
	if(info->capture_mode == V4L2_MODE_VIDEO)
	{
	//video
	} else {
	//capture image
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
		return v4l2_ctrl_query_fill(qc, 1*32, 16*32, 1, 32);
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

