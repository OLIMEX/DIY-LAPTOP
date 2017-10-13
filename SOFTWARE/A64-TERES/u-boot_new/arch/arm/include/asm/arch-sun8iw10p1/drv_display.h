#ifndef __DRV_DISPLAY_H__
#define __DRV_DISPLAY_H__

#define __bool signed char
#define bool signed char

typedef struct {unsigned char  alpha;unsigned char red;unsigned char green; unsigned char blue; }disp_color_info;
typedef struct {int x; int y; unsigned int width; unsigned int height;}disp_window;
typedef struct {unsigned int width;unsigned int height;                   }disp_size;
typedef struct {int x; int y;                           }disp_position;

typedef enum
{
	DISP_FORMAT_ARGB_8888                    = 0x00,//MSB  A-R-G-B  LSB
	DISP_FORMAT_ABGR_8888                    = 0x01,
	DISP_FORMAT_RGBA_8888                    = 0x02,
	DISP_FORMAT_BGRA_8888                    = 0x03,
	DISP_FORMAT_XRGB_8888                    = 0x04,
	DISP_FORMAT_XBGR_8888                    = 0x05,
	DISP_FORMAT_RGBX_8888                    = 0x06,
	DISP_FORMAT_BGRX_8888                    = 0x07,
	DISP_FORMAT_RGB_888                      = 0x08,
	DISP_FORMAT_BGR_888                      = 0x09,
	DISP_FORMAT_RGB_565                      = 0x0a,
	DISP_FORMAT_BGR_565                      = 0x0b,
	DISP_FORMAT_ARGB_4444                    = 0x0c,
	DISP_FORMAT_ABGR_4444                    = 0x0d,
	DISP_FORMAT_RGBA_4444                    = 0x0e,
	DISP_FORMAT_BGRA_4444                    = 0x0f,
	DISP_FORMAT_ARGB_1555                    = 0x10,
	DISP_FORMAT_ABGR_1555                    = 0x11,
	DISP_FORMAT_RGBA_5551                    = 0x12,
	DISP_FORMAT_BGRA_5551                    = 0x13,

	/* SP: semi-planar, P:planar, I:interleaved
	 * UVUV: U in the LSBs;     VUVU: V in the LSBs */
	DISP_FORMAT_YUV444_I_AYUV                = 0x40,//MSB  A-Y-U-V  LSB
	DISP_FORMAT_YUV444_I_VUYA                = 0x41,//MSB  V-U-Y-A  LSB
	DISP_FORMAT_YUV422_I_YVYU                = 0x42,//MSB  Y-V-Y-U  LSB
	DISP_FORMAT_YUV422_I_YUYV                = 0x43,//MSB  Y-U-Y-V  LSB
	DISP_FORMAT_YUV422_I_UYVY                = 0x44,//MSB  U-Y-V-Y  LSB
	DISP_FORMAT_YUV422_I_VYUY                = 0x45,//MSB  V-Y-U-Y  LSB
	DISP_FORMAT_YUV444_P                     = 0x46,//MSB  P3-2-1-0 LSB,  YYYY UUUU VVVV
	DISP_FORMAT_YUV422_P                     = 0x47,//MSB  P3-2-1-0 LSB   YYYY UU   VV
	DISP_FORMAT_YUV420_P                     = 0x48,//MSB  P3-2-1-0 LSB   YYYY U    V
	DISP_FORMAT_YUV411_P                     = 0x49,//MSB  P3-2-1-0 LSB   YYYY U    V
	DISP_FORMAT_YUV422_SP_UVUV               = 0x4a,//MSB  V-U-V-U  LSB
	DISP_FORMAT_YUV422_SP_VUVU               = 0x4b,//MSB  U-V-U-V  LSB
	DISP_FORMAT_YUV420_SP_UVUV               = 0x4c,
	DISP_FORMAT_YUV420_SP_VUVU               = 0x4d,
	DISP_FORMAT_YUV411_SP_UVUV               = 0x4e,
	DISP_FORMAT_YUV411_SP_VUVU               = 0x4f,
	DISP_FORMAT_YUV422_SP_TILE_UVUV         = 0x50,
	DISP_FORMAT_YUV422_SP_TILE_VUVU         = 0x51,
	DISP_FORMAT_YUV420_SP_TILE_UVUV         = 0x52,
	DISP_FORMAT_YUV420_SP_TILE_VUVU         = 0x53,
	DISP_FORMAT_YUV411_SP_TILE_UVUV         = 0x54,
	DISP_FORMAT_YUV411_SP_TILE_VUVU         = 0x55,
	DISP_FORMAT_YUV422_SP_TILE_128X32_UVUV  = 0x56,
	DISP_FORMAT_YUV422_SP_TILE_128X32_VUVU  = 0x57,
	DISP_FORMAT_YUV420_SP_TILE_128X32_UVUV  = 0x58,
	DISP_FORMAT_YUV420_SP_TILE_128X32_VUVU  = 0x59,
	DISP_FORMAT_YUV411_SP_TILE_128X32_UVUV  = 0x5a,
	DISP_FORMAT_YUV411_SP_TILE_128X32_VUVU  = 0x5b,
}disp_pixel_format;

typedef enum
{
	DISP_3D_SRC_MODE_TB = 0x0,//top bottom
	DISP_3D_SRC_MODE_FP = 0x1,//frame packing
	DISP_3D_SRC_MODE_SSF = 0x2,//side by side full
	DISP_3D_SRC_MODE_SSH = 0x3,//side by side half
	DISP_3D_SRC_MODE_LI = 0x4,//line interleaved
}disp_3d_src_mode;

typedef enum
{
	DISP_3D_OUT_MODE_CI_1 = 0x5,//column interlaved 1
	DISP_3D_OUT_MODE_CI_2 = 0x6,//column interlaved 2
	DISP_3D_OUT_MODE_CI_3 = 0x7,//column interlaved 3
	DISP_3D_OUT_MODE_CI_4 = 0x8,//column interlaved 4
	DISP_3D_OUT_MODE_LIRGB = 0x9,//line interleaved rgb

	DISP_3D_OUT_MODE_TB = 0x0,//top bottom
	DISP_3D_OUT_MODE_FP = 0x1,//frame packing
	DISP_3D_OUT_MODE_SSF = 0x2,//side by side full
	DISP_3D_OUT_MODE_SSH = 0x3,//side by side half
	DISP_3D_OUT_MODE_LI = 0x4,//line interleaved
	DISP_3D_OUT_MODE_FA = 0xa,//field alternative
}disp_3d_out_mode;

typedef enum
{
	DISP_BT601  = 0,
	DISP_BT709  = 1,
	DISP_YCC    = 2,
	DISP_VXYCC  = 3,
}disp_cs_mode;

typedef enum
{
	DISP_COLOR_RANGE_16_255 = 0,
	DISP_COLOR_RANGE_0_255  = 1,
	DISP_COLOR_RANGE_16_235 = 2,
}disp_color_range;

typedef enum
{
	DISP_OUTPUT_TYPE_NONE   = 0,
	DISP_OUTPUT_TYPE_LCD    = 1,
	DISP_OUTPUT_TYPE_TV     = 2,
	DISP_OUTPUT_TYPE_HDMI   = 4,
	DISP_OUTPUT_TYPE_VGA    = 8,
}disp_output_type;

typedef enum
{
	DISP_TV_MOD_480I                = 0,
	DISP_TV_MOD_576I                = 1,
	DISP_TV_MOD_480P                = 2,
	DISP_TV_MOD_576P                = 3,
	DISP_TV_MOD_720P_50HZ           = 4,
	DISP_TV_MOD_720P_60HZ           = 5,
	DISP_TV_MOD_1080I_50HZ          = 6,
	DISP_TV_MOD_1080I_60HZ          = 7,
	DISP_TV_MOD_1080P_24HZ          = 8,
	DISP_TV_MOD_1080P_50HZ          = 9,
	DISP_TV_MOD_1080P_60HZ          = 0xa,
	DISP_TV_MOD_1080P_24HZ_3D_FP    = 0x17,
	DISP_TV_MOD_720P_50HZ_3D_FP     = 0x18,
	DISP_TV_MOD_720P_60HZ_3D_FP     = 0x19,
	DISP_TV_MOD_1080P_25HZ          = 0x1a,
	DISP_TV_MOD_1080P_30HZ          = 0x1b,
	DISP_TV_MOD_PAL                 = 0xb,
	DISP_TV_MOD_PAL_SVIDEO          = 0xc,
	DISP_TV_MOD_NTSC                = 0xe,
	DISP_TV_MOD_NTSC_SVIDEO         = 0xf,
	DISP_TV_MOD_PAL_M               = 0x11,
	DISP_TV_MOD_PAL_M_SVIDEO        = 0x12,
	DISP_TV_MOD_PAL_NC              = 0x14,
	DISP_TV_MOD_PAL_NC_SVIDEO       = 0x15,
	DISP_TV_MOD_3840_2160P_30HZ     = 0x1c,
	DISP_TV_MOD_3840_2160P_25HZ     = 0x1d,
	DISP_TV_MOD_3840_2160P_24HZ     = 0x1e,
	DISP_TV_MODE_NUM                = 0x1f,
}disp_tv_mode;

typedef enum
{
	DISP_LCDC_SRC_DE_CH1    = 0,
	DISP_LCDC_SRC_DE_CH2    = 1,
	DISP_LCDC_SRC_DMA888    = 2,
	DISP_LCDC_SRC_DMA565    = 3,
	DISP_LCDC_SRC_WHITE     = 4,
	DISP_LCDC_SRC_BLACK     = 5,
	DISP_LCDC_SRC_BLUE      = 6,
}disp_lcd_src;

typedef enum
{
	DISP_LAYER_WORK_MODE_NORMAL     = 0,    //normal work mode
	DISP_LAYER_WORK_MODE_SCALER     = 4,    //scaler work mode
}disp_layer_mode;

typedef enum
{
	DISP_VIDEO_NATUAL       = 0,
	DISP_VIDEO_SOFT         = 1,
	DISP_VIDEO_VERYSOFT     = 2,
	DISP_VIDEO_SHARP        = 3,
	DISP_VIDEO_VERYSHARP    = 4
}disp_video_smooth_level;

typedef enum
{
	DISP_EXIT_MODE_CLEAN_ALL    = 0,
	DISP_EXIT_MODE_CLEAN_PARTLY = 1,//only clean interrupt temply
}disp_exit_mode;

typedef enum
{
	DISP_ENHANCE_MODE_RED       = 0x0,
	DISP_ENHANCE_MODE_GREEN     = 0x1,
	DISP_ENHANCE_MODE_BLUE      = 0x2,
	DISP_ENHANCE_MODE_CYAN      = 0x3,
	DISP_ENHANCE_MODE_MAGENTA   = 0x4,
	DISP_ENHANCE_MODE_YELLOW    = 0x5,
	DISP_ENHANCE_MODE_FLESH     = 0x6,
	DISP_ENHANCE_MODE_STANDARD  = 0x7,
	DISP_ENHANCE_MODE_VIVID     = 0x8,
	DISP_ENHANCE_MODE_SCENERY   = 0xa,
}disp_enhance_mode;

/* todo?  mv to internal head file */
typedef enum
{
	DISP_OUT_CSC_TYPE_LCD        = 0,
	DISP_OUT_CSC_TYPE_TV         = 1,
	DISP_OUT_CSC_TYPE_HDMI_YUV   = 2,
	DISP_OUT_CSC_TYPE_SAT        = 3,
	DISP_OUT_CSC_TYPE_HDMI_RGB   = 4,
}disp_out_csc_type;

typedef enum
{
    DISP_HWC_MOD_32X32_8BPP = 0,
    DISP_HWC_MOD_64X64_2BPP = 1,
    DISP_HWC_MOD_64X32_4BPP = 2,
    DISP_HWC_MOD_32X64_4BPP = 3,
}disp_cursor_mode;

/* todo?  mv to internal head file */
typedef enum
{
	DISP_MOD_CCMU         = 0,
	DISP_MOD_PIOC         = 1,
	DISP_MOD_PWM          = 2,
	DISP_MOD_FE0          = 3,
	DISP_MOD_FE1          = 4,
	DISP_MOD_FE2          = 5,
	DISP_MOD_BE0          = 6,
	DISP_MOD_BE1          = 7,
	DISP_MOD_BE2          = 8,
	DISP_MOD_LCD0         = 9,
	DISP_MOD_LCD1         = 10,
	DISP_MOD_LCD2         = 11,
	DISP_MOD_TVE0         = 12,
	DISP_MOD_TVE1         = 13,
	DISP_MOD_TVE2         = 14,
	DISP_MOD_DEU0         = 15,
	DISP_MOD_DEU1         = 16,
	DISP_MOD_DEU2         = 17,
	DISP_MOD_CMU0         = 18,
	DISP_MOD_CMU1         = 19,
	DISP_MOD_CMU2         = 20,
	DISP_MOD_DRC0         = 21,
	DISP_MOD_DRC1         = 22,
	DISP_MOD_DRC2         = 23,
	DISP_MOD_DSI0         = 24,
	DISP_MOD_DSI0_DPHY    = 25,
	DISP_MOD_DSI1         = 26,
	DISP_MOD_DSI1_DPHY    = 27,
	DISP_MOD_DSI2         = 28,
	DISP_MOD_DSI2_DPHY    = 29,
	DISP_MOD_HDMI         = 30,
	DISP_MOD_EDP          = 31,
	DISP_MOD_TOP          = 32,
	DISP_MOD_WB0          = 33,
	DISP_MOD_WB1          = 34,
	DISP_MOD_WB2          = 35,
	DISP_MOD_SAT0         = 35,
	DISP_MOD_SAT1         = 36,
	DISP_MOD_SAT2         = 37,
	DISP_MOD_NUM          = 38,
}disp_mod_id;

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
	unsigned int          addr[3];    // frame buffer的内容地址，对于rgb类型，只有addr[0]有效
	disp_size             size;//单位是pixel
	disp_pixel_format     format;
	disp_cs_mode          cs_mode;    //color space
	bool                  b_trd_src; //if 3d source, used for scaler mode layer
	disp_3d_src_mode      trd_mode; //source 3d mode, used for scaler mode layer
	unsigned int          trd_right_addr[3];//used when in frame packing 3d mode
	bool                  pre_multiply; //TRUE: pre-multiply fb
	disp_window           src_win;    // framebuffer source window,only take x,y if is not scaler mode
#if 0
	bool                  interlace;
	bool                  top_field_first;
	bool                  pre_frame_valid;
#endif
}disp_fb_info;

typedef struct
{
    disp_layer_mode           mode;       //layer work mode
    unsigned char             pipe;       //layer pipe,0/1
    unsigned char             zorder;     //layer priority,can get layer prio,but never set layer prio,从底至顶,优先级由低至高
    unsigned char             alpha_mode;   //0: pixel alpha;  1: global alpha;  2: global pixel alpha
    unsigned char             alpha_value;  //layer global alpha value
    bool                      ck_enable;  //layer color key enable
    disp_window               screen_win;    // screen window
    disp_fb_info              fb;         //framebuffer
    bool                      b_trd_out;  //if output 3d mode, used for scaler mode layer
    disp_3d_out_mode          out_trd_mode; //output 3d mode, used for scaler mode layer

    unsigned int              id;          //get the id of the frame display now by DISP_CMD_LAYER_GET_FRAME_ID
}disp_layer_info;

typedef struct
{
    disp_color_info    ck_max;
    disp_color_info    ck_min;
    unsigned int       red_match_rule;//0/1:always match; 2:match if min<=color<=max; 3:match if color>max or color<min
    unsigned int       green_match_rule;//0/1:always match; 2:match if min<=color<=max; 3:match if color>max or color<min
    unsigned int       blue_match_rule;//0/1:always match; 2:match if min<=color<=max; 3:match if color>max or color<min
}disp_colorkey;

typedef struct
{
	disp_size     screen_size;//used when the screen is not displaying on any output device(lcd/hdmi/vga/tv)
	disp_fb_info  output_fb[3];
	unsigned int  buffer_num;//1,2,3
	unsigned int  mode;      //0:single,  1:continue
	unsigned int  fps;       //0:fps of lcd,  1:1/2 fps of lcd
	disp_window   capture_window;
	disp_window   output_window;

	unsigned int  cur_buffer_id;  //no need to care about it
	unsigned int  capture_request;
	unsigned int  scaler_id;
	unsigned int  got_frame;
}disp_capture_para;

typedef struct
{
	disp_cursor_mode mode;
	unsigned int  addr;
}disp_cursor_fb;

typedef struct
{
	unsigned int	vic;             //video infomation code
	unsigned int	pixel_clk;//khz
	unsigned int	avi_pr;
	unsigned int	x_res;
	unsigned int	y_res;
	unsigned int	hor_total_time;
	unsigned int	hor_back_porch;
	unsigned int	hor_front_porch;
	unsigned int	hor_sync_time;
	unsigned int	ver_total_time;
	unsigned int	ver_back_porch;
	unsigned int	ver_front_porch;
	unsigned int	ver_sync_time;
	unsigned int	hor_sync_polarity;
	unsigned int	ver_sync_polarity;
	unsigned int	interlace;
	unsigned int	vactive_space;
	unsigned int	trd_mode;
}disp_video_timing;


typedef struct
{
	int (*hdmi_open)(void);
	int (*hdmi_close)(void);
	int (*hdmi_set_mode)(disp_tv_mode mode);
	int (*hdmi_mode_support)(disp_tv_mode mode);
	int (*hdmi_get_HPD_status)(void);
	int (*hdmi_set_pll)(unsigned int pll, unsigned int clk);
	int (*hdmi_dvi_enable)(unsigned int mode);
	int (*hdmi_dvi_support)(void);
	int (*hdmi_get_input_csc)(void);
	int (*hdmi_get_hdcp_enable)(void);
	int (*hdmi_get_video_timing_info)(disp_video_timing **video_info);
	int (*hdmi_get_video_info_index)(u32 mode_id);
	int (*hdmi_suspend)(void);
	int (*hdmi_resume)(void);
	int (*hdmi_early_suspend)(void);
	int (*hdmi_late_resume)(void);
}disp_hdmi_func;

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
	unsigned int   lcd_gamma_tbl[256];
	unsigned int   lcd_cmap_tbl[2][3][4];
	unsigned int   lcd_bright_curve_tbl[256];
}panel_extend_para;

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
	unsigned int            lcd_edp_swing_level;

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

typedef void (*LCD_FUNC) (unsigned int sel);
typedef struct lcd_function
{
	LCD_FUNC func;
	unsigned int delay;//ms
}disp_lcd_function;

typedef struct lcd_flow
{
	disp_lcd_function func[5];
	unsigned int func_num;
	unsigned int cur_step;
}disp_lcd_flow;

typedef struct
{
	void (*cfg_panel_info)(panel_extend_para * info);
	int (*cfg_open_flow)(unsigned int sel);
	int (*cfg_close_flow)(unsigned int sel);
	int (*lcd_user_defined_func)(unsigned int sel, unsigned int para1, unsigned int para2, unsigned int para3);
}disp_lcd_panel_fun;

struct sunxi_disp_source_ops
{
  int (*sunxi_lcd_delay_ms)(unsigned int ms);
  int (*sunxi_lcd_delay_us)(unsigned int us);
  int (*sunxi_lcd_tcon_enable)(unsigned int scree_id);
  int (*sunxi_lcd_tcon_disable)(unsigned int scree_id);
  int (*sunxi_lcd_pwm_set_duty_ns)(unsigned int pwm_channel, unsigned int duty_ns);
  int (*sunxi_lcd_pwm_enable)(unsigned int pwm_channel);
  int (*sunxi_lcd_pwm_disable)(unsigned int pwm_channel);
  int (*sunxi_lcd_cpu_write)(unsigned int scree_id, unsigned int command, unsigned int *para, unsigned int para_num);
  int (*sunxi_lcd_cpu_write_index)(unsigned int scree_id, unsigned int index);
  int (*sunxi_lcd_cpu_write_data)(unsigned int scree_id, unsigned int data);
  int (*sunxi_lcd_dsi_write)(unsigned int scree_id, unsigned char command, unsigned char *para, unsigned int para_num);
  int (*sunxi_lcd_dsi_clk_enable)(__u32 screen_id, __u32 en);
  int (*sunxi_disp_get_num_screens)(void);
  int (*sunxi_lcd_backlight_enable)(unsigned int screen_id);
  int (*sunxi_lcd_backlight_disable)(unsigned int screen_id);
  int (*sunxi_lcd_power_enable)(unsigned int screen_id, unsigned int pwr_id);
  int (*sunxi_lcd_power_disable)(unsigned int screen_id, unsigned int pwr_id);
  int (*sunxi_lcd_get_driver_name)(unsigned int screen_id, char *name);
  int (*sunxi_lcd_set_panel_funs)(char *drv_name, disp_lcd_panel_fun * lcd_cfg);
  int (*sunxi_lcd_pin_cfg)(unsigned int screen_id, unsigned int bon);
  int (*sunxi_lcd_gpio_set_value)(unsigned int screen_id, unsigned int io_index, u32 value);
  int (*sunxi_lcd_gpio_set_direction)(unsigned int screen_id, unsigned int io_index, u32 direction);
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum
{
	FB_MODE_SCREEN0 = 0,
	FB_MODE_SCREEN1 = 1,
	FB_MODE_DUAL_SAME_SCREEN_TB = 2,//two screen, top buffer for screen0, bottom buffer for screen1
	FB_MODE_DUAL_DIFF_SCREEN_SAME_CONTENTS = 3,//two screen, they have same contents;
}disp_fb_mode;

typedef struct
{
	disp_fb_mode       fb_mode;
	disp_layer_mode    mode;
	unsigned int       buffer_num;
	unsigned int       width;
	unsigned int       height;

	unsigned int       output_width;//used when scaler mode
	unsigned int       output_height;//used when scaler mode

	unsigned int       primary_screen_id;//used when FB_MODE_DUAL_DIFF_SCREEN_SAME_CONTENTS
	unsigned int       aux_output_width;//used when FB_MODE_DUAL_DIFF_SCREEN_SAME_CONTENTS
	unsigned int       aux_output_height;//used when FB_MODE_DUAL_DIFF_SCREEN_SAME_CONTENTS
}disp_fb_create_info;

typedef enum
{
	DISP_INIT_MODE_SCREEN0 = 0,//fb0 for screen0
	DISP_INIT_MODE_SCREEN1 = 1,//fb0 for screen1
	DISP_INIT_MODE_TWO_DIFF_SCREEN = 2,//fb0 for screen0 and fb1 for screen1
	DISP_INIT_MODE_TWO_SAME_SCREEN = 3,//fb0(up buffer for screen0, down buffer for screen1)
	DISP_INIT_MODE_TWO_DIFF_SCREEN_SAME_CONTENTS = 4,//fb0 for two different screen(screen0 layer is normal layer, screen1 layer is scaler layer);
}disp_init_mode;

typedef struct
{
	bool                  b_init;
	disp_init_mode        disp_mode;//0:single screen0(fb0); 1:single screen1(fb0);  2:dual diff screen(fb0, fb1); 3:dual same screen(fb0 up and down); 4:dual diff screen same contents(fb0)

	//for screen0 and screen1
	disp_output_type      output_type[2];
	unsigned int          output_mode[2];

	//for fb0 and fb1
	unsigned int          buffer_num[2];
	bool                  scaler_mode[2];
	disp_pixel_format     format[2];
	unsigned int          fb_width[2];
	unsigned int          fb_height[2];
}disp_init_para;

typedef struct
{
	int                 post2_layers;
	disp_layer_info     layer_info[8];
	disp_window         fb_scn_win;

	int                 primary_display_layer_num;
	int                 show_black[2];
	int                 time_stamp;

	int                 acquireFenceFd[8];
	struct sync_fence   *acquireFence[8];
}setup_dispc_data_t;

typedef enum tag_DISP_CMD
{
	//----disp global----
	DISP_CMD_RESERVE0 = 0x00,
	DISP_CMD_RESERVE1 = 0x01,
	DISP_CMD_SET_BKCOLOR = 0x03,
	DISP_CMD_GET_BKCOLOR = 0x04,
	DISP_CMD_SET_COLORKEY = 0x05,
	DISP_CMD_GET_COLORKEY = 0x06,
	DISP_CMD_GET_SCN_WIDTH = 0x07,
	DISP_CMD_GET_SCN_HEIGHT = 0x08,
	DISP_CMD_GET_OUTPUT_TYPE = 0x09,
	DISP_CMD_SET_EXIT_MODE = 0x0A,
	DISP_CMD_VSYNC_EVENT_EN = 0x0B,
	DISP_CMD_BLANK = 0x0C,
	DISP_CMD_SHADOW_PROTECT = 0x0D,
	DISP_CMD_HWC_COMMIT = 0x0E,

	//----layer----
	DISP_CMD_LAYER_ENABLE = 0x40,
	DISP_CMD_LAYER_DISABLE = 0x41,
	DISP_CMD_LAYER_SET_INFO = 0x42,
	DISP_CMD_LAYER_GET_INFO = 0x43,
	DISP_CMD_LAYER_TOP = 0x44,
	DISP_CMD_LAYER_BOTTOM = 0x45,
	DISP_CMD_LAYER_GET_FRAME_ID = 0x46,

	//----hwc----
	DISP_CMD_CURSOR_ENABLE = 0x80,
	DISP_CMD_CURSOR_DISABLE = 0x81,
	DISP_CMD_CURSOR_SET_POS = 0x82,
	DISP_CMD_CURSOR_GET_POS = 0x83,
	DISP_CMD_CURSOR_SET_FB = 0x84,
	DISP_CMD_CURSOR_SET_PALETTE = 0x85,

	//----hdmi----
	DISP_CMD_HDMI_ENABLE = 0xc0,
	DISP_CMD_HDMI_DISABLE = 0xc1,
	DISP_CMD_HDMI_SET_MODE = 0xc2,
	DISP_CMD_HDMI_GET_MODE = 0xc3,
	DISP_CMD_HDMI_SUPPORT_MODE = 0xc4,
	DISP_CMD_HDMI_GET_HPD_STATUS = 0xc5,
	DISP_CMD_HDMI_SET_SRC = 0xc6,

	//----lcd----
	DISP_CMD_LCD_ENABLE = 0x100,
	DISP_CMD_LCD_DISABLE = 0x101,
	DISP_CMD_LCD_SET_BRIGHTNESS = 0x102,
	DISP_CMD_LCD_GET_BRIGHTNESS = 0x103,
	DISP_CMD_LCD_BACKLIGHT_ENABLE  = 0x104,
	DISP_CMD_LCD_BACKLIGHT_DISABLE  = 0x105,
	DISP_CMD_LCD_SET_SRC = 0x106,
	DISP_CMD_LCD_SET_FPS  = 0x107,
	DISP_CMD_LCD_GET_FPS  = 0x108,
	DISP_CMD_LCD_GET_SIZE = 0x109,
	DISP_CMD_LCD_GET_MODEL_NAME = 0x10a,
	DISP_CMD_LCD_SET_GAMMA_TABLE = 0x10b,
	DISP_CMD_LCD_GAMMA_CORRECTION_ENABLE = 0x10c,
	DISP_CMD_LCD_GAMMA_CORRECTION_DISABLE = 0x10d,
	DISP_CMD_LCD_USER_DEFINED_FUNC = 0x10e,
	DISP_CMD_LCD_CHECK_OPEN_FINISH = 0x10f,
	DISP_CMD_LCD_CHECK_CLOSE_FINISH = 0x110,

	//---- capture ---
	DISP_CMD_CAPTURE_SCREEN = 0x140,//caputre screen and scaler to dram
	DISP_CMD_CAPTURE_SCREEN_STOP = 0x141,//for continue mode

	//---enhance ---
	DISP_CMD_ENHANCE_ENABLE = 0x180,
	DISP_CMD_ENHANCE_DISABLE = 0x181,
	DISP_CMD_GET_ENHANCE_EN = 0x182,
	DISP_CMD_SET_BRIGHT = 0x183,
	DISP_CMD_GET_BRIGHT = 0x184,
	DISP_CMD_SET_CONTRAST = 0x185,
	DISP_CMD_GET_CONTRAST = 0x186,
	DISP_CMD_SET_SATURATION = 0x187,
	DISP_CMD_GET_SATURATION = 0x188,
	DISP_CMD_SET_HUE = 0x189,
	DISP_CMD_GET_HUE = 0x18a,
	DISP_CMD_SET_ENHANCE_WINDOW = 0x18b,
	DISP_CMD_GET_ENHANCE_WINDOW = 0x18c,
	DISP_CMD_SET_ENHANCE_MODE = 0x18d,
	DISP_CMD_GET_ENHANCE_MODE = 0x18e,

	DISP_CMD_LAYER_ENHANCE_ENABLE = 0x1c0,
	DISP_CMD_LAYER_ENHANCE_DISABLE = 0x1c1,
	DISP_CMD_LAYER_GET_ENHANCE_EN = 0x1c2,
	DISP_CMD_LAYER_SET_BRIGHT = 0x1c3,
	DISP_CMD_LAYER_GET_BRIGHT = 0x1c4,
	DISP_CMD_LAYER_SET_CONTRAST = 0x1c5,
	DISP_CMD_LAYER_GET_CONTRAST = 0x1c6,
	DISP_CMD_LAYER_SET_SATURATION = 0x1c7,
	DISP_CMD_LAYER_GET_SATURATION = 0x1c8,
	DISP_CMD_LAYER_SET_HUE = 0x1c9,
	DISP_CMD_LAYER_GET_HUE = 0x1ca,
	DISP_CMD_LAYER_SET_ENHANCE_WINDOW = 0X1cb,
	DISP_CMD_LAYER_GET_ENHANCE_WINDOW = 0X1cc,
	DISP_CMD_LAYER_SET_ENHANCE_MODE = 0x1cd,
	DISP_CMD_LAYER_GET_ENHANCE_MODE = 0x1ce,

	DISP_CMD_DRC_ENABLE = 0x200,
	DISP_CMD_DRC_DISABLE = 0x201,
	DISP_CMD_GET_DRC_EN = 0x202,
	DISP_CMD_DRC_SET_WINDOW = 0x203,
	DISP_CMD_DRC_GET_WINDOW = 0x204,

	//---- for test
	DISP_CMD_FB_REQUEST = 0x280,
	DISP_CMD_FB_RELEASE = 0x281,

	DISP_CMD_MEM_REQUEST = 0x2c0,
	DISP_CMD_MEM_RELEASE = 0x2c1,
	DISP_CMD_MEM_GETADR = 0x2c2,
	DISP_CMD_MEM_SELIDX = 0x2c3,

	DISP_CMD_PRINT_REG = 0x2e0,
}__disp_cmd_t;

#define FBIOGET_LAYER_HDL_0 0x4700
#define FBIOGET_LAYER_HDL_1 0x4701

#endif
