#ifndef _DISP_PRIVATE_H_
#define _DISP_PRIVATE_H_

#include "include.h"
#include "disp_features.h"

typedef enum
{
	DISP_MOD_DE           = 0,
	DISP_MOD_LCD0         = 1,
	DISP_MOD_LCD1         = 2,
	DISP_MOD_LCD2         = 3,
	DISP_MOD_DSI0         = 4,
	DISP_MOD_DSI1         = 5,
	DISP_MOD_DSI2         = 6,
	DISP_MOD_HDMI         = 7,
	DISP_MOD_NUM          = 8,
}disp_mod_id;

typedef struct
{
	u32 reg_base[DISP_MOD_NUM];
	u32 reg_size[DISP_MOD_NUM];
	u32 irq_no[DISP_MOD_NUM];

	s32 (*disp_int_process)(u32 sel);
	s32 (*vsync_event)(u32 sel);
	s32 (*start_process)(void);
	s32 (*capture_event)(u32 sel);
	s32 (*shadow_protect)(u32 sel, bool protect);
}disp_bsp_init_para;

typedef void (*LCD_FUNC) (unsigned int sel);
typedef struct lcd_function
{
	LCD_FUNC func;
	unsigned int delay;//ms
}disp_lcd_function;

#define LCD_MAX_SEQUENCES 7
typedef struct lcd_flow
{
    disp_lcd_function func[LCD_MAX_SEQUENCES];
    unsigned int func_num;
    unsigned int cur_step;
}disp_lcd_flow;

typedef struct
{
	void (*cfg_panel_info)(panel_extend_para * info);
	int (*cfg_open_flow)(unsigned int sel);
	int (*cfg_close_flow)(unsigned int sel);
	int (*lcd_user_defined_func)(unsigned int sel, unsigned int para1, unsigned int para2, unsigned int para3);
	int (*set_bright)(unsigned int sel, unsigned int bright);
}disp_lcd_panel_fun;

typedef struct
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
}disp_enhance_para;

struct disp_device {
	struct list_head list;
	/* data fields */
	char name[32];
	u32 disp;
	u32 fix_timing;
	disp_output_type type;
	struct disp_manager *manager;
	disp_video_timings timings;
	void* priv_data;

	/* function fileds  */
	/* init: script init && clock init && pwm init && register irq
	 * exit: clock exit && unregister irq
	 */
	s32 (*init)(struct disp_device *dispdev);
	s32 (*exit)(struct disp_device *dispdev);

	s32 (*set_manager)(struct disp_device *dispdev, struct disp_manager *mgr);
	s32 (*unset_manager)(struct disp_device *dispdev);

	s32 (*enable)(struct disp_device *dispdev);
	s32 (*disable)(struct disp_device *dispdev);
	s32 (*is_enabled)(struct disp_device *dispdev);
	s32 (*is_used)(struct disp_device *dispdev);
	s32 (*get_resolution)(struct disp_device *dispdev, u32 *xres, u32 *yres);
	s32 (*get_dimensions)(struct disp_device *dispdev, u32 *width, u32 *height);
	s32 (*set_timings)(struct disp_device *dispdev, disp_video_timings *timings);
	s32 (*get_timings)(struct disp_device *dispdev, disp_video_timings *timings);
	s32 (*check_timings)(struct disp_device *dispdev, disp_video_timings *timings);
	s32 (*detect)(struct disp_device *dispdev);

	s32 (*get_input_csc)(struct disp_device *dispdev);
	s32 (*get_input_color_range)(struct disp_device *dispdev);
	s32 (*is_interlace)(struct disp_device *dispdev);

	/* power manager */
	s32 (*early_suspend)(struct disp_device *dispdev);
	s32 (*late_resume)(struct disp_device *dispdev);
	s32 (*suspend)(struct disp_device *dispdev);
	s32 (*resume)(struct disp_device *dispdev);

	s32 (*dump)(struct disp_device *dispdev, char *buf);

	/* HDMI /TV */
	s32 (*set_mode)(struct disp_device *dispdev, u32 mode);
	s32 (*get_mode)(struct disp_device *dispdev);
	s32 (*check_support_mode)(struct disp_device* dispdev, u32 mode);
	s32 (*set_func)(struct disp_device*  dispdev, disp_hdmi_func * func);
	s32 (*set_tv_func)(struct disp_device*  dispdev, disp_tv_func * func);

	/* LCD */
	s32 (*set_bright)(struct disp_device *dispdev, u32 bright);
	s32 (*get_bright)(struct disp_device *dispdev);
	s32 (*backlight_enable)(struct disp_device *dispdev);
	s32 (*backlight_disable)(struct disp_device *dispdev);
	s32 (*pwm_enable)(struct disp_device *dispdev);
	s32 (*pwm_disable)(struct disp_device *dispdev);
	s32 (*power_enable)(struct disp_device *dispdev, u32 power_id);
	s32 (*power_disable)(struct disp_device *dispdev, u32 power_id);
	s32 (*tcon_enable)(struct disp_device *dispdev);
	s32 (*tcon_disable)(struct disp_device *dispdev);
	s32 (*set_bright_dimming)(struct disp_device *dispdev, u32 dimming);
	disp_lcd_flow *(*get_open_flow)(struct disp_device *dispdev);
	disp_lcd_flow *(*get_close_flow)(struct disp_device *dispdev);
	s32 (*pin_cfg)(struct disp_device *dispdev, u32 bon);
	s32 (*set_gamma_tbl)(struct disp_device* dispdev, u32 *tbl, u32 size);
	s32 (*enable_gamma)(struct disp_device* dispdev);
	s32 (*disable_gamma)(struct disp_device* dispdev);
	s32 (*set_panel_func)(struct disp_device *lcd, char *name, disp_lcd_panel_fun * lcd_cfg);
	s32 (*set_open_func)(struct disp_device* lcd, LCD_FUNC func, u32 delay);
	s32 (*set_close_func)(struct disp_device* lcd, LCD_FUNC func, u32 delay);
	int (*gpio_set_value)(struct disp_device* dispdev, unsigned int io_index, u32 value);
	int (*gpio_set_direction)(struct disp_device* dispdev, unsigned int io_index, u32 direction);
	int (*get_panel_info)(struct disp_device* dispdev, disp_panel_para *info);
};

/* manager */
struct disp_manager {
	/* data fields */
	char name[32];
	u32 disp;
	u32 num_chns;
	u32 num_layers;
	struct disp_device *device;
	struct disp_smbl *smbl;
	struct disp_enhance *enhance;
	struct disp_capture *cptr;

	struct list_head lyr_list;

	/* function fields */
	s32 (*enable)(struct disp_manager *mgr);
	s32 (*disable)(struct disp_manager *mgr);
	s32 (*is_enabled)(struct disp_manager *mgr);

	/* init: clock init && reg init && register irq
	 * exit: clock exit && unregister irq
	 */
	s32 (*init)(struct disp_manager *mgr);
	s32 (*exit)(struct disp_manager *mgr);

	s32 (*set_back_color)(struct disp_manager *mgr,	disp_color *bk_color);
	s32 (*get_back_color)(struct disp_manager *mgr,	disp_color *bk_color);
	s32 (*set_color_key)(struct disp_manager *mgr, disp_colorkey *ck);
	s32 (*get_color_key)(struct disp_manager *mgr, disp_colorkey *ck);

	s32 (*get_screen_size)(struct disp_manager *mgr, u32 *width, u32 *height);
	s32 (*set_screen_size)(struct disp_manager *mgr, u32 width, u32 height);

	/* layer mamage */
	s32 (*check_layer_zorder)(struct disp_manager *mgr, disp_layer_config *config, u32 layer_num);
	s32 (*set_layer_config)(struct disp_manager *mgr, disp_layer_config *config, unsigned int layer_num);
	s32 (*get_layer_config)(struct disp_manager *mgr, disp_layer_config *config, unsigned int layer_num);
	s32 (*extend_layer_config)(struct disp_manager *mgr, disp_layer_config *info, unsigned int layer_num);

	s32 (*apply)(struct disp_manager *mgr);
	s32 (*force_apply)(struct disp_manager *mgr);
	s32 (*update_regs)(struct disp_manager *mgr);
	s32 (*sync)(struct disp_manager *mgr);

	/* debug interface, dump manager info */
	s32 (*dump)(struct disp_manager *mgr, char *buf);
};

struct disp_layer {
	/* data fields */
	char name[32];
	u32 disp;
	u32 chn;
	u32 id;

	//enum disp_layer_feat caps;
	struct disp_manager *manager;
	struct list_head list;
	void* data;

	/* function fileds */

//	s32 (*is_support_caps)(struct disp_layer* layer, enum disp_layer_feat caps);
	s32 (*is_support_format)(struct disp_layer* layer, disp_pixel_format fmt);
	s32 (*set_manager)(struct disp_layer* layer, struct disp_manager *mgr);
	s32 (*unset_manager)(struct disp_layer* layer);

	s32 (*check)(struct disp_layer* layer, disp_layer_config *config);
	s32 (*save_and_dirty_check)(struct disp_layer* layer, disp_layer_config *config);
	s32 (*get_config)(struct disp_layer* layer, disp_layer_config *config);
	s32 (*apply)(struct disp_layer* layer);
	s32 (*force_apply)(struct disp_layer* layer);
	s32 (*is_dirty)(struct disp_layer* layer);
	s32 (*dirty_clear)(struct disp_layer* layer);

	/* init: NULL
	 * exit: NULL
	 */
	s32 (*init)(struct disp_layer *layer);
	s32 (*exit)(struct disp_layer *layer);

	s32 (*get_frame_id)(struct disp_layer *layer);

	s32 (*dump)(struct disp_layer* layer, char *buf);
};

struct disp_smbl {
	/* static fields */
	char *name;
	u32 disp;
	u32 backlight;
	struct disp_manager *manager;

	/*
	 * The following functions do not block:
	 *
	 * is_enabled
	 * set_layer_info
	 * get_layer_info
	 *
	 * The rest of the functions may block and cannot be called from
	 * interrupt context
	 */

	s32 (*enable)(struct disp_smbl *smbl);
	s32 (*disable)(struct disp_smbl *smbl);
	bool (*is_enabled)(struct disp_smbl *smbl);
	s32 (*set_manager)(struct disp_smbl* smbl, struct disp_manager *mgr);
	s32 (*unset_manager)(struct disp_smbl* smbl);
	s32 (*update_backlight)(struct disp_smbl* smbl, unsigned int bl);

	/* init: NULL
	 * exit: NULL
	 */
	s32 (*init)(struct disp_smbl *smbl);
	s32 (*exit)(struct disp_smbl *smbl);

	s32 (*apply)(struct disp_smbl *smbl);
	s32 (*update_regs)(struct disp_smbl *smbl);
	s32 (*force_apply)(struct disp_smbl *smbl);
	s32 (*sync)(struct disp_smbl *smbl);

	s32 (*set_window)(struct disp_smbl* smbl, disp_rect *window);
	s32 (*get_window)(struct disp_smbl* smbl, disp_rect *window);
};

struct disp_enhance {
	/* static fields */
	char *name;
	u32 disp;
	struct disp_manager *manager;

	/*
	 * The following functions do not block:
	 *
	 * is_enabled
	 * set_layer_info
	 * get_layer_info
	 *
	 * The rest of the functions may block and cannot be called from
	 * interrupt context
	 */

	s32 (*enable)(struct disp_enhance *enhance);
	s32 (*disable)(struct disp_enhance *enhance);
	bool (*is_enabled)(struct disp_enhance *enhance);
	s32 (*set_manager)(struct disp_enhance* enhance, struct disp_manager *mgr);
	s32 (*unset_manager)(struct disp_enhance* enhance);

	/* init: NULL
	 * exit: NULL
	 */
	s32 (*init)(struct disp_enhance *enhance);
	s32 (*exit)(struct disp_enhance *enhance);

	s32 (*apply)(struct disp_enhance *enhance);
	s32 (*update_regs)(struct disp_enhance *enhance);
	s32 (*force_apply)(struct disp_enhance *enhance);
	s32 (*sync)(struct disp_enhance *enhance);

	/* power manager */
	s32 (*early_suspend)(struct disp_enhance* enhance);
	s32 (*late_resume)(struct disp_enhance* enhance);
	s32 (*suspend)(struct disp_enhance* enhance);
	s32 (*resume)(struct disp_enhance* enhance);

	s32 (*set_bright)(struct disp_enhance* enhance, u32 val);
	s32 (*set_saturation)(struct disp_enhance* enhance, u32 val);
	s32 (*set_contrast)(struct disp_enhance* enhance, u32 val);
	s32 (*set_hue)(struct disp_enhance* enhance, u32 val);
	s32 (*set_mode)(struct disp_enhance* enhance, u32 val);
	s32 (*set_window)(struct disp_enhance* enhance, disp_rect *window);
	s32 (*get_bright)(struct disp_enhance* enhance);
	s32 (*get_saturation)(struct disp_enhance* enhance);
	s32 (*get_contrast)(struct disp_enhance* enhance);
	s32 (*get_hue)(struct disp_enhance* enhance);
	s32 (*get_mode)(struct disp_enhance* enhance);
	s32 (*get_window)(struct disp_enhance* enhance, disp_rect *window);
	s32 (*set_para)(struct disp_enhance *enhance, disp_enhance_para *para);
};

struct disp_capture {
	char * name;
	u32 disp;
	struct disp_manager * manager;

	s32 (*set_manager)(struct disp_capture *cptr, struct disp_manager *mgr);
	s32 (*unset_manager)(struct disp_capture *cptr);
	s32 (*start)(struct disp_capture *cptr);
	s32 (*commmit)(struct disp_capture *cptr, disp_capture_info *info);
	s32 (*stop)(struct disp_capture *cptr);
	s32 (*sync)(struct disp_capture *cptr);
	s32 (*init)(struct disp_capture *cptr);
	s32 (*exit)(struct disp_capture *cptr);

	/* inner interface */
	s32 (*apply)(struct disp_capture *cptr);
};

extern struct disp_device* disp_get_lcd(u32 disp);

extern struct disp_device* disp_get_hdmi(u32 disp);

extern struct disp_manager* disp_get_layer_manager(u32 disp);

extern struct disp_layer* disp_get_layer(u32 disp, u32 chn, u32 layer_id);
extern struct disp_layer* disp_get_layer_1(u32 disp, u32 layer_id);
extern struct disp_smbl* disp_get_smbl(u32 disp);
extern struct disp_enhance* disp_get_enhance(u32 disp);
extern struct disp_capture* disp_get_capture(u32 disp);

extern s32 disp_delay_ms(u32 ms);
extern s32 disp_delay_us(u32 us);
extern s32 disp_init_lcd(disp_bsp_init_para * para);
extern s32 disp_init_hdmi(disp_bsp_init_para *para);
extern s32 disp_init_tv(disp_bsp_init_para * para);
extern s32 disp_init_feat(void);
extern s32 disp_init_mgr(disp_bsp_init_para * para);
extern s32 disp_init_enhance(disp_bsp_init_para * para);
extern s32 disp_init_smbl(disp_bsp_init_para * para);
extern s32 disp_init_capture(disp_bsp_init_para *para);

#if defined(CONFIG_ARCH_SUN8IW6)
#include "./lowlevel_sun8iw6/disp_al.h"
#elif defined(CONFIG_ARCH_SUN8IW7)
#include "./lowlevel_sun8iw7/disp_al.h"
#elif defined(CONFIG_ARCH_SUN8IW8)
#include "./lowlevel_sun8iw8/disp_al.h"
#elif defined(CONFIG_ARCH_SUN8IW9)
#include "./lowlevel_sun8iw9/disp_al.h"
#elif defined(CONFIG_ARCH_SUN50IW1P1)
#include "./lowlevel_sun50iw1p1/disp_al.h"
#else
#error "undefined platform!!!"
#endif
#include "disp_device.h"

u32 dump_layer_config(struct disp_layer_config_data *data);

#endif

