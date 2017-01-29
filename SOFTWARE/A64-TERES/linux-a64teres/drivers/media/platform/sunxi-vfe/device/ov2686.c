/*
 * A V4L2 driver for ov2686 cameras.
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

MODULE_AUTHOR("zw");
MODULE_DESCRIPTION("A low-level driver for ov2686 sensors");
MODULE_LICENSE("GPL");

#define AF_WIN_NEW_COORD
//for internel driver debug
#define DEV_DBG_EN    0
#if(DEV_DBG_EN == 1)
#define vfe_dev_dbg(x,arg...) printk("[OV2686]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...)
#endif
#define vfe_dev_err(x,arg...) printk("[OV2686]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[OV2686]"x,##arg)

#define LOG_ERR_RET(x)  { \
                          int ret;  \
                          ret = x; \
                          if(ret < 0) {\
                            vfe_dev_err("error at %s\n",__func__);  \
                            return ret; \
                          } \
                        }

//define module timing
#define MCLK              			(24*1000*1000)
#define VREF_POL          			V4L2_MBUS_VSYNC_ACTIVE_LOW
#define HREF_POL          			V4L2_MBUS_HSYNC_ACTIVE_HIGH
#define CLK_POL           			V4L2_MBUS_PCLK_SAMPLE_RISING
#define V4L2_IDENT_SENSOR 	0x2686

#define SENSOR_NAME "ov2686"
/*
 * Our nominal (default) frame rate.
 */
#define SENSOR_FRAME_RATE 	30

/*
 * The ov2686 sits on i2c with ID 0x20
 */
#define I2C_ADDR 	0x20


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
	{0x0103, 0x01},//software reset
	{REG_DLY,0x1e},//delay 30ms	
	{0x3000, 0x03},//Y[9:8] output
	{0x3001, 0xff},//Y[7:0] output
	{0x3002, 0x1a},//Vsync output, FSIN input
	{0x3011, 0x00},//Pad
	{0x301d, 0xf0},//sclk_fc, sclk_grp, sclk_bist, daclk_sel0
	{0x3020, 0x00},//output raw
	{0x3021, 0x23},//software standby enter at l_blk, frex_ef_sel, cen_blobal_o
	{0x3082, 0x2c},//PLL 
	{0x3083, 0x00},
	{0x3084, 0x07},
	{0x3085, 0x03},
	{0x3086, 0x01},
	{0x3087, 0x00},
	{0x3106, 0x01},//PLL
	{0x3501, 0x26},//exposure M
	{0x3502, 0x40},//exposure L
	{0x3503, 0x03},//gain delay 1 frame, vts auto, agc off, aec off
	{0x350b, 0x36},//gain L
	{0x3600, 0xb4},//Analog Control
	{0x3603, 0x35},//
	{0x3604, 0x24},//
	{0x3605, 0x00},//
	{0x3620, 0x25},//
	{0x3621, 0x37},//
	{0x3622, 0x23},//
	{0x3628, 0x10},//Analog Control
	{0x3701, 0x64},//Sensor Conrol
	{0x3705, 0x3c},
	{0x370a, 0x23},
	{0x370c, 0x50},
	{0x370d, 0xc0},
	{0x3717, 0x58},
	{0x3718, 0x80},
	{0x3720, 0x00},
	{0x3721, 0x00},
	{0x3722, 0x00},
	{0x3723, 0x00},
	{0x3738, 0x00},//Sensor Control
	{0x3781, 0x80},//PSRAM control
	{0x3789, 0x60},//PSRAM control
	{0x3800, 0x00},//Timing, x start H
	{0x3801, 0x00},//x start L
	{0x3802, 0x00},//y start H
	{0x3803, 0x00},//y start L
	{0x3804, 0x06},//x end H
	{0x3805, 0x4f},//x end L
	{0x3806, 0x04},//y end H
	{0x3807, 0xbf},//y end L
	{0x3808, 0x03},//x output size H
	{0x3809, 0x20},//x output size L
	{0x380a, 0x02},//y output size H
	{0x380b, 0x58},//y output size L
	{0x380c, 0x06},//HTS H
	{0x380d, 0xac},//HTS L
	{0x380e, 0x02},//VTS H
	{0x380f, 0x84},//VTS L
	{0x3810, 0x00},//isp x win H
	{0x3811, 0x04},//isp x win L
	{0x3812, 0x00},//isp y win H
	{0x3813, 0x04},//isp y win L
	{0x3814, 0x31},//x inc
	{0x3815, 0x31},//y inc
	{0x3819, 0x04},//vsync_end_row L
	{0x3820, 0xc2},//vsub48_blc, vflip_blc, vbinf
	{0x3821, 0x01},//hbin
	{0x3a06, 0x00},//B50 H
	{0x3a07, 0xc2},//B50 L
	{0x3a08, 0x00},//B60 H
	{0x3a09, 0xA1},//B60 L
	{0x3a0a, 0x07},//max exp 50 H
	{0x3a0b, 0x94},//max exp 50 L
	{0x3a0c, 0x07},//max exp 60 H
	{0x3a0d, 0x94},//max exp 60 L
	{0x3a0e, 0x02},//VTS band 50 H
	{0x3a0f, 0x46},//VTS band 50 L
	{0x3a10, 0x02},//VTS band 60 H
	{0x3a11, 0x84},//VTS band 60 L
	{0x4000, 0x81},//avg_weight, mf_en
	{0x4001, 0x40},//format_trig_beh
	{0x4008, 0x00},//bl_start
	{0x4009, 0x03},//bl_end
	{0x4300, 0x30},//YUV422
	{0x430e, 0x00},//no swap, no bypass
	{0x4602, 0x02},//Frame reset enable
	{0x5000, 0xff},//lenc_en, awb_gain_en, lcd_en, avg_en, dgc_en, bc_en, wc_en, blc-en
	{0x5001, 0x05},//avg_sel after LCD
	{0x5002, 0x32},//dpc_href_s, sof_sel, bias_plus
	{0x5003, 0x04},//bias_man
	{0x5004, 0xff},//uv_dns_en, rng_dns_en, gamma_en, cmxen, cip_en, raw_dns_en, stretch_en, awb_en
	{0x5005, 0x12},//sde_en, rgb2yuv_en
	{0x5180, 0xf4},//AWB
	{0x5181, 0x11},
	{0x5182, 0x41},
	{0x5183, 0x42},
	{0x5184, 0x6e},
	{0x5185, 0x56},
	{0x5186, 0xb4},
	{0x5187, 0xb2},
	{0x5188, 0x08},
	{0x5189, 0x0e},
	{0x518a, 0x0e},
	{0x518b, 0x46},
	{0x518c, 0x38},
	{0x518d, 0xf8},
	{0x518e, 0x04},
	{0x518f, 0x7f},
	{0x5190, 0x40},
	{0x5191, 0x5f},
	{0x5192, 0x40},
	{0x5193, 0xff},
	{0x5194, 0x40},
	{0x5195, 0x07},	
	{0x5196, 0x04},
	{0x5197, 0x04},
	{0x5198, 0x00},
	{0x5199, 0x05},
	{0x519a, 0xd2},
	{0x519b, 0x04},//AWB
	{0x5200, 0x09},//stretch minimum value is 3096, auto
	{0x5201, 0x00},//stretch min low level
	{0x5202, 0x06},//stretch max low level
	{0x5203, 0x20},//stretch min high level
	{0x5204, 0x41},//stretch step2, stretch step1
	{0x5205, 0x16},//stretch current low level
	{0x5206, 0x00},//stretch current high level L
	{0x5207, 0x05},//stretch current high level H
	{0x520b, 0x30},//stretch_thres1 L
	{0x520c, 0x75},//stretch_thres1 M
	{0x520d, 0x00},//stretch_thres1 H
	{0x520e, 0x30},//stretch_thres2 L
	{0x520f, 0x75},//stretch_thres2 M
	{0x5210, 0x00},//stretch_thres2 H
	{0x5280, 0x14},//raw_DNS, m_nNoiseYslop = 5
	{0x5281, 0x02},//m_nNoiseList[0]
	{0x5282, 0x02},//m_nNoiseList[1]
	{0x5283, 0x04},//m_nNoiseList[2]
	{0x5284, 0x06},//m_nNoiseList[3]
	{0x5285, 0x08},//m_nNoiseList[4]
	{0x5286, 0x0c},//m_nNoiseList[5]
	{0x5287, 0x10},//m_nMaxEdgeThre
	{0x5300, 0xc5},//CIP, m_bColorEdgeEnable, m_bAntiAliasing, m_nDetailSlop=0, m_nNoiseYSlop=5
	{0x5301, 0xa0},//m_nSharpenSlope=1, m_nGbGrShift=0 
	{0x5302, 0x06},//m_nNoiseList[0]
	{0x5303, 0x0a},//m_nNoiseList[1]
	{0x5304, 0x30},//m_nNoiseList[2]
	{0x5305, 0x60},//m_nNoiseList[3]
	{0x5306, 0x90},//m_nNoiseList[4]
	{0x5307, 0xc0},//m_nNoiseList[5]
	{0x5308, 0x82},//m_nMaxSharpenGain=8, m_nMinSharpenGain=2
	{0x5309, 0x00},//m_nMinSharpen
	{0x530a, 0x26},//m_nMaxSharpen
	{0x530b, 0x02},//m_nMinDetail
	{0x530c, 0x02},//m_nMaxDetail
	{0x530d, 0x00},//m_nDetailRatioList[0]
	{0x530e, 0x0c},//m_nDetailRatioList[1]
	{0x530f, 0x14},//m_nDetailRatioList[2]
	{0x5310, 0x1a},//m_nSharpenNegEdgeRatio
	{0x5311, 0x20},//m_nClrEdgeShpT1
	{0x5312, 0x80},//m_nClrEdgeShpT2
	{0x5313, 0x4b},//m_nClrEdgeShpSlope
	{0x5380, 0x01},//nCCM_D[0][0] H
	{0x5381, 0x0c},//nCCM_D[0][0] L
	{0x5382, 0x00},//nCCM_D[0][1] H
	{0x5383, 0x16},//nCCM_D[0][1] L
	{0x5384, 0x00},//nCCM_D[1][0] H
	{0x5385, 0xb3},//nCCM_D[1][0] L
	{0x5386, 0x00},//nCCM_D[1][1] H
	{0x5387, 0x7e},//nCCM_D[1][1] L
	{0x5388, 0x00},//nCCM_D[2][0] H
	{0x5389, 0x07},//nCCM_D[2][0] L
	{0x538a, 0x01},//nCCM_D[2][1] H
	{0x538b, 0x35},//nCCM_D[2][1] L
	{0x538c, 0x00},//sign bit of nCCM_D[2][1], [2][0], [1][1], [1][0], [0][1], [0][0]
	{0x5400, 0x0d},//Gamma, m_pCurveYlist[0]
	{0x5401, 0x18},//m_pCurveYlist[1]
	{0x5402, 0x31},//m_pCurveYlist[2]
	{0x5403, 0x5a},//m_pCurveYlist[3]
	{0x5404, 0x65},//m_pCurveYlist[4]
	{0x5405, 0x6f},//m_pCurveYlist[5]
	{0x5406, 0x77},//m_pCurveYlist[6]
	{0x5407, 0x80},//m_pCurveYlist[7]
	{0x5408, 0x87},//m_pCurveYlist[8]
	{0x5409, 0x8f},//m_pCurveYlist[9]
	{0x540a, 0xa2},//m_pCurveYlist[10]
	{0x540b, 0xb2},//m_pCurveYlist[11]
	{0x540c, 0xcc},//m_pCurveYlist[12]
	{0x540d, 0xe4},//m_pCurveYlist[13]
	{0x540e, 0xf0},//m_pCurveYlist[14]
	{0x540f, 0xa0},//m_nMaxShadowHGain
	{0x5410, 0x6e},//m_nMidTondHGain
	{0x5411, 0x06},//m_nHighLightGain
	{0x5480, 0x19},//RGB_DNS, m_nShadowExtraNoise = 12, m_bSmoothYEnable
	{0x5481, 0x00},//m_nNoiseYList[1], m_nNoiseYList[0]
	{0x5482, 0x09},//m_nNoiseYList[3], m_nNoiseYList[2]
	{0x5483, 0x12},//m_nNoiseYList[5], m_nNoiseYList[4]
	{0x5484, 0x04},//m_nNoiseUVList[0]
	{0x5485, 0x06},//m_nNoiseUVList[1]
	{0x5486, 0x08},//m_nNoiseUVList[2]
	{0x5487, 0x0c},//m_nNoiseUVList[3]
	{0x5488, 0x10},//m_nNoiseUVList[4]
	{0x5489, 0x18},//m_nNoiseUVList[5]
	{0x5500, 0x02},//UV_DNS, m_nNoiseList[0]
	{0x5501, 0x03},//m_nNoiseList[1]
	{0x5502, 0x04},//m_nNoiseList[2]
	{0x5503, 0x05},//m_nNoiseList[3]
	{0x5504, 0x06},//m_nNoiseList[4]
	{0x5505, 0x08},//m_nNoiseList[5]
	{0x5506, 0x00},//m_nShadowExtraNoise = 0
	{0x5600, 0x06},//SDE, saturation_en, contrast_en
	{0x5603, 0x40},//sat u
	{0x5604, 0x28},//sat v
	{0x5609, 0x20},//uvadjust_th1
	{0x560a, 0x60},//uvadjust_th2
	{0x560b, 0x00},//uvadjust_auto
	{0x5780, 0x3e},//DPC
	{0x5781, 0x0f},
	{0x5782, 0x04},
	{0x5783, 0x02},
	{0x5784, 0x01},
	{0x5785, 0x01},
	{0x5786, 0x00},
	{0x5787, 0x04},
	{0x5788, 0x02},
	{0x5789, 0x00},
	{0x578a, 0x01},
	{0x578b, 0x02},
	{0x578c, 0x03},
	{0x578d, 0x03},
	{0x578e, 0x08},
	{0x578f, 0x0c},
	{0x5790, 0x08},
	{0x5791, 0x04},
	{0x5792, 0x00},
	{0x5793, 0x00},
	{0x5794, 0x03},//DPC
	{0x5800, 0x03},//Lenc, red x H
	{0x5801, 0x14},//red x L
	{0x5802, 0x02},//red y H
	{0x5803, 0x64},//red y L
	{0x5804, 0x1e},//red_a1
	{0x5805, 0x05},//red_a2
	{0x5806, 0x2a},//red_b1
	{0x5807, 0x05},//red_b2
	{0x5808, 0x03},//green x H
	{0x5809, 0x17},//green x L
	{0x580a, 0x02},//green y H
	{0x580b, 0x63},//green y L
	{0x580c, 0x1a},//green a1
	{0x580d, 0x05},//green a2
	{0x580e, 0x1f},//green b1
	{0x580f, 0x05},//green b2
	{0x5810, 0x03},//blue x H
	{0x5811, 0x0c},//blue x L
	{0x5812, 0x02},//blue y H
	{0x5813, 0x5e},//blue Y L
	{0x5814, 0x18},//blue a1
	{0x5815, 0x05},//blue a2
	{0x5816, 0x19},//blue b1
	{0x5817, 0x05},//blue b2
	{0x5818, 0x0d},//rst_seed, rnd_en, gcoef_en
	{0x5819, 0x40},//lenc_coef_th
	{0x581a, 0x04},//lenc_gain_thre1
	{0x581b, 0x0c},//lenc_gain__thre2
	{0x3106, 0x21},//byp_arb
	{0x3784, 0x08},
	//AEC/AGC							  
//	{0x3a03, 0x4c},//AEC H					 
//	{0x3a04, 0x40},//AEC L
	{0x3a03, 0x24},
	{0x3a04, 0x18},
	{0x3503, 0x00},//AEC auto, AGC auto 	 
	{0x3a02, 0x90},//50Hz, speed ratio = 0x10
	{0x0100, 0x01},//stream on
};

static struct regval_list sensor_uxga_regs[] = {    //1600*1200
	{0x3503, 0x03},// AEC off, AGC off
	{0x3086, 0x03},// PLL
	{0x370a, 0x21},// Sensor Conrol
	{0x3801, 0x00},// x start L
	{0x3803, 0x00},// y start L
	{0x3804, 0x06},// x end H
	{0x3805, 0x4f},// x end L
	{0x3806, 0x04},// y end H
	{0x3807, 0xbf},// y end L
	{0x3808, 0x06},// x output size H
	{0x3809, 0x40},// x output size L
	{0x380a, 0x04},// y output size H
	{0x380b, 0xb0},// y output size L
	{0x380c, 0x06},// HTS H
	{0x380d, 0xa4},// HTS L
	{0x380e, 0x05},// VTS H
	{0x380f, 0x0e},// VTS L
	{0x3811, 0x08},// isp x win L
	{0x3813, 0x08},// isp y win L
	{0x3814, 0x11},// x inc
	{0x3815, 0x11},// y inc
	{0x3820, 0xc0},//vsub48_blc, vflip_blc
	{0x3821, 0x00},//hbin off
	{0x3a07, 0x61},// B50 L
	{0x3a09, 0x50},// B60 L
	{0x3a0a, 0x07},// max exp 50 H
	{0x3a0b, 0x94},// max exp 50 L
	{0x3a0c, 0x07},// max exp 60 H
	{0x3a0d, 0x94},// max exp 60 L
	{0x3a0e, 0x04},// VTS band 50 H
	{0x3a0f, 0xed},// VTS band 50 L
	{0x3a10, 0x05},// VTS band 60 H
	{0x3a11, 0x08},// VTS band 60 L
	{0x4008, 0x02},// bl_start
	{0x4009, 0x09},// bl_end
};
static struct regval_list sensor_720p_regs[] = { //1280*720
	{0x3503, 0x00},//AGC on, AEC on
	{0x3086, 0x01},//PLL
	{0x370a, 0x21},//Sensor Conrol
	{0x3801, 0xa0},//x start L
	{0x3803, 0xf2},//y start L
	{0x3804, 0x05},//x end H
	{0x3805, 0xaf},//x end L
	{0x3806, 0x03},//y end H
	{0x3807, 0xcd},//y end L
	{0x3808, 0x05},//x output size H
	{0x3809, 0x00},//x output size L
	{0x380a, 0x02},//y output size H
	{0x380b, 0xd0},//y output size L
	{0x380c, 0x05},//HTS H
	{0x380d, 0xa6},//HTS L
	{0x380e, 0x02},//VTS H
	{0x380f, 0xf8},//VTS L
	{0x3811, 0x08},//isp x win L
	{0x3813, 0x06},//isp y win L
	{0x3814, 0x11},//x inc
	{0x3815, 0x11},//y inc
	{0x3820, 0xc0},//vsub48_blc, vflip_blc
	{0x3821, 0x00},//hbin off
	{0x3a07, 0xe4},//B50 L
	{0x3a09, 0xbd},//B60 L
	{0x3a0a, 0x0e},//max exp 50 H
	{0x3a0b, 0x40},//max exp 50 L
	{0x3a0c, 0x17},//max exp 60 H
	{0x3a0d, 0xc0},//max exp 60 L
	{0x3a0e, 0x02},//VTS band 50 H
	{0x3a0f, 0xac},//VTS band 50 L
	{0x3a10, 0x02},//VTS band 60 H
	{0x3a11, 0xf8},//VTS band 60 L
	{0x4008, 0x02},//bl_start
	{0x4009, 0x09},//bl_end
};

static struct regval_list sensor_svga_regs[] = { //SVGA: 800*600
	{0x3503, 0x00},//AGC on, AEC on
	{0x3086, 0x01},//PLL
	{0x370a, 0x23},//Sensor Conrol
	{0x3801, 0x00},//x start L
	{0x3803, 0x00},//y start L
	{0x3804, 0x06},//x end H
	{0x3805, 0x4f},//x end L
	{0x3806, 0x04},//y end H
	{0x3807, 0xbf},//y end L
	{0x3808, 0x03},//x output size H
	{0x3809, 0x20},//x output size L
	{0x380a, 0x02},//y output size H
	{0x380b, 0x58},//y output size L
	{0x380c, 0x06},//HTS H
	{0x380d, 0xac},//HTS L
	{0x380e, 0x02},//VTS H
	{0x380f, 0x84},//VTS L
	{0x3811, 0x04},//isp x win L
	{0x3813, 0x04},//isp y win L
	{0x3814, 0x31},//x inc
	{0x3815, 0x31},//y inc
	{0x3820, 0xc2},//vsub48_blc, vflip_blc, vbinf
	{0x3821, 0x01},//hbin
	{0x3a07, 0xc2},//B50 L
	{0x3a09, 0xa1},//B60 L
	{0x3a0a, 0x07},//max exp 50 H
	{0x3a0b, 0x94},//max exp 50 L
	{0x3a0c, 0x07},//max exp 60 H
	{0x3a0d, 0x94},//max exp 60 L
	{0x3a0e, 0x02},//VTS band 50 H
	{0x3a0f, 0x46},//VTS band 50 L
	{0x3a10, 0x02},//VTS band 60 H
	{0x3a11, 0x84},//VTS band 60 L
	{0x4008, 0x00},//bl_start
	{0x4009, 0x03},//bl_end
};
/*
 * The white balance settings
 * Here only tune the R G B channel gain.
 * The white balance enalbe bit is modified in sensor_s_autowb and sensor_s_wb
 */

static struct regval_list sensor_wb_manual[] = {
};

static struct regval_list sensor_wb_auto_regs[] = {
	{0x3208, 0x00},//start group 1 
	{0x5180, 0xf4},//			   
	{0x3208, 0x10},//end group 1   
	{0x3208, 0xa0},//launch group 1
};

static struct regval_list sensor_wb_incandescence_regs[] = {
	{0x3208, 0x00},// start group 1 
	{0x5180, 0xf6},//				
	{0x5195, 0x04},//R Gain 		
	{0x5196, 0x90},//				
	{0x5197, 0x04},//G Gain 		
	{0x5198, 0x00},//				
	{0x5199, 0x09},//B Gain 		
	{0x519a, 0x20},//				
	{0x3208, 0x10},// end group 1	
	{0x3208, 0xa0},// launch group 1
};

static struct regval_list sensor_wb_fluorescent_regs[] = {
	{0x3208, 0x00},// start group 1 
	{0x5180, 0xf6},//               
	{0x5195, 0x06},//R Gain         
	{0x5196, 0xb8},//               
	{0x5197, 0x04},//G Gain         
	{0x5198, 0x00},//               
	{0x5199, 0x06},//B Gain         
	{0x519a, 0x5f},//               
	{0x3208, 0x10},// end group 1   
	{0x3208, 0xa0},// launch group 1 
};

static struct regval_list sensor_wb_tungsten_regs[] = {
	{0x3208, 0x00},// start group 1 
	{0x5180, 0xf6},//               
	{0x5195, 0x04},//R Gain         
	{0x5196, 0x90},//               
	{0x5197, 0x04},//G Gain         
	{0x5198, 0x00},//               
	{0x5199, 0x09},//B Gain         
	{0x519a, 0x20},//               
	{0x3208, 0x10},// end group 1   
	{0x3208, 0xa0},// launch group 1
};

static struct regval_list sensor_wb_horizon[] = {
};

static struct regval_list sensor_wb_daylight_regs[] = {
	{0x3208, 0x00},// start group 1 
	{0x5180, 0xf6},//				
	{0x5195, 0x07},//R Gain 		
	{0x5196, 0x9c},//				
	{0x5197, 0x04},//G Gain 		
	{0x5198, 0x00},//				
	{0x5199, 0x05},//B Gain 		
	{0x519a, 0xf3},//				
	{0x3208, 0x10},// end group 1	
	{0x3208, 0xa0},// launch group 1
};

static struct regval_list sensor_wb_flash[] = {
};

static struct regval_list sensor_wb_cloud_regs[] = {
	{0x3208, 0x00},// start group 1 	
	{0x5180, 0xf6},//					
	{0x5195, 0x07},//R Gain 			
	{0x5196, 0xdc},//					
	{0x5197, 0x04},//G Gain 			
	{0x5198, 0x00},//					
	{0x5199, 0x05},//B Gain 			
	{0x519a, 0xd3},//					
	{0x3208, 0x10},// end group 1		
	{0x3208, 0xa0},// launch group 1
};

static struct regval_list sensor_wb_shade[] = {
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
	{0x3208, 0x00},//start group 1 
	{0x5600, 0x06},//			   
	{0x5603, 0x40},//			   
	{0x5604, 0x28},//			   
	{0x3208, 0x10},//end group 1   
	{0x3208, 0xa0},//launch group 1
};

static struct regval_list sensor_colorfx_bw_regs[] = {
	{0x3208, 0x00},//start group 1
	{0x5600, 0x1c},//fixu_en <4>, fixv_en <3>
	{0x5603, 0x80},//Ureg 
	{0x5604, 0x80},//Yreg
	{0x3208, 0x10},//end group 1
	{0x3208, 0xa0},//launch group 1
};

static struct regval_list sensor_colorfx_sepia_regs[] = {
	{0x3208, 0x00},//start group 1
	{0x5600, 0x1c},//
	{0x5603, 0x40},//
	{0x5604, 0xa0},//
	{0x3208, 0x10},//end group 1
	{0x3208, 0xa0},//launch group 1
};

static struct regval_list sensor_colorfx_negative_regs[] = {
	{0x3208, 0x00},//start group 1
	{0x5600, 0x46},//
	{0x5603, 0x40},//
	{0x5604, 0x28},//
	{0x3208, 0x10},//end group 1
	{0x3208, 0xa0},//launch group 1
};

static struct regval_list sensor_colorfx_emboss_regs[] = {
};

static struct regval_list sensor_colorfx_sketch_regs[] = {
};

static struct regval_list sensor_colorfx_sky_blue_regs[] = {
	{0x3208, 0x00},//start group 1
	{0x5600, 0x1c},//
	{0x5603, 0xa0},//
	{0x5604, 0x40},//
	{0x3208, 0x10},//end group 1
	{0x3208, 0xa0},//launch group 1
};

static struct regval_list sensor_colorfx_grass_green_regs[] = {
	{0x3208, 0x00},//start group 1
	{0x5600, 0x1c},//
	{0x5603, 0x60},//
	{0x5604, 0x60},//
	{0x3208, 0x10},//end group 1
	{0x3208, 0xa0},//launch group 1
};

static struct regval_list sensor_colorfx_skin_whiten_regs[] = {
};

static struct regval_list sensor_colorfx_vivid_regs[] = { 
};

static struct regval_list sensor_colorfx_aqua_regs[] = {
};

static struct regval_list sensor_colorfx_art_freeze_regs[] = {
};

static struct regval_list sensor_colorfx_silhouette_regs[] = {
};

static struct regval_list sensor_colorfx_solarization_regs[] = {
};

static struct regval_list sensor_colorfx_antique_regs[] = {
};

static struct regval_list sensor_colorfx_set_cbcr_regs[] = {
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
//NULL
};

static struct regval_list sensor_brightness_neg3_regs[] = {
	{0x3208, 0x00},//start group 1	 
	{0x5607, 0x30},//				 
	{0x5608, 0x08},//				 
	{0x3208, 0x10},//end group 1	 
	{0x3208, 0xa0},//launch group 1 
};

static struct regval_list sensor_brightness_neg2_regs[] = {
	{0x3208, 0x00},//start group 1	
	{0x5607, 0x20},//				
	{0x5608, 0x08},//				
	{0x3208, 0x10},//end group 1	
	{0x3208, 0xa0},//launch group 1
};

static struct regval_list sensor_brightness_neg1_regs[] = {
	{0x3208, 0x00},//start group 1	
	{0x5607, 0x10},//				
	{0x5608, 0x08},//				
	{0x3208, 0x10},//end group 1	
	{0x3208, 0xa0},//launch group 1 
};

static struct regval_list sensor_brightness_zero_regs[] = {
	{0x3208, 0x00},//start group 1	
	{0x5607, 0x00},//				
	{0x5608, 0x00},//				
	{0x3208, 0x10},//end group 1	
	{0x3208, 0xa0},//launch group 1 	
};

static struct regval_list sensor_brightness_pos1_regs[] = {
	{0x3208, 0x00},//start group 1 
	{0x5607, 0x10},//			   
	{0x5608, 0x00},//			   
	{0x3208, 0x10},//end group 1   
	{0x3208, 0xa0},//launch group 1
};

static struct regval_list sensor_brightness_pos2_regs[] = {
	{0x3208, 0x00},//start group 1	  
	{0x5607, 0x20},//				  
	{0x5608, 0x00},//				  
	{0x3208, 0x10},//end group 1	  
	{0x3208, 0xa0},//launch group 1 
};

static struct regval_list sensor_brightness_pos3_regs[] = {
	{0x3208, 0x00},//start group 1  
	{0x5607, 0x30},//			   
	{0x5608, 0x00},//			   
	{0x3208, 0x10},//end group 1    
	{0x3208, 0xa0},//launch group 1
};

static struct regval_list sensor_brightness_pos4_regs[] = {
//NULL
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
//NULL
};

static struct regval_list sensor_contrast_neg3_regs[] = {
	{0x3208, 0x00},//start group 1				   
	{0x5002, 0x33},//nable manual offset in contrast
	{0x5606, 0x14},//							   
	{0x5605, 0x14},//							   
	{0x3208, 0x10},//end group 1 				   
	{0x3208, 0xa0},//launch group 1
};

static struct regval_list sensor_contrast_neg2_regs[] = {
	{0x3208, 0x00},//start group 1				   
	{0x5002, 0x33},//nable manual offset in contrast
	{0x5606, 0x18},//							   
	{0x5605, 0x18},//							   
	{0x3208, 0x10},//end group 1 				   
	{0x3208, 0xa0},//launch group 1 
};

static struct regval_list sensor_contrast_neg1_regs[] = {
	{0x3208, 0x00},//start group 1				   
	{0x5002, 0x33},//nable manual offset in contrast
	{0x5606, 0x1c},//							   
	{0x5605, 0x1c},//							   
	{0x3208, 0x10},//end group 1 				   
	{0x3208, 0xa0},//launch group 1
};

static struct regval_list sensor_contrast_zero_regs[] = {
	{0x3208, 0x00},//start group 1				   
	{0x5002, 0x32},//nable manual offset in contrast
	{0x5606, 0x20},//ain 						   
	{0x5605, 0x00},//ffset						   
	{0x3208, 0x10},//end group 1 				   
	{0x3208, 0xa0},//launch group 1
};

static struct regval_list sensor_contrast_pos1_regs[] = {
	{0x3208, 0x00},//start group 1				   
	{0x5002, 0x33},//nable manual offset in contrast
	{0x5606, 0x24},//							   
	{0x5605, 0x10},//							   
	{0x3208, 0x10},//end group 1 				   
	{0x3208, 0xa0},//launch group 1
};

static struct regval_list sensor_contrast_pos2_regs[] = {
	{0x3208, 0x00},//start group 1				   
	{0x5002, 0x33},//nable manual offset in contrast
	{0x5606, 0x28},//							   
	{0x5605, 0x18},//							   
	{0x3208, 0x10},//end group 1 				   
	{0x3208, 0xa0},//launch group 1
};

static struct regval_list sensor_contrast_pos3_regs[] = {
	{0x3208, 0x00},//start group 1				   
	{0x5002, 0x33},//nable manual offset in contrast
	{0x5606, 0x2c},//							   
	{0x5605, 0x1c},//							   
	{0x3208, 0x10},//end group 1 				   
	{0x3208, 0xa0},//launch group 1 
};

static struct regval_list sensor_contrast_pos4_regs[] = {
//NULL
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
//NULL
};

static struct regval_list sensor_saturation_neg3_regs[] = {
	{0x3208, 0x00},//enable group 0
	{0x5380, 0x0 },//
	{0x5381, 0xE9},//
	{0x5382, 0x0 },//
	{0x5383, 0x13},//
	{0x5384, 0x0 },//
	{0x5385, 0x9C},//
	{0x5386, 0x0 },//
	{0x5387, 0x6E},//
	{0x5388, 0x0 },//
	{0x5389, 0x6 },//
	{0x538a, 0x1 },//
	{0x538b, 0xD },//
	{0x538c, 0x0 },//
	{0x3208, 0x10},//end group 0
	{0x3208, 0xa0},//launch group 0	
};

static struct regval_list sensor_saturation_neg2_regs[] = {
	{0x3208, 0x00},// enable group 0
	{0x5380, 0x0 },//
	{0x5381, 0xF4},//
	{0x5382, 0x0 },//
	{0x5383, 0x14},//
	{0x5384, 0x0 },//
	{0x5385, 0xA3},//
	{0x5386, 0x0 },//
	{0x5387, 0x73},//
	{0x5388, 0x0 },//
	{0x5389, 0x6 },//
	{0x538a, 0x1 },//
	{0x538b, 0x19},//
	{0x538c, 0x0 },//
	{0x3208, 0x10},//end group 0
	{0x3208, 0xa0},//launch group 0
};

static struct regval_list sensor_saturation_neg1_regs[] = {
	{0x3208, 0x00},//enable group 0
	{0x5380, 0x0 },//
	{0x5381, 0xFF},//
	{0x5382, 0x0 },//
	{0x5383, 0x15},//
	{0x5384, 0x0 },//
	{0x5385, 0xAA},//
	{0x5386, 0x0 },//
	{0x5387, 0x78},//
	{0x5388, 0x0 },//
	{0x5389, 0x7 },//
	{0x538a, 0x1 },//
	{0x538b, 0x26},//
	{0x538c, 0x0 },//
	{0x3208, 0x10},//end group 0
	{0x3208, 0xa0},//launch group 0
};

static struct regval_list sensor_saturation_zero_regs[] = {
	{0x3208, 0x00},//enable group 0
	{0x5380, 0x1 },//
	{0x5381, 0xC },//
	{0x5382, 0x0 },//
	{0x5383, 0x16},//
	{0x5384, 0x0 },//
	{0x5385, 0xB3},//
	{0x5386, 0x0 },//
	{0x5387, 0x7E},//
	{0x5388, 0x0 },//
	{0x5389, 0x7 },//
	{0x538a, 0x1 },//
	{0x538b, 0x35},//
	{0x538c, 0x0 },//
	{0x3208, 0x10},//end group 0
	{0x3208, 0xa0},//launch group 0
};

static struct regval_list sensor_saturation_pos1_regs[] = {
	{0x3208, 0x00},//enable group 0
	{0x5380, 0x1 },//
	{0x5381, 0x19},//
	{0x5382, 0x0 },//
	{0x5383, 0x17},//
	{0x5384, 0x0 },//
	{0x5385, 0xBC},//
	{0x5386, 0x0 },//
	{0x5387, 0x84},//
	{0x5388, 0x0 },//
	{0x5389, 0x7 },//
	{0x538a, 0x1 },//
	{0x538b, 0x44},//
	{0x538c, 0x0 },//
	{0x3208, 0x10},//end group 0
	{0x3208, 0xa0},//launch group 0
};

static struct regval_list sensor_saturation_pos2_regs[] = {
	{0x3208, 0x00},//enable group 0
	{0x5380, 0x1 },//
	{0x5381, 0x27},//
	{0x5382, 0x0 },//
	{0x5383, 0x18},//
	{0x5384, 0x0 },//
	{0x5385, 0xC5},//
	{0x5386, 0x0 },//
	{0x5387, 0x8B},//
	{0x5388, 0x0 },//
	{0x5389, 0x8 },//
	{0x538a, 0x1 },//
	{0x538b, 0x54},//
	{0x538c, 0x0 },//
	{0x3208, 0x10},//end group 0
	{0x3208, 0xa0},//launch group 0
};

static struct regval_list sensor_saturation_pos3_regs[] = {
	{0x3208, 0x00},//enable group 0
	{0x5380, 0x1 },//
	{0x5381, 0x34},//
	{0x5382, 0x0 },//
	{0x5383, 0x19},//
	{0x5384, 0x0 },//
	{0x5385, 0xCE},//
	{0x5386, 0x0 },//
	{0x5387, 0x91},//
	{0x5388, 0x0 },//
	{0x5389, 0x8 },//
	{0x538a, 0x1 },//
	{0x538b, 0x63},//
	{0x538c, 0x0 },//
	{0x3208, 0x10},//end group 0
	{0x3208, 0xa0},//launch group 0
};

static struct regval_list sensor_saturation_pos4_regs[] = {
//NULL
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
//NULL
};

static struct regval_list sensor_ev_neg3_regs[] = {
	{0x3a03, 0x0c},
	{0x3a04, 0x00},
};

static struct regval_list sensor_ev_neg2_regs[] = {
	{0x3a03, 0x14},
	{0x3a04, 0x08},
};

static struct regval_list sensor_ev_neg1_regs[] = {
	{0x3a03, 0x1c},
	{0x3a04, 0x10},
};

static struct regval_list sensor_ev_zero_regs[] = {
	{0x3a03, 0x24},
	{0x3a04, 0x18},
};

static struct regval_list sensor_ev_pos1_regs[] = {
	{0x3a03, 0x2c},
	{0x3a04, 0x20},
};

static struct regval_list sensor_ev_pos2_regs[] = {
	{0x3a03, 0x34}, 
	{0x3a04, 0x28},
};

static struct regval_list sensor_ev_pos3_regs[] = {
	{0x3a03, 0x3c},
	{0x3a04, 0x30},
};

static struct regval_list sensor_ev_pos4_regs[] = {
//NULL
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
  	{0x4300,0x30},  //YUYV 
};

static struct regval_list sensor_fmt_yuv422_yvyu[] = {
   	{0x4300,0x31},  //YVYU
};

static struct regval_list sensor_fmt_yuv422_vyuy[] = {
   	{0x4300,0x33},  //VYUY
};

static struct regval_list sensor_fmt_yuv422_uyvy[] = {
  	{0x4300,0x32},  //UYVY 
};

/* *********************************************begin of ******************************************** */

static int sensor_g_hflip(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);
	data_type val;

	LOG_ERR_RET(sensor_read(sd, 0x3821, &val));

	val &= (1<<2);
	val >>= 2;		//0x3821 bit2 is mirror
	*value = val;
	info->hflip = *value;
	
	return 0;
}

static int sensor_s_hflip(struct v4l2_subdev *sd, int value)
{
	struct sensor_info *info = to_state(sd);
	data_type val;
	
	if(info->hflip == value)
		return 0;
	
	LOG_ERR_RET(sensor_read(sd, 0x3821, &val))

	switch (value) {
	case 0:
		val &= 0xfb;
		break;
	case 1:
		val |= 0x04;
		break;
	default:
		return -EINVAL;
	}
	
	LOG_ERR_RET(sensor_write(sd, 0x3821, val))
	msleep(50);
	info->hflip = value;

	return 0;
}

static int sensor_g_vflip(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);
	data_type val;

	LOG_ERR_RET(sensor_read(sd, 0x3820, &val));

	val &= (1<<2);
	val >>= 2;		//0x3820 bit2 is flip
	*value = val;
	info->hflip = *value;
	
	return 0;

}

static int sensor_s_vflip(struct v4l2_subdev *sd, int value)
{
	struct sensor_info *info = to_state(sd);
	data_type val;

	if(info->vflip == value)
		return 0;

	LOG_ERR_RET(sensor_read(sd, 0x3820, &val))

	switch (value) {
	case 0:
		val &= 0xfb;
		break;
	case 1:
		val |= 0x04;
		break;
	default:
		return -EINVAL;
	}

	LOG_ERR_RET(sensor_write(sd, 0x3820, val))		
	msleep(50);
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
	struct sensor_info *info = to_state(sd);
	data_type val;
	
	LOG_ERR_RET(sensor_read(sd, 0x3503, &val))
	
	if ((val&0x01) == 0x01)
	{
		*value = V4L2_EXPOSURE_MANUAL;
	}
	else
	{
	  	*value = V4L2_EXPOSURE_AUTO;
	}
	
	info->autoexp = *value;
	return 0;

}

static int sensor_s_autoexp(struct v4l2_subdev *sd,
    enum v4l2_exposure_auto_type value)
{
	struct sensor_info *info = to_state(sd);
	data_type val;

	LOG_ERR_RET(sensor_read(sd, 0x3503, &val))

	switch (value) {
	case V4L2_EXPOSURE_AUTO:
		val &= 0xfe;
		break;
	case V4L2_EXPOSURE_MANUAL:
		val |= 0x01;
		break;
	case V4L2_EXPOSURE_SHUTTER_PRIORITY:
		return -EINVAL;	 
	case V4L2_EXPOSURE_APERTURE_PRIORITY:
		return -EINVAL;
	default:
		return -EINVAL;
	}

	LOG_ERR_RET(sensor_write(sd, 0x3503, val))
	msleep(50);
	info->autoexp = value; 
	
	return 0;

}

static int sensor_g_autowb(struct v4l2_subdev *sd, int *value)
{
	struct sensor_info *info = to_state(sd);
	data_type val;
	
	LOG_ERR_RET(sensor_read(sd, 0x5004, &val))
	
	val &= 0x01;		//0x5004 bit0 is awb enable	  
	*value = val;
	info->autowb = *value;
	
	return 0;

}

static int sensor_s_autowb(struct v4l2_subdev *sd, int value)
{
	struct sensor_info *info = to_state(sd);
	data_type val;
	
	if(info->autowb == value)
		return 0;
	
	LOG_ERR_RET(sensor_write_array(sd, sensor_wb_auto_regs ,ARRAY_SIZE(sensor_wb_auto_regs)))  
	LOG_ERR_RET(sensor_read(sd, 0x5004, &val))
	
	switch(value) {
	case 0:
		val |= 0x01;
		break;
	case 1:
		val &= 0xfe;
		break;
	default:
		break;
	}
	
	LOG_ERR_RET(sensor_write(sd, 0x5004, val))
	
	msleep(50);	
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
static int sensor_g_band_filter(struct v4l2_subdev *sd, 
    __s32 *value)
{
	struct sensor_info *info = to_state(sd);
	data_type rdval;

	LOG_ERR_RET(sensor_read(sd, 0x3a00, &rdval))

	if((rdval & 0x01) == 0)
		info->band_filter = V4L2_CID_POWER_LINE_FREQUENCY_DISABLED;
	else 
	{
		LOG_ERR_RET(sensor_read(sd, 0x3a02, &rdval))
		if((rdval & 0x80) == 0x80)
			info->band_filter = V4L2_CID_POWER_LINE_FREQUENCY_50HZ;
		else
			info->band_filter = V4L2_CID_POWER_LINE_FREQUENCY_60HZ;
	}
	return 0;
}

static int sensor_s_band_filter(struct v4l2_subdev *sd, 
    enum v4l2_power_line_frequency value)
{
	struct sensor_info *info = to_state(sd);
	data_type rdval;

	if(info->band_filter == value)
		return 0;

	switch(value) {
	case V4L2_CID_POWER_LINE_FREQUENCY_DISABLED:  
		LOG_ERR_RET(sensor_read(sd,0x3a00,&rdval))
		LOG_ERR_RET(sensor_write(sd,0x3a00,rdval&0xfe))//turn off band filter ,bit[0] = 0
		break;
	case V4L2_CID_POWER_LINE_FREQUENCY_50HZ:
		LOG_ERR_RET(sensor_read(sd,0x3a02,&rdval))//50hz 
		LOG_ERR_RET(sensor_write(sd,0x3a02,rdval|0x80))//manual band filter, bit[7]=1
		LOG_ERR_RET(sensor_read(sd,0x3a00,&rdval))
		LOG_ERR_RET(sensor_write(sd,0x3a00,rdval|0x01))//turn on band filter, bit[0] = 1
		break;
	case V4L2_CID_POWER_LINE_FREQUENCY_60HZ:
		LOG_ERR_RET(sensor_read(sd,0x3a02,&rdval))//60hz 
		LOG_ERR_RET(sensor_write(sd,0x3a02,rdval&0x7f))//manual band filter, bit[7]=0
		LOG_ERR_RET(sensor_read(sd,0x3a00,&rdval))
		LOG_ERR_RET(sensor_write(sd,0x3a00,rdval|0x01))//turn on band filter, bit[0] = 1
		break;
	case V4L2_CID_POWER_LINE_FREQUENCY_AUTO:
		break;
	default:
		break;
	}
	msleep(50);
	info->band_filter = value;
	return 0;
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

	if(info->wb == value)
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
	switch(on) {
	case CSI_SUBDEV_STBY_ON:
		vfe_dev_dbg("CSI_SUBDEV_STBY_ON!\n");
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
		cci_unlock(sd);
		break;
	case CSI_SUBDEV_PWR_ON:
		vfe_dev_dbg("CSI_SUBDEV_PWR_ON!\n");
		cci_lock(sd);
		vfe_gpio_set_status(sd,PWDN,1);//set the gpio to output
		vfe_gpio_set_status(sd,RESET,1);//set the gpio to output
		vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
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
		vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
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
		vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
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
	data_type rdval;
	
	LOG_ERR_RET(sensor_read(sd, 0x300a, &rdval))
	
	if(rdval != 0x26)
		return -ENODEV;

	LOG_ERR_RET(sensor_read(sd, 0x300b, &rdval))
	if(rdval != 0x85)
	   	return -ENODEV;
			
	return 0;
}

static int sensor_init(struct v4l2_subdev *sd, u32 val)
{
	int ret;
	struct sensor_info *info = to_state(sd);

	vfe_dev_dbg("sensor_init 0x%x\n",val);

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
	info->width = 0;
	info->height = 0;
	info->brightness = 0;
	info->contrast = 0;
	info->saturation = 0;
	info->hue = 0;
	info->hflip = 0;
	info->vflip = 0;
	info->gain = 0;
	info->autogain = 1;
	info->exp_bias = 0;
	info->autoexp = 1;
	info->autowb = 1;
	info->wb = V4L2_WHITE_BALANCE_AUTO;
	info->clrfx = V4L2_COLORFX_NONE;
	info->band_filter = V4L2_CID_POWER_LINE_FREQUENCY_50HZ;

	//  info->af_ctrl = V4L2_AF_RELEASE;
	info->tpf.numerator = 1;
	info->tpf.denominator = 30;    /* 30fps */

	ret = sensor_write_array(sd, sensor_default_regs, ARRAY_SIZE(sensor_default_regs));
	if(ret < 0) {
		vfe_dev_err("write sensor_default_regs error\n");
		return ret;
	}

	sensor_s_band_filter(sd, V4L2_CID_POWER_LINE_FREQUENCY_50HZ);

	if(info->stby_mode == 0)
		info->init_first_flag = 0;

	info->preview_first_flag = 1;
	//  INIT_DELAYED_WORK(&sensor_s_ae_ratio_work, sensor_s_ae_ratio);

	return 0;
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
};
#define N_FMTS ARRAY_SIZE(sensor_formats)

/*
 * Then there is the issue of window sizes.  Try to capture the info here.
 */

static struct sensor_win_size sensor_win_sizes[] = {
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
		.regs       = sensor_720p_regs,
		.regs_size  = ARRAY_SIZE(sensor_720p_regs),
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

	if(info->capture_mode == V4L2_MODE_IMAGE)
	{
		ret = sensor_s_autoexp(sd,V4L2_EXPOSURE_MANUAL);
		if (ret < 0)
			vfe_dev_err("sensor_s_autoexp off err when capturing image!\n");
		ret = sensor_s_autogain(sd,0);
		if (ret < 0)
			vfe_dev_err("sensor_s_autogain off err when capturing image!\n");
		ret = sensor_s_autowb(sd,0); //lock wb
		if (ret < 0)
			vfe_dev_err("sensor_s_autowb off err when capturing image!\n");
	}
	sensor_write_array(sd, sensor_fmt->regs , sensor_fmt->regs_size);

	if (wsize->regs)
		LOG_ERR_RET(sensor_write_array(sd, wsize->regs, wsize->regs_size))
	if (wsize->set_size)
		LOG_ERR_RET(wsize->set_size(sd))

	sensor_s_hflip(sd,info->hflip);
	sensor_s_vflip(sd,info->vflip);

	if(info->capture_mode == V4L2_MODE_VIDEO ||
		info->capture_mode == V4L2_MODE_PREVIEW)
	{
		ret = sensor_s_autoexp(sd,V4L2_EXPOSURE_AUTO);
		if (ret < 0)
			vfe_dev_err("sensor_s_autoexp on err when capturing video!\n");
		ret = sensor_s_autogain(sd,1);
		if (ret < 0)
			vfe_dev_err("sensor_s_autogain on err when capturing video!\n");
		if (info->wb == V4L2_WHITE_BALANCE_AUTO) {
			ret = sensor_s_autowb(sd,1); //unlock wb
			if (ret < 0)
				vfe_dev_err("sensor_s_autowb on err when capturing image!\n");
		}
	}
	info->fmt = sensor_fmt;
	info->width = wsize->width;
	info->height = wsize->height;

	vfe_dev_print("s_fmt set width = %d, height = %d\n",wsize->width,wsize->height);
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
	cp->timeperframe.numerator = info->tpf.numerator;
	cp->timeperframe.denominator = info->tpf.denominator;

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
	case V4L2_CID_BRIGHTNESS:
		return v4l2_ctrl_query_fill(qc, -4, 4, 1, 1);
	case V4L2_CID_CONTRAST:
		return v4l2_ctrl_query_fill(qc, -4, 4, 1, 1);
	case V4L2_CID_SATURATION:
		return v4l2_ctrl_query_fill(qc, -4, 4, 1, 1);
	case V4L2_CID_HUE:
		return v4l2_ctrl_query_fill(qc, -180, 180, 5, 0);
	case V4L2_CID_VFLIP:
	case V4L2_CID_HFLIP:
		return v4l2_ctrl_query_fill(qc, 0, 1, 1, 0);
	case V4L2_CID_GAIN:
		return v4l2_ctrl_query_fill(qc, 0, 255, 1, 128);
	case V4L2_CID_AUTOGAIN:
		return v4l2_ctrl_query_fill(qc, 0, 1, 1, 1);
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
	//vfe_dev_dbg("sensor_g_ctrl ctrl->id=0x%8x\n", ctrl->id);
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
	case V4L2_CID_POWER_LINE_FREQUENCY:
		return sensor_g_band_filter(sd, &ctrl->value);
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
	case V4L2_CID_POWER_LINE_FREQUENCY:
		return sensor_s_band_filter(sd, (enum v4l2_power_line_frequency) ctrl->value);
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
	info->auto_focus = 0;

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

