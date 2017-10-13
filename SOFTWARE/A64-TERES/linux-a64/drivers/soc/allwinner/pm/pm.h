#ifndef _PM_H
#define _PM_H

/*
 * Copyright (c) 2011-2015 yanggq.young@allwinnertech.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

#include "pm_types.h"
#include "pm_config.h"
#include "pm_errcode.h"
#include "pm_debug.h"
#include "pm_of.h"
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/of_address.h>
#include "mem_cpu.h"
#include "mem_ccmu.h"
#include "mem_serial.h"
#include "mem_key.h"
#include "mem_ir.h"
#include "mem_usb.h"
#include "mem_printk.h"
#include "mem_divlibc.h"
#include "mem_int.h"
#include "mem_gpio.h"
#include "mem_tmr.h"
#include "mem_tmstmp.h"
#include "mem_clk.h"
#include "mem_timing.h"
#include "mem_mapping.h"
#include "mem_hwspinlock.h"
#ifdef CONFIG_ARCH_SUN9IW1P1
#include "mem_cci400.h"
#include "mem_gtbus.h"
#endif

#include "linux/power/aw_pm.h"

extern __u32 debug_mask;
typedef enum{
	PM_STANDBY_PRINT_STANDBY 		= (1U << 0),
	PM_STANDBY_PRINT_RESUME 		= (1U << 1),
	PM_STANDBY_ENABLE_JTAG			= (1U << 2),
	PM_STANDBY_PRINT_PORT		 	= (1U << 3),
	PM_STANDBY_PRINT_IO_STATUS 		= (1U << 4),
	PM_STANDBY_PRINT_CACHE_TLB_MISS 	= (1U << 5),
	PM_STANDBY_PRINT_CCU_STATUS 		= (1U << 6),
	PM_STANDBY_PRINT_PWR_STATUS 		= (1U << 7),
	PM_STANDBY_PRINT_CPUS_IO_STATUS 	= (1U << 8),
	PM_STANDBY_PRINT_CCI400_REG		= (1U << 9),
	PM_STANDBY_PRINT_GTBUS_REG		= (1U << 10),
	PM_STANDBY_TEST				= (1U << 11),
	PM_STANDBY_PRINT_RESUME_IO_STATUS 	= (1U << 12)
}debug_mask_flag;

extern unsigned int parse_bitmap_en;
typedef enum{
	DEBUG_WAKEUP_SRC		=   (0x01<<0),
	DEBUG_WAKEUP_GPIO_MAP	    	=   (0x01<<1),
	DEBUG_WAKEUP_GPIO_GROUP_MAP 	=   (0x01<<2),
	DEBUG_PWR_DM_MAP	    	=   (0x01<<3)

}parse_bitmap_en_flag;

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

#define uk_printf(s, size, fmt, args...) do { \
	if(NULL != s){			\
	    s += scnprintf(s, size, fmt, ## args);	\
	}else{				\
	    printk(fmt, ## args);		\
	}					\
    }while(0)

struct mmu_state {
	/* CR0 */
	__u32 cssr;	/* Cache Size Selection */
	/* CR1 */
	__u32 cr;		/* Control */
	__u32 cacr;	/* Coprocessor Access Control */
	/* CR2 */
	__u32  ttb_0r;	/* Translation Table Base 0 */
	__u32  ttb_1r;	/* Translation Table Base 1 */
	__u32  ttbcr;	/* Translation Talbe Base Control */
	
	/* CR3 */
	__u32 dacr;	/* Domain Access Control */

	/*cr10*/
	__u32 prrr;	/* Primary Region Remap Register */
	__u32 nrrr;	/* Normal Memory Remap Register */
};

/**
*@brief struct of super mem
*/
struct aw_mem_para{
	void **resume_pointer;
	volatile __u32 mem_flag;
	__u32 monitor_vector;
	__u32 axp_event;
	__u32 sys_event;
	__u32 debug_mask;
	__u32 suspend_delay_ms;
	__u32 saved_runtime_context_svc[RUNTIME_CONTEXT_SIZE];
	struct clk_div_t clk_div;
	struct clk_misc_t clk_misc;	//miscellaneous para.
	struct pll_factor_t pll_factor;
	struct mmu_state saved_mmu_state;
};

typedef  int (*suspend_func)(void);

/*mem_mmu_pc_asm.S*/
extern unsigned int save_sp_nommu(void);
extern unsigned int save_sp(void);
extern void clear_reg_context(void);
extern void restore_sp(unsigned int sp);

//cache
extern void invalidate_dcache(void);
extern void flush_icache(void);
extern void flush_dcache(void);
extern void disable_cache(void);
extern void disable_dcache(void);
extern void disable_l2cache(void);
extern void enable_cache(void);
extern void enable_icache(void);

extern void disable_program_flow_prediction(void);
extern void invalidate_branch_predictor(void);
extern void enable_program_flow_prediction(void);

extern void mem_flush_tlb(void);
extern void mem_preload_tlb(void);

void disable_mmu(void);
void enable_mmu(void);

extern int jump_to_resume(void* pointer, __u32 *addr);
extern int jump_to_resume0(void* pointer);
void jump_to_suspend(__u32 ttbr1, suspend_func p);
extern int jump_to_resume0_nommu(void* pointer);

/*mmu_pc.c*/
extern void save_mmu_state(struct mmu_state *saved_mmu_state);
extern void restore_mmu_state(struct mmu_state *saved_mmu_state);
void set_ttbr0(void);
extern void invalidate_dcache(void);

typedef struct __MEM_TWIC_REG
{
    volatile __u32 reg_saddr;
    volatile __u32 reg_xsaddr;
    volatile __u32 reg_data;
    volatile __u32 reg_ctl;
    volatile __u32 reg_status;
    volatile __u32 reg_clkr;
    volatile __u32 reg_reset;
    volatile __u32 reg_efr;
    volatile __u32 reg_lctl;

} __mem_twic_reg_t;

struct gic_distributor_state{
	//distributor
	volatile __u32 GICD_CTLR;					//offset 0x00, distributor Contrl reg
	volatile __u32 GICD_IGROUPRn;					//offset 0x80, Interrupt Grp Reg ?           
	volatile __u32 GICD_ISENABLERn[0x180/4];			//offset 0x100-0x17c, Interrupt Set-Enable Reg       
	volatile __u32 GICD_ICENABLERn[0x180/4];			//offset 0x180-0x1fc, Iterrupt Clear-Enable Reg      
	volatile __u32 GICD_ISPENDRn[0x180/4];				//offset 0x200-0x27c, Iterrupt Set-Pending Reg       
	volatile __u32 GICD_ICPENDRn[0x180/4];				//offset 0x280-0x2fc, Iterrupt Clear-Pending Reg     
	volatile __u32 GICD_ISACTIVERn[0x180/4]; 			//offset 0x300-0x37c, GICv2 Iterrupt Set-Active Reg        
	volatile __u32 GICD_ICACTIVERn[0x180/4]; 			//offset 0x380-0x3fc, Iterrupt Clear-Active Reg        
	volatile __u32 GICD_IPRIORITYRn[0x180/4]; 			//offset 0x400-0x7F8, Iterrupt Priority Reg          
	volatile __u32 GICD_ITARGETSRn[(0x400-0x20)/4];			//offset 0x820-0xbf8, Iterrupt Processor Targets Reg         
	volatile __u32 GICD_ICFGRn[0x100/4];				//offset 0xc00-0xcfc, Iterrupt Config Reg            
	volatile __u32 GICD_NSACRn[0x100/4];				//offset 0xE00-0xEfc, non-secure Access Ctrl Reg ?   
	volatile __u32 GICD_CPENDSGIRn[0x10/4];				//offset 0xf10-0xf1c, SGI Clear-Pending Reg          
	volatile __u32 GICD_SPENDSGIRn[0x10/4];				//offset 0xf20-0xf2c, SGI Set-Pending Reg       	
};

struct gic_cpu_interface_state{
	//cpu interface reg
	volatile __u32 GICC_CTLR_PMR_BPR[0xc/4];			//offset 0x00-0x08, cpu interface Ctrl Reg	+ 	Interrupt Priority Mask Reg	 + 	  Binary Point Reg	
	volatile __u32 GICC_ABPR;				//offset 0x1c, Aliased Binary Point Reg	
	volatile __u32 GICC_APRn[0x10/4];			//offset 0xd0-0xdc, Active Priorities Reg 		  
	volatile __u32 GICC_NSAPRn[0x10/4];			//offset 0xe0-0xec, Non-secure Active Priorities Reg
};

struct gic_distributor_disc{
	//distributor
	volatile __u32 GICD_CTLR;					//offset 0x00, distributor Contrl reg
	volatile __u32 reserved0[0x7c/4];				//0ffset 0x04-0x7c
	volatile __u32 GICD_IGROUPRn;					//offset 0x80, Interrupt Grp Reg ?           
	volatile __u32 reserved1[0x7c/4];				//0ffset 0x84-0xfc
	volatile __u32 GICD_ISENABLERn[0x180/4];			//offset 0x100-0x17c, Interrupt Set-Enable Reg       
	volatile __u32 GICD_ICENABLERn[0x180/4];			//offset 0x180-0x1fc, Iterrupt Clear-Enable Reg      
	volatile __u32 GICD_ISPENDRn[0x180/4];				//offset 0x200-0x27c, Iterrupt Set-Pending Reg       
	volatile __u32 GICD_ICPENDRn[0x180/4];				//offset 0x280-0x2fc, Iterrupt Clear-Pending Reg     
	volatile __u32 GICD_ISACTIVERn[0x180/4]; 			//offset 0x300-0x37c, GICv2 Iterrupt Set-Active Reg        
	volatile __u32 GICD_ICACTIVERn[0x180/4]; 			//offset 0x380-0x3fc, Iterrupt Clear-Active Reg        
	volatile __u32 GICD_IPRIORITYRn[0x180/4]; 			//offset 0x400-0x7F8, Iterrupt Priority Reg          
	volatile __u32 reserved2[0x24/4];				//0ffset 0x7fc-0x81c
	volatile __u32 GICD_ITARGETSRn[(0x400-0x20)/4];			//offset 0x820-0xbf8, Iterrupt Processor Targets Reg         
	volatile __u32 reserved3;					//0ffset 0xbfc
	volatile __u32 GICD_ICFGRn[0x100/4];				//offset 0xc00-0xcfc, Iterrupt Config Reg            
	volatile __u32 reserved4[0x100/4];				//0ffset 0xd00-0xdfc
	volatile __u32 GICD_NSACRn[0x100/4];				//offset 0xE00-0xEfc, non-secure Access Ctrl Reg ?   
	volatile __u32 reserved5[0x10/4];				//0ffset 0xf00-0xf0c
	volatile __u32 GICD_CPENDSGIRn[0x10/4];				//offset 0xf10-0xf1c, SGI Clear-Pending Reg          
	volatile __u32 GICD_SPENDSGIRn[0x10/4];				//offset 0xf20-0xf2c, SGI Set-Pending Reg       
	volatile __u32 reserved6[0xd0/4];				//0ffset 0xf30-0xffc         
	
};

struct gic_cpu_interface_disc{
	//cpu interface reg
	volatile __u32 GICC_CTLR_PMR_BPR[0xc/4];			//offset 0x00-0x08, cpu interface Ctrl Reg	+ 	Interrupt Priority Mask Reg	 + 	  Binary Point Reg	
	volatile __u32 reserved7[0x10/4];			//0ffset 0xC-0x18, readonly or writeonly      
	volatile __u32 GICC_ABPR;				//offset 0x1c, Aliased Binary Point Reg	
	volatile __u32 reserved8[0xb0/4];			//0ffset 0x20-0xcf, readonly or writeonly      
	volatile __u32 GICC_APRn[0x10/4];			//offset 0xd0-0xdc, Active Priorities Reg 		  
	volatile __u32 GICC_NSAPRn[0x10/4];			//offset 0xe0-0xec, Non-secure Active Priorities Reg
	volatile __u32 reserved9[0x10/4];			//0ffset 0xf0-0xfc, readonly or writeonly  
	volatile __u32 reserved10[0xf00/4];			//0ffset 0x100-0xffc, readonly or writeonly  
	volatile __u32 reserved11;				//0ffset 0x1000, readonly or writeonly   

};

struct gic_state{
	struct gic_distributor_state m_distributor;
	struct gic_cpu_interface_state m_interface;
};

struct twi_state{
	__mem_twic_reg_t *twi_reg;
	__u32 twi_reg_backup[7];
};

struct sram_state{
	__u32 sram_reg_back[SRAM_REG_LENGTH];
};

//save module state
__s32 mem_twi_save(struct twi_state *ptwi_state);
__s32 mem_twi_restore(struct twi_state *ptwi_state);
__s32 mem_sram_init(void);
__s32 mem_sram_save(struct sram_state *psram_state);
__s32 mem_sram_restore(struct sram_state *psram_state);
__s32 mem_ccu_save(struct ccm_state *ccm_reg);
__s32 mem_ccu_restore(struct ccm_state *ccm_reg);

extern struct aw_mem_para mem_para_info;
extern struct super_standby_para super_standby_para_info;

#if defined(CONFIG_ARCH_SUN8IW8P1)

#else
#ifdef CONFIG_SUNXI_ARISC
#else
//#warning "ARISC driver is not enabled!!!!!!!!!"
#endif
#endif

#endif /*_PM_H*/

