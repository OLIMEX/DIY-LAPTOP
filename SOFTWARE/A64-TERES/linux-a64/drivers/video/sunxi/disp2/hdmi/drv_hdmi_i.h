#ifndef  _DRV_HDMI_I_H_
#define  _DRV_HDMI_I_H_
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

extern u32 hdmi_print;
extern u32 rgb_only;
extern u32 hdmi_hpd_mask;//0x10: force unplug; 0x11: force plug

#define OSAL_PRINTF(msg...) do{printk(KERN_WARNING "[HDMI] ");printk(msg);}while (0)
#define __inf(msg...)       do{if (hdmi_print){printk(KERN_WARNING "[HDMI] ");printk(msg);}}while (0)
#define __msg(msg...)       do{if (hdmi_print){printk(KERN_WARNING "[HDMI] file:%s,line:%d:",__FILE__,__LINE__);printk(msg);}}while (0)
#define __wrn(msg...)       do{printk(KERN_WARNING "[HDMI WRN] file:%s,line:%d:    ",__FILE__,__LINE__);printk(msg);}while (0)
#define __here__            do{if (hdmi_print){printk(KERN_WARNING "[HDMI] file:%s,line:%d\n",__FILE__,__LINE__);}}while (0)


s32 hdmi_init(struct platform_device *pdev);
s32 hdmi_exit(void);
extern s32 Fb_Init(u32 from);
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
int register_sunxi_hdmi_notifier(struct notifier_block *nb);
int unregister_sunxi_hdmi_notifier(struct notifier_block *nb);
extern int sunxi_hdmi_notifier_call_chain(unsigned long val);

extern s32 hdmi_i2c_add_driver(void);
extern s32 hdmi_i2c_del_driver(void);

extern int disp_sys_script_get_item(char *main_name, char *sub_name, int value[], int type);

struct disp_hdmi_mode
{
	enum disp_tv_mode mode;
	int hdmi_mode;//vic
};

#endif
