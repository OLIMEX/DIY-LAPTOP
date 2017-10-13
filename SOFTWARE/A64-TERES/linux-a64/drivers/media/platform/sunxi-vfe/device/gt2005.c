	/*
 * A V4L2 driver for GalaxyCore gt2005 cameras.
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
MODULE_DESCRIPTION("A low-level driver for GalaxyCore GT2005 sensors");
MODULE_LICENSE("GPL");



//for internel driver debug
#define DEV_DBG_EN   		0 
#if(DEV_DBG_EN == 1)		
#define vfe_dev_dbg(x,arg...) printk("[CSI_DEBUG][GT2005]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...) 
#endif

#define vfe_dev_err(x,arg...) printk("[CSI_ERR][GT2005]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[CSI][GT2005]"x,##arg)

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
#define V4L2_IDENT_SENSOR 0x2005

/*
 * Our nominal (default) frame rate.
 */
#define SENSOR_FRAME_RATE 30

/*
 * The gt2005 sits on i2c with ID 0x78
 */
#define I2C_ADDR                    0x78   

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
	{0x0101, 0x00},
	{0x0103, 0x00},

	//Hcount&Vcount
	{0x0105, 0x00},
	{0x0106, 0xF0},
	{0x0107, 0x00},
	{0x0108, 0x1C},

	//Binning&Resoultion
	//1600*1200
	{0x0109, 0x01},
	{0x010A, 0x00},
	{0x010B, 0x00},
	{0x010C, 0x00},
	{0x010D, 0x08},
	{0x010E, 0x00},
	{0x010F, 0x08},
	{0x0110, 0x06},
	{0x0111, 0x40},
	{0x0112, 0x04},
	{0x0113, 0xB0},

	//YUV Mode
	{0x0114, 0x04},//YUYV

	//Picture Effect
	{0x0115, 0x00},

	//PLL&Frame Rate
	{0x0116, 0x02},
	{0x0117, 0x00},
	{0x0118, 0x67},
	{0x0119, 0x02},
	{0x011A, 0x04},
	{0x011B, 0x01},

	//DCLK Polarity
	{0x011C, 0x00},

	//Do not change
	{0x011D, 0x02},
	{0x011E, 0x00},
	
	{0x011F, 0x00},
	{0x0120, 0x1C},
	{0x0121, 0x00},
	{0x0122, 0x04},
	{0x0123, 0x00},
	{0x0124, 0x00},
	{0x0125, 0x00},
	{0x0126, 0x00},
	{0x0127, 0x00},
	{0x0128, 0x00},

	//Contrast
	{0x0200, 0x00},

	//Brightness
	{0x0201, 0x00},

	//Saturation
	{0x0202, 0x40},

	//Do not change
	{0x0203, 0x00},
	{0x0204, 0x03},
	{0x0205, 0x1F},
	{0x0206, 0x0B},
	{0x0207, 0x20},
	{0x0208, 0x00},
	{0x0209, 0x2A},
	{0x020A, 0x01},
                       
	//Sharpness          
	{0x020B, 0x48},
	{0x020C, 0x64},

	//Do not change
	{0x020D, 0xC8},
	{0x020E, 0xBC},
	{0x020F, 0x08},
	{0x0210, 0xD6},
	{0x0211, 0x00},
	{0x0212, 0x20},
	{0x0213, 0x81},
	{0x0214, 0x15},
	{0x0215, 0x00},
	{0x0216, 0x00},
	{0x0217, 0x00},
	{0x0218, 0x46},
	{0x0219, 0x30},
	{0x021A, 0x03},
	{0x021B, 0x28},
	{0x021C, 0x02},
	{0x021D, 0x60},
	{0x021E, 0x00},
	{0x021F, 0x00},
	{0x0220, 0x08},
	{0x0221, 0x08},
	{0x0222, 0x04},
	{0x0223, 0x00},
	{0x0224, 0x1F},
	{0x0225, 0x1E},
	{0x0226, 0x18},
	{0x0227, 0x1D},
	{0x0228, 0x1F},
	{0x0229, 0x1F},
	{0x022A, 0x01},
	{0x022B, 0x04},
	{0x022C, 0x05},
	{0x022D, 0x05},
	{0x022E, 0x04},
	{0x022F, 0x03},
	{0x0230, 0x02},
	{0x0231, 0x1F},
	{0x0232, 0x1A},
	{0x0233, 0x19},
	{0x0234, 0x19},
	{0x0235, 0x1B},
	{0x0236, 0x1F},
	{0x0237, 0x04},
	{0x0238, 0xEE},
	{0x0239, 0xFF},
	{0x023A, 0x00},
	{0x023B, 0x00},
	{0x023C, 0x00},
	{0x023D, 0x00},
	{0x023E, 0x00},
	{0x023F, 0x00},
	{0x0240, 0x00},
	{0x0241, 0x00},
	{0x0242, 0x00},
	{0x0243, 0x21},
	{0x0244, 0x42},
	{0x0245, 0x53},
	{0x0246, 0x54},
	{0x0247, 0x54},
	{0x0248, 0x54},
	{0x0249, 0x33},
	{0x024A, 0x11},
	{0x024B, 0x00},
	{0x024C, 0x00},
	{0x024D, 0xFF},
	{0x024E, 0xEE},
	{0x024F, 0xDD},
	{0x0250, 0x00},
	{0x0251, 0x00},
	{0x0252, 0x00},
	{0x0253, 0x00},
	{0x0254, 0x00},
	{0x0255, 0x00},
	{0x0256, 0x00},
	{0x0257, 0x00},
	{0x0258, 0x00},
	{0x0259, 0x00},
	{0x025A, 0x00},
	{0x025B, 0x00},
	{0x025C, 0x00},
	{0x025D, 0x00},
	{0x025E, 0x00},
	{0x025F, 0x00},
	{0x0260, 0x00},
	{0x0261, 0x00},
	{0x0262, 0x00},
	{0x0263, 0x00},
	{0x0264, 0x00},
	{0x0265, 0x00},
	{0x0266, 0x00},
	{0x0267, 0x00},
	{0x0268, 0x8F},
	{0x0269, 0xA3},
	{0x026A, 0xB4},
	{0x026B, 0x90},
	{0x026C, 0x00},
	{0x026D, 0xD0},
	{0x026E, 0x60},
	{0x026F, 0xA0},
	{0x0270, 0x40},
	{0x0300, 0x81},
	{0x0301, 0x80},
	{0x0302, 0x22},
	{0x0303, 0x06},
	{0x0304, 0x03},
	{0x0305, 0x83},
	{0x0306, 0x00},
	{0x0307, 0x22},
	{0x0308, 0x00},
	{0x0309, 0x55},
	{0x030A, 0x55},
	{0x030B, 0x55},
	{0x030C, 0x54},
	{0x030D, 0x1F},
	{0x030E, 0x0A},
	{0x030F, 0x10},
	{0x0310, 0x04},
	{0x0311, 0xFF},
	{0x0312, 0x08},
	{0x0313, 0x28},
	{0x0314, 0x66},
	{0x0315, 0x96},
	{0x0316, 0x26},
	{0x0317, 0x02},
	{0x0318, 0x08},
	{0x0319, 0x0C},
	
#ifndef A_LIGHT_CORRECTION
	//Normal AWB Setting
	{0x031A, 0x81},
	{0x031B, 0x00},
	{0x031C, 0x3D},
	{0x031D, 0x00},
	{0x031E, 0xF9},
	{0x031F, 0x00},
	{0x0320, 0x24},
	{0x0321, 0x14},
	{0x0322, 0x1A},
	{0x0323, 0x24},
	{0x0324, 0x08},
	{0x0325, 0xF0},
	{0x0326, 0x30},
	{0x0327, 0x17},
	{0x0328, 0x11},
	{0x0329, 0x22},
	{0x032A, 0x2F},
	{0x032B, 0x21},
	{0x032C, 0xDA},
	{0x032D, 0x10},
	{0x032E, 0xEA},
	{0x032F, 0x18},
	{0x0330, 0x29},
	{0x0331, 0x25},
	{0x0332, 0x12},
	{0x0333, 0x0F},
	{0x0334, 0xE0},
	{0x0335, 0x13},
	{0x0336, 0xFF},
	{0x0337, 0x20},
	{0x0338, 0x46},
	{0x0339, 0x04},
	{0x033A, 0x04},
	{0x033B, 0xFF},
	{0x033C, 0x01},
	{0x033D, 0x00},

#else
	//A LIGHT CORRECTION
	{0x031A, 0x81},
	{0x031B, 0x00},
	{0x031C, 0x1D},
	{0x031D, 0x00},
	{0x031E, 0xFD},
	{0x031F, 0x00},
	{0x0320, 0xE1},
	{0x0321, 0x1A},
	{0x0322, 0xDE},
	{0x0323, 0x11},
	{0x0324, 0x1A},
	{0x0325, 0xEE},
	{0x0326, 0x50},
	{0x0327, 0x18},
	{0x0328, 0x25},
	{0x0329, 0x37},
	{0x032A, 0x24},
	{0x032B, 0x32},
	{0x032C, 0xA9},
	{0x032D, 0x32},
	{0x032E, 0xFF},
	{0x032F, 0x7F},
	{0x0330, 0xBA},
	{0x0331, 0x7F},
	{0x0332, 0x7F},
	{0x0333, 0x14},
	{0x0334, 0x81},
	{0x0335, 0x14},
	{0x0336, 0xFF},
	{0x0337, 0x20},
	{0x0338, 0x46},
	{0x0339, 0x04},
	{0x033A, 0x04},
	{0x033B, 0x00},
	{0x033C, 0x00},
	{0x033D, 0x00},
#endif

	//Do not change
	{0x033E, 0x03},
	{0x033F, 0x28},
	{0x0340, 0x02},
	{0x0341, 0x60},
	{0x0342, 0xAC},
	{0x0343, 0x97},
	{0x0344, 0x7F},
	{0x0400, 0xE8},
	{0x0401, 0x40},
	{0x0402, 0x00},
	{0x0403, 0x00},
	{0x0404, 0xF8},
	{0x0405, 0x03},
	{0x0406, 0x03},
	{0x0407, 0x85},
	{0x0408, 0x44},
	{0x0409, 0x1F},
	{0x040A, 0x40},
	{0x040B, 0x33},

	//Lens Shading Correction
	{0x040C, 0xA0},
	{0x040D, 0x00},
	{0x040E, 0x00},
	{0x040F, 0x00},
	{0x0410, 0x0D},
	{0x0411, 0x0D},
	{0x0412, 0x0C},
	{0x0413, 0x04},
	{0x0414, 0x00},
	{0x0415, 0x00},
	{0x0416, 0x07},
	{0x0417, 0x09},
	{0x0418, 0x16},
	{0x0419, 0x14},
	{0x041A, 0x11},
	{0x041B, 0x14},
	{0x041C, 0x07},
	{0x041D, 0x07},
	{0x041E, 0x06},
	{0x041F, 0x02},
	{0x0420, 0x42},
	{0x0421, 0x42},
	{0x0422, 0x47},
	{0x0423, 0x39},
	{0x0424, 0x3E},
	{0x0425, 0x4D},
	{0x0426, 0x46},
	{0x0427, 0x3A},
	{0x0428, 0x21},
	{0x0429, 0x21},
	{0x042A, 0x26},
	{0x042B, 0x1C},
	{0x042C, 0x25},
	{0x042D, 0x25},
	{0x042E, 0x28},
	{0x042F, 0x20},
	{0x0430, 0x3E},
	{0x0431, 0x3E},
	{0x0432, 0x33},
	{0x0433, 0x2E},
	{0x0434, 0x54},
	{0x0435, 0x53},
	{0x0436, 0x3C},
	{0x0437, 0x51},
	{0x0438, 0x2B},
	{0x0439, 0x2B},
	{0x043A, 0x38},
	{0x043B, 0x22},
	{0x043C, 0x3B},
	{0x043D, 0x3B},
	{0x043E, 0x31},
	{0x043F, 0x37},

	//PWB Gain
	{0x0440, 0x00},
	{0x0441, 0x4B},
	{0x0442, 0x00},
	{0x0443, 0x00},
	{0x0444, 0x31},

	{0x0445, 0x00},
	{0x0446, 0x00},
	{0x0447, 0x00},
	{0x0448, 0x00},
	{0x0449, 0x00},
	{0x044A, 0x00},
	{0x044D, 0xE0},
	{0x044E, 0x05},
	{0x044F, 0x07},
	{0x0450, 0x00},
	{0x0451, 0x00},
	{0x0452, 0x00},
	{0x0453, 0x00},
	{0x0454, 0x00},
	{0x0455, 0x00},
	{0x0456, 0x00},
	{0x0457, 0x00},
	{0x0458, 0x00},
	{0x0459, 0x00},
	{0x045A, 0x00},
	{0x045B, 0x00},
	{0x045C, 0x00},
	{0x045D, 0x00},
	{0x045E, 0x00},
	{0x045F, 0x00},

	//GAMMA Correction
	{0x0460, 0x80},
	{0x0461, 0x10},
	{0x0462, 0x10},
	{0x0463, 0x10},
	{0x0464, 0x08},
	{0x0465, 0x08},
	{0x0466, 0x11},
	{0x0467, 0x09},
	{0x0468, 0x23},
	{0x0469, 0x2A},
	{0x046A, 0x2A},
	{0x046B, 0x47},
	{0x046C, 0x52},
	{0x046D, 0x42},
	{0x046E, 0x36},
	{0x046F, 0x46},
	{0x0470, 0x3A},
	{0x0471, 0x32},
	{0x0472, 0x32},
	{0x0473, 0x38},
	{0x0474, 0x3D},
	{0x0475, 0x2F},
	{0x0476, 0x29},
	{0x0477, 0x48},

	//Do not change
	{0x0600, 0x00},
	{0x0601, 0x24},
	{0x0602, 0x45},
	{0x0603, 0x0E},
	{0x0604, 0x14},
	{0x0605, 0x2F},
	{0x0606, 0x01},
	{0x0607, 0x0E},
	{0x0608, 0x0E},
	{0x0609, 0x37},
	{0x060A, 0x18},
	{0x060B, 0xA0},
	{0x060C, 0x20},
	{0x060D, 0x07},
	{0x060E, 0x47},
	{0x060F, 0x90},
	{0x0610, 0x06},
	{0x0611, 0x0C},
	{0x0612, 0x28},
	{0x0613, 0x13},
	{0x0614, 0x0B},
	{0x0615, 0x10},
	{0x0616, 0x14},
	{0x0617, 0x19},
	{0x0618, 0x52},
	{0x0619, 0xA0},
	{0x061A, 0x11},
	{0x061B, 0x33},
	{0x061C, 0x56},
	{0x061D, 0x20},
	{0x061E, 0x28},
	{0x061F, 0x2B},
	{0x0620, 0x22},
	{0x0621, 0x11},
	{0x0622, 0x75},
	{0x0623, 0x49},
	{0x0624, 0x6E},
	{0x0625, 0x80},
	{0x0626, 0x02},
	{0x0627, 0x0C},
	{0x0628, 0x51},
	{0x0629, 0x25},
	{0x062A, 0x01},
	{0x062B, 0x3D},
	{0x062C, 0x04},
	{0x062D, 0x01},
	{0x062E, 0x0C},
	{0x062F, 0x2C},
	{0x0630, 0x0D},
	{0x0631, 0x14},
	{0x0632, 0x12},
	{0x0633, 0x34},
	{0x0634, 0x00},
	{0x0635, 0x00},
	{0x0636, 0x00},
	{0x0637, 0xB1},
	{0x0638, 0x22},
	{0x0639, 0x32},
	{0x063A, 0x0E},
	{0x063B, 0x18},
	{0x063C, 0x88},
	{0x0640, 0xB2},
	{0x0641, 0xC0},
	{0x0642, 0x01},
	{0x0643, 0x26},
	{0x0644, 0x13},
	{0x0645, 0x88},
	{0x0646, 0x64},
	{0x0647, 0x00},
	{0x0681, 0x1B},
	{0x0682, 0xA0},
	{0x0683, 0x28},
	{0x0684, 0x00},
	{0x0685, 0xB0},
	{0x0686, 0x6F},
	{0x0687, 0x33},
	{0x0688, 0x1F},
	{0x0689, 0x44},
	{0x068A, 0xA8},
	{0x068B, 0x44},
	{0x068C, 0x08},
	{0x068D, 0x08},
	{0x068E, 0x00},
	{0x068F, 0x00},
	{0x0690, 0x01},
	{0x0691, 0x00},
	{0x0692, 0x01},
	{0x0693, 0x00},
	{0x0694, 0x00},
	{0x0695, 0x00},
	{0x0696, 0x00},
	{0x0697, 0x00},
	{0x0698, 0x2A},
	{0x0699, 0x80},
	{0x069A, 0x1F},
	{0x069B, 0x00},
	{0x069C, 0x02},
	{0x069D, 0xF5},
	{0x069E, 0x03},
	{0x069F, 0x6D},
	{0x06A0, 0x0C},
	{0x06A1, 0xB8},
	{0x06A2, 0x0D},
	{0x06A3, 0x74},
	{0x06A4, 0x00},
	{0x06A5, 0x2F},
	{0x06A6, 0x00},
	{0x06A7, 0x2F},
	{0x0F00, 0x00},
	{0x0F01, 0x00},

	//Output Enable
	{0x0100, 0x01},
	{0x0102, 0x02},
	{0x0104, 0x03},                    
};

static struct regval_list sensor_uxga_regs[] = {
	//Resoltion Setting : 1600*1200
	//Output Disable
	{0x0100, 0x00},
	{0x0102, 0x00},
	{0x0104, 0x03},  
	
	{0x0109, 0x01},
	{0x010A, 0x00},	
	{0x010B, 0x00},					
	{0x0110, 0x06},					
	{0x0111, 0x40},					
	{0x0112, 0x04},					
	{0x0113, 0xb0},
	
	//PLL&Frame Rate 15fps
	{0x0116, 0x02},
	{0x0118, 0x69},//0x67
	{0x0119, 0x01},//0x02
	{0x011A, 0x04},
	{0x011B, 0x00},//PCLK DIV
	
	//banding
	{0x0315, 0x16},
	{0x0313, 0x38},//0x2866 for uxga ,0x388b  for hd720,0x3536 for svga 
	{0x0314, 0x8B},//

	//Output Enable
	{0x0100, 0x01},
	{0x0102, 0x02},
	{0x0104, 0x03},
};

static struct regval_list sensor_720p_regs[] = {
//Resolution Setting : 1280*720
	//Output Disable
	{0x0100, 0x00},
	{0x0102, 0x00},
	{0x0104, 0x03}, 
	{0x0109, 0x00},
	{0x010A, 0x00},	
	{0x010B, 0x03},					
	{0x0110, 0x05},					
	{0x0111, 0x00},					
	{0x0112, 0x02},					
	{0x0113, 0xD0},

	//PLL Setting 15FPS Under 24MHz PCLK
	{0x0116, 0x02},
	{0x0118, 0x69}, 
	{0x0119, 0x01},
	{0x011a, 0x04},	
	{0x011B, 0x00},//PCLK DIV
	
	//banding
	{0x0315, 0x16},                  			
	{0x0313, 0x38},
	{0x0314, 0x8B},
      {0x0300, 0x81},   // alc on  2012-2-17
	
	//Output Enable
	{0x0100, 0x01},
	{0x0102, 0x02},
	{0x0104, 0x03},
};

static struct regval_list sensor_svga_regs[] = {
	//Resolution Setting : 800*600
	//Output Disable
	{0x0100, 0x00},
	{0x0102, 0x00},
	{0x0104, 0x03}, 
	
	{0x0109, 0x00},
	{0x010A, 0x04},	
	{0x010B, 0x03},					
	{0x0110, 0x03},					
	{0x0111, 0x20},					
	{0x0112, 0x02},					
	{0x0113, 0x58},

	//PLL Setting 30FPS Under 24MHz PCLK
	{0x0116, 0x02},
	{0x0118, 0x69},//0x40 
	{0x0119, 0x01},
	{0x011a, 0x04},	
	{0x011B, 0x00},//PCLK DIV

	//banding
	{0x0315, 0x16},                  			
	{0x0313, 0x38},//0x35
	{0x0314, 0x8B},//0x36
	{0x0300, 0x81},   // alc on  2012-2-17
	
	//Output Enable
	{0x0100, 0x01},
	{0x0102, 0x02},
	{0x0104, 0x03},
};

static struct regval_list sensor_vga_regs[] = {
	//Resolution Setting : 640*480
	//Output Disable
	{0x0100, 0x00},
	{0x0102, 0x00},
	{0x0104, 0x03}, 
	{0x0109, 0x00},
	{0x010A, 0x04},	
	{0x010B, 0x03},					
	{0x0110, 0x02},					
	{0x0111, 0x80},					
	{0x0112, 0x01},					
	{0x0113, 0xE0},

	//PLL Setting 30FPS Under 24MHz PCLK
	{0x0116, 0x02},
	{0x0118, 0x69},//0x40 
	{0x0119, 0x01},
	{0x011a, 0x04},	
	{0x011B, 0x00},//PCLK DIV

	//banding
	{0x0315, 0x16},                  			
	{0x0313, 0x38},//0x35
	{0x0314, 0x8B},//0x36
	{0xffff, 0x64},
	{0x0300, 0x81},   // alc on  2012-2-17
	
	//Output Enable
	{0x0100, 0x01},
	{0x0102, 0x02},
	{0x0104, 0x03},  
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
	{0x0320, 0x24},
	{0x0321, 0x14},
	{0x0322, 0x1a},
	{0x0323, 0x24},
	{0x0441, 0x4B},
	{0x0442, 0x00},
	{0x0443, 0x00},
	{0x0444, 0x31},
};


static struct regval_list sensor_wb_incandescence_regs[] = {
	//bai re guang
	{0x0320, 0x02},
	{0x0321, 0x02},
	{0x0322, 0x02},
	{0x0323, 0x02},
	{0x0441, 0x50},
	{0x0442, 0x00},
	{0x0443, 0x00},
	{0x0444, 0x30},
};

static struct regval_list sensor_wb_fluorescent_regs[] = {
	//ri guang deng
	{0x0320, 0x02},
	{0x0321, 0x02},
	{0x0322, 0x02},
	{0x0323, 0x02},
	{0x0441, 0x43},
	{0x0442, 0x00},
	{0x0443, 0x00},
	{0x0444, 0x4B},
};

static struct regval_list sensor_wb_tungsten_regs[] = {
	//wu si deng
	{0x0320, 0x02},
	{0x0321, 0x02},
	{0x0322, 0x02},
	{0x0323, 0x02},
	{0x0441, 0x0B},
	{0x0442, 0x00},
	{0x0443, 0x00},
	{0x0444, 0x5E},
};

static struct regval_list sensor_wb_horizon[] = { 
//null
};

static struct regval_list sensor_wb_daylight_regs[] = {
	//tai yang guang
	{0x0320, 0x02},
	{0x0321, 0x02},
	{0x0322, 0x02},
	{0x0323, 0x02},
	{0x0441, 0x60},
	{0x0442, 0x00},
	{0x0443, 0x00},
	{0x0444, 0x14},
};

static struct regval_list sensor_wb_flash[] = { 
//null
};

static struct regval_list sensor_wb_cloud_regs[] = {
	{0x0320, 0x02},
	{0x0321, 0x02},
	{0x0322, 0x02},
	{0x0323, 0x02},
	{0x0441, 0x80},
	{0x0442, 0x00},
	{0x0443, 0x00},
	{0x0444, 0x0D},
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
	{0x0115}	,	{0x00},
};

static struct regval_list sensor_colorfx_bw_regs[] = {
	{0x0115}	,	{0x06},
};

static struct regval_list sensor_colorfx_sepia_regs[] = {
	{0x0115}	,	{0x0a},
	{0x026e}	,	{0x60},
	{0x026f}	,	{0xa0},
};

static struct regval_list sensor_colorfx_negative_regs[] = {
	{0x0115}	,	{0x09},
};

static struct regval_list sensor_colorfx_emboss_regs[] = {
//NULL
};

static struct regval_list sensor_colorfx_sketch_regs[] = {
//NULL
};

static struct regval_list sensor_colorfx_sky_blue_regs[] = {
	{0x0115}	,	{0x0a},
	{0x026e}	,	{0xfb},
	{0x026f}	,	{0x00},
};

static struct regval_list sensor_colorfx_grass_green_regs[] = {
	{0x0115}	,	{0x0a},
	{0x026e}	,	{0x20},
	{0x026f}	,	{0x00},
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
//NULL
};

static struct regval_list sensor_brightness_neg3_regs[] = {
//NULL
};

static struct regval_list sensor_brightness_neg2_regs[] = {
//NULL
};

static struct regval_list sensor_brightness_neg1_regs[] = {
//NULL
};

static struct regval_list sensor_brightness_zero_regs[] = {
//NULL	
};

static struct regval_list sensor_brightness_pos1_regs[] = {
	//NULL
};

static struct regval_list sensor_brightness_pos2_regs[] = {
//NULL	
};

static struct regval_list sensor_brightness_pos3_regs[] = {
//NULL	
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

};

static struct regval_list sensor_contrast_neg3_regs[] = {
	 
};

static struct regval_list sensor_contrast_neg2_regs[] = {

};

static struct regval_list sensor_contrast_neg1_regs[] = {

};

static struct regval_list sensor_contrast_zero_regs[] = {

};

static struct regval_list sensor_contrast_pos1_regs[] = {

};

static struct regval_list sensor_contrast_pos2_regs[] = {

};

static struct regval_list sensor_contrast_pos3_regs[] = {
 
};

static struct regval_list sensor_contrast_pos4_regs[] = {
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
//NULL
};

static struct regval_list sensor_saturation_neg2_regs[] = {
//NULL
};

static struct regval_list sensor_saturation_neg1_regs[] = {
//NULL
};

static struct regval_list sensor_saturation_zero_regs[] = {
//NULL
};

static struct regval_list sensor_saturation_pos1_regs[] = {
//NULL
};

static struct regval_list sensor_saturation_pos2_regs[] = {
//NULL
};

static struct regval_list sensor_saturation_pos3_regs[] = {
//NULL
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
	{0x0301, 0x40},
	{0x0201, 0x90},
};

static struct regval_list sensor_ev_neg3_regs[] = {
	{0x0301, 0x50},
	{0x0201, 0xa0},
};

static struct regval_list sensor_ev_neg2_regs[] = {
	{0x0301, 0x60},
	{0x0201, 0xb0},
};

static struct regval_list sensor_ev_neg1_regs[] = {
	{0x0301, 0x70},
	{0x0201, 0xd0},
};                     

static struct regval_list sensor_ev_zero_regs[] = {
	{0x0301, 0x80},
	{0x0201, 0x0c},
};

static struct regval_list sensor_ev_pos1_regs[] = {
	{0x0301, 0x90},
	{0x0201, 0x30},
};

static struct regval_list sensor_ev_pos2_regs[] = {
	{0x0301, 0xa0},
	{0x0201, 0x50},
};

static struct regval_list sensor_ev_pos3_regs[] = {
	{0x0301, 0xb0},
	{0x0201, 0x60},
};

static struct regval_list sensor_ev_pos4_regs[] = {
	{0x0301, 0xc0},
	{0x0201, 0x70},
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
	{0x0114, 0x04},	//YCbYCr
};


static struct regval_list sensor_fmt_yuv422_yvyu[] = {
	{0x0114, 0x06},	//YCrYCb
};

static struct regval_list sensor_fmt_yuv422_vyuy[] = {
	{0x0114, 0x02},	//CrYCbY
};

static struct regval_list sensor_fmt_yuv422_uyvy[] = {
	{0x0114, 0x00},	//CbYCrY
};

static struct regval_list sensor_fmt_raw[] = {
	{0x0114, 0x01},//raw
};

static int sensor_g_hflip(struct v4l2_subdev *sd, __s32 *value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	data_type val;
	
	ret = sensor_read(sd, 0x0101, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_g_hflip!\n");
		return ret;
	}
	
	val &= (1<<0);
	val = val>>0;		//0x0101 bit0 is mirror
		
	*value = val;

	info->hflip = *value;
	return 0;
}

static int sensor_s_hflip(struct v4l2_subdev *sd, int value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	data_type val;
	
	ret = sensor_read(sd, 0x0101, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_s_hflip!\n");
		return ret;
	}
	
	switch (value) {
		case 0:
		  val &= 0xfe;
			break;
		case 1:
			val |= 0x01;
			break;
		default:
			return -EINVAL;
	}
	ret = sensor_write(sd, 0x0101, val);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_hflip!\n");
		return ret;
	}
	
	msleep(100);
	
	info->hflip = value;
	return 0;
}

static int sensor_g_vflip(struct v4l2_subdev *sd, __s32 *value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	data_type val;
	
	ret = sensor_read(sd, 0x0101, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_g_vflip!\n");
		return ret;
	}
	
	val &= (1<<1);
	val = val>>1;		//0x0101 bit1 is upsidedown
		
	*value = val;

	info->vflip = *value;
	return 0;
}

static int sensor_s_vflip(struct v4l2_subdev *sd, int value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	data_type val;
	
	ret = sensor_read(sd, 0x0101, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_s_vflip!\n");
		return ret;
	}
	
	switch (value) {
		case 0:
		  val &= 0xfd;
			break;
		case 1:
			val |= 0x02;
			break;
		default:
			return -EINVAL;
	}
	ret = sensor_write(sd, 0x0101, val);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_vflip!\n");
		return ret;
	}
	
	msleep(100);
	
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
	
	ret = sensor_read(sd, 0x0300, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_g_autoexp!\n");
		return ret;
	}

	val &= 0x80;
	if (val == 0x80) {
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
	
	ret = sensor_read(sd, 0x0300, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_s_autoexp!\n");
		return ret;
	}

	switch (value) {
		case V4L2_EXPOSURE_AUTO:
		  val |= 0x80;
			break;
		case V4L2_EXPOSURE_MANUAL:
			val &= 0x7f;
			break;
		case V4L2_EXPOSURE_SHUTTER_PRIORITY:
			return -EINVAL;    
		case V4L2_EXPOSURE_APERTURE_PRIORITY:
			return -EINVAL;
		default:
			return -EINVAL;
	}
		
	ret = sensor_write(sd, 0x0300, val);
	if (ret < 0) {
		vfe_dev_err("sensor_write err at sensor_s_autoexp!\n");
		return ret;
	}
	
	usleep_range(10000,12000);
	
	info->autoexp = value;
	return 0;
}

static int sensor_g_autowb(struct v4l2_subdev *sd, int *value)
{
	int ret;
	struct sensor_info *info = to_state(sd);
	data_type val;
	
	ret = sensor_read(sd, 0x031a, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_g_autowb!\n");
		return ret;
	}

	val &= (1<<7);
	val = val>>7;		//0x031a bit7 is awb enable
		
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
	
	ret = sensor_read(sd, 0x031a, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_s_autowb!\n");
		return ret;
	}

	switch(value) {
	case 0:
		val &= 0x7f;
		break;
	case 1:
		val |= 0x80;
		break;
	default:
		break;
	}	
	ret = sensor_write(sd, 0x031a, val);
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
	cci_lock(sd);
	switch(on)
	{
		case CSI_SUBDEV_STBY_ON:
			vfe_dev_dbg("CSI_SUBDEV_STBY_ON\n");
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
			msleep(100);
			vfe_set_mclk(sd,OFF);
			break;
		case CSI_SUBDEV_STBY_OFF:
			vfe_dev_dbg("CSI_SUBDEV_STBY_OFF\n");
			vfe_set_mclk_freq(sd,MCLK);
			vfe_set_mclk(sd,ON);
			usleep_range(30000,31000);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
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
			vfe_set_pmu_channel(sd,IOVDD,ON);
			vfe_set_pmu_channel(sd,AVDD,ON);
			vfe_set_pmu_channel(sd,DVDD,ON);
			vfe_set_pmu_channel(sd,AFVDD,ON);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
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
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
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
	int ret;
	data_type val;
	
	ret = sensor_read(sd, 0x0000, &val);
	if (ret < 0) {
		vfe_dev_err("sensor_read err at sensor_detect!\n");
		return ret;
	}

	if(val != 0x51)
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
	enum v4l2_mbus_pixelcode mbus_code;//linux-3.0
	struct regval_list *regs;
	int	regs_size;
	int bpp;   /* Bytes per pixel */
} sensor_formats[] = {
	{
		.desc		= "YUYV 4:2:2",
		.mbus_code	= V4L2_MBUS_FMT_YUYV8_2X8,//linux-3.0
		.regs 		= sensor_fmt_yuv422_yuyv,
		.regs_size = ARRAY_SIZE(sensor_fmt_yuv422_yuyv),
		.bpp		= 2,
	},
	{
		.desc		= "YVYU 4:2:2",
		.mbus_code	= V4L2_MBUS_FMT_YVYU8_2X8,//linux-3.0
		.regs 		= sensor_fmt_yuv422_yvyu,
		.regs_size = ARRAY_SIZE(sensor_fmt_yuv422_yvyu),
		.bpp		= 2,
	},
	{
		.desc		= "UYVY 4:2:2",
		.mbus_code	= V4L2_MBUS_FMT_UYVY8_2X8,//linux-3.0
		.regs 		= sensor_fmt_yuv422_uyvy,
		.regs_size = ARRAY_SIZE(sensor_fmt_yuv422_uyvy),
		.bpp		= 2,
	},
	{
		.desc		= "VYUY 4:2:2",
		.mbus_code	= V4L2_MBUS_FMT_VYUY8_2X8,//linux-3.0
		.regs 		= sensor_fmt_yuv422_vyuy,
		.regs_size = ARRAY_SIZE(sensor_fmt_yuv422_vyuy),
		.bpp		= 2,
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

	return 0;
}

static int sensor_try_fmt(struct v4l2_subdev *sd, 
             struct v4l2_mbus_framefmt *fmt)//linux-3.0
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
	struct sensor_info *info = to_state(sd);

	if (parms->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	memset(cp, 0, sizeof(struct v4l2_captureparm));
	cp->capability = V4L2_CAP_TIMEPERFRAME;
	cp->timeperframe.numerator = 1;
	
	if (info->width > SVGA_WIDTH && info->height > SVGA_HEIGHT) {
		cp->timeperframe.denominator = SENSOR_FRAME_RATE/2;
	} 
	else {
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
		return sensor_g_colorfx(sd, &ctrl->value);
	case V4L2_CID_FLASH_LED_MODE:
		return sensor_g_flash_mode(sd, &ctrl->value);
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

	if (qc.type == V4L2_CTRL_TYPE_MENU ||
		qc.type == V4L2_CTRL_TYPE_INTEGER ||
		qc.type == V4L2_CTRL_TYPE_BOOLEAN)
	{
		if (ctrl->value < qc.minimum || ctrl->value > qc.maximum) {
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
		return sensor_s_autoexp(sd, (enum v4l2_exposure_auto_type) ctrl->value);
    case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:
  		return sensor_s_wb(sd, (enum v4l2_auto_n_preset_white_balance) ctrl->value); 
    case V4L2_CID_AUTO_WHITE_BALANCE:
		return sensor_s_autowb(sd, ctrl->value);
    case V4L2_CID_COLORFX:
		return sensor_s_colorfx(sd, (enum v4l2_colorfx) ctrl->value);
    case V4L2_CID_FLASH_LED_MODE:
		return sensor_s_flash_mode(sd, (enum v4l2_flash_led_mode) ctrl->value);
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
	.name = "gt2005",
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
	cci_dev_probe_helper(sd, client, &sensor_ops, &cci_drv);

	client->addr=0x78>>1;

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
	{ "gt2005", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, sensor_id);
static struct i2c_driver sensor_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "gt2005",
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
