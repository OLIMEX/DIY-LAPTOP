#ifndef _DISP_INCLUDE_H_
#define _DISP_INCLUDE_H_

//#define __LINUX_PLAT__
#define __UBOOT_PLAT__

#if defined(__LINUX_PLAT__)
#include <linux/module.h>
#include "linux/kernel.h"
#include "linux/mm.h"
#include <asm/uaccess.h>
#include <asm/memory.h>
#include <asm/unistd.h>
#include "linux/semaphore.h"
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/dma-mapping.h>
#include <linux/fb.h>
#include <linux/sched.h>   //wake_up_process()
#include <linux/kthread.h> //kthread_create()??kthread_run()
#include <linux/err.h> //IS_ERR()??PTR_ERR()
#include <linux/delay.h>
#include <linux/platform_device.h>
#include "asm-generic/int-ll64.h"
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/pwm.h>
#include <mach/sys_config.h>
#include <mach/irqs.h>
#include <mach/platform.h>
#include <linux/ion_sunxi.h>
#include <linux/sync.h>
#include <linux/sw_sync.h>
#include <asm/div64.h>

#include <video/sunxi_display2.h>
#include "../disp_sys_intf.h"

#define __inf(msg...)       do{if(bsp_disp_get_print_level()){printk(KERN_WARNING "[DISP] %s,line:%d:",__func__,__LINE__);printk(msg);}}while(0)
#define __msg(msg...)       do{if(bsp_disp_get_print_level()){printk(KERN_WARNING "[DISP] %s,line:%d:",__func__,__LINE__);printk(msg);}}while(0)
#define __wrn(msg...)       do{{printk(KERN_WARNING "[DISP] %s,line:%d:",__func__,__LINE__);printk(msg);}}while(0)
#define __here__            do{if(bsp_disp_get_print_level()==2){printk(KERN_WARNING "[DISP] %s,line:%d\n",__func__,__LINE__);}}while(0)
#define __debug(msg...)     do{if(bsp_disp_get_print_level()==2){printk(KERN_WARNING "[DISP] %s,line:%d:",__func__,__LINE__);printk(msg);}}while(0)

#endif//end of define __LINUX_PLAT__

#ifdef __UBOOT_PLAT__
#include <common.h>
#include <malloc.h>
#include <sunxi_display2.h>
#include <sys_config.h>
#include <asm/arch/intc.h>
#include <asm/arch/platform.h>
#include <power/sunxi/pmu.h>
#include <pwm.h>
#include <asm/arch/timer.h>
#include <asm/arch/platform.h>
#include <linux/list.h>
#include <asm/memory.h>
#include <div64.h>
#include "../disp_sys_intf.h"

#define OSAL_PRINTF
#define __inf(msg...)
#define __msg(msg...)
#define __wrn(msg...) printf(msg)
#define __here
#define __debug

#define false 0
#define true 1
#endif

#if defined(__LINUX_PLAT__)
#define DE_INF __inf
#define DE_MSG __msg
#define DE_WRN __wrn
#define DE_DBG __debug
#define DISP_IRQ_RETURN IRQ_HANDLED
#else
#define DE_INF(msg...)
#define DE_MSG __msg
#define DE_WRN __wrn
#define DE_DBG __debug
#define DISP_IRQ_RETURN DIS_SUCCESS
#endif

#define DEFAULT_PRINT_LEVLE 0
#if defined(CONFIG_FPGA_V4_PLATFORM) || defined(CONFIG_FPGA_V7_PLATFORM) || defined(CONFIG_A67_FPGA)
#define __FPGA_DEBUG__
#endif

#define SETMASK(width, shift)   ((width?((-1U) >> (32-width)):0)  << (shift))
#define CLRMASK(width, shift)   (~(SETMASK(width, shift)))
#define GET_BITS(shift, width, reg)     \
	(((reg) & SETMASK(width, shift)) >> (shift))
#define SET_BITS(shift, width, reg, val) \
	(((reg) & CLRMASK(width, shift)) | (val << (shift)))

#define DISPALIGN(value, align) ((align==0)?value:(((value) + ((align) - 1)) & ~((align) - 1)))

#define disp_readl(addr)         (*((volatile unsigned int *)(addr)))          /* word input */
#define disp_writel(value,addr)  (*((volatile unsigned int *)(addr))  = (value))   /* word output */

#ifndef abs
#define abs(x) (((x)&0x80000000)? (0-(x)):(x))
#endif


typedef struct
{
	unsigned int   lcd_gamma_en;
	unsigned int   lcd_gamma_tbl[256];
	unsigned int   lcd_cmap_en;
	unsigned int   lcd_cmap_tbl[2][3][4];
	unsigned int   lcd_bright_curve_tbl[256];
}panel_extend_para;

typedef enum
{
	DIS_SUCCESS=0,
	DIS_FAIL=-1,
	DIS_PARA_FAILED=-2,
	DIS_PRIO_ERROR=-3,
	DIS_OBJ_NOT_INITED=-4,
	DIS_NOT_SUPPORT=-5,
	DIS_NO_RES=-6,
	DIS_OBJ_COLLISION=-7,
	DIS_DEV_NOT_INITED=-8,
	DIS_DEV_SRAM_COLLISION=-9,
	DIS_TASK_ERROR = -10,
	DIS_PRIO_COLLSION = -11
}disp_return_value;

/*basic data information definition*/
enum disp_layer_feat {
	DISP_LAYER_FEAT_GLOBAL_ALPHA        = 1 << 0,
	DISP_LAYER_FEAT_PIXEL_ALPHA         = 1 << 1,
	DISP_LAYER_FEAT_GLOBAL_PIXEL_ALPHA  = 1 << 2,
	DISP_LAYER_FEAT_PRE_MULT_ALPHA      = 1 << 3,
	DISP_LAYER_FEAT_COLOR_KEY           = 1 << 4,
	DISP_LAYER_FEAT_ZORDER              = 1 << 5,
	DISP_LAYER_FEAT_POS                 = 1 << 6,
	DISP_LAYER_FEAT_3D                  = 1 << 7,
	DISP_LAYER_FEAT_SCALE               = 1 << 8,
	DISP_LAYER_FEAT_DE_INTERLACE        = 1 << 9,
	DISP_LAYER_FEAT_COLOR_ENHANCE       = 1 << 10,
	DISP_LAYER_FEAT_DETAIL_ENHANCE      = 1 << 11,
};

typedef enum
{
	DISP_PIXEL_TYPE_RGB=0x0,
	DISP_PIXEL_TYPE_YUV=0x1,
}disp_pixel_type;

typedef enum
{
	LAYER_ATTR_DIRTY       = 0x00000001,
	LAYER_VI_FC_DIRTY      = 0x00000002,
	LAYER_HADDR_DIRTY      = 0x00000004,
	LAYER_SIZE_DIRTY       = 0x00000008,
	BLEND_ENABLE_DIRTY     = 0x00000010,
	BLEND_ATTR_DIRTY       = 0x00000020,
	BLEND_CTL_DIRTY        = 0x00000040,
	BLEND_OUT_DIRTY        = 0x00000080,
	LAYER_ALL_DIRTY        = 0x000000ff,
}disp_layer_dirty_flags;

typedef enum
{
	MANAGER_ENABLE_DIRTY     = 0x00000001,
	MANAGER_CK_DIRTY         = 0x00000002,
	MANAGER_BACK_COLOR_DIRTY = 0x00000004,
	MANAGER_SIZE_DIRTY       = 0x00000008,
	MANAGER_ALL_DIRTY        = 0x0000000f,
}disp_manager_dirty_flags;

struct disp_layer_config_data
{
	disp_layer_config config;
	disp_layer_dirty_flags flag;
};

struct disp_manager_info {
	disp_color back_color;
	disp_colorkey ck;
	disp_rectsz size;
	disp_csc_type cs;
	u32 color_range;
	u32 interlace;
	bool enable;
	u32 disp_device;//disp of device
};

struct disp_manager_data
{
	struct disp_manager_info config;
	disp_manager_dirty_flags flag;
};

struct disp_clk_info
{
		u32                     clk;
		u32                     clk_div;
		u32                     h_clk;
		u32                     clk_src;
		u32                     clk_div2;

		u32                     clk_p;
		u32                     clk_div_p;
		u32                     h_clk_p;
		u32                     clk_src_p;

		u32                     ahb_clk;
		u32                     h_ahb_clk;
		u32                     dram_clk;
		u32                     h_dram_clk;

		bool                    enabled;
};

struct disp_enhance_info
{
	//basic adjust
	u32         bright;
	u32         contrast;
	u32         saturation;
	u32         hue;
	u32         mode;
	//ehnance
	u32         sharp;	//0-off; 1~3-on.
	u32         auto_contrast;	//0-off; 1~3-on.
	u32					auto_color;	//0-off; 1-on.
	u32         fancycolor_red; //0-Off; 1-2-on.
	u32         fancycolor_green;//0-Off; 1-2-on.
	u32         fancycolor_blue;//0-Off; 1-2-on.
	disp_rect   window;
	u32         enable;
	disp_rectsz size;
	u32         demo_enable;//1: enable demo mode
};

typedef enum
{
	ENHANCE_NONE_DIRTY   = 0x0,
	ENHANCE_ENABLE_DIRTY = 0x1 << 0,
	ENHANCE_BRIGHT_DIRTY = 0x1 << 1,
	ENHANCE_SHARP_DIRTY  = 0x1 << 2,
	ENHANCE_SIZE_DIRTY   = 0x1 << 3,
	ENHANCE_MODE_DIRTY   = 0X1 << 4,
	ENHANCE_DEMO_DIRTY   = 0x1 << 5,
	ENHANCE_ALL_DIRTY    = 0x3f,
}disp_enhance_dirty_flags;

struct disp_enhance_config
{
	struct disp_enhance_info info;
	disp_enhance_dirty_flags flags;
};

typedef enum
{
	SMBL_DIRTY_NONE      = 0x00000000,
	SMBL_DIRTY_ENABLE    = 0x00000001,
	SMBL_DIRTY_WINDOW    = 0x00000002,
	SMBL_DIRTY_SIZE      = 0x00000004,
	SMBL_DIRTY_BL        = 0x00000008,
	SMBL_DIRTY_ALL       = 0x0000000F,
}disp_smbl_dirty_flags;

struct disp_smbl_info
{
	disp_rect                window;
	u32                      enable;
	disp_rectsz              size;
	u32                      backlight;
	u32                      backlight_dimming;
	disp_smbl_dirty_flags    flags;
};

struct disp_csc_config
{
	u32 in_fmt;
	u32 in_mode;
	u32 out_fmt;
	u32 out_mode;
	u32 out_color_range;
	u32 brightness;
	u32 contrast;
	u32 saturation;
	u32 hue;
	//u32 alpha;
};

enum
{
	DE_RGB = 0,
	DE_YUV = 1,
};

typedef enum
{
  CAPTURE_DIRTY_ADDRESS   = 0x00000001,
  CAPTURE_DIRTY_WINDOW	  = 0x00000002,
  CAPTURE_DIRTY_SIZE	  = 0x00000004,
  CAPTURE_DIRTY_ALL 	  = 0x00000007,
}disp_capture_dirty_flags;

struct disp_capture_config
{
	disp_s_frame in_frame;   //only format/size/crop valid
	disp_s_frame out_frame;
	u32 disp;              //which disp channel to be capture
	disp_capture_dirty_flags flags;
};

typedef enum
{
	LCD_IF_HV			  = 0,
	LCD_IF_CPU			= 1,
	LCD_IF_LVDS			= 3,
	LCD_IF_DSI			= 4,
	LCD_IF_EDP      = 5,
	LCD_IF_EXT_DSI  = 6,
}disp_lcd_if;

typedef enum
{
	LCD_HV_IF_PRGB_1CYC		  = 0,  //parallel hv
	LCD_HV_IF_SRGB_3CYC		  = 8,  //serial hv
	LCD_HV_IF_DRGB_4CYC		  = 10, //Dummy RGB
	LCD_HV_IF_RGBD_4CYC		  = 11, //RGB Dummy
	LCD_HV_IF_CCIR656_2CYC	= 12,
}disp_lcd_hv_if;

typedef enum
{
	LCD_HV_SRGB_SEQ_RGB_RGB	= 0,
	LCD_HV_SRGB_SEQ_RGB_BRG	= 1,
	LCD_HV_SRGB_SEQ_RGB_GBR	= 2,
	LCD_HV_SRGB_SEQ_BRG_RGB	= 4,
	LCD_HV_SRGB_SEQ_BRG_BRG	= 5,
	LCD_HV_SRGB_SEQ_BRG_GBR	= 6,
	LCD_HV_SRGB_SEQ_GRB_RGB	= 8,
	LCD_HV_SRGB_SEQ_GRB_BRG	= 9,
	LCD_HV_SRGB_SEQ_GRB_GBR	= 10,
}disp_lcd_hv_srgb_seq;

typedef enum
{
	LCD_HV_SYUV_SEQ_YUYV	= 0,
	LCD_HV_SYUV_SEQ_YVYU	= 1,
	LCD_HV_SYUV_SEQ_UYUV	= 2,
	LCD_HV_SYUV_SEQ_VYUY	= 3,
}disp_lcd_hv_syuv_seq;

typedef enum
{
	LCD_HV_SYUV_FDLY_0LINE	= 0,
	LCD_HV_SRGB_FDLY_2LINE	= 1, //ccir ntsc
	LCD_HV_SRGB_FDLY_3LINE	= 2, //ccir pal
}disp_lcd_hv_syuv_fdly;

typedef enum
{
	LCD_CPU_IF_RGB666_18PIN = 0,
	LCD_CPU_IF_RGB666_9PIN  = 10,
	LCD_CPU_IF_RGB666_6PIN  = 12,
	LCD_CPU_IF_RGB565_16PIN = 8,
	LCD_CPU_IF_RGB565_8PIN  = 14,
}disp_lcd_cpu_if;

typedef enum
{
	LCD_TE_DISABLE	= 0,
	LCD_TE_RISING		= 1,
	LCD_TE_FALLING  = 2,
}disp_lcd_te;

typedef enum
{
	LCD_LVDS_IF_SINGLE_LINK		= 0,
	LCD_LVDS_IF_DUAL_LINK		  = 1,
}disp_lcd_lvds_if;

typedef enum
{
	LCD_LVDS_8bit		= 0,
	LCD_LVDS_6bit		= 1,
}disp_lcd_lvds_colordepth;

typedef enum
{
	LCD_LVDS_MODE_NS		  = 0,
	LCD_LVDS_MODE_JEIDA		= 1,
}disp_lcd_lvds_mode;

typedef enum
{
	LCD_DSI_IF_VIDEO_MODE	  = 0,
	LCD_DSI_IF_COMMAND_MODE	= 1,
	LCD_DSI_IF_BURST_MODE   = 2,
}disp_lcd_dsi_if;

typedef enum
{
	LCD_DSI_1LANE			= 1,
	LCD_DSI_2LANE			= 2,
	LCD_DSI_3LANE			= 3,
	LCD_DSI_4LANE			= 4,
}disp_lcd_dsi_lane;

typedef enum
{
	LCD_DSI_FORMAT_RGB888	  = 0,
	LCD_DSI_FORMAT_RGB666	  = 1,
	LCD_DSI_FORMAT_RGB666P	= 2,
	LCD_DSI_FORMAT_RGB565	  = 3,
}disp_lcd_dsi_format;


typedef enum
{
	LCD_FRM_BYPASS	= 0,
	LCD_FRM_RGB666	= 1,
	LCD_FRM_RGB565	= 2,
}disp_lcd_frm;

typedef enum
{
	LCD_CMAP_B0	= 0x0,
	LCD_CMAP_G0	= 0x1,
	LCD_CMAP_R0	= 0x2,
	LCD_CMAP_B1	= 0x4,
	LCD_CMAP_G1	= 0x5,
	LCD_CMAP_R1	= 0x6,
	LCD_CMAP_B2	= 0x8,
	LCD_CMAP_G2	= 0x9,
	LCD_CMAP_R2	= 0xa,
	LCD_CMAP_B3	= 0xc,
	LCD_CMAP_G3	= 0xd,
	LCD_CMAP_R3	= 0xe,
}disp_lcd_cmap_color;

typedef struct
{
	unsigned int lp_clk_div;
	unsigned int hs_prepare;
	unsigned int hs_trail;
	unsigned int clk_prepare;
	unsigned int clk_zero;
	unsigned int clk_pre;
	unsigned int clk_post;
	unsigned int clk_trail;
	unsigned int hs_dly_mode;
	unsigned int hs_dly;
	unsigned int lptx_ulps_exit;
	unsigned int hstx_ana0;
	unsigned int hstx_ana1;
}__disp_dsi_dphy_timing_t;

typedef struct
{
	disp_lcd_if              lcd_if;

	disp_lcd_hv_if           lcd_hv_if;
	disp_lcd_hv_srgb_seq     lcd_hv_srgb_seq;
	disp_lcd_hv_syuv_seq     lcd_hv_syuv_seq;
	disp_lcd_hv_syuv_fdly    lcd_hv_syuv_fdly;

	disp_lcd_lvds_if         lcd_lvds_if;
	disp_lcd_lvds_colordepth lcd_lvds_colordepth; //color depth, 0:8bit; 1:6bit
	disp_lcd_lvds_mode       lcd_lvds_mode;
	unsigned int             lcd_lvds_io_polarity;

	disp_lcd_cpu_if          lcd_cpu_if;
	disp_lcd_te              lcd_cpu_te;

	disp_lcd_dsi_if          lcd_dsi_if;
	disp_lcd_dsi_lane        lcd_dsi_lane;
	disp_lcd_dsi_format      lcd_dsi_format;
	unsigned int             lcd_dsi_eotp;
	unsigned int             lcd_dsi_vc;
	disp_lcd_te              lcd_dsi_te;

	unsigned int             lcd_dsi_dphy_timing_en;
	__disp_dsi_dphy_timing_t*	lcd_dsi_dphy_timing_p;

	unsigned int            lcd_edp_rate; //1(1.62G); 2(2.7G); 3(5.4G)
	unsigned int            lcd_edp_lane; //  1/2/4lane
	unsigned int            lcd_edp_colordepth; //color depth, 0:8bit; 1:6bit
	unsigned int            lcd_edp_fps;

	unsigned int            lcd_dclk_freq;
	unsigned int            lcd_x; //horizontal resolution
	unsigned int            lcd_y; //vertical resolution
	unsigned int            lcd_width; //width of lcd in mm
	unsigned int            lcd_height;//height of lcd in mm
	unsigned int            lcd_xtal_freq;

	unsigned int            lcd_pwm_used;
	unsigned int            lcd_pwm_ch;
	unsigned int            lcd_pwm_freq;
	unsigned int            lcd_pwm_pol;

	unsigned int            lcd_rb_swap;
	unsigned int            lcd_rgb_endian;

	unsigned int            lcd_vt;
	unsigned int            lcd_ht;
	unsigned int            lcd_vbp;
	unsigned int            lcd_hbp;
	unsigned int            lcd_vspw;
	unsigned int            lcd_hspw;

	unsigned int            lcd_hv_clk_phase;
	unsigned int            lcd_hv_sync_polarity;

	unsigned int            lcd_frm;
	unsigned int            lcd_gamma_en;
	unsigned int            lcd_cmap_en;
	unsigned int            lcd_bright_curve_en;
	panel_extend_para       lcd_extend_para;

	char                    lcd_size[8]; //e.g. 7.9, 9.7
	char                    lcd_model_name[32];

	unsigned int            tcon_index; //not need to config for user
	unsigned int            lcd_fresh_mode;//not need to config for user
	unsigned int            lcd_dclk_freq_original; //not need to config for user
}disp_panel_para;

extern s32 disp_delay_ms(u32 ms);
extern s32 disp_delay_us(u32 us);

#endif

