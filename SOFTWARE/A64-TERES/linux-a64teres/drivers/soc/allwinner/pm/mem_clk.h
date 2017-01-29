/*
*********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2011-2015, gq.yang China
*                                             All Rights Reserved
*
* File    : mem_clk.h
* By      : gq.yang
* Version : v1.0
* Date    : 2012-11-31 15:23
* Descript:
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/
#ifndef __MEM_CLK_H__
#define __MEM_CLK_H__

#include "pm.h"

#if defined(CONFIG_ARCH_SUN8I) || defined(CONFIG_ARCH_SUN50IW1P1)
struct clk_div_t {
	__u32	axi_div;      	/* division of axi clock, divide cpu clock*/
	__u32	ahb_apb_div;	/* ahb1/apb1 clock divide ratio */
};

struct clk_misc_t {
	__u32	pll1_bias;		//0x220
	__u32	pll6_bias;		//0x234, not need restore.
	__u32	pll1_tun;		//0x250
#if defined(CONFIG_ARCH_SUN8IW5P1) || defined(CONFIG_ARCH_SUN50IW1P1)
	__u32   Pll3Ctl;		//0x10, vedio
	__u32	Pll4Ctl;		//0x18, ve
	__u32	PllVedioBias;           //0x228,  pll vedio bias reg
	__u32	PllVeBias;              //0x22c,  pll ve    bias reg
	__u32	PllVedioPattern;        //0x288,  pll vedio pattern reg
	__u32	PllVePattern;           //0x28c,  pll ve    pattern reg	
#endif

#ifdef CONFIG_ARCH_SUN8IW6P1
	__u32   Pll_C1_Bias;            //0x0238
	__u32   PllC1Tun;             //0x0254
	__u32   PllC1Ctl;    	        //0x0004
	__u32   Pll3Ctl;		//0x10, video
	__u32	Pll4Ctl;		//0x18, ve
	__u32	Pll6Ctl;		//0x28, periph
	__u32	PllVideo0Bias;           //0x228,  pll video0 bias reg
	__u32	PllVeBias;              //0x22c,  pll ve    bias reg
	__u32	PllPeriphBias;              //0x234,  pll periph  bias reg
	__u32	PllVideo0Reg0Pattern;        //0x288,  pll video0 pattern reg
	__u32	PllVideo0Reg1Pattern;        //0x288,  pll video0 pattern reg
	__u32   Apb2Div;		    //0x58, apb2 clk divide ratio
#endif

};

struct pll_factor_t {
    __u8    FactorN;
    __u8    FactorK;
    __u8    FactorM;
    __u8    FactorP;
    __u32   Pll;
};

struct clk_state{
	__ccmu_reg_list_t   *CmuReg;
	__u32    ccu_reg_back[15];
};

#elif defined(CONFIG_ARCH_SUN9IW1P1) 
struct clk_div_t {
	__u32	Axi0_Cfg;             
	__u32	Axi1_Cfg;             
};

struct clk_misc_t {
	__u32	Pll_C0_Bias     ;
	__u32	Pll_Periph1_Bias;
	__u32	Pll_Periph2_Bias;
	__u32	Pll_C0_Tun      ;
	__u32   Pll_C1_Bias;            //0x00a4
	__u32   Pll_Vedio1_Bias;        //0x00b8
	__u32   Pll_Vedio2_Bias;        //0x00bc
	__u32   Pll_C1_Tun;             //0x00e4
	__u32   Pll_Video1_Pat_Cfg;     //0x0118
	__u32   Pll_Video2_Pat_Cfg;     //0x011c
	__u32   Pll_C0_Cfg;    	        //0x0000
	__u32   Pll_C1_Cfg;    	        //0x0004
	__u32   Pll_Video1_Cfg;    	//0x0018
	__u32   Pll_Video2_Cfg;    	//0x001c
};              

struct pll_factor_t {
    __u8    FactorN;
    __u8    FactorK;
    __u8    FactorM;
    __u8    FactorP;
    __u32   Pll;
};

struct clk_state{
	__ccmu_reg_list_t		*ccm_reg;
	__ccmu_reg_list_t		ccm_reg_backup;
	__ccmu_mod_reg_list_t		*ccm_mod_reg;
	__ccmu_mod_reg_list_t		ccm_mod_reg_backup;
	
};
#endif

__s32 mem_clk_save(struct clk_state *pclk_state);
__s32 mem_clk_restore(struct clk_state *pclk_state);
__ccmu_reg_list_t * mem_clk_init(__u32 mmu_flag);
__ccmu_reg_list_t * mem_get_ba(void);
__s32 mem_clk_setdiv(struct clk_div_t *clk_div);
__s32 mem_clk_getdiv(struct clk_div_t  *clk_div);
__s32 mem_clk_set_pll_factor(struct pll_factor_t *pll_factor);
__s32 mem_clk_get_pll_factor(struct pll_factor_t *pll_factor);

__s32 mem_clk_get_misc(struct clk_misc_t *clk_misc);
__s32 mem_clk_set_misc(struct clk_misc_t *clk_misc);

#ifdef CONFIG_ARCH_SUN8IW8P1
void mem_pio_clk_src_init(void);
void mem_pio_clk_src_exit(void);
#else
static inline void mem_pio_clk_src_init(void) {return; }
static inline void mem_pio_clk_src_exit(void) {return; }
#endif

__u32 mem_clk_get_cpu_freq(void);
#endif  //__MEM_CLK_H__


