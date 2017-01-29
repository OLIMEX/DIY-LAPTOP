#ifndef __DEV_DISP_H__
#define __DEV_DISP_H__

#include "dev_disp_debugfs.h"
#include "de/bsp_display.h"
#include "de/disp_display.h"
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#ifdef CONFIG_ION_SUNXI
#define FB_RESERVED_MEM
#endif

typedef enum
{
	DISPLAY_NORMAL = 0,
	DISPLAY_LIGHT_SLEEP = 1,
	DISPLAY_DEEP_SLEEP = 2,
	DISPLAY_BLANK = 4,
}disp_standby_flags;

struct info_mm
{
	void *info_base;	/* Virtual address */
	uintptr_t mem_start;	/* Start of frame buffer mem */
				/* (physical address) */
	u32 mem_len;			/* Length of frame buffer mem */
};

struct proc_list
{
	void (*proc)(u32 screen_id);
	struct list_head list;
};

struct ioctl_list
{
	unsigned int cmd;
	int (*func)(unsigned int cmd, unsigned long arg);
	struct list_head list;
};

struct standby_cb_list
{
	int (*suspend)(void);
	int (*resume)(void);
	struct list_head list;
};

typedef struct
{
	bool                  b_init;
	enum disp_init_mode        disp_mode;//0:single screen0(fb0); 1:single screen1(fb0); 2:single screen2(fb0)

	//for screen0/1/2
	enum disp_output_type      output_type[3];
	unsigned int          output_mode[3];

	//for fb0/1/2
	unsigned int          buffer_num[3];
	enum disp_pixel_format     format[3];
	unsigned int          fb_width[3];
	unsigned int          fb_height[3];
}disp_init_para;

typedef struct
{
	struct device           *dev;
	uintptr_t               reg_base[DISP_MOD_NUM];
	u32                     irq_no[DISP_MOD_NUM];
	struct clk              *mclk[DISP_MOD_NUM];

	disp_init_para          disp_init;
	struct disp_manager     *mgr[DEVICE_NUM];

	struct proc_list        sync_proc_list;
	struct proc_list        sync_finish_proc_list;
	struct ioctl_list       ioctl_extend_list;
	struct ioctl_list       compat_ioctl_extend_list;
	struct standby_cb_list  stb_cb_list;
	struct mutex            mlock;
	struct work_struct      resume_work[3];
	struct work_struct      start_work;

	u32    		              exit_mode;//0:clean all  1:disable interrupt
	bool			              b_lcd_enabled[3];
	bool                    inited;//indicate driver if init
	disp_bsp_init_para      para;
#if defined(CONFIG_ION_SUNXI)
	struct ion_client *client;
	struct ion_handle *handle;
#endif
}disp_drv_info;

struct sunxi_disp_mod {
	disp_mod_id id;
	char name[32];
};

struct __fb_addr_para
{
	uintptr_t fb_paddr;
	int fb_size;
};

#define DISP_RESOURCE(res_name, res_start, res_end, res_flags) \
{\
	.start = (int __force)res_start, \
	.end = (int __force)res_end, \
	.flags = res_flags, \
	.name = #res_name \
},

typedef struct bmp_color_table_entry {
	u8	blue;
	u8	green;
	u8	red;
	u8	reserved;
} __attribute__ ((packed)) bmp_color_table_entry_t;

typedef struct bmp_header {
	/* Header */
	char signature[2];
	u32	file_size;
	u32	reserved;
	u32	data_offset;
	/* InfoHeader */
	u32	size;
	u32	width;
	u32	height;
	u16	planes;
	u16	bit_count;
	u32	compression;
	u32	image_size;
	u32	x_pixels_per_m;
	u32	y_pixels_per_m;
	u32	colors_used;
	u32	colors_important;
	/* ColorTable */
} __attribute__ ((packed)) bmp_header_t;

typedef struct bmp_pad_head {
	char data[2]; //pading 2 byte
	char signature[2];
	u32	file_size;
	u32	reserved;
	u32	data_offset;
	/* InfoHeader */
	u32	size;
	u32	width;
	u32	height;
	u16	planes;
	u16	bit_count;
	u32	compression;
	u32	image_size;
	u32	x_pixels_per_m;
	u32	y_pixels_per_m;
	u32	colors_used;
	u32	colors_important;
}__attribute__ ((packed)) bmp_pad_header_t;

typedef struct bmp_image {
	bmp_header_t header;
	/* We use a zero sized array just as a placeholder for variable
	   sized array */
	bmp_color_table_entry_t color_table[0];
} bmp_image_t;

typedef struct
{
	int x;
	int y;
	int bit;
	void *buffer;
}
sunxi_bmp_store_t;


int disp_open(struct inode *inode, struct file *file);
int disp_release(struct inode *inode, struct file *file);
ssize_t disp_read(struct file *file, char __user *buf, size_t count, loff_t *ppos);
ssize_t disp_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos);
int disp_mmap(struct file *file, struct vm_area_struct * vma);
long disp_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

s32 disp_create_heap(u32 pHeapHead, u32 pHeapHeadPhy, u32 nHeapSize);
void *disp_malloc(u32 num_bytes, void *phy_addr);
void  disp_free(void *virt_addr, void* phy_addr, u32 num_bytes);

extern s32 disp_register_sync_proc(void (*proc)(u32));
extern s32 disp_unregister_sync_proc(void (*proc)(u32));
extern s32 disp_register_sync_finish_proc(void (*proc)(u32));
extern s32 disp_unregister_sync_finish_proc(void (*proc)(u32));
extern s32 disp_register_ioctl_func(unsigned int cmd, int (*proc)(unsigned int cmd, unsigned long arg));
extern s32 disp_unregister_ioctl_func(unsigned int cmd);
extern s32 disp_register_compat_ioctl_func(unsigned int cmd, int (*proc)(unsigned int cmd, unsigned long arg));
extern s32 disp_register_standby_func(int (*suspend)(void), int (*resume)(void));
extern s32 disp_unregister_standby_func(int (*suspend)(void), int (*resume)(void));
extern s32 composer_init(disp_drv_info *psg_disp_drv);
extern unsigned int composer_dump(char* buf);
extern s32 disp_tv_register(struct disp_tv_func * func);
extern s32 disp_set_hdmi_detect(bool hpd);

extern disp_drv_info    g_disp_drv;

extern int sunxi_disp_get_source_ops(struct sunxi_disp_source_ops *src_ops);
extern s32 disp_lcd_open(u32 sel);
extern s32 disp_lcd_close(u32 sel);
extern s32 fb_init(struct platform_device *pdev);
extern s32 fb_exit(void);
extern int lcd_init(void);

s32 disp_set_hdmi_func(struct disp_device_func * func);
s32 sunxi_get_fb_addr_para(struct __fb_addr_para *fb_addr_para);
s32 fb_draw_colorbar(char* base, u32 width, u32 height, struct fb_var_screeninfo *var);
s32 fb_draw_gray_pictures(char* base, u32 width, u32 height, struct fb_var_screeninfo *var);
s32 drv_disp_vsync_event(u32 sel);
void DRV_disp_int_process(u32 sel);
s32 Display_set_fb_timming(u32 sel);
unsigned int disp_boot_para_parse(const char *name);
int disp_get_parameter_for_cmdlind(char *cmdline, char *name, char *value);

#endif
