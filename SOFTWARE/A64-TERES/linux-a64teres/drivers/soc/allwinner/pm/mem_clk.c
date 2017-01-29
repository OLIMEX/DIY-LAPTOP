#include "pm_i.h"

//#define CHECK_RESTORE_STATUS
static __ccmu_reg_list_t   *CmuReg;


/*
*********************************************************************************************************
*                           mem_clk_init
*
*Description: ccu init for platform mem, after switch mmu state, u need to re init.
*
*Arguments  : none
*
*Return     : result,
*
*Notes      :
*
*********************************************************************************************************
*/
__ccmu_reg_list_t * mem_clk_init(__u32 mmu_flag )
{
	if(1 == mmu_flag){
		CmuReg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);
	}else{
		CmuReg = (__ccmu_reg_list_t *)(AW_CCM_BASE);
	}

	return CmuReg;
}

/*
*********************************************************************************************************
*                           mem_get_ba
*
*Description: get ccu mod base.
*
*Arguments  : none
*
*Return     : result,
*
*Notes      :
*
*********************************************************************************************************
*/
__ccmu_reg_list_t * mem_get_ba(void)
{

	return CmuReg;
}

#if defined(CONFIG_ARCH_SUN8I) || defined(CONFIG_ARCH_SUN50IW1P1)
static __ccmu_pll1_reg0000_t		CmuReg_Pll1Ctl_tmp;
static __ccmu_sysclk_ratio_reg0050_t	CmuReg_SysClkDiv_tmp;

#ifdef CONFIG_ARCH_SUN8IW8P1
#define PIO_INT_DEB_REG (AW_GPIO_BASE_PA + 0x258)
static __u32 pio_int_deb_back = 0;
void mem_pio_clk_src_init(void)
{
    pio_int_deb_back = *(__u32*)PIO_INT_DEB_REG;
    *(__u32*)PIO_INT_DEB_REG = pio_int_deb_back & (~1);
}

void mem_pio_clk_src_exit(void)
{
    *(__u32*)PIO_INT_DEB_REG = pio_int_deb_back;
}
#endif

/*
*********************************************************************************************************
*                           mem_clk_save
*
*Description: save ccu for platform mem
*
*Arguments  : none
*
*Return     : result,
*
*Notes      :
*
*********************************************************************************************************
*/
__s32 mem_clk_save(struct clk_state *pclk_state)
{
	pclk_state->CmuReg = CmuReg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);

	/*backup clk src and ldo*/
	pclk_state->ccu_reg_back[0] = *(volatile __u32 *)&CmuReg->SysClkDiv;
	pclk_state->ccu_reg_back[1] = *(volatile __u32 *)&CmuReg->Apb2Div;
	pclk_state->ccu_reg_back[2] = *(volatile __u32 *)&CmuReg->Ahb1Div;
	
	return 0;
}


/*
*********************************************************************************************************
*                           mem_clk_exit
*
*Description: restore ccu for platform mem
*
*Arguments  : none
*
*Return     : result,
*
*Notes      :
*
*********************************************************************************************************
*/
__s32 mem_clk_restore(struct clk_state *pclk_state)
{
	/* initialise the CCU io base */
	CmuReg = pclk_state->CmuReg;

	/* 
	* consider: pll6 already configed.
	* config the bus to orginal status 
	*/
	
	/*restore clk src*/
	*(volatile __u32 *)&CmuReg->SysClkDiv   = pclk_state->ccu_reg_back[0];  
	*(volatile __u32 *)&CmuReg->Apb2Div     = pclk_state->ccu_reg_back[1];  
	*(volatile __u32 *)&CmuReg->Ahb1Div     = pclk_state->ccu_reg_back[2];

	return 0;
}


/*
*********************************************************************************************************
*                                     mem_clk_setdiv
*
* Description: set div ratio
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/
__s32 mem_clk_setdiv(struct clk_div_t *clk_div)
{
	if(!clk_div){
		return -1;
	}

	CmuReg = (__ccmu_reg_list_t *)(AW_CCM_BASE);
	
	//1st: set axi ratio
	CmuReg_SysClkDiv_tmp.dwval = CmuReg->SysClkDiv.dwval;
	CmuReg_SysClkDiv_tmp.bits.AXIClkDiv = clk_div->axi_div;
	CmuReg->SysClkDiv.dwval = CmuReg_SysClkDiv_tmp.dwval;
	
	//set ahb1/apb1 clock divide ratio
	//first, config ratio; 
	*(volatile __u32 *)(&CmuReg->Ahb1Div) = (((clk_div->ahb_apb_div)&(~0x3000)) | (0x1000));
#ifdef CONFIG_ARCH_SUN8IW10P1
	change_runtime_env();
	delay_us(5);
#else
	udelay(5);
#endif
	//sec, config src.
	*(volatile __u32 *)(&CmuReg->Ahb1Div) = (clk_div->ahb_apb_div);
#ifdef	CONFIG_ARCH_SUN8IW10P1
	delay_us(5);
#else
	udelay(5);
#endif
	//notice: pll6 is enabled by cpus.
	//the relationship between pll6&mbus&dram?

	return 0;
}

/*
*********************************************************************************************************
*                                     mem_clk_getdiv
*
* Description: 
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/
__s32 mem_clk_getdiv(struct clk_div_t  *clk_div)
{
	if(!clk_div){
		return -1;
	}
	CmuReg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);
	CmuReg_SysClkDiv_tmp.dwval = CmuReg->SysClkDiv.dwval;
	clk_div->axi_div = CmuReg_SysClkDiv_tmp.bits.AXIClkDiv;
	clk_div->ahb_apb_div = *(volatile __u32 *)(&CmuReg->Ahb1Div);
	
	return 0;
}


/*
*********************************************************************************************************
*                                     mem_clk_set_pll_factor
*
* Description: set pll factor, target cpu freq is ?M hz
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/

__s32 mem_clk_set_pll_factor(struct pll_factor_t *pll_factor)
{
	CmuReg = (__ccmu_reg_list_t *)(AW_CCM_BASE);
	CmuReg_Pll1Ctl_tmp.dwval = CmuReg->Pll1Ctl.dwval;
	//set pll factor: notice: when raise freq, N must be the last to set
#if defined(CONFIG_ARCH_SUN8IW3P1) || defined(CONFIG_ARCH_SUN8IW5P1) || defined(CONFIG_ARCH_SUN8IW6P1)
	CmuReg_Pll1Ctl_tmp.bits.FactorP = pll_factor->FactorP;
#endif
	CmuReg_Pll1Ctl_tmp.bits.FactorM = pll_factor->FactorM;
#ifndef CONFIG_ARCH_SUN8IW6P1
	CmuReg_Pll1Ctl_tmp.bits.FactorK = pll_factor->FactorK;
#endif
	CmuReg_Pll1Ctl_tmp.bits.FactorN = pll_factor->FactorN;
	CmuReg->Pll1Ctl.dwval = CmuReg_Pll1Ctl_tmp.dwval;
	//need delay?
	//busy_waiting();
	
	return 0;
}

/*
*********************************************************************************************************
*                                     mem_clk_get_pll_factor
*
* Description: get pll factor
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/

__s32 mem_clk_get_pll_factor(struct pll_factor_t *pll_factor)
{
	CmuReg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);
	CmuReg_Pll1Ctl_tmp.dwval = CmuReg->Pll1Ctl.dwval;
	pll_factor->FactorN = CmuReg_Pll1Ctl_tmp.bits.FactorN;
#ifndef CONFIG_ARCH_SUN8IW6P1
	pll_factor->FactorK = CmuReg_Pll1Ctl_tmp.bits.FactorK;
#endif

	pll_factor->FactorM = CmuReg_Pll1Ctl_tmp.bits.FactorM;
#if defined(CONFIG_ARCH_SUN8IW3P1) || defined(CONFIG_ARCH_SUN8IW5P1) || defined(CONFIG_ARCH_SUN8IW6P1)
	pll_factor->FactorP = CmuReg_Pll1Ctl_tmp.bits.FactorP;
#endif
	//busy_waiting();
	
	return 0;
}

/*
*********************************************************************************************************
*                                     mem_clk_set_misc
*
* Description: set clk_misc
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/
__s32 mem_clk_set_misc(struct clk_misc_t *clk_misc)
{
	CmuReg = (__ccmu_reg_list_t *)(AW_CCM_BASE);
	
	CmuReg->PllxBias[0]	= clk_misc->pll1_bias;
	
	//pll_periph's bias do not need config?
	CmuReg->Pll1Tun		= clk_misc->pll1_tun;

#ifdef CONFIG_ARCH_SUN8IW5P1	
	CmuReg->PllVedioBias		= clk_misc->PllVedioBias			;           //0x228,  pll vedio bias reg
	CmuReg->PllVeBias		= clk_misc->PllVeBias				;           //0x22c,  pll ve    bias reg
	CmuReg->PllVedioPattern		= clk_misc->PllVedioPattern			;           //0x288,  pll vedio pattern reg
	CmuReg->PllVePattern		= clk_misc->PllVePattern			;           //0x28c,  pll ve    pattern reg	
	CmuReg->Pll3Ctl			= clk_misc->Pll3Ctl				;	    //0x10, vedio
	CmuReg->Pll4Ctl			= clk_misc->Pll4Ctl				;	    //0x18, ve
#endif

#ifdef CONFIG_ARCH_SUN8IW6P1
	CmuReg->PllC1CpuxBias	=	clk_misc->Pll_C1_Bias;	 
	CmuReg->PllC1Tun	=	clk_misc->PllC1Tun;	
	CmuReg->PllC1Ctl	=	clk_misc->PllC1Ctl;	
	CmuReg->PllVideo0Bias		= clk_misc->PllVideo0Bias			;           //0x228,  pll vedio bias reg
	CmuReg->PllVeBias		= clk_misc->PllVeBias				;           //0x22c,  pll ve    bias reg
	CmuReg->PllPeriphBias		= clk_misc->PllPeriphBias			;	    //0x234, pll periph bias
	CmuReg->PllVideo0Reg0Pattern		= clk_misc->PllVideo0Reg0Pattern			;           //0x288,  pll vedio pattern reg
	CmuReg->PllVideo0Reg1Pattern		= clk_misc->PllVideo0Reg1Pattern			;           //0x288,  pll vedio pattern reg
	CmuReg->Pll3Ctl			= clk_misc->Pll3Ctl				;	    //0x10, vedio
	CmuReg->Pll4Ctl			= clk_misc->Pll4Ctl				;	    //0x18, ve
	CmuReg->Pll6Ctl			= clk_misc->Pll6Ctl				;	    //0x28, pll periph ctrl
	*(volatile __u32 *)(&CmuReg->Apb2Div)= clk_misc->Apb2Div			;	    //0x58, apb2 divide ratio
#endif	

	//config axi ratio to 1+1 = 2;
	//axi can not exceed 300M;
	CmuReg_SysClkDiv_tmp.dwval = CmuReg->SysClkDiv.dwval;
	CmuReg_SysClkDiv_tmp.bits.AXIClkDiv = 2;
	CmuReg->SysClkDiv.dwval = CmuReg_SysClkDiv_tmp.dwval;
	
	CmuReg_SysClkDiv_tmp.bits.AXIClkDiv = 1;
	CmuReg->SysClkDiv.dwval = CmuReg_SysClkDiv_tmp.dwval;
	return 0;
}

/*
*********************************************************************************************************
*                                     mem_clk_get_misc
*
* Description: get clk_misc
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/

__s32 mem_clk_get_misc(struct clk_misc_t *clk_misc)
{
	CmuReg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);
	clk_misc->pll1_bias	=	CmuReg->PllxBias[0];	 
	clk_misc->pll1_tun	=	CmuReg->Pll1Tun;	

#ifdef CONFIG_ARCH_SUN8IW6P1
	clk_misc->Pll_C1_Bias		=	CmuReg->PllC1CpuxBias;	 
	clk_misc->PllC1Tun		=	CmuReg->PllC1Tun;	
	clk_misc->PllC1Ctl		=	CmuReg->PllC1Ctl;	
	clk_misc->PllVideo0Bias		= CmuReg->PllVideo0Bias			;           //0x228,  pll video bias reg
	clk_misc->PllVeBias		= CmuReg->PllVeBias			;           //0x22c,  pll ve    bias reg
	clk_misc->PllVideo0Reg0Pattern	= CmuReg->PllVideo0Reg0Pattern		;           //0x288,  pll video pattern reg
	clk_misc->PllVideo0Reg1Pattern	= CmuReg->PllVideo0Reg1Pattern		;           //0x288,  pll video pattern reg
	clk_misc->Pll3Ctl		= CmuReg->Pll3Ctl			;           //0x10, video
	clk_misc->Pll4Ctl		= CmuReg->Pll4Ctl			;           //0x18, ve
	clk_misc->PllPeriphBias		= CmuReg->PllPeriphBias			;	    //0x234, pll periph bias
	clk_misc->Pll6Ctl		= CmuReg->Pll6Ctl			;	    //0x28, pll periph ctrl
	clk_misc->Apb2Div		= *(volatile __u32 *)(&CmuReg->Apb2Div) ;	    //0x58, apb2 divide ratio
#endif	
	
#ifdef CONFIG_ARCH_SUN8IW5P1	
	clk_misc->PllVedioBias		= CmuReg->PllVedioBias			;           //0x228,  pll vedio bias reg
	clk_misc->PllVeBias		= CmuReg->PllVeBias			;           //0x22c,  pll ve    bias reg
	clk_misc->PllVedioPattern	= CmuReg->PllVedioPattern		;           //0x288,  pll vedio pattern reg
	clk_misc->PllVePattern		= CmuReg->PllVePattern			;           //0x28c,  pll ve    pattern reg	
	clk_misc->Pll3Ctl		= CmuReg->Pll3Ctl			;           //0x10, vedio
	clk_misc->Pll4Ctl		= CmuReg->Pll4Ctl			;           //0x18, ve
#endif	

	//busy_waiting();
	return 0;
}

#ifdef CONFIG_ARCH_SUN8IW6P1
/*
*********************************************************************************************************
*                                     mem_clk_get_cpu_freq
*
* Description: get current cpu freq
*
* Arguments  : none
*
* Returns    : cpu freq.
*********************************************************************************************************
*/
__u32 mem_clk_get_cpu_freq(void)
{
	__u32 FactorN = 1;
	__u32 FactorK = 1;
	__u32 FactorM = 1;
	__u32 FactorP = 1;
	__u32 reg_val  = 0;
	__u32 cpu_freq = 0;
	
	CmuReg_SysClkDiv_tmp.dwval = CmuReg->SysClkDiv.dwval;
	//get runtime freq: clk src + divider ratio
	//src selection
	reg_val = CmuReg_SysClkDiv_tmp.bits.CpuClkSrc;
	if(0 == reg_val){
	    //hosc, 24Mhz
	    cpu_freq = 24000; 			//unit is khz

	}else if(1 == reg_val){
	    CmuReg_Pll1Ctl_tmp.dwval = CmuReg->Pll1Ctl.dwval;
	    FactorN = CmuReg_Pll1Ctl_tmp.bits.FactorN;
	    FactorM = (CmuReg_Pll1Ctl_tmp.bits.FactorM) + 1;
	    FactorP = (0==CmuReg_Pll1Ctl_tmp.bits.FactorP)?1:4;
	    cpu_freq = raw_lib_udiv(24000*FactorN*FactorK, FactorP*FactorM);
	}
	//printk("cpu_freq = dec(%d). \n", cpu_freq);
	//busy_waiting();

	return cpu_freq;
}

#elif defined(CONFIG_ARCH_SUN8IW10P1)

__u32 mem_clk_get_cpu_freq(void)
{
	__u32 FactorN = 1;
	__u32 FactorK = 1;
	__u32 FactorM = 1;
	__u32 FactorP = 1;
	__u32 reg_val  = 0;
	__u32 cpu_freq = 0;

	CmuReg_SysClkDiv_tmp.dwval = CmuReg->SysClkDiv.dwval;
	//get runtime freq: clk src + divider ratio
	//src selection
	reg_val = CmuReg_SysClkDiv_tmp.bits.CpuClkSrc;
	if(0 == reg_val){
	    //32khz osc
	    cpu_freq = 32;

	}else if(1 == reg_val){
	    //hosc, 26Mhz
	    cpu_freq = 26000; 			//unit is khz
	}else if(2 == reg_val || 3 == reg_val){
	    CmuReg_Pll1Ctl_tmp.dwval = CmuReg->Pll1Ctl.dwval;
	    FactorN = CmuReg_Pll1Ctl_tmp.bits.FactorN + 1;
	    FactorK = CmuReg_Pll1Ctl_tmp.bits.FactorK + 1;
	    FactorM = CmuReg_Pll1Ctl_tmp.bits.FactorM + 1;
	    FactorP = 1<<(CmuReg_Pll1Ctl_tmp.bits.FactorP);
	    cpu_freq = raw_lib_udiv(24000*FactorN*FactorK, FactorP*FactorM);
	}
	//printk("cpu_freq = dec(%d). \n", cpu_freq);
	//busy_waiting();

	return cpu_freq;
}

#else

__u32 mem_clk_get_cpu_freq(void)
{
	__u32 FactorN = 1;
	__u32 FactorK = 1;
	__u32 FactorM = 1;
	__u32 FactorP = 1;
	__u32 reg_val  = 0;
	__u32 cpu_freq = 0;
	
	CmuReg_SysClkDiv_tmp.dwval = CmuReg->SysClkDiv.dwval;
	//get runtime freq: clk src + divider ratio
	//src selection
	reg_val = CmuReg_SysClkDiv_tmp.bits.CpuClkSrc;
	if(0 == reg_val){
	    //32khz osc
	    cpu_freq = 32;

	}else if(1 == reg_val){
	    //hosc, 24Mhz
	    cpu_freq = 24000; 			//unit is khz
	}else if(2 == reg_val || 3 == reg_val){
	    CmuReg_Pll1Ctl_tmp.dwval = CmuReg->Pll1Ctl.dwval;
	    FactorN = CmuReg_Pll1Ctl_tmp.bits.FactorN + 1;
	    FactorK = CmuReg_Pll1Ctl_tmp.bits.FactorK + 1;
	    FactorM = CmuReg_Pll1Ctl_tmp.bits.FactorM + 1;
#if defined(CONFIG_ARCH_SUN8IW3P1) || defined(CONFIG_ARCH_SUN8IW5P1)
	    FactorP = 1<<(CmuReg_Pll1Ctl_tmp.bits.FactorP);
#endif
	    cpu_freq = raw_lib_udiv(24000*FactorN*FactorK, FactorP*FactorM);
	}
	//printk("cpu_freq = dec(%d). \n", cpu_freq);
	//busy_waiting();

	return cpu_freq;
}
#endif

#endif

#ifdef CONFIG_ARCH_SUN9IW1P1
static AXI0_CFG_REG_t CmuReg_Axi0_Cfg_tmp;
static PLL_C0_CFG_REG_t CmuReg_Pll_C0_Cfg_tmp;
static AXI1_CFG_REG_t CmuReg_Axi1_Cfg_tmp;
static PLL_C1_CFG_REG_t CmuReg_Pll_C1_Cfg_tmp;
static CPU_CLK_SRC_REG_t CmuReg_Cpu_Clk_Src_tmp;
#define MCTL_COM_BASE   0xf1c62000
#define MC_RMCR         (MCTL_COM_BASE + 0x10)

#define MCTL_CTL_BASE    0xf1c63000
#define M0_UPD2          (MCTL_CTL_BASE + 0x1A8)

#define MCTL_PHY_BASE    0xf1c65000
#define P0_ZQ0CR         (MCTL_PHY_BASE + 0x240)

/*
*********************************************************************************************************
*                           setting_ahb0_para
*
*Description: setting ahb0 related para: dram phy update delay para.
*		the para is related with dram performance, so, after restore the ahb0 freq,
*		it is recommended to call this function.
*Arguments  : none
*
*Return     : result,
*
*Notes      :
*
*********************************************************************************************************
*/
static __s32 setting_ahb0_para(void)
{
	__u32 reg_val = 0;
	__u32 zqcr_ch1 = 0;
	__u32 zqcr_ch0 = 0;
#if 0
	asm("dmb");
	asm("isb");
	//save_pm_secure_mem_status(CLK_RESUME_START | 0xb);
	//open protect registers
	reg_val = *(volatile __u32 *)(MC_RMCR);
	reg_val |= (0x1<<3);
	*(volatile __u32 *)(MC_RMCR) = reg_val;
	asm("dmb");
	asm("isb");

	//disable ZCAL of channel 0
	zqcr_ch0 = *(volatile __u32 *)(P0_ZQ0CR);
	reg_val = ( zqcr_ch0 & ~(0x7<<11) );
	//reg_val = ( zqcr_ch0 | (0x7<<11) );
	*(volatile __u32 *)(P0_ZQ0CR) = reg_val;
	asm("dmb");
	asm("isb");
	
	//disable ZCAL of channel 1
	zqcr_ch1 = *(volatile __u32 *)(P0_ZQ0CR + 0x1000);
	//reg_val = ( zqcr_ch1 | (0x7<<11) );
	reg_val = ( zqcr_ch1 & ~(0x7<<11) );
	*(volatile __u32 *)(P0_ZQ0CR + 0x1000) =  reg_val;
	asm("dmb");
	asm("isb");
	
	delay_us(50);

	reg_val = *(volatile __u32 *)(M0_UPD2); //channel 0
	reg_val &= ~(0xfff<<16);
	reg_val |= (0x40<<16);//correspond to AHB 60MHz, DRAM 800MHz, 0x10 margin
	*(volatile __u32 *)(M0_UPD2) = reg_val;
	asm("dmb");
	asm("isb");

	reg_val = *(volatile __u32 *)(M0_UPD2 + 0x1000);
	reg_val &= ~(0xfff<<16);
	reg_val |= (0x40<<16);//correspond to AHB 60MHz, DRAM 800MHz, 0x10 margin
	*(volatile __u32 *)(M0_UPD2 + 0x1000) = reg_val;
	asm("dmb");
	asm("isb");
	
	//write back ZQCR value of channel 0
	*(volatile __u32 *)(P0_ZQ0CR) = zqcr_ch0;
	asm("dmb");
	asm("isb");

	//write back ZQCR value of channel 1
	*(volatile __u32 *)(P0_ZQ0CR + 0x1000) = zqcr_ch1;
	asm("dmb");
	asm("isb");

	//save_pm_secure_mem_status(CLK_RESUME_START | 0xe);
	//close protect registers
	reg_val = *(volatile __u32 *)(MC_RMCR);
	reg_val &= ~(0x1<<3);
	*(volatile __u32 *)(MC_RMCR) = reg_val;
	asm("dmb");
	asm("isb");
	delay_us(10);
#endif	
	return 0;
}

/*
*********************************************************************************************************
*                           mem_clk_save
*
*Description: save ccu for platform mem
*
*Arguments  : none
*
*Return     : result,
*
*Notes      :
*
*********************************************************************************************************
*/
__s32 mem_clk_save(struct clk_state *ccm_reg)
{

	ccm_reg->ccm_reg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);
	ccm_reg->ccm_mod_reg = (__ccmu_mod_reg_list_t *)IO_ADDRESS(SUNXI_CCM_MOD_PBASE);

	//donot need care pll1 & pll5 & pll6, it is dangerous to 
	//write the reg after enable the pllx.
	//FIXME: which pllx is enabled when restore? 
	//dram need which before exit selfresh?
	
	//ccm_reg->ccm_reg_backup.Pll_C0_Bias.dwval            	= ccm_reg->ccm_reg->Pll_C0_Bias.dwval           ; 
	ccm_reg->ccm_reg_backup.Pll_C1_Bias.dwval		= ccm_reg->ccm_reg->Pll_C1_Bias.dwval		;
	ccm_reg->ccm_reg_backup.Pll_Audio_Bias.dwval         	= ccm_reg->ccm_reg->Pll_Audio_Bias.dwval        ; 
	ccm_reg->ccm_reg_backup.Pll_Periph1_Bias.dwval       	= ccm_reg->ccm_reg->Pll_Periph1_Bias.dwval      ; 
	ccm_reg->ccm_reg_backup.Pll_Ve_Bias.dwval            	= ccm_reg->ccm_reg->Pll_Ve_Bias.dwval           ; 
	//ccm_reg->ccm_reg_backup.Pll_Ddr_Bias.dwval           	= ccm_reg->ccm_reg->Pll_Ddr_Bias.dwval          ; 
	ccm_reg->ccm_reg_backup.Pll_Vedio1_Bias.dwval        	= ccm_reg->ccm_reg->Pll_Vedio1_Bias.dwval       ; 
	ccm_reg->ccm_reg_backup.Pll_Vedio2_Bias.dwval        	= ccm_reg->ccm_reg->Pll_Vedio2_Bias.dwval       ; 
	ccm_reg->ccm_reg_backup.Pll_Gpu_Bias.dwval           	= ccm_reg->ccm_reg->Pll_Gpu_Bias.dwval          ; 
	ccm_reg->ccm_reg_backup.Pll_De_Bias.dwval            	= ccm_reg->ccm_reg->Pll_De_Bias.dwval           ; 
	ccm_reg->ccm_reg_backup.Pll_Isp_Bias.dwval           	= ccm_reg->ccm_reg->Pll_Isp_Bias.dwval          ; 
	ccm_reg->ccm_reg_backup.Pll_Periph2_Bias.dwval       	= ccm_reg->ccm_reg->Pll_Periph2_Bias.dwval      ; 
	//ccm_reg->ccm_reg_backup.Pll_C0_Tun.dwval             	= ccm_reg->ccm_reg->Pll_C0_Tun.dwval            ; 
	ccm_reg->ccm_reg_backup.Pll_C1_Tun.dwval             	= ccm_reg->ccm_reg->Pll_C1_Tun.dwval            ; 
	
	ccm_reg->ccm_reg_backup.Pll_Audio_Pat_Cfg.dwval      	= ccm_reg->ccm_reg->Pll_Audio_Pat_Cfg.dwval     ; 
	ccm_reg->ccm_reg_backup.Pll_Periph1_Pat_Cfg     	= ccm_reg->ccm_reg->Pll_Periph1_Pat_Cfg         ;
	ccm_reg->ccm_reg_backup.Pll_Ve_Pat_Cfg.dwval         	= ccm_reg->ccm_reg->Pll_Ve_Pat_Cfg.dwval        ; 
	//ccm_reg->ccm_reg_backup.Pll_Ddr_Pat_Cfg.dwval        	= ccm_reg->ccm_reg->Pll_Ddr_Pat_Cfg.dwval       ; 
	ccm_reg->ccm_reg_backup.Pll_Video1_Pat_Cfg.dwval     	= ccm_reg->ccm_reg->Pll_Video1_Pat_Cfg.dwval    ; 
	ccm_reg->ccm_reg_backup.Pll_Video2_Pat_Cfg.dwval     	= ccm_reg->ccm_reg->Pll_Video2_Pat_Cfg.dwval    ; 
	ccm_reg->ccm_reg_backup.Pll_Gpu_Pat_Cfg.dwval        	= ccm_reg->ccm_reg->Pll_Gpu_Pat_Cfg.dwval       ; 
	ccm_reg->ccm_reg_backup.Pll_De_Pat_Cfg.dwval         	= ccm_reg->ccm_reg->Pll_De_Pat_Cfg.dwval        ; 
	ccm_reg->ccm_reg_backup.Pll_Isp_Pat_Cfg.dwval        	= ccm_reg->ccm_reg->Pll_Isp_Pat_Cfg.dwval       ; 
	ccm_reg->ccm_reg_backup.Pll_Periph2_Pat_Cfg     	= ccm_reg->ccm_reg->Pll_Periph2_Pat_Cfg         ;
	
	//C0
	ccm_reg->ccm_reg_backup.Pll_C1_Cfg.dwval    	        = ccm_reg->ccm_reg->Pll_C1_Cfg.dwval    	;        
	ccm_reg->ccm_reg_backup.Pll_Audio_Cfg.dwval    		= ccm_reg->ccm_reg->Pll_Audio_Cfg.dwval    	;
	ccm_reg->ccm_reg_backup.Pll_Periph1_Cfg.dwval        	= ccm_reg->ccm_reg->Pll_Periph1_Cfg.dwval       ; 
	ccm_reg->ccm_reg_backup.Pll_Ve_Cfg.dwval    	        = ccm_reg->ccm_reg->Pll_Ve_Cfg.dwval    	;        
	//ccm_reg->ccm_reg_backup.Pll_Ddr_Cfg.dwval    		= ccm_reg->ccm_reg->Pll_Ddr_Cfg.dwval    	;
	ccm_reg->ccm_reg_backup.Pll_Video1_Cfg.dwval         	= ccm_reg->ccm_reg->Pll_Video1_Cfg.dwval        ; 
	ccm_reg->ccm_reg_backup.Pll_Video2_Cfg.dwval         	= ccm_reg->ccm_reg->Pll_Video2_Cfg.dwval        ; 
	ccm_reg->ccm_reg_backup.Pll_Gpu_Cfg.dwval    		= ccm_reg->ccm_reg->Pll_Gpu_Cfg.dwval    	;
	ccm_reg->ccm_reg_backup.Pll_De_Cfg.dwval    	        = ccm_reg->ccm_reg->Pll_De_Cfg.dwval    	;        
	ccm_reg->ccm_reg_backup.Pll_Isp_Cfg.dwval    		= ccm_reg->ccm_reg->Pll_Isp_Cfg.dwval    	;
	ccm_reg->ccm_reg_backup.Pll_Periph2_Cfg.dwval        	= ccm_reg->ccm_reg->Pll_Periph2_Cfg.dwval       ; 
	ccm_reg->ccm_reg_backup.Cpu_Clk_Src.dwval            	= ccm_reg->ccm_reg->Cpu_Clk_Src.dwval           ; 
	ccm_reg->ccm_reg_backup.Axi0_Cfg.dwval	        	= ccm_reg->ccm_reg->Axi0_Cfg.dwval	        ;
	ccm_reg->ccm_reg_backup.Axi1_Cfg.dwval	        	= ccm_reg->ccm_reg->Axi1_Cfg.dwval	        ;
	//
	ccm_reg->ccm_reg_backup.Ahb0_Cfg.dwval	        	= ccm_reg->ccm_reg->Ahb0_Cfg.dwval	        ;
	ccm_reg->ccm_reg_backup.Ahb1_Cfg.dwval	        	= ccm_reg->ccm_reg->Ahb1_Cfg.dwval	        ;
	ccm_reg->ccm_reg_backup.Ahb2_Cfg.dwval	        	= ccm_reg->ccm_reg->Ahb2_Cfg.dwval	        ;
	ccm_reg->ccm_reg_backup.Apb0_Cfg.dwval	        	= ccm_reg->ccm_reg->Apb0_Cfg.dwval	        ;
	ccm_reg->ccm_reg_backup.Apb1_Cfg.dwval	        	= ccm_reg->ccm_reg->Apb1_Cfg.dwval	        ;
	ccm_reg->ccm_reg_backup.Ats_Clk.dwval                	= ccm_reg->ccm_reg->Ats_Clk.dwval               ; 
	ccm_reg->ccm_reg_backup.Trace_Clk.dwval              	= ccm_reg->ccm_reg->Trace_Clk.dwval             ; 
	ccm_reg->ccm_reg_backup.Pll_Lock_Cfg.dwval           	= ccm_reg->ccm_reg->Pll_Lock_Cfg.dwval          ; 
	ccm_reg->ccm_reg_backup.Pll_Lock_Cfg_1.dwval         	= ccm_reg->ccm_reg->Pll_Lock_Cfg_1.dwval        ; 
	ccm_reg->ccm_reg_backup.Pll_Lock_Status.dwval        	= ccm_reg->ccm_reg->Pll_Lock_Status.dwval       ; 
	ccm_reg->ccm_reg_backup.Clk_Outa.dwval  	        = ccm_reg->ccm_reg->Clk_Outa.dwval  	        ;
	ccm_reg->ccm_reg_backup.Clk_Outb.dwval  	        = ccm_reg->ccm_reg->Clk_Outb.dwval  	        ;
	//backup cci400 + gtbus
	ccm_reg->ccm_reg_backup.Cci400_Cfg.dwval             	= ccm_reg->ccm_reg->Cci400_Cfg.dwval            ; 
	ccm_reg->ccm_reg_backup.Gtclk_Cfg.dwval	        	= ccm_reg->ccm_reg->Gtclk_Cfg.dwval	        ;

	return 0;
}


/*
*********************************************************************************************************
*                           mem_clk_exit
*
*Description: restore ccu for platform mem
*
*Arguments  : none
*
*Return     : result,
*
*Notes      :
*
*********************************************************************************************************
*/
__s32 mem_clk_restore(struct clk_state *ccm_reg)
{
	/* restore pll para first.
	** then, restore bus;
 	** reason: pll src need be ready before restore cci,gtbus,ahb... bus src.
	**/
	//save_pm_secure_mem_status(CLK_RESUME_START | 0x0);
	change_runtime_env();
	//first, sys related clk.
	//ccm_reg->ccm_reg->Pll_C0_Bias.dwval		= ccm_reg->ccm_reg_backup.Pll_C0_Bias.dwval		; 
	ccm_reg->ccm_reg->Pll_Audio_Bias.dwval		= ccm_reg->ccm_reg_backup.Pll_Audio_Bias.dwval		; 
	ccm_reg->ccm_reg->Pll_Periph1_Bias.dwval	= ccm_reg->ccm_reg_backup.Pll_Periph1_Bias.dwval	; 
	ccm_reg->ccm_reg->Pll_Ve_Bias.dwval		= ccm_reg->ccm_reg_backup.Pll_Ve_Bias.dwval		; 
	//ccm_reg->ccm_reg->Pll_Ddr_Bias.dwval		= ccm_reg->ccm_reg_backup.Pll_Ddr_Bias.dwval		; 
	ccm_reg->ccm_reg->Pll_Gpu_Bias.dwval		= ccm_reg->ccm_reg_backup.Pll_Gpu_Bias.dwval		; 
	ccm_reg->ccm_reg->Pll_De_Bias.dwval		= ccm_reg->ccm_reg_backup.Pll_De_Bias.dwval		; 
	ccm_reg->ccm_reg->Pll_Isp_Bias.dwval		= ccm_reg->ccm_reg_backup.Pll_Isp_Bias.dwval		; 
	ccm_reg->ccm_reg->Pll_Periph2_Bias.dwval	= ccm_reg->ccm_reg_backup.Pll_Periph2_Bias.dwval	; 
	//ccm_reg->ccm_reg->Pll_C0_Tun.dwval		= ccm_reg->ccm_reg_backup.Pll_C0_Tun.dwval		; 
	
	//save_pm_secure_mem_status(CLK_RESUME_START | 0x2);
	ccm_reg->ccm_reg->Pll_Audio_Pat_Cfg.dwval	= ccm_reg->ccm_reg_backup.Pll_Audio_Pat_Cfg.dwval	; 
	ccm_reg->ccm_reg->Pll_Periph1_Pat_Cfg		= ccm_reg->ccm_reg_backup.Pll_Periph1_Pat_Cfg		;
	ccm_reg->ccm_reg->Pll_Ve_Pat_Cfg.dwval		= ccm_reg->ccm_reg_backup.Pll_Ve_Pat_Cfg.dwval		; 
	//ccm_reg->ccm_reg->Pll_Ddr_Pat_Cfg.dwval	= ccm_reg->ccm_reg_backup.Pll_Ddr_Pat_Cfg.dwval 	; 
	ccm_reg->ccm_reg->Pll_Gpu_Pat_Cfg.dwval 	= ccm_reg->ccm_reg_backup.Pll_Gpu_Pat_Cfg.dwval 	; 
	ccm_reg->ccm_reg->Pll_De_Pat_Cfg.dwval		= ccm_reg->ccm_reg_backup.Pll_De_Pat_Cfg.dwval		; 
	ccm_reg->ccm_reg->Pll_Isp_Pat_Cfg.dwval 	= ccm_reg->ccm_reg_backup.Pll_Isp_Pat_Cfg.dwval 	; 
	ccm_reg->ccm_reg->Pll_Periph2_Pat_Cfg		= ccm_reg->ccm_reg_backup.Pll_Periph2_Pat_Cfg		;
							
							
	//save_pm_secure_mem_status(CLK_RESUME_START | 0x4);
	delay_us(10);
	ccm_reg->ccm_reg->Pll_Audio_Cfg.dwval		= ccm_reg->ccm_reg_backup.Pll_Audio_Cfg.dwval		;
	delay_us(10);
	ccm_reg->ccm_reg->Pll_Periph1_Cfg.dwval 	= ccm_reg->ccm_reg_backup.Pll_Periph1_Cfg.dwval 	; 
	delay_us(10);
	ccm_reg->ccm_reg->Pll_Ve_Cfg.dwval		= ccm_reg->ccm_reg_backup.Pll_Ve_Cfg.dwval		;	 
	delay_us(10);
	//ccm_reg->ccm_reg->Pll_Ddr_Cfg.dwval		= ccm_reg->ccm_reg_backup.Pll_Ddr_Cfg.dwval		;
	ccm_reg->ccm_reg->Pll_Gpu_Cfg.dwval		= ccm_reg->ccm_reg_backup.Pll_Gpu_Cfg.dwval		;
	delay_us(10);
	ccm_reg->ccm_reg->Pll_De_Cfg.dwval		= ccm_reg->ccm_reg_backup.Pll_De_Cfg.dwval		;	 
	delay_us(10);
	ccm_reg->ccm_reg->Pll_Isp_Cfg.dwval		= ccm_reg->ccm_reg_backup.Pll_Isp_Cfg.dwval		;
	delay_us(10);
	//busy_waiting();
	//config perihp1 freq -> 240M 
	ccm_reg->ccm_reg->Pll_Periph2_Cfg.dwval 	= 0x80041400;		//24*20/2 = 240M;
	delay_us(10);
	//save_pm_secure_mem_status(CLK_RESUME_START | 0x5);
	ccm_reg->ccm_reg->Cpu_Clk_Src.dwval		= ccm_reg->ccm_reg_backup.Cpu_Clk_Src.dwval		; 
	delay_us(10);
	ccm_reg->ccm_reg->Axi0_Cfg.dwval		= ccm_reg->ccm_reg_backup.Axi0_Cfg.dwval		;
	delay_us(10);
	ccm_reg->ccm_reg->Axi1_Cfg.dwval		= ccm_reg->ccm_reg_backup.Axi1_Cfg.dwval		;
	delay_us(200);
	
	//switch gtbus src: gtbus clk = 240M, at this time, ahb0 src is gtbus.
	ccm_reg->ccm_reg->Gtclk_Cfg.dwval		= (0x3000000)&(ccm_reg->ccm_reg_backup.Gtclk_Cfg.dwval);
	delay_us(10);
	//save_pm_secure_mem_status(CLK_RESUME_START | 0x6);
	//config ahb0 divide ratio = 8, ahb0 freq = 240/8 = 30M.
	ccm_reg->ccm_reg->Ahb0_Cfg.dwval		= (~0x3000000)&(ccm_reg->ccm_reg_backup.Ahb0_Cfg.dwval)	;
	delay_us(10);
	//config ahb0 src. ahb0 freq = 960M/8 = 120M
	ccm_reg->ccm_reg->Ahb0_Cfg.dwval		= ccm_reg->ccm_reg_backup.Ahb0_Cfg.dwval		;
	delay_us(10);
	//save_pm_secure_mem_status(CLK_RESUME_START | 0x7);
	//config gtbus divide. gtbus clk = 240M /3 = 80M;
	ccm_reg->ccm_reg->Gtclk_Cfg.dwval		= ccm_reg->ccm_reg_backup.Gtclk_Cfg.dwval		;
	//delay 10us
	delay_us(10);
	//gtbus clk = 1.2G/3 = 400M
	ccm_reg->ccm_reg->Pll_Periph2_Cfg.dwval 	= ccm_reg->ccm_reg_backup.Pll_Periph2_Cfg.dwval 	; 
	delay_us(200);
	//save_pm_secure_mem_status(CLK_RESUME_START | 0x8);
	
	ccm_reg->ccm_reg->Ahb1_Cfg.dwval		= (~0x3000000)&(ccm_reg->ccm_reg_backup.Ahb1_Cfg.dwval)	;
	ccm_reg->ccm_reg->Ahb2_Cfg.dwval		= (~0x3000000)&(ccm_reg->ccm_reg_backup.Ahb2_Cfg.dwval)	;
	ccm_reg->ccm_reg->Apb0_Cfg.dwval		= (~0x3000000)&(ccm_reg->ccm_reg_backup.Apb0_Cfg.dwval)	;
	delay_us(10);
	ccm_reg->ccm_reg->Apb1_Cfg.dwval		= (~0x3000000)&(ccm_reg->ccm_reg_backup.Apb1_Cfg.dwval)	;
	ccm_reg->ccm_reg->Cci400_Cfg.dwval		= (~0x3000000)&(ccm_reg->ccm_reg_backup.Cci400_Cfg.dwval); 
	delay_us(10);
	ccm_reg->ccm_reg->Ats_Clk.dwval 		= ccm_reg->ccm_reg_backup.Ats_Clk.dwval 		; 
	ccm_reg->ccm_reg->Trace_Clk.dwval		= ccm_reg->ccm_reg_backup.Trace_Clk.dwval		; 
	ccm_reg->ccm_reg->Pll_Lock_Cfg.dwval		= ccm_reg->ccm_reg_backup.Pll_Lock_Cfg.dwval		; 
	ccm_reg->ccm_reg->Pll_Lock_Cfg_1.dwval		= ccm_reg->ccm_reg_backup.Pll_Lock_Cfg_1.dwval		; 
	ccm_reg->ccm_reg->Pll_Lock_Status.dwval 	= ccm_reg->ccm_reg_backup.Pll_Lock_Status.dwval 	; 
	ccm_reg->ccm_reg->Clk_Outa.dwval		= ccm_reg->ccm_reg_backup.Clk_Outa.dwval		;
	ccm_reg->ccm_reg->Clk_Outb.dwval		= ccm_reg->ccm_reg_backup.Clk_Outb.dwval		;
	//config src.
	//save_pm_secure_mem_status(CLK_RESUME_START | 0x8);
	ccm_reg->ccm_reg->Cci400_Cfg.dwval		= ccm_reg->ccm_reg_backup.Cci400_Cfg.dwval		; 
	delay_us(10);
	ccm_reg->ccm_reg->Ahb1_Cfg.dwval		= ccm_reg->ccm_reg_backup.Ahb1_Cfg.dwval		;
	ccm_reg->ccm_reg->Ahb2_Cfg.dwval		= ccm_reg->ccm_reg_backup.Ahb2_Cfg.dwval		;
	ccm_reg->ccm_reg->Apb0_Cfg.dwval		= ccm_reg->ccm_reg_backup.Apb0_Cfg.dwval		;
	//delay 10us
	delay_us(10);
	ccm_reg->ccm_reg->Apb1_Cfg.dwval		= ccm_reg->ccm_reg_backup.Apb1_Cfg.dwval		;
	//delay_us(10);
	//save_pm_secure_mem_status(CLK_RESUME_START | 0xa);

	setting_ahb0_para();
	//save_pm_secure_mem_status(CLK_RESUME_START | 0xf);
	return 0;
}


/*
*********************************************************************************************************
*                                     mem_clk_setdiv
*
* Description: set div ratio
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/
__s32 mem_clk_setdiv(struct clk_div_t *clk_div)
{
	if(!clk_div){
		return -1;
	}

	CmuReg = (__ccmu_reg_list_t *)(AW_CCM_BASE);

	//restore axi division.
	if(get_cur_cluster_id()){
	    *(volatile __u32 *)&CmuReg->Axi1_Cfg    = clk_div->Axi1_Cfg	;  
	}else{
	    *(volatile __u32 *)&CmuReg->Axi0_Cfg    = clk_div->Axi0_Cfg	;  
	}
	//asm volatile ("dsb");
	//asm volatile ("isb");
	
	return 0;
}

/*
*********************************************************************************************************
*                                     mem_clk_getdiv
*
* Description: 
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/
__s32 mem_clk_getdiv(struct clk_div_t  *clk_div)
{
	if(!clk_div){
		return -1;
	}
	CmuReg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);
	    
	if(get_cur_cluster_id()){
	    clk_div->Axi1_Cfg	=	*(volatile __u32 *)&CmuReg->Axi1_Cfg;           
	}else{
	    clk_div->Axi0_Cfg	=	*(volatile __u32 *)&CmuReg->Axi0_Cfg;           
	}
	return 0;
}


/*
*********************************************************************************************************
*                                     mem_clk_set_pll_factor
*
* Description: set pll factor, target cpu freq is ?M hz
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/

__s32 mem_clk_set_pll_factor(struct pll_factor_t *pll_factor)
{
	CmuReg = (__ccmu_reg_list_t *)(AW_CCM_BASE);
	if(get_cur_cluster_id()){
	    CmuReg_Pll_C1_Cfg_tmp.dwval = CmuReg->Pll_C1_Cfg.dwval;
	    //set pll factor: notice: when raise freq, N must be the last to set
	    CmuReg_Pll_C1_Cfg_tmp.bits.pll_postdiv_m	= pll_factor->FactorM;
	    CmuReg_Pll_C1_Cfg_tmp.bits.pll_out_ext_divp	= pll_factor->FactorP;
	    CmuReg_Pll_C1_Cfg_tmp.bits.pll_factor_n		= pll_factor->FactorN;
	    CmuReg->Pll_C1_Cfg.dwval = CmuReg_Pll_C1_Cfg_tmp.dwval;
	}else{
	    CmuReg_Pll_C0_Cfg_tmp.dwval = CmuReg->Pll_C0_Cfg.dwval;
	    CmuReg_Pll_C0_Cfg_tmp.bits.pll_postdiv_m	= pll_factor->FactorM;
	    CmuReg_Pll_C0_Cfg_tmp.bits.pll_out_ext_divp	= pll_factor->FactorP;
	    CmuReg_Pll_C0_Cfg_tmp.bits.pll_factor_n		= pll_factor->FactorN;
	    CmuReg->Pll_C0_Cfg.dwval = CmuReg_Pll_C0_Cfg_tmp.dwval;
	}	
	
	return 0;
}

/*
*********************************************************************************************************
*                                     mem_clk_get_pll_factor
*
* Description: get pll factor
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/

__s32 mem_clk_get_pll_factor(struct pll_factor_t *pll_factor)
{
	CmuReg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);
	if(get_cur_cluster_id()){
	    CmuReg_Pll_C1_Cfg_tmp.dwval = CmuReg->Pll_C1_Cfg.dwval;
	    pll_factor->FactorN = CmuReg_Pll_C1_Cfg_tmp.bits.pll_factor_n;
	    pll_factor->FactorM = CmuReg_Pll_C1_Cfg_tmp.bits.pll_postdiv_m;
	    pll_factor->FactorP = CmuReg_Pll_C1_Cfg_tmp.bits.pll_out_ext_divp;
	}else{
	    CmuReg_Pll_C0_Cfg_tmp.dwval = CmuReg->Pll_C0_Cfg.dwval;
	    pll_factor->FactorN = CmuReg_Pll_C0_Cfg_tmp.bits.pll_factor_n;
	    pll_factor->FactorM = CmuReg_Pll_C0_Cfg_tmp.bits.pll_postdiv_m;
	    pll_factor->FactorP = CmuReg_Pll_C0_Cfg_tmp.bits.pll_out_ext_divp;
	}	
	
	return 0;
}

/*
*********************************************************************************************************
*                                     mem_clk_set_misc
*
* Description: set clk_misc
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/
__s32 mem_clk_set_misc(struct clk_misc_t *clk_misc)
{
	CmuReg = (__ccmu_reg_list_t *)(AW_CCM_BASE);
	//
	CmuReg->Pll_C0_Bias.dwval	= clk_misc->Pll_C0_Bias     ;
	CmuReg->Pll_C0_Tun.dwval      	= clk_misc->Pll_C0_Tun      ;
	CmuReg->Pll_Periph1_Bias.dwval	= clk_misc->Pll_Periph1_Bias;
	CmuReg->Pll_Periph2_Bias.dwval	= clk_misc->Pll_Periph2_Bias;
	//set vedio, cluster1 pll para
	//1st bias
	CmuReg->Pll_C1_Bias.dwval	= clk_misc->Pll_C1_Bias           	;       //0x00a4
	CmuReg->Pll_Vedio1_Bias.dwval	= clk_misc->Pll_Vedio1_Bias       	;       //0x00b8
	CmuReg->Pll_Vedio2_Bias.dwval	= clk_misc->Pll_Vedio2_Bias       	;       //0x00bc
	//2nd: tune& pattern
	CmuReg->Pll_C1_Tun.dwval	= clk_misc->Pll_C1_Tun            	;       //0x00e4
	CmuReg->Pll_Video1_Pat_Cfg.dwval= clk_misc->Pll_Video1_Pat_Cfg    	;       //0x0118
	CmuReg->Pll_Video2_Pat_Cfg.dwval= clk_misc->Pll_Video2_Pat_Cfg    	;       //0x011c
	//3rd: cfg

	if(get_cur_cluster_id()){
	    CmuReg->Pll_C0_Cfg.dwval	= clk_misc->Pll_C0_Cfg     	  	;    	//0x0000
	}else{
	    CmuReg->Pll_C1_Cfg.dwval	= clk_misc->Pll_C1_Cfg     	  	;    	//0x0004
	}
	CmuReg->Pll_Video1_Cfg.dwval	= clk_misc->Pll_Video1_Cfg     	  	;    	//0x0018
	CmuReg->Pll_Video2_Cfg.dwval	= clk_misc->Pll_Video2_Cfg    	  	;    	//0x001c

	//axi src is cpu clk src.
	//need set axi division before change pll1 to 408M, 
	//config axi ratio to 1+1 = 2;
	//axi can not exceed 300M;
	//need to update axi divide ratio.
	if(get_cur_cluster_id()){
	    *(volatile __u32 *)&CmuReg->Axi1_Cfg = 0x102; //atb=2; axi div = 3;
	}else{
	    *(volatile __u32 *)&CmuReg->Axi0_Cfg = 0x102; //atb=2; axi div = 3;
	}
//	asm volatile ("dsb");
//	asm volatile ("isb");
	
	if(get_cur_cluster_id()){
	    *(volatile __u32 *)&CmuReg->Axi1_Cfg = 0x203; //atb=4; axi div = 4;
	}else{
	    *(volatile __u32 *)&CmuReg->Axi0_Cfg = 0x203; //atb=4; axi div = 4;
	}
//	asm volatile ("dsb");
//	asm volatile ("isb");
	
	if(get_cur_cluster_id()){
	    CmuReg_Axi1_Cfg_tmp.dwval = CmuReg->Axi1_Cfg.dwval;
	    CmuReg_Axi1_Cfg_tmp.bits.axi1_clk_div_ratio = 1;
	    CmuReg->Axi1_Cfg.dwval = CmuReg_Axi1_Cfg_tmp.dwval;
	}else{
	    CmuReg_Axi0_Cfg_tmp.dwval = CmuReg->Axi0_Cfg.dwval;
	    CmuReg_Axi0_Cfg_tmp.bits.axi0_clk_div_ratio = 1;
	    CmuReg->Axi0_Cfg.dwval = CmuReg_Axi0_Cfg_tmp.dwval;
	}

	return 0;
}

/*
*********************************************************************************************************
*                                     mem_clk_get_misc
*
* Description: get clk_misc
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/

__s32 mem_clk_get_misc(struct clk_misc_t *clk_misc)
{
	CmuReg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);
	clk_misc->Pll_C0_Bias     	=	CmuReg->Pll_C0_Bias.dwval     	;	 
	clk_misc->Pll_C0_Tun      	=       CmuReg->Pll_C0_Tun.dwval      	;	
	clk_misc->Pll_Periph1_Bias	=	CmuReg->Pll_Periph1_Bias.dwval	;	
	clk_misc->Pll_Periph2_Bias	=	CmuReg->Pll_Periph2_Bias.dwval	;	
	clk_misc->Pll_C1_Bias           = 	CmuReg->Pll_C1_Bias.dwval	;       //0x00a4
	clk_misc->Pll_Vedio1_Bias       = 	CmuReg->Pll_Vedio1_Bias.dwval	;       //0x00b8
	clk_misc->Pll_Vedio2_Bias       = 	CmuReg->Pll_Vedio2_Bias.dwval	;       //0x00bc
	clk_misc->Pll_C1_Tun            = 	CmuReg->Pll_C1_Tun.dwval	;       //0x00e4
	clk_misc->Pll_Video1_Pat_Cfg    = 	CmuReg->Pll_Video1_Pat_Cfg.dwval;       //0x0118
	clk_misc->Pll_Video2_Pat_Cfg    = 	CmuReg->Pll_Video2_Pat_Cfg.dwval;       //0x011c
	clk_misc->Pll_C0_Cfg     	= 	CmuReg->Pll_C0_Cfg.dwval	;    	//0x0000
	clk_misc->Pll_C1_Cfg     	= 	CmuReg->Pll_C1_Cfg.dwval	;    	//0x0004
	clk_misc->Pll_Video1_Cfg     	= 	CmuReg->Pll_Video1_Cfg.dwval	;    	//0x0018
	clk_misc->Pll_Video2_Cfg    	= 	CmuReg->Pll_Video2_Cfg.dwval	;    	//0x001c

	return 0;
}

/*
*********************************************************************************************************
*                                     mem_clk_get_cpu_freq
*
* Description: get current cpu freq
*
* Arguments  : none
*
* Returns    : cpu freq.
*********************************************************************************************************
*/
__u32 mem_clk_get_cpu_freq(void)
{
    __u32 FactorN  = 1;
    __u32 FactorK  = 1;
    __u32 FactorM  = 1;
    __u32 FactorP  = 1;
    __u32 reg_val  = 0;
    __u32 cpu_freq = 0;

    CmuReg_Cpu_Clk_Src_tmp.dwval = CmuReg->Cpu_Clk_Src.dwval;
    //get runtime freq: clk src + divider ratio
    //src selection
    if(get_cur_cluster_id()){
	reg_val = CmuReg_Cpu_Clk_Src_tmp.bits.cpu_c1_clk_src_sel;
    }else{
	    reg_val = CmuReg_Cpu_Clk_Src_tmp.bits.cpu_c0_clk_src_sel;
	}

    if(0 == reg_val){
	//hosc, 24Mhz
	cpu_freq = 24000; 			//unit is khz
    }else if(1 == reg_val){
	if(get_cur_cluster_id()){
	    CmuReg_Pll_C1_Cfg_tmp.dwval = CmuReg->Pll_C1_Cfg.dwval;
	    FactorN = CmuReg_Pll_C1_Cfg_tmp.bits.pll_factor_n;
	    FactorM = (CmuReg_Pll_C1_Cfg_tmp.bits.pll_postdiv_m) + 1;
	    FactorP = (0==CmuReg_Pll_C1_Cfg_tmp.bits.pll_out_ext_divp)?1:4;
	}else{
	    CmuReg_Pll_C0_Cfg_tmp.dwval = CmuReg->Pll_C0_Cfg.dwval;
	    FactorN = CmuReg_Pll_C0_Cfg_tmp.bits.pll_factor_n;
	    FactorM = (CmuReg_Pll_C0_Cfg_tmp.bits.pll_postdiv_m) + 1;
	    FactorP = (0==CmuReg_Pll_C0_Cfg_tmp.bits.pll_out_ext_divp)?1:4;
	}
	cpu_freq = raw_lib_udiv(24000*FactorN*FactorK, FactorP*FactorM);
    }
//    asm volatile ("dsb");
  //  asm volatile ("isb");
    //printk("cpu_freq = dec(%d). \n", cpu_freq);

    return cpu_freq;
}
#endif
