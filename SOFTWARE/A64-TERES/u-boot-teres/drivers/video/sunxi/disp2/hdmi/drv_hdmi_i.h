#ifndef  _DRV_HDMI_I_H_
#define  _DRV_HDMI_I_H_
#if defined(__LINUX_PLAT__)
#include <linux/module.h>
#include <asm/uaccess.h>
#include <asm/memory.h>
#include <asm/unistd.h>
#include "asm-generic/int-ll64.h"
#include "linux/kernel.h"
#include "linux/mm.h"
#include "linux/semaphore.h"
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/dma-mapping.h>
#include <linux/sched.h>   //wake_up_process()
#include <linux/kthread.h> //kthread_create()??kthread_run()
#include <linux/err.h> //IS_ERR()??PTR_ERR()
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/switch.h>
#include <linux/types.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of_iommu.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>

#include <video/sunxi_display2.h>
#include <video/drv_hdmi.h>
#else
#include "../disp/disp_sys_intf.h"
#include "../disp/de/bsp_display.h"
#endif

extern u32 hdmi_print;
extern u32 rgb_only;
extern u32 hdmi_hpd_mask;//0x10: force unplug; 0x11: force plug

#if 0
#define __inf(msg...)       do{if (hdmi_print){printk(KERN_WARNING "[HDMI] ");printk(msg);}}while (0)
#define __msg(msg...)       do{if (hdmi_print){printk(KERN_WARNING "[HDMI] file:%s,line:%d:",__FILE__,__LINE__);printk(msg);}}while (0)
#define __wrn(msg...)       do{printk(KERN_WARNING "[HDMI WRN] file:%s,line:%d:    ",__FILE__,__LINE__);printk(msg);}while (0)
#define __here__            do{if (hdmi_print){printk(KERN_WARNING "[HDMI] file:%s,line:%d\n",__FILE__,__LINE__);}}while (0)
#else
#endif

s32 hdmi_init(void);
s32 hdmi_exit(void);
s32 hdmi_hpd_state(u32 state);
s32 hdmi_hpd_event(void);

typedef struct
{
	struct device           *dev;
	bool                    bopen;
	enum disp_tv_mode            mode;//vic
	u32                   base_hdmi;
	struct work_struct      hpd_work;
}hdmi_info_t;

extern hdmi_info_t ghdmi;
extern struct disp_video_timings video_timing[];

extern s32 hdmi_i2c_add_driver(void);
extern s32 hdmi_i2c_del_driver(void);

extern int disp_sys_script_get_item(char *main_name, char *sub_name, int value[], int type);
extern uintptr_t disp_getprop_regbase(char *main_name, char *sub_name, u32 index);
extern u32 disp_getprop_irq(char *main_name, char *sub_name, u32 index);

struct disp_hdmi_mode
{
	enum disp_tv_mode mode;
	int hdmi_mode;//vic
};

#endif
