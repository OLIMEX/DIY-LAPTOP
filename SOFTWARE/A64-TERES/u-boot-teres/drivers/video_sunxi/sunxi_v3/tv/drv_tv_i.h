
#ifndef  _DRV_TV_I_H_
#define  _DRV_TV_I_H_

#if 0
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
#include <mach/sys_config.h>
//#include <linux/switch.h>
#endif

#include <asm/arch/sunxi_display2.h>
#include "de_tvec_i.h"
#include "../disp_sys_intf.h"
#include "../dev_disp.h"

typedef unsigned char      	u8;
typedef signed char        	s8;
typedef unsigned short     	u16;
typedef signed short       	s16;
typedef unsigned int       	u32;
typedef signed int         	s32;
typedef unsigned long long 	u64;

typedef struct
{
	disp_tv_dac_source   	dac_source[4];
	disp_tv_mode         	tv_mode;
	u32                   		base_address;
}tv_screen_t;


typedef struct
{
    	u32                  		enable;
	u32 					dac_count;
	tv_screen_t			screen[2];
}tv_info_t;

extern tv_info_t g_tv_info;

s32 tv_init(void);

s32 tv_exit(void);

s32 tv_get_mode(u32 sel);

s32 tv_set_mode(u32 sel, disp_tv_mode tv_mod);

s32 tv_get_input_csc(void);

s32 tv_get_video_timing_info(u32 sel, disp_video_timings **video_info);

s32 tv_enable(u32 sel);

s32 tv_disable(u32 sel);

s32 tv_suspend(void);

s32 tv_resume(void);

s32 tv_mode_support(disp_tv_mode mode);

s32 tv_get_dac_hpd(u32 sel);

#endif
