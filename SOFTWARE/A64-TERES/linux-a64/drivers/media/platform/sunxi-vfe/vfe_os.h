/*
 * sunxi operation system resource
 * Author:raymonxiu
 */
#ifndef __VFE__OS__H__
#define __VFE__OS__H__

#include <linux/device.h>
#include <linux/clk.h>
#include <linux/clk/sunxi.h>
#include <linux/interrupt.h>
//#include <linux/gpio.h>
#include "platform_cfg.h"

#ifdef SUNXI_MEM
#include <linux/ion.h>          //for all "ion api"
#include <linux/ion_sunxi.h>    //for import global variable "sunxi_ion_client_create"
#include <linux/dma-mapping.h>  //just include"PAGE_SIZE" macro
#else
#include <linux/dma-mapping.h>
#endif

extern unsigned int vfe_dbg_en;
extern unsigned int vfe_dbg_lv;

#define VFE_NOT_ADDR      -1

//for internel driver debug
#define vfe_dbg(l,x,arg...) if(vfe_dbg_en && l <= vfe_dbg_lv) printk(KERN_DEBUG"[VFE_DEBUG]"x,##arg)
//print when error happens
#define vfe_err(x,arg...) printk(KERN_ERR"[VFE_ERR]"x,##arg)
#define vfe_warn(x,arg...) printk(KERN_WARNING"[VFE_WARN]"x,##arg)
//print unconditional, for important info
#define vfe_print(x,arg...) printk(KERN_NOTICE"[VFE]"x,##arg)

struct vfe_mm {    
	size_t size;
	void* phy_addr;
	void* vir_addr;
	void* dma_addr;
    struct ion_client *client;
    struct ion_handle *handle;
};

struct vfe_gpio_cfg {
	u32 gpio;
	u32 mul_sel;
	u32 pull;
	u32 drv_level;
	u32 data;
};

struct clk *os_clk_get(struct device_node *np, int index);
extern void  os_clk_put(struct clk *clk);
extern int os_clk_set_parent(struct clk *clk, struct clk *parent);
extern int os_clk_set_rate(struct clk *clk, unsigned long rate);
extern int os_clk_enable(struct clk *clk);
extern int os_clk_prepare_enable(struct clk *clk);
extern void os_clk_disable(struct clk *clk);
extern void os_clk_disable_unprepare(struct clk *clk);
extern int os_clk_reset_assert(struct clk *clk);
extern int os_clk_reset_deassert(struct clk *clk); 
extern int os_gpio_request(struct vfe_gpio_cfg *gpio_list, __u32 group_count_max);
extern int os_gpio_set(struct vfe_gpio_cfg *gpio_list, __u32 group_count_max);

//extern __hdle os_gpio_request_ex(char *main_name, const char *sub_name);
extern int os_gpio_release(u32 p_handler, __s32 if_release_to_default_status); 
extern int os_gpio_write(u32 p_handler, __u32 value_to_gpio, const char *gpio_name, int force_value_flag);
extern int os_gpio_set_status(u32 p_handler, __u32 if_set_to_output_status, const char *gpio_name);
extern int os_mem_alloc(struct vfe_mm *mem_man);
extern void os_mem_free(struct vfe_mm *mem_man);

#endif //__VFE__OS__H__
