#ifndef __DE_TVE_I_H__
#define __DE_TVE_I_H__


#ifdef __LINUX_OSAL__
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
#include <linux/switch.h>
#include <mach/platform.h>
#include <video/sunxi_display2.h>
#endif

typedef unsigned char      	u8;
typedef signed char        	s8;
typedef unsigned short     	u16;
typedef signed short       	s16;
typedef unsigned int       	u32;
typedef signed int         	s32;
typedef unsigned long long 	u64;
/*tv encoder registers offset*/
#define TVE_000    (0x000)
#define TVE_004    (0x004)
#define TVE_008    (0x008)
#define TVE_00C    (0x00c)
#define TVE_010    (0x010)
#define TVE_014    (0x014)
#define TVE_018    (0x018)
#define TVE_01C    (0x01c)
#define TVE_020    (0x020)
#define TVE_024    (0x024)
#define TVE_030    (0X030)
#define TVE_034    (0x034)
#define TVE_038    (0x038)
#define TVE_03C    (0x03c)
#define TVE_040    (0x040)
#define TVE_044    (0x044)
#define TVE_048    (0x048)
#define TVE_04C    (0x04c)
#define TVE_0F8    (0x0f8)
#define TVE_0FC    (0x0fc)
#define TVE_100    (0x100)
#define TVE_104    (0x104)
#define TVE_108    (0x108)
#define TVE_10C    (0x10c)
#define TVE_110    (0x110)
#define TVE_114    (0x114)
#define TVE_118    (0x118)
#define TVE_11C    (0x11c)
#define TVE_120    (0x120)
#define TVE_124    (0x124)
#define TVE_128    (0x128)
#define TVE_12C    (0x12c)
#define TVE_130    (0x130)
#define TVE_134    (0x134)
#define TVE_138    (0x138)
#define TVE_13C    (0x13C)
#define TVE_304    (0x304)



#define TVE_GET_REG_BASE(sel)					((sel)==0?(tve_reg_base0):(tve_reg_base1))

#define TVE_WUINT32(sel,offset,value)			(*((volatile u32 *)( TVE_GET_REG_BASE(sel) + (offset) ))=(value))
#define TVE_RUINT32(sel,offset)					(*((volatile u32 *)( TVE_GET_REG_BASE(sel) + (offset) )))

#define TVE_SET_BIT(sel,offset,bit)				(*((volatile u32 *)( TVE_GET_REG_BASE(sel) + (offset) )) |= (bit))
#define TVE_CLR_BIT(sel,offset,bit)				(*((volatile u32 *)( TVE_GET_REG_BASE(sel) + (offset) )) &= (~(bit)))
#define TVE_INIT_BIT(sel,offset,c,s)			(*((volatile u32 *)( TVE_GET_REG_BASE(sel) + (offset) )) = \
												(((*(volatile u32 *)( TVE_GET_REG_BASE(sel) + (offset) )) & (~(c))) | (s)))


s32 tv_low_detect_enable(void);
s32 tv_low_detect_disable(void);
s32 tv_low_get_video_info(s32 mode);
s32 tve_low_set_reg_base(u32 sel,u32 address);
s32 tve_low_open(u32 sel);
s32 tve_low_exit(u32 sel);
s32 tv_low_get_list_num(void);
void tve_low_dac_cfg(u32 sel, u8 mode);
s32 tve_low_set_tv_mode(u32 sel, u8 mode);
s32  tve_low_init(u32 sel,u32 cali);
u8 tve_low_dac_disable(u32 sel,u8 index);
s32 tve_low_close(u32 sel);
void tv_low_print_base_reg(void);
int tv_low_detect_thread(void *parg);
s32 tv_low_get_dac_hpd(u32 sel);
s32 tve_low_close(u32 sel);
void tve_low_dac_cfg(u32 sel, u8 mode);
s32 tve_low_set_vga_mode(u32 sel);
u8 tve_low_query_int(u32 sel);
u8  tve_low_clear_int(u32 sel);
s32 tve_low_get_dac_status(u32 index);
u8 tve_low_dac_int_enable(u32 sel,u8 index);
u8 tve_low_dac_int_disable(u32 sel,u8 index);
u8 tve_low_dac_autocheck_enable(u32 sel,u8 index);
u8 tve_low_dac_autocheck_disable(u32 sel,u8 index);
u8 tve_low_dac_enable(u32 sel,u8 index);
u8 tve_low_dac_disable(u32 sel,u8 index);
s32 tve_low_dac_set_source(u32 sel,u32 index,u32 source);
s32 tve_low_dac_get_source(u32 sel,u32 index);
u8 tve_low_dac_set_de_bounce(u32 sel,u8 index,u32 times);
u8 tve_low_dac_get_de_bounce(u32 sel,u8 index);
s32 tve_low_dac_sel(u32 sel,u32 dac, u32 index);
u8 tve_low_csc_init(u32 sel,u8 type);
u8 tve_low_csc_enable(u32 sel);
u8 tve_low_csc_disable(u32 sel);
u32 TVE_filter_mode(u32 sel,u32 vector,u32 lti);
s32 TVE_resync_enable(u32 sel);
s32 TVE_resync_disable(u32 sel);


#endif
