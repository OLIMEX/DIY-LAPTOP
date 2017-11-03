#include "pm_types.h"
#include "pm_i.h"


#if defined(CONFIG_ARCH_SUN8IW8P1)
/*
*********************************************************************************************************
*                                       MEM CCU INITIALISE
*
* Description: mem interrupt initialise.
*
* Arguments  : none.
*
* Returns    : 0/-1;
*********************************************************************************************************
*/
__s32 mem_ccu_save(struct ccm_state *ccm_reg)
{
    return -1;
}

__s32 mem_ccu_restore(struct ccm_state *ccm_reg)
{
    return -1;
}

#endif

#if defined(CONFIG_ARCH_SUN8IW1P1)
static int i = 0;
/*
*********************************************************************************************************
*                                       MEM CCU INITIALISE
*
* Description: mem interrupt initialise.
*
* Arguments  : none.
*
* Returns    : 0/-1;
*********************************************************************************************************
*/
__s32 mem_ccu_save(struct ccm_state *ccm_reg)
{
	ccm_reg->ccm_reg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);
	i = 1;
	while(i < 11){
		if(0 != i && 4 != i && 5 != i){
			//donot need care pll1 & pll5 & pll6, it is dangerous to 
			//write the reg after enable the pllx.
			ccm_reg->ccm_reg_backup.PllxBias[i]	= ccm_reg->ccm_reg->PllxBias[i];
		}
		i++;
	}

	//ccm_reg->ccm_reg_backup.Pll1Tun	= ccm_reg->ccm_reg->Pll1Tun;
	//ccm_reg->ccm_reg_backup.Pll5Tun	= ccm_reg->ccm_reg->Pll5Tun;
	ccm_reg->ccm_reg_backup.MipiPllTun	= ccm_reg->ccm_reg->MipiPllTun;

	//ccm_reg->ccm_reg_backup.Pll1Ctl      	= ccm_reg->ccm_reg->Pll1Ctl;
	ccm_reg->ccm_reg_backup.Pll2Ctl      	= ccm_reg->ccm_reg->Pll2Ctl;
	ccm_reg->ccm_reg_backup.Pll3Ctl      	= ccm_reg->ccm_reg->Pll3Ctl;
	ccm_reg->ccm_reg_backup.Pll4Ctl      	= ccm_reg->ccm_reg->Pll4Ctl;
	//ccm_reg->ccm_reg_backup.Pll6Ctl      	= ccm_reg->ccm_reg->Pll6Ctl;
	ccm_reg->ccm_reg_backup.Pll7Ctl      	= ccm_reg->ccm_reg->Pll7Ctl;
	ccm_reg->ccm_reg_backup.Pll8Ctl      	= ccm_reg->ccm_reg->Pll8Ctl;
	ccm_reg->ccm_reg_backup.MipiPllCtl   	= ccm_reg->ccm_reg->MipiPllCtl;
	ccm_reg->ccm_reg_backup.Pll9Ctl      	= ccm_reg->ccm_reg->Pll9Ctl;
	ccm_reg->ccm_reg_backup.Pll10Ctl      	= ccm_reg->ccm_reg->Pll10Ctl;

	ccm_reg->ccm_reg_backup.SysClkDiv    	= ccm_reg->ccm_reg->SysClkDiv;
	ccm_reg->ccm_reg_backup.Ahb1Div  	= ccm_reg->ccm_reg->Ahb1Div;
	ccm_reg->ccm_reg_backup.Apb2Div   	= ccm_reg->ccm_reg->Apb2Div;

	/*actually, the gating & reset ctrl reg 
	** should not affect corresponding module's recovery.
	*/
	ccm_reg->ccm_reg_backup.AxiGate      	= ccm_reg->ccm_reg->AxiGate;
	ccm_reg->ccm_reg_backup.AhbGate0     	= ccm_reg->ccm_reg->AhbGate0;
	ccm_reg->ccm_reg_backup.AhbGate1     	= ccm_reg->ccm_reg->AhbGate1;
	ccm_reg->ccm_reg_backup.Apb1Gate     	= ccm_reg->ccm_reg->Apb1Gate;
	ccm_reg->ccm_reg_backup.Apb2Gate    	= ccm_reg->ccm_reg->Apb2Gate;
	ccm_reg->ccm_reg_backup.Nand0		= ccm_reg->ccm_reg->Nand0;
	ccm_reg->ccm_reg_backup.Nand1      	= ccm_reg->ccm_reg->Nand1;

	ccm_reg->ccm_reg_backup.Sd0	    	= ccm_reg->ccm_reg->Sd0;
	ccm_reg->ccm_reg_backup.Sd1  		= ccm_reg->ccm_reg->Sd1;
	ccm_reg->ccm_reg_backup.Sd2    		= ccm_reg->ccm_reg->Sd2;
	ccm_reg->ccm_reg_backup.Sd3    		= ccm_reg->ccm_reg->Sd3;
	ccm_reg->ccm_reg_backup.Ts		= ccm_reg->ccm_reg->Ts;
	ccm_reg->ccm_reg_backup.Ss        	= ccm_reg->ccm_reg->Ss;
	ccm_reg->ccm_reg_backup.Spi0      	= ccm_reg->ccm_reg->Spi0;
	ccm_reg->ccm_reg_backup.Spi1      	= ccm_reg->ccm_reg->Spi1;
	ccm_reg->ccm_reg_backup.Spi2      	= ccm_reg->ccm_reg->Spi2;
	ccm_reg->ccm_reg_backup.Spi3      	= ccm_reg->ccm_reg->Spi3;

	ccm_reg->ccm_reg_backup.I2s0       	= ccm_reg->ccm_reg->I2s0;
	ccm_reg->ccm_reg_backup.I2s1       	= ccm_reg->ccm_reg->I2s1;
	ccm_reg->ccm_reg_backup.Spdif	    	= ccm_reg->ccm_reg->Spdif;
	ccm_reg->ccm_reg_backup.Usb	      	= ccm_reg->ccm_reg->Usb;
	ccm_reg->ccm_reg_backup.Mdfs		= ccm_reg->ccm_reg->Mdfs;
	ccm_reg->ccm_reg_backup.DramGate     	= ccm_reg->ccm_reg->DramGate;
	
	ccm_reg->ccm_reg_backup.Be0     	= ccm_reg->ccm_reg->Be0;
	ccm_reg->ccm_reg_backup.Be1    		= ccm_reg->ccm_reg->Be1;
	ccm_reg->ccm_reg_backup.Fe0    		= ccm_reg->ccm_reg->Fe0;
	ccm_reg->ccm_reg_backup.Fe1    		= ccm_reg->ccm_reg->Fe1;
	ccm_reg->ccm_reg_backup.Mp      	= ccm_reg->ccm_reg->Mp;
	ccm_reg->ccm_reg_backup.Lcd0Ch0   	= ccm_reg->ccm_reg->Lcd0Ch0;
	ccm_reg->ccm_reg_backup.Lcd1Ch0   	= ccm_reg->ccm_reg->Lcd1Ch0;
	ccm_reg->ccm_reg_backup.Lcd0Ch1   	= ccm_reg->ccm_reg->Lcd0Ch1;
	ccm_reg->ccm_reg_backup.Lcd1Ch1   	= ccm_reg->ccm_reg->Lcd1Ch1;
	ccm_reg->ccm_reg_backup.Csi0      	= ccm_reg->ccm_reg->Csi0;
	ccm_reg->ccm_reg_backup.Csi1      	= ccm_reg->ccm_reg->Csi1;
	ccm_reg->ccm_reg_backup.Ve        	= ccm_reg->ccm_reg->Ve;
	ccm_reg->ccm_reg_backup.Adda      	= ccm_reg->ccm_reg->Adda;
	ccm_reg->ccm_reg_backup.Avs       	= ccm_reg->ccm_reg->Avs;
	ccm_reg->ccm_reg_backup.Hdmi      	= ccm_reg->ccm_reg->Hdmi;
	ccm_reg->ccm_reg_backup.Ps     	 	= ccm_reg->ccm_reg->Ps;
	ccm_reg->ccm_reg_backup.MtcAcc    	= ccm_reg->ccm_reg->MtcAcc;

	//ccm_reg->ccm_reg_backup.MBus0    	= ccm_reg->ccm_reg->MBus0;
	ccm_reg->ccm_reg_backup.MBus1    	= ccm_reg->ccm_reg->MBus1;
	ccm_reg->ccm_reg_backup.MipiDsi    	= ccm_reg->ccm_reg->MipiDsi;
	ccm_reg->ccm_reg_backup.MipiCsi    	= ccm_reg->ccm_reg->MipiCsi;
	ccm_reg->ccm_reg_backup.IepDrc0		= ccm_reg->ccm_reg->IepDrc0;
	ccm_reg->ccm_reg_backup.IepDrc1		= ccm_reg->ccm_reg->IepDrc1;
	ccm_reg->ccm_reg_backup.IepDeu0		= ccm_reg->ccm_reg->IepDeu0;
	ccm_reg->ccm_reg_backup.IepDeu1		= ccm_reg->ccm_reg->IepDeu1;

	ccm_reg->ccm_reg_backup.GpuCore		= ccm_reg->ccm_reg->GpuCore;
	ccm_reg->ccm_reg_backup.GpuMem		= ccm_reg->ccm_reg->GpuMem;
	ccm_reg->ccm_reg_backup.GpuHyd		= ccm_reg->ccm_reg->GpuHyd;

	ccm_reg->ccm_reg_backup.PllLock		= ccm_reg->ccm_reg->PllLock;
	ccm_reg->ccm_reg_backup.Pll1Lock	= ccm_reg->ccm_reg->Pll1Lock;
	
	ccm_reg->ccm_reg_backup.AhbReset0	= ccm_reg->ccm_reg->AhbReset0;
	ccm_reg->ccm_reg_backup.AhbReset1	= ccm_reg->ccm_reg->AhbReset1;
	ccm_reg->ccm_reg_backup.AhbReset2	= ccm_reg->ccm_reg->AhbReset2;

	ccm_reg->ccm_reg_backup.Apb1Reset	= ccm_reg->ccm_reg->Apb1Reset;
	ccm_reg->ccm_reg_backup.Apb2Reset	= ccm_reg->ccm_reg->Apb2Reset;

	ccm_reg->ccm_reg_backup.ClkOutA		= ccm_reg->ccm_reg->ClkOutA;
	ccm_reg->ccm_reg_backup.ClkOutB		= ccm_reg->ccm_reg->ClkOutB;
	ccm_reg->ccm_reg_backup.ClkOutC		= ccm_reg->ccm_reg->ClkOutC;

	return 0;
}

__s32 mem_ccu_restore(struct ccm_state *ccm_reg)
{
	i = 1;
	while(i < 11){
		if(0 != i && 4 != i && 5 != i){
			//donot need care pll1 & pll5 & pll6, it is dangerous to 
			//write the reg after enable the pllx.
			ccm_reg->ccm_reg->PllxBias[i]	= ccm_reg->ccm_reg_backup.PllxBias[i];
		}
		i++;
	}
	
	//ccm_reg->ccm_reg->Pll1Tun		= ccm_reg->ccm_reg_backup.Pll1Tun;
	//ccm_reg->ccm_reg->Pll5Tun		= ccm_reg->ccm_reg_backup.Pll5Tun;
	ccm_reg->ccm_reg->MipiPllTun	= ccm_reg->ccm_reg_backup.MipiPllTun;

	// ccm_reg->ccm_reg->Pll1Ctl      	= ccm_reg->ccm_reg_backup.Pll1Ctl;
	ccm_reg->ccm_reg->Pll2Ctl      	= ccm_reg->ccm_reg_backup.Pll2Ctl;
	ccm_reg->ccm_reg->Pll3Ctl      	= ccm_reg->ccm_reg_backup.Pll3Ctl;
	ccm_reg->ccm_reg->Pll4Ctl      	= ccm_reg->ccm_reg_backup.Pll4Ctl;
	//ccm_reg->ccm_reg->Pll6Ctl      	= ccm_reg->ccm_reg_backup.Pll6Ctl;
	ccm_reg->ccm_reg->Pll7Ctl      	= ccm_reg->ccm_reg_backup.Pll7Ctl;
	ccm_reg->ccm_reg->Pll8Ctl      	= ccm_reg->ccm_reg_backup.Pll8Ctl;
	ccm_reg->ccm_reg->MipiPllCtl   	= ccm_reg->ccm_reg_backup.MipiPllCtl;
	ccm_reg->ccm_reg->Pll9Ctl     	= ccm_reg->ccm_reg_backup.Pll9Ctl;
	ccm_reg->ccm_reg->Pll10Ctl    	= ccm_reg->ccm_reg_backup.Pll10Ctl;

	ccm_reg->ccm_reg->SysClkDiv    	= ccm_reg->ccm_reg_backup.SysClkDiv;
	ccm_reg->ccm_reg->Ahb1Div  	= ccm_reg->ccm_reg_backup.Ahb1Div;
	ccm_reg->ccm_reg->Apb2Div   	= ccm_reg->ccm_reg_backup.Apb2Div;

	/*actually, the gating & reset ctrl reg 
	** should not affect corresponding module's recovery.
	*/
	//first: reset; then, gating
	ccm_reg->ccm_reg->AhbReset0	= ccm_reg->ccm_reg_backup.AhbReset0;
	ccm_reg->ccm_reg->AhbReset1	= ccm_reg->ccm_reg_backup.AhbReset1;
	ccm_reg->ccm_reg->AhbReset2	= ccm_reg->ccm_reg_backup.AhbReset2;
	ccm_reg->ccm_reg->Apb1Reset	= ccm_reg->ccm_reg_backup.Apb1Reset;
	ccm_reg->ccm_reg->Apb2Reset	= ccm_reg->ccm_reg_backup.Apb2Reset;


	ccm_reg->ccm_reg->Nand0		= ccm_reg->ccm_reg_backup.Nand0;
	ccm_reg->ccm_reg->Nand1      	= ccm_reg->ccm_reg_backup.Nand1;

	ccm_reg->ccm_reg->Sd0	    	= ccm_reg->ccm_reg_backup.Sd0;
	ccm_reg->ccm_reg->Sd1  		= ccm_reg->ccm_reg_backup.Sd1;
	ccm_reg->ccm_reg->Sd2    	= ccm_reg->ccm_reg_backup.Sd2;
	ccm_reg->ccm_reg->Sd3    	= ccm_reg->ccm_reg_backup.Sd3;
	ccm_reg->ccm_reg->Ts		= ccm_reg->ccm_reg_backup.Ts;
	ccm_reg->ccm_reg->Ss        	= ccm_reg->ccm_reg_backup.Ss;
	ccm_reg->ccm_reg->Spi0      	= ccm_reg->ccm_reg_backup.Spi0;
	ccm_reg->ccm_reg->Spi1      	= ccm_reg->ccm_reg_backup.Spi1;
	ccm_reg->ccm_reg->Spi2      	= ccm_reg->ccm_reg_backup.Spi2;
	ccm_reg->ccm_reg->Spi3      	= ccm_reg->ccm_reg_backup.Spi3;

	ccm_reg->ccm_reg->I2s0       	= ccm_reg->ccm_reg_backup.I2s0;
	ccm_reg->ccm_reg->I2s1       	= ccm_reg->ccm_reg_backup.I2s1;
	ccm_reg->ccm_reg->Spdif	    	= ccm_reg->ccm_reg_backup.Spdif;
	ccm_reg->ccm_reg->Usb	      	= ccm_reg->ccm_reg_backup.Usb;
	ccm_reg->ccm_reg->Mdfs		= ccm_reg->ccm_reg_backup.Mdfs;
	ccm_reg->ccm_reg->DramGate    	= ccm_reg->ccm_reg_backup.DramGate;

	ccm_reg->ccm_reg->Be0     	= ccm_reg->ccm_reg_backup.Be0;
	ccm_reg->ccm_reg->Be1    	= ccm_reg->ccm_reg_backup.Be1;
	ccm_reg->ccm_reg->Fe0    	= ccm_reg->ccm_reg_backup.Fe0;
	ccm_reg->ccm_reg->Fe1    	= ccm_reg->ccm_reg_backup.Fe1;
	ccm_reg->ccm_reg->Mp      	= ccm_reg->ccm_reg_backup.Mp;
	ccm_reg->ccm_reg->Lcd0Ch0   	= ccm_reg->ccm_reg_backup.Lcd0Ch0;
	ccm_reg->ccm_reg->Lcd1Ch0   	= ccm_reg->ccm_reg_backup.Lcd1Ch0;
	ccm_reg->ccm_reg->Lcd0Ch1   	= ccm_reg->ccm_reg_backup.Lcd0Ch1;
	ccm_reg->ccm_reg->Lcd1Ch1   	= ccm_reg->ccm_reg_backup.Lcd1Ch1;
	ccm_reg->ccm_reg->Csi0      	= ccm_reg->ccm_reg_backup.Csi0;
	ccm_reg->ccm_reg->Csi1      	= ccm_reg->ccm_reg_backup.Csi1;
	ccm_reg->ccm_reg->Ve        	= ccm_reg->ccm_reg_backup.Ve;
	ccm_reg->ccm_reg->Adda      	= ccm_reg->ccm_reg_backup.Adda;
	ccm_reg->ccm_reg->Avs       	= ccm_reg->ccm_reg_backup.Avs;
	ccm_reg->ccm_reg->Hdmi      	= ccm_reg->ccm_reg_backup.Hdmi;
	ccm_reg->ccm_reg->Ps     	= ccm_reg->ccm_reg_backup.Ps;
	ccm_reg->ccm_reg->MtcAcc    	= ccm_reg->ccm_reg_backup.MtcAcc;

	//ccm_reg->ccm_reg->MBus0    	= ccm_reg->ccm_reg_backup.MBus0;
	ccm_reg->ccm_reg->MBus1    	= ccm_reg->ccm_reg_backup.MBus1;
	ccm_reg->ccm_reg->MipiDsi    	= ccm_reg->ccm_reg_backup.MipiDsi;
	ccm_reg->ccm_reg->MipiCsi    	= ccm_reg->ccm_reg_backup.MipiCsi;
	ccm_reg->ccm_reg->IepDrc0	= ccm_reg->ccm_reg_backup.IepDrc0;
	ccm_reg->ccm_reg->IepDrc1	= ccm_reg->ccm_reg_backup.IepDrc1;
	ccm_reg->ccm_reg->IepDeu0	= ccm_reg->ccm_reg_backup.IepDeu0;
	ccm_reg->ccm_reg->IepDeu1	= ccm_reg->ccm_reg_backup.IepDeu1;

	ccm_reg->ccm_reg->GpuCore	= ccm_reg->ccm_reg_backup.GpuCore;
	ccm_reg->ccm_reg->GpuMem	= ccm_reg->ccm_reg_backup.GpuMem;
	ccm_reg->ccm_reg->GpuHyd	= ccm_reg->ccm_reg_backup.GpuHyd;

	ccm_reg->ccm_reg->PllLock	= ccm_reg->ccm_reg_backup.PllLock;
	ccm_reg->ccm_reg->Pll1Lock	= ccm_reg->ccm_reg_backup.Pll1Lock;


	ccm_reg->ccm_reg->ClkOutA	= ccm_reg->ccm_reg_backup.ClkOutA;
	ccm_reg->ccm_reg->ClkOutB	= ccm_reg->ccm_reg_backup.ClkOutB;
	ccm_reg->ccm_reg->ClkOutC	= ccm_reg->ccm_reg_backup.ClkOutC;

	change_runtime_env();
	delay_us(1);
	ccm_reg->ccm_reg->AxiGate      	= ccm_reg->ccm_reg_backup.AxiGate;
	ccm_reg->ccm_reg->AhbGate0     	= ccm_reg->ccm_reg_backup.AhbGate0;
	ccm_reg->ccm_reg->AhbGate1     	= ccm_reg->ccm_reg_backup.AhbGate1;
	ccm_reg->ccm_reg->Apb1Gate     	= ccm_reg->ccm_reg_backup.Apb1Gate;
	ccm_reg->ccm_reg->Apb2Gate     	= ccm_reg->ccm_reg_backup.Apb2Gate;
	return 0;
}
#endif



#if defined(CONFIG_ARCH_SUN8IW3P1)
static int i = 0;
/*
*********************************************************************************************************
*                                       MEM CCU INITIALISE
*
* Description: mem interrupt initialise.
*
* Arguments  : none.
*
* Returns    : 0/-1;
*********************************************************************************************************
*/
__s32 mem_ccu_save(struct ccm_state *ccm_reg)
{
	ccm_reg->ccm_reg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);
	i = 1;
	while(i < 11){
		if(0 != i && 4 != i && 5 != i && 6 != i){
			//donot need care pll1 & pll5 & pll6, it is dangerous to 
			//write the reg after enable the pllx.
			//pll7 bias ctrl does not exist.
			ccm_reg->ccm_reg_backup.PllxBias[i]	= ccm_reg->ccm_reg->PllxBias[i];
		}
		i++;
	}
	
	//ccm_reg->ccm_reg_backup.Pll1Tun	= ccm_reg->ccm_reg->Pll1Tun;
	//ccm_reg->ccm_reg_backup.Pll5Tun	= ccm_reg->ccm_reg->Pll5Tun;
	ccm_reg->ccm_reg_backup.MipiPllTun	= ccm_reg->ccm_reg->MipiPllTun;

	//ccm_reg->ccm_reg_backup.Pll1Ctl      	= ccm_reg->ccm_reg->Pll1Ctl;
	ccm_reg->ccm_reg_backup.Pll2Ctl      	= ccm_reg->ccm_reg->Pll2Ctl;
	ccm_reg->ccm_reg_backup.Pll3Ctl      	= ccm_reg->ccm_reg->Pll3Ctl;
	ccm_reg->ccm_reg_backup.Pll4Ctl      	= ccm_reg->ccm_reg->Pll4Ctl;
	//ccm_reg->ccm_reg_backup.Pll6Ctl      	= ccm_reg->ccm_reg->Pll6Ctl;
	ccm_reg->ccm_reg_backup.Pll8Ctl      	= ccm_reg->ccm_reg->Pll8Ctl;
	ccm_reg->ccm_reg_backup.MipiPllCtl   	= ccm_reg->ccm_reg->MipiPllCtl;
	ccm_reg->ccm_reg_backup.Pll9Ctl      	= ccm_reg->ccm_reg->Pll9Ctl;
	ccm_reg->ccm_reg_backup.Pll10Ctl      	= ccm_reg->ccm_reg->Pll10Ctl;

	ccm_reg->ccm_reg_backup.SysClkDiv    	= ccm_reg->ccm_reg->SysClkDiv;
	ccm_reg->ccm_reg_backup.Ahb1Div  	= ccm_reg->ccm_reg->Ahb1Div;
	ccm_reg->ccm_reg_backup.Apb2Div   	= ccm_reg->ccm_reg->Apb2Div;

	/*actually, the gating & reset ctrl reg 
	** should not affect corresponding module's recovery.
	*/
	ccm_reg->ccm_reg_backup.AhbGate0     	= ccm_reg->ccm_reg->AhbGate0;
	ccm_reg->ccm_reg_backup.AhbGate1     	= ccm_reg->ccm_reg->AhbGate1;
	ccm_reg->ccm_reg_backup.Apb1Gate     	= ccm_reg->ccm_reg->Apb1Gate;
	ccm_reg->ccm_reg_backup.Apb2Gate    	= ccm_reg->ccm_reg->Apb2Gate;
	
	ccm_reg->ccm_reg_backup.Nand0		= ccm_reg->ccm_reg->Nand0;

	ccm_reg->ccm_reg_backup.Sd0	    	= ccm_reg->ccm_reg->Sd0;
	ccm_reg->ccm_reg_backup.Sd1  		= ccm_reg->ccm_reg->Sd1;
	ccm_reg->ccm_reg_backup.Sd2    		= ccm_reg->ccm_reg->Sd2;
	
	ccm_reg->ccm_reg_backup.Spi0      	= ccm_reg->ccm_reg->Spi0;
	ccm_reg->ccm_reg_backup.Spi1      	= ccm_reg->ccm_reg->Spi1;
	
	ccm_reg->ccm_reg_backup.I2s0       	= ccm_reg->ccm_reg->I2s0;
	ccm_reg->ccm_reg_backup.I2s1       	= ccm_reg->ccm_reg->I2s1;
	
	ccm_reg->ccm_reg_backup.Usb	      	= ccm_reg->ccm_reg->Usb;
	ccm_reg->ccm_reg_backup.DramCfg		= ccm_reg->ccm_reg->DramCfg;
	ccm_reg->ccm_reg_backup.DramGate     	= ccm_reg->ccm_reg->DramGate;
	
	ccm_reg->ccm_reg_backup.Be0     	= ccm_reg->ccm_reg->Be0;
	ccm_reg->ccm_reg_backup.Fe0    		= ccm_reg->ccm_reg->Fe0;
	
	ccm_reg->ccm_reg_backup.Lcd0Ch0   	= ccm_reg->ccm_reg->Lcd0Ch0;
	ccm_reg->ccm_reg_backup.Lcd0Ch1   	= ccm_reg->ccm_reg->Lcd0Ch1;
	
	ccm_reg->ccm_reg_backup.Csi0      	= ccm_reg->ccm_reg->Csi0;
	
	ccm_reg->ccm_reg_backup.Ve        	= ccm_reg->ccm_reg->Ve;
	ccm_reg->ccm_reg_backup.Adda      	= ccm_reg->ccm_reg->Adda;
	ccm_reg->ccm_reg_backup.Avs       	= ccm_reg->ccm_reg->Avs;
	//ccm_reg->ccm_reg_backup.MBus0    	= ccm_reg->ccm_reg->MBus0;
	ccm_reg->ccm_reg_backup.MipiDsi    	= ccm_reg->ccm_reg->MipiDsi;
	ccm_reg->ccm_reg_backup.IepDrc0		= ccm_reg->ccm_reg->IepDrc0;

	ccm_reg->ccm_reg_backup.GpuCore		= ccm_reg->ccm_reg->GpuCore;
	ccm_reg->ccm_reg_backup.GpuMem		= ccm_reg->ccm_reg->GpuMem;
	ccm_reg->ccm_reg_backup.GpuHyd		= ccm_reg->ccm_reg->GpuHyd;

	ccm_reg->ccm_reg_backup.PllLock		= ccm_reg->ccm_reg->PllLock;
	ccm_reg->ccm_reg_backup.Pll1Lock	= ccm_reg->ccm_reg->Pll1Lock;
	
	ccm_reg->ccm_reg_backup.AhbReset0	= ccm_reg->ccm_reg->AhbReset0;
	ccm_reg->ccm_reg_backup.AhbReset1	= ccm_reg->ccm_reg->AhbReset1;
	ccm_reg->ccm_reg_backup.AhbReset2	= ccm_reg->ccm_reg->AhbReset2;
	ccm_reg->ccm_reg_backup.Apb1Reset	= ccm_reg->ccm_reg->Apb1Reset;
	ccm_reg->ccm_reg_backup.Apb2Reset	= ccm_reg->ccm_reg->Apb2Reset;

	return 0;
}

__s32 mem_ccu_restore(struct ccm_state *ccm_reg)
{
	i = 1;
	while(i < 11){
		if(0 != i && 4 != i && 5 != i && 6 != i){
			//donot need care pll1 & pll5 & pll6, it is dangerous to 
			//write the reg after enable the pllx.
			//pll7 bias ctrl does not exist.
			ccm_reg->ccm_reg->PllxBias[i]	= ccm_reg->ccm_reg_backup.PllxBias[i];
		}
		i++;
	}
	
	//ccm_reg->ccm_reg->Pll1Tun		= ccm_reg->ccm_reg_backup.Pll1Tun;
	//ccm_reg->ccm_reg->Pll5Tun		= ccm_reg->ccm_reg_backup.Pll5Tun;
	ccm_reg->ccm_reg->MipiPllTun	= ccm_reg->ccm_reg_backup.MipiPllTun;

	//ccm_reg->ccm_reg->Pll1Ctl      	= ccm_reg->ccm_reg_backup.Pll1Ctl;
	ccm_reg->ccm_reg->Pll2Ctl      	= ccm_reg->ccm_reg_backup.Pll2Ctl;
	ccm_reg->ccm_reg->Pll3Ctl      	= ccm_reg->ccm_reg_backup.Pll3Ctl;
	ccm_reg->ccm_reg->Pll4Ctl      	= ccm_reg->ccm_reg_backup.Pll4Ctl;
	//ccm_reg->ccm_reg->Pll6Ctl      	= ccm_reg->ccm_reg_backup.Pll6Ctl;
	ccm_reg->ccm_reg->Pll8Ctl      	= ccm_reg->ccm_reg_backup.Pll8Ctl;
	ccm_reg->ccm_reg->MipiPllCtl   	= ccm_reg->ccm_reg_backup.MipiPllCtl;
	ccm_reg->ccm_reg->Pll9Ctl     	= ccm_reg->ccm_reg_backup.Pll9Ctl;
	ccm_reg->ccm_reg->Pll10Ctl    	= ccm_reg->ccm_reg_backup.Pll10Ctl;

	ccm_reg->ccm_reg->SysClkDiv    	= ccm_reg->ccm_reg_backup.SysClkDiv;
	ccm_reg->ccm_reg->Ahb1Div  	= ccm_reg->ccm_reg_backup.Ahb1Div;
	change_runtime_env();
	delay_us(1);
	ccm_reg->ccm_reg->Apb2Div   	= ccm_reg->ccm_reg_backup.Apb2Div;

	/*actually, the gating & reset ctrl reg 
	** should not affect corresponding module's recovery.
	*/
	//first, reset, then, gating.
	ccm_reg->ccm_reg->AhbReset0	= ccm_reg->ccm_reg_backup.AhbReset0;
	ccm_reg->ccm_reg->AhbReset1	= ccm_reg->ccm_reg_backup.AhbReset1;
	ccm_reg->ccm_reg->AhbReset2	= ccm_reg->ccm_reg_backup.AhbReset2;
	ccm_reg->ccm_reg->Apb1Reset	= ccm_reg->ccm_reg_backup.Apb1Reset;
	ccm_reg->ccm_reg->Apb2Reset	= ccm_reg->ccm_reg_backup.Apb2Reset;
	
	
	ccm_reg->ccm_reg->Nand0		= ccm_reg->ccm_reg_backup.Nand0;

	ccm_reg->ccm_reg->Sd0	    	= ccm_reg->ccm_reg_backup.Sd0;
	ccm_reg->ccm_reg->Sd1  		= ccm_reg->ccm_reg_backup.Sd1;
	ccm_reg->ccm_reg->Sd2    	= ccm_reg->ccm_reg_backup.Sd2;

	ccm_reg->ccm_reg->Spi0      	= ccm_reg->ccm_reg_backup.Spi0;
	ccm_reg->ccm_reg->Spi1      	= ccm_reg->ccm_reg_backup.Spi1;

	ccm_reg->ccm_reg->I2s0       	= ccm_reg->ccm_reg_backup.I2s0;
	ccm_reg->ccm_reg->I2s1       	= ccm_reg->ccm_reg_backup.I2s1;

	ccm_reg->ccm_reg->Usb	      	= ccm_reg->ccm_reg_backup.Usb;
        ccm_reg->ccm_reg->DramCfg    	= ccm_reg->ccm_reg_backup.DramCfg;
	ccm_reg->ccm_reg->DramGate    	= ccm_reg->ccm_reg_backup.DramGate;

	ccm_reg->ccm_reg->Be0     	= ccm_reg->ccm_reg_backup.Be0;
	ccm_reg->ccm_reg->Fe0    	= ccm_reg->ccm_reg_backup.Fe0;

	ccm_reg->ccm_reg->Lcd0Ch0   	= ccm_reg->ccm_reg_backup.Lcd0Ch0;
	ccm_reg->ccm_reg->Lcd0Ch1   	= ccm_reg->ccm_reg_backup.Lcd0Ch1;

	ccm_reg->ccm_reg->Csi0      	= ccm_reg->ccm_reg_backup.Csi0;
	ccm_reg->ccm_reg->Ve        	= ccm_reg->ccm_reg_backup.Ve;
	ccm_reg->ccm_reg->Adda      	= ccm_reg->ccm_reg_backup.Adda;
	ccm_reg->ccm_reg->Avs       	= ccm_reg->ccm_reg_backup.Avs;

	//ccm_reg->ccm_reg->MBus0    	= ccm_reg->ccm_reg_backup.MBus0;
	ccm_reg->ccm_reg->MipiDsi    	= ccm_reg->ccm_reg_backup.MipiDsi;
	ccm_reg->ccm_reg->IepDrc0	= ccm_reg->ccm_reg_backup.IepDrc0;

	ccm_reg->ccm_reg->GpuCore	= ccm_reg->ccm_reg_backup.GpuCore;
	ccm_reg->ccm_reg->GpuMem	= ccm_reg->ccm_reg_backup.GpuMem;
	ccm_reg->ccm_reg->GpuHyd	= ccm_reg->ccm_reg_backup.GpuHyd;

	ccm_reg->ccm_reg->PllLock	= ccm_reg->ccm_reg_backup.PllLock;
	ccm_reg->ccm_reg->Pll1Lock	= ccm_reg->ccm_reg_backup.Pll1Lock;

	change_runtime_env();
	delay_us(1);
	ccm_reg->ccm_reg->AhbGate0     	= ccm_reg->ccm_reg_backup.AhbGate0;
	ccm_reg->ccm_reg->AhbGate1     	= ccm_reg->ccm_reg_backup.AhbGate1;
	ccm_reg->ccm_reg->Apb1Gate     	= ccm_reg->ccm_reg_backup.Apb1Gate;
	ccm_reg->ccm_reg->Apb2Gate     	= ccm_reg->ccm_reg_backup.Apb2Gate;

	return 0;
}
#endif


#if defined(CONFIG_ARCH_SUN8IW5P1) || defined(CONFIG_ARCH_SUN50IW1P1)

/*
*********************************************************************************************************
*                                       MEM CCU INITIALISE
*
* Description: mem interrupt initialise.
*
* Arguments  : none.
*
* Returns    : 0/-1;
*********************************************************************************************************
*/
__s32 mem_ccu_save(struct ccm_state *ccm_reg)
{
	ccm_reg->ccm_reg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);

	//Pll1Bias;									    //0x220,  pll cpux  bias reg 
	ccm_reg->ccm_reg_backup.PllAudioBias	= ccm_reg->ccm_reg->PllAudioBias;           //0x224,  pll audio bias reg 
	//PllVedioBias;									    //0x228,  pll vedio bias reg 
	//PllVeBias;									    //0x22c,  pll ve    bias reg 
	//PllDram0Bias;									    //0x230,  pll dram0 bias reg 
	//PllPeriphBias;								    //0x234,  pll periph bias reg
	//reserved202;									    //0x238,  reserved       
	ccm_reg->ccm_reg_backup.PllGpuBias  	= ccm_reg->ccm_reg->PllGpuBias;             //0x23c,  pll gpu   bias reg 
	ccm_reg->ccm_reg_backup.PllMipiBias   	= ccm_reg->ccm_reg->PllMipiBias;            //0x240,  pll mipi  bias reg 
	ccm_reg->ccm_reg_backup.PllHsicBias   	= ccm_reg->ccm_reg->PllHsicBias;            //0x244,  pll hsic  bias reg 
	ccm_reg->ccm_reg_backup.PllDeBias    	= ccm_reg->ccm_reg->PllDeBias;              //0x248,  pll de    bias reg 
	//ccm_reg->ccm_reg_backup.PllDram1BiasReg;					    //0x24c,  pll dram1 bias     
	
	
	//ccm_reg->ccm_reg_backup.Pll1Tun	= ccm_reg->ccm_reg->Pll1Tun;
	//ccm_reg->ccm_reg_backup.Pll5Tun	= ccm_reg->ccm_reg->Pll5Tun;
	ccm_reg->ccm_reg_backup.MipiPllTun	= ccm_reg->ccm_reg->MipiPllTun;

	//ccm_reg->ccm_reg_backup.Pll1Ctl      	= ccm_reg->ccm_reg->Pll1Ctl;
	ccm_reg->ccm_reg_backup.Pll2Ctl      	= ccm_reg->ccm_reg->Pll2Ctl;
	//ccm_reg->ccm_reg_backup.Pll3Ctl      	= ccm_reg->ccm_reg->Pll3Ctl;
	//ccm_reg->ccm_reg_backup.Pll4Ctl      	= ccm_reg->ccm_reg->Pll4Ctl;
	//ccm_reg->ccm_reg_backup.Pll6Ctl      	= ccm_reg->ccm_reg->Pll6Ctl;
	ccm_reg->ccm_reg_backup.Pll8Ctl      	= ccm_reg->ccm_reg->Pll8Ctl;
	ccm_reg->ccm_reg_backup.MipiPllCtl   	= ccm_reg->ccm_reg->MipiPllCtl;
	ccm_reg->ccm_reg_backup.Pll9Ctl      	= ccm_reg->ccm_reg->Pll9Ctl;
	ccm_reg->ccm_reg_backup.Pll10Ctl      	= ccm_reg->ccm_reg->Pll10Ctl;

	ccm_reg->ccm_reg_backup.SysClkDiv    	= ccm_reg->ccm_reg->SysClkDiv;
	ccm_reg->ccm_reg_backup.Ahb1Div  	= ccm_reg->ccm_reg->Ahb1Div;
	ccm_reg->ccm_reg_backup.Apb2Div   	= ccm_reg->ccm_reg->Apb2Div;

	/*actually, the gating & reset ctrl reg 
	** should not affect corresponding module's recovery.
	*/
	ccm_reg->ccm_reg_backup.AhbGate0     	= ccm_reg->ccm_reg->AhbGate0;
	ccm_reg->ccm_reg_backup.AhbGate1     	= ccm_reg->ccm_reg->AhbGate1;
	ccm_reg->ccm_reg_backup.Apb1Gate     	= ccm_reg->ccm_reg->Apb1Gate;
	ccm_reg->ccm_reg_backup.Apb2Gate    	= ccm_reg->ccm_reg->Apb2Gate;
	
	ccm_reg->ccm_reg_backup.Nand0		= ccm_reg->ccm_reg->Nand0;

	ccm_reg->ccm_reg_backup.Sd0	    	= ccm_reg->ccm_reg->Sd0;
	ccm_reg->ccm_reg_backup.Sd1  		= ccm_reg->ccm_reg->Sd1;
	ccm_reg->ccm_reg_backup.Sd2    		= ccm_reg->ccm_reg->Sd2;
	ccm_reg->ccm_reg_backup.Ss    		= ccm_reg->ccm_reg->Ss;
	
	ccm_reg->ccm_reg_backup.Spi0      	= ccm_reg->ccm_reg->Spi0;
	ccm_reg->ccm_reg_backup.Spi1      	= ccm_reg->ccm_reg->Spi1;
	
	ccm_reg->ccm_reg_backup.I2s0       	= ccm_reg->ccm_reg->I2s0;
	ccm_reg->ccm_reg_backup.I2s1       	= ccm_reg->ccm_reg->I2s1;
	
	ccm_reg->ccm_reg_backup.Usb	      	= ccm_reg->ccm_reg->Usb;
	ccm_reg->ccm_reg_backup.DramCfg		= ccm_reg->ccm_reg->DramCfg;
	ccm_reg->ccm_reg_backup.PllDdrCfg	= ccm_reg->ccm_reg->PllDdrCfg;
	ccm_reg->ccm_reg_backup.MbusResetReg	= ccm_reg->ccm_reg->MbusResetReg;
	ccm_reg->ccm_reg_backup.DramGate     	= ccm_reg->ccm_reg->DramGate;
	
	ccm_reg->ccm_reg_backup.Be0     	= ccm_reg->ccm_reg->Be0;
	ccm_reg->ccm_reg_backup.Fe0    		= ccm_reg->ccm_reg->Fe0;
	
	ccm_reg->ccm_reg_backup.Lcd0Ch0   	= ccm_reg->ccm_reg->Lcd0Ch0;
	ccm_reg->ccm_reg_backup.Lcd0Ch1   	= ccm_reg->ccm_reg->Lcd0Ch1;
	
	ccm_reg->ccm_reg_backup.Csi0      	= ccm_reg->ccm_reg->Csi0;
	
	ccm_reg->ccm_reg_backup.Ve        	= ccm_reg->ccm_reg->Ve;
	ccm_reg->ccm_reg_backup.Adda      	= ccm_reg->ccm_reg->Adda;
	ccm_reg->ccm_reg_backup.Avs       	= ccm_reg->ccm_reg->Avs;
	//ccm_reg->ccm_reg_backup.MBus0    	= ccm_reg->ccm_reg->MBus0;
	ccm_reg->ccm_reg_backup.MipiDsi    	= ccm_reg->ccm_reg->MipiDsi;
	ccm_reg->ccm_reg_backup.IepDrc0		= ccm_reg->ccm_reg->IepDrc0;

	ccm_reg->ccm_reg_backup.GpuCore		= ccm_reg->ccm_reg->GpuCore;
	ccm_reg->ccm_reg_backup.GpuMem		= ccm_reg->ccm_reg->GpuMem;
	ccm_reg->ccm_reg_backup.GpuHyd		= ccm_reg->ccm_reg->GpuHyd;

	ccm_reg->ccm_reg_backup.PllLock		= ccm_reg->ccm_reg->PllLock;
	ccm_reg->ccm_reg_backup.Pll1Lock	= ccm_reg->ccm_reg->Pll1Lock;
	
	ccm_reg->ccm_reg_backup.AhbReset0	= ccm_reg->ccm_reg->AhbReset0;
	ccm_reg->ccm_reg_backup.AhbReset1	= ccm_reg->ccm_reg->AhbReset1;
	ccm_reg->ccm_reg_backup.AhbReset2	= ccm_reg->ccm_reg->AhbReset2;
	ccm_reg->ccm_reg_backup.Apb1Reset	= ccm_reg->ccm_reg->Apb1Reset;
	ccm_reg->ccm_reg_backup.Apb2Reset	= ccm_reg->ccm_reg->Apb2Reset;

	return 0;
}

__s32 mem_ccu_restore(struct ccm_state *ccm_reg)
{
	
	//Pll1Bias;										//0x220,  pll cpux  bias reg 
	ccm_reg->ccm_reg->PllAudioBias   	= ccm_reg->ccm_reg_backup.PllAudioBias;         //0x224,  pll audio bias reg 
	//PllVedioBias;           								//0x228,  pll vedio bias reg 
	//PllVeBias;              								//0x22c,  pll ve    bias reg 
	//PllDram0Bias;           								//0x230,  pll dram0 bias reg 
        //PllPeriphBias;          								//0x234,  pll periph bias reg
	//reserved202;            								//0x238,  reserved       
	ccm_reg->ccm_reg->PllGpuBias  	 	= ccm_reg->ccm_reg_backup.PllGpuBias;           //0x23c,  pll gpu   bias reg 
	ccm_reg->ccm_reg->PllMipiBias		= ccm_reg->ccm_reg_backup.PllMipiBias;		//0x240,  pll mipi  bias reg 
	ccm_reg->ccm_reg->PllHsicBias		= ccm_reg->ccm_reg_backup.PllHsicBias;		//0x244,  pll hsic  bias reg 
	ccm_reg->ccm_reg->PllDeBias		= ccm_reg->ccm_reg_backup.PllDeBias;		//0x248,  pll de    bias reg 
	//ccm_reg->ccm_reg_backup.PllDram1BiasReg;    						//0x24c,  pll dram1 bias     
	
	//ccm_reg->ccm_reg->Pll1Tun		= ccm_reg->ccm_reg_backup.Pll1Tun;
	//ccm_reg->ccm_reg->Pll5Tun		= ccm_reg->ccm_reg_backup.Pll5Tun;
	ccm_reg->ccm_reg->MipiPllTun	= ccm_reg->ccm_reg_backup.MipiPllTun;

	//ccm_reg->ccm_reg->Pll1Ctl      	= ccm_reg->ccm_reg_backup.Pll1Ctl;
	ccm_reg->ccm_reg->Pll2Ctl      	= ccm_reg->ccm_reg_backup.Pll2Ctl;
	//ccm_reg->ccm_reg->Pll3Ctl      	= ccm_reg->ccm_reg_backup.Pll3Ctl;
	//ccm_reg->ccm_reg->Pll4Ctl      	= ccm_reg->ccm_reg_backup.Pll4Ctl;
	//ccm_reg->ccm_reg->Pll6Ctl      	= ccm_reg->ccm_reg_backup.Pll6Ctl;
	ccm_reg->ccm_reg->Pll8Ctl      	= ccm_reg->ccm_reg_backup.Pll8Ctl;
	ccm_reg->ccm_reg->MipiPllCtl   	= ccm_reg->ccm_reg_backup.MipiPllCtl;
	ccm_reg->ccm_reg->Pll9Ctl     	= ccm_reg->ccm_reg_backup.Pll9Ctl;
	ccm_reg->ccm_reg->Pll10Ctl    	= ccm_reg->ccm_reg_backup.Pll10Ctl;

	ccm_reg->ccm_reg->SysClkDiv    	= ccm_reg->ccm_reg_backup.SysClkDiv;
	ccm_reg->ccm_reg->Ahb1Div  	= ccm_reg->ccm_reg_backup.Ahb1Div;
	change_runtime_env();
	delay_us(1);
	ccm_reg->ccm_reg->Apb2Div   	= ccm_reg->ccm_reg_backup.Apb2Div;

	/*actually, the gating & reset ctrl reg 
	** should not affect corresponding module's recovery.
	*/
	//first, reset, then, gating.
	ccm_reg->ccm_reg->AhbReset0	= ccm_reg->ccm_reg_backup.AhbReset0;
	ccm_reg->ccm_reg->AhbReset1	= ccm_reg->ccm_reg_backup.AhbReset1;
	ccm_reg->ccm_reg->AhbReset2	= ccm_reg->ccm_reg_backup.AhbReset2;
	ccm_reg->ccm_reg->Apb1Reset	= ccm_reg->ccm_reg_backup.Apb1Reset;
	ccm_reg->ccm_reg->Apb2Reset	= ccm_reg->ccm_reg_backup.Apb2Reset;
	ccm_reg->ccm_reg->Nand0		= ccm_reg->ccm_reg_backup.Nand0;

	ccm_reg->ccm_reg->Sd0	    	= ccm_reg->ccm_reg_backup.Sd0;
	ccm_reg->ccm_reg->Sd1  		= ccm_reg->ccm_reg_backup.Sd1;
	ccm_reg->ccm_reg->Sd2    	= ccm_reg->ccm_reg_backup.Sd2;
	
	ccm_reg->ccm_reg->Ss    	= ccm_reg->ccm_reg_backup.Ss;

	ccm_reg->ccm_reg->Spi0      	= ccm_reg->ccm_reg_backup.Spi0;
	ccm_reg->ccm_reg->Spi1      	= ccm_reg->ccm_reg_backup.Spi1;

	ccm_reg->ccm_reg->I2s0       	= ccm_reg->ccm_reg_backup.I2s0;
	ccm_reg->ccm_reg->I2s1       	= ccm_reg->ccm_reg_backup.I2s1;

	ccm_reg->ccm_reg->Usb	      	= ccm_reg->ccm_reg_backup.Usb;

#if 0
        ccm_reg->ccm_reg->DramCfg		= ccm_reg->ccm_reg_backup.DramCfg;
	ccm_reg->ccm_reg_backup.PllDdrCfg       = ccm_reg->ccm_reg->PllDdrCfg;
        ccm_reg->ccm_reg_backup.MbusResetReg    = ccm_reg->ccm_reg->MbusResetReg;
#endif
	ccm_reg->ccm_reg->DramGate		= ccm_reg->ccm_reg_backup.DramGate;

	ccm_reg->ccm_reg->Be0     	= ccm_reg->ccm_reg_backup.Be0;
	ccm_reg->ccm_reg->Fe0    	= ccm_reg->ccm_reg_backup.Fe0;

	ccm_reg->ccm_reg->Lcd0Ch0   	= ccm_reg->ccm_reg_backup.Lcd0Ch0;
	ccm_reg->ccm_reg->Lcd0Ch1   	= ccm_reg->ccm_reg_backup.Lcd0Ch1;

	ccm_reg->ccm_reg->Csi0      	= ccm_reg->ccm_reg_backup.Csi0;
	ccm_reg->ccm_reg->Ve        	= ccm_reg->ccm_reg_backup.Ve;
	ccm_reg->ccm_reg->Adda      	= ccm_reg->ccm_reg_backup.Adda;
	ccm_reg->ccm_reg->Avs       	= ccm_reg->ccm_reg_backup.Avs;

	//ccm_reg->ccm_reg->MBus0    	= ccm_reg->ccm_reg_backup.MBus0;
	ccm_reg->ccm_reg->MipiDsi    	= ccm_reg->ccm_reg_backup.MipiDsi;
	ccm_reg->ccm_reg->IepDrc0	= ccm_reg->ccm_reg_backup.IepDrc0;

	ccm_reg->ccm_reg->GpuCore	= ccm_reg->ccm_reg_backup.GpuCore;
	ccm_reg->ccm_reg->GpuMem	= ccm_reg->ccm_reg_backup.GpuMem;
	ccm_reg->ccm_reg->GpuHyd	= ccm_reg->ccm_reg_backup.GpuHyd;

	ccm_reg->ccm_reg->PllLock	= ccm_reg->ccm_reg_backup.PllLock;
	ccm_reg->ccm_reg->Pll1Lock	= ccm_reg->ccm_reg_backup.Pll1Lock;

	change_runtime_env();
	delay_us(1);
	ccm_reg->ccm_reg->AhbGate0     	= ccm_reg->ccm_reg_backup.AhbGate0;
	ccm_reg->ccm_reg->AhbGate1     	= ccm_reg->ccm_reg_backup.AhbGate1;
	ccm_reg->ccm_reg->Apb1Gate     	= ccm_reg->ccm_reg_backup.Apb1Gate;
	ccm_reg->ccm_reg->Apb2Gate     	= ccm_reg->ccm_reg_backup.Apb2Gate;

	return 0;
}
#endif

#if defined(CONFIG_ARCH_SUN8IW6P1)
/*
*********************************************************************************************************
*                                       MEM CCU INITIALISE
*
* Description: mem interrupt initialise.
*
* Arguments  : none.
*
* Returns    : 0/-1;
*********************************************************************************************************
*/
__s32 mem_ccu_save(struct ccm_state *ccm_reg)
{
	ccm_reg->ccm_reg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);
	
	//bias:
	ccm_reg->ccm_reg_backup.PllAudioBias      	= ccm_reg->ccm_reg->PllAudioBias;
	ccm_reg->ccm_reg_backup.PllGpuBias      	= ccm_reg->ccm_reg->PllGpuBias;
	ccm_reg->ccm_reg_backup.PllHsicBias      	= ccm_reg->ccm_reg->PllHsicBias;
	ccm_reg->ccm_reg_backup.PllDeBias		= ccm_reg->ccm_reg->PllDeBias;
	ccm_reg->ccm_reg_backup.PllVideo1Bias      	= ccm_reg->ccm_reg->PllVideo1Bias;
	
	//tune&pattern:
	ccm_reg->ccm_reg_backup.PllAudioReg0Pattern		= ccm_reg->ccm_reg->PllAudioReg0Pattern;
	ccm_reg->ccm_reg_backup.PllAudioReg1Pattern		= ccm_reg->ccm_reg->PllAudioReg1Pattern;

	//cfg:
	ccm_reg->ccm_reg_backup.Pll2Ctl      	= ccm_reg->ccm_reg->Pll2Ctl;
	ccm_reg->ccm_reg_backup.Pll8Ctl      	= ccm_reg->ccm_reg->Pll8Ctl;
	ccm_reg->ccm_reg_backup.Pll9Ctl      	= ccm_reg->ccm_reg->Pll9Ctl;
	ccm_reg->ccm_reg_backup.Pll10Ctl      	= ccm_reg->ccm_reg->Pll10Ctl;
	ccm_reg->ccm_reg_backup.PllVe1Ctl      	= ccm_reg->ccm_reg->PllVe1Ctl;

	//notice: be care to cpu, axi restore sequence!.
	ccm_reg->ccm_reg_backup.SysClkDiv    	= ccm_reg->ccm_reg->SysClkDiv;
	ccm_reg->ccm_reg_backup.Ahb1Div  	= ccm_reg->ccm_reg->Ahb1Div;
	ccm_reg->ccm_reg_backup.Apb2Div   	= ccm_reg->ccm_reg->Apb2Div;
	ccm_reg->ccm_reg_backup.Ahb2Cfg   	= ccm_reg->ccm_reg->Ahb2Cfg;

	/*actually, the gating & reset ctrl reg 
	** should not affect corresponding module's recovery.
	*/
	ccm_reg->ccm_reg_backup.AhbGate0     	= ccm_reg->ccm_reg->AhbGate0;
	ccm_reg->ccm_reg_backup.AhbGate1     	= ccm_reg->ccm_reg->AhbGate1;
	ccm_reg->ccm_reg_backup.Apb1Gate     	= ccm_reg->ccm_reg->Apb1Gate;
	ccm_reg->ccm_reg_backup.Apb2Gate    	= ccm_reg->ccm_reg->Apb2Gate;
	
	ccm_reg->ccm_reg_backup.Cci400Clk	= ccm_reg->ccm_reg->Cci400Clk;
	ccm_reg->ccm_reg_backup.Nand0		= ccm_reg->ccm_reg->Nand0;

	ccm_reg->ccm_reg_backup.Sd0	    	= ccm_reg->ccm_reg->Sd0;
	ccm_reg->ccm_reg_backup.Sd1  		= ccm_reg->ccm_reg->Sd1;
	ccm_reg->ccm_reg_backup.Sd2    		= ccm_reg->ccm_reg->Sd2;
	ccm_reg->ccm_reg_backup.Ss    		= ccm_reg->ccm_reg->Ss;
	
	ccm_reg->ccm_reg_backup.Spi0      	= ccm_reg->ccm_reg->Spi0;
	ccm_reg->ccm_reg_backup.Spi1      	= ccm_reg->ccm_reg->Spi1;
	
	ccm_reg->ccm_reg_backup.I2s0       	= ccm_reg->ccm_reg->I2s0;
	ccm_reg->ccm_reg_backup.I2s1       	= ccm_reg->ccm_reg->I2s1;
	ccm_reg->ccm_reg_backup.I2s2       	= ccm_reg->ccm_reg->I2s2;
	
	ccm_reg->ccm_reg_backup.TdmClk	      	= ccm_reg->ccm_reg->TdmClk;
	ccm_reg->ccm_reg_backup.SpdifClk      	= ccm_reg->ccm_reg->SpdifClk;
	ccm_reg->ccm_reg_backup.Usb	      	= ccm_reg->ccm_reg->Usb;

	// do not touch dram related config.
	//  ccm_reg->ccm_reg_backup.PllDdrCfg	= ccm_reg->ccm_reg->PllDdrCfg;
	//  ccm_reg->ccm_reg_backup.MbusResetReg	= ccm_reg->ccm_reg->MbusResetReg;
	//  ccm_reg->ccm_reg_backup.DramCfg		= ccm_reg->ccm_reg->DramCfg;
	//  ccm_reg->ccm_reg_backup.DramGate     	= ccm_reg->ccm_reg->DramGate;
	
	ccm_reg->ccm_reg_backup.Lcd0Ch0   	= ccm_reg->ccm_reg->Lcd0Ch0;
	ccm_reg->ccm_reg_backup.Lcd0Ch1   	= ccm_reg->ccm_reg->Lcd0Ch1;
	
	ccm_reg->ccm_reg_backup.MipiCsi      	= ccm_reg->ccm_reg->MipiCsi;
	ccm_reg->ccm_reg_backup.Csi0      	= ccm_reg->ccm_reg->Csi0;
	
	ccm_reg->ccm_reg_backup.Ve        	= ccm_reg->ccm_reg->Ve;
	ccm_reg->ccm_reg_backup.Avs       	= ccm_reg->ccm_reg->Avs;
	ccm_reg->ccm_reg_backup.HdmiClk    	= ccm_reg->ccm_reg->HdmiClk;
	ccm_reg->ccm_reg_backup.HdmiSlowClk    	= ccm_reg->ccm_reg->HdmiSlowClk;
	//mbus? function? 
	//ccm_reg->ccm_reg_backup.MBus0    	= ccm_reg->ccm_reg->MBus0;
	ccm_reg->ccm_reg_backup.MipiDsiReg0    	= ccm_reg->ccm_reg->MipiDsiReg0;
	ccm_reg->ccm_reg_backup.MipiDsiReg1    	= ccm_reg->ccm_reg->MipiDsiReg1;

	ccm_reg->ccm_reg_backup.GpuCore		= ccm_reg->ccm_reg->GpuCore;
	ccm_reg->ccm_reg_backup.GpuMem		= ccm_reg->ccm_reg->GpuMem;
	ccm_reg->ccm_reg_backup.GpuHyd		= ccm_reg->ccm_reg->GpuHyd;

	ccm_reg->ccm_reg_backup.PllLock		= ccm_reg->ccm_reg->PllLock;
	ccm_reg->ccm_reg_backup.Pll1Lock	= ccm_reg->ccm_reg->Pll1Lock;
	
	ccm_reg->ccm_reg_backup.PllStableStatus	= ccm_reg->ccm_reg->PllStableStatus;
	
	ccm_reg->ccm_reg_backup.AhbReset0	= ccm_reg->ccm_reg->AhbReset0;
	ccm_reg->ccm_reg_backup.AhbReset1	= ccm_reg->ccm_reg->AhbReset1;
	ccm_reg->ccm_reg_backup.AhbReset2	= ccm_reg->ccm_reg->AhbReset2;
	ccm_reg->ccm_reg_backup.Apb1Reset	= ccm_reg->ccm_reg->Apb1Reset;
	ccm_reg->ccm_reg_backup.Apb2Reset	= ccm_reg->ccm_reg->Apb2Reset;

	return 0;
}

__s32 mem_ccu_restore(struct ccm_state *ccm_reg)
{
	//bias:
	ccm_reg->ccm_reg->PllAudioBias     	= ccm_reg->ccm_reg_backup.PllAudioBias;
	ccm_reg->ccm_reg->PllGpuBias      	= ccm_reg->ccm_reg_backup.PllGpuBias;
	ccm_reg->ccm_reg->PllHsicBias      	= ccm_reg->ccm_reg_backup.PllHsicBias;
	ccm_reg->ccm_reg->PllDeBias					= ccm_reg->ccm_reg_backup.PllDeBias;
	ccm_reg->ccm_reg->PllVideo1Bias     = ccm_reg->ccm_reg_backup.PllVideo1Bias;
	
	//tune&pattern:
	ccm_reg->ccm_reg->PllAudioReg0Pattern		= ccm_reg->ccm_reg_backup.PllAudioReg0Pattern;
	ccm_reg->ccm_reg->PllAudioReg1Pattern		= ccm_reg->ccm_reg_backup.PllAudioReg1Pattern;
	
	//cfg
	ccm_reg->ccm_reg->Pll2Ctl      	= ccm_reg->ccm_reg_backup.Pll2Ctl;
	ccm_reg->ccm_reg->Pll8Ctl      	= ccm_reg->ccm_reg_backup.Pll8Ctl;
	ccm_reg->ccm_reg->Pll9Ctl     	= ccm_reg->ccm_reg_backup.Pll9Ctl;
	ccm_reg->ccm_reg->Pll10Ctl    	= ccm_reg->ccm_reg_backup.Pll10Ctl;
	ccm_reg->ccm_reg->PllVe1Ctl    	= ccm_reg->ccm_reg_backup.PllVe1Ctl;

	change_runtime_env();
	delay_us(1);
	
	//notice: be care to cpu, axi restore sequence!.
	ccm_reg->ccm_reg->SysClkDiv    	= ccm_reg->ccm_reg_backup.SysClkDiv;
	ccm_reg->ccm_reg->Ahb1Div  	= ccm_reg->ccm_reg_backup.Ahb1Div;
	delay_us(1);
	ccm_reg->ccm_reg->Apb2Div   	= ccm_reg->ccm_reg_backup.Apb2Div;
	ccm_reg->ccm_reg->Ahb2Cfg   	= ccm_reg->ccm_reg_backup.Ahb2Cfg;

	/*actually, the gating & reset ctrl reg 
	** should not affect corresponding module's recovery.
	*/
	//first, reset, then, gating.
	ccm_reg->ccm_reg->AhbReset0	= ccm_reg->ccm_reg_backup.AhbReset0;
	ccm_reg->ccm_reg->AhbReset1	= ccm_reg->ccm_reg_backup.AhbReset1;
	ccm_reg->ccm_reg->AhbReset2	= ccm_reg->ccm_reg_backup.AhbReset2;
	ccm_reg->ccm_reg->Apb1Reset	= ccm_reg->ccm_reg_backup.Apb1Reset;
	ccm_reg->ccm_reg->Apb2Reset	= ccm_reg->ccm_reg_backup.Apb2Reset;

	ccm_reg->ccm_reg->Cci400Clk	= (~0x3000000)&ccm_reg->ccm_reg_backup.Cci400Clk;
	delay_us(10);
	ccm_reg->ccm_reg->Nand0		= ccm_reg->ccm_reg_backup.Nand0;

	ccm_reg->ccm_reg->Sd0	    	= ccm_reg->ccm_reg_backup.Sd0;
	ccm_reg->ccm_reg->Sd1  		= ccm_reg->ccm_reg_backup.Sd1;
	ccm_reg->ccm_reg->Sd2    	= ccm_reg->ccm_reg_backup.Sd2;
	ccm_reg->ccm_reg->Ss    	= ccm_reg->ccm_reg_backup.Ss;

	ccm_reg->ccm_reg->Spi0      	= ccm_reg->ccm_reg_backup.Spi0;
	ccm_reg->ccm_reg->Spi1      	= ccm_reg->ccm_reg_backup.Spi1;

	ccm_reg->ccm_reg->I2s0       	= ccm_reg->ccm_reg_backup.I2s0;
	ccm_reg->ccm_reg->I2s1       	= ccm_reg->ccm_reg_backup.I2s1;
	ccm_reg->ccm_reg->I2s2       	= ccm_reg->ccm_reg_backup.I2s2;

	ccm_reg->ccm_reg->TdmClk	= ccm_reg->ccm_reg_backup.TdmClk;
	ccm_reg->ccm_reg->SpdifClk	= ccm_reg->ccm_reg_backup.SpdifClk;
	ccm_reg->ccm_reg->Usb	      	= ccm_reg->ccm_reg_backup.Usb;
#if 0
        ccm_reg->ccm_reg->DramCfg		= ccm_reg->ccm_reg_backup.DramCfg;
	ccm_reg->ccm_reg_backup.PllDdrCfg       = ccm_reg->ccm_reg->PllDdrCfg;
        ccm_reg->ccm_reg_backup.MbusResetReg    = ccm_reg->ccm_reg->MbusResetReg;
	ccm_reg->ccm_reg->DramGate		= ccm_reg->ccm_reg_backup.DramGate;
#endif

	ccm_reg->ccm_reg->Lcd0Ch0   	= ccm_reg->ccm_reg_backup.Lcd0Ch0;
	ccm_reg->ccm_reg->Lcd0Ch1   	= ccm_reg->ccm_reg_backup.Lcd0Ch1;

	ccm_reg->ccm_reg->MipiCsi      	= ccm_reg->ccm_reg_backup.MipiCsi;
	ccm_reg->ccm_reg->Csi0      	= ccm_reg->ccm_reg_backup.Csi0;
	ccm_reg->ccm_reg->Ve        	= ccm_reg->ccm_reg_backup.Ve;
	ccm_reg->ccm_reg->Avs       	= ccm_reg->ccm_reg_backup.Avs;
	ccm_reg->ccm_reg->HdmiClk    	= ccm_reg->ccm_reg_backup.HdmiClk;
	ccm_reg->ccm_reg->HdmiSlowClk  	= ccm_reg->ccm_reg_backup.HdmiSlowClk;
	//ccm_reg->ccm_reg->MBus0    	= ccm_reg->ccm_reg_backup.MBus0;
	ccm_reg->ccm_reg->MipiDsiReg0    	= ccm_reg->ccm_reg_backup.MipiDsiReg0;
	ccm_reg->ccm_reg->MipiDsiReg1    	= ccm_reg->ccm_reg_backup.MipiDsiReg1;

	ccm_reg->ccm_reg->GpuCore	= ccm_reg->ccm_reg_backup.GpuCore;
	ccm_reg->ccm_reg->GpuMem	= ccm_reg->ccm_reg_backup.GpuMem;
	ccm_reg->ccm_reg->GpuHyd	= ccm_reg->ccm_reg_backup.GpuHyd;

	ccm_reg->ccm_reg->PllLock	= ccm_reg->ccm_reg_backup.PllLock;
	ccm_reg->ccm_reg->Pll1Lock	= ccm_reg->ccm_reg_backup.Pll1Lock;
	ccm_reg->ccm_reg->PllStableStatus	= ccm_reg->ccm_reg_backup.PllStableStatus;

	change_runtime_env();
	delay_us(1);
	ccm_reg->ccm_reg->AhbGate0     	= ccm_reg->ccm_reg_backup.AhbGate0;
	ccm_reg->ccm_reg->AhbGate1     	= ccm_reg->ccm_reg_backup.AhbGate1;
	ccm_reg->ccm_reg->Apb1Gate     	= ccm_reg->ccm_reg_backup.Apb1Gate;
	ccm_reg->ccm_reg->Apb2Gate     	= ccm_reg->ccm_reg_backup.Apb2Gate;
	//config src.
	ccm_reg->ccm_reg->Cci400Clk	= ccm_reg->ccm_reg_backup.Cci400Clk; 
	delay_us(10);

	return 0;
}
#endif

#if defined(CONFIG_ARCH_SUN8IW10P1)
/*
*********************************************************************************************************
*                                       MEM CCU INITIALISE
*
* Description: mem interrupt initialise.
*
* Arguments  : none.
*
* Returns    : 0/-1;
*********************************************************************************************************
*/
__s32 mem_ccu_save(struct ccm_state *ccm_reg)
{
	ccm_reg->ccm_reg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);

	//bias:
	ccm_reg->ccm_reg_backup.PllAudioBias      	= ccm_reg->ccm_reg->PllAudioBias;
	ccm_reg->ccm_reg_backup.PllDeBias		= ccm_reg->ccm_reg->PllDeBias;
	ccm_reg->ccm_reg_backup.PllVideo0Bias      	= ccm_reg->ccm_reg->PllVideo0Bias;
	ccm_reg->ccm_reg_backup.PllVideo1Bias      	= ccm_reg->ccm_reg->PllVideo1Bias;

	//tune&pattern:
	ccm_reg->ccm_reg_backup.PllAudioPattern		= ccm_reg->ccm_reg->PllAudioPattern;

	//cfg:
	ccm_reg->ccm_reg_backup.Pll2Ctl      	= ccm_reg->ccm_reg->Pll2Ctl;
	ccm_reg->ccm_reg_backup.Pll9Ctl      	= ccm_reg->ccm_reg->Pll9Ctl;
	ccm_reg->ccm_reg_backup.Pll10Ctl      	= ccm_reg->ccm_reg->Pll10Ctl;

	//notice: be care to cpu, axi restore sequence!.
	ccm_reg->ccm_reg_backup.SysClkDiv    	= ccm_reg->ccm_reg->SysClkDiv;
	ccm_reg->ccm_reg_backup.Ahb1Div  	= ccm_reg->ccm_reg->Ahb1Div;
	ccm_reg->ccm_reg_backup.Apb2Div   	= ccm_reg->ccm_reg->Apb2Div;

	/*actually, the gating & reset ctrl reg
	** should not affect corresponding module's recovery.
	*/
	ccm_reg->ccm_reg_backup.AhbGate0     	= ccm_reg->ccm_reg->AhbGate0;
	ccm_reg->ccm_reg_backup.AhbGate1     	= ccm_reg->ccm_reg->AhbGate1;
	ccm_reg->ccm_reg_backup.Apb1Gate     	= ccm_reg->ccm_reg->Apb1Gate;

	ccm_reg->ccm_reg_backup.Nand0		= ccm_reg->ccm_reg->Nand0;

	ccm_reg->ccm_reg_backup.Sd0	    	= ccm_reg->ccm_reg->Sd0;
	ccm_reg->ccm_reg_backup.Sd1  		= ccm_reg->ccm_reg->Sd1;
	ccm_reg->ccm_reg_backup.Sd2    		= ccm_reg->ccm_reg->Sd2;
	ccm_reg->ccm_reg_backup.Sd3    		= ccm_reg->ccm_reg->Sd3;

	ccm_reg->ccm_reg_backup.Spi0      	= ccm_reg->ccm_reg->Spi0;
	ccm_reg->ccm_reg_backup.Spi1      	= ccm_reg->ccm_reg->Spi1;
	ccm_reg->ccm_reg_backup.Spi2      	= ccm_reg->ccm_reg->Spi2;

	ccm_reg->ccm_reg_backup.I2s0       	= ccm_reg->ccm_reg->I2s0;
	ccm_reg->ccm_reg_backup.I2s1       	= ccm_reg->ccm_reg->I2s1;

	ccm_reg->ccm_reg_backup.SpdifClk      	= ccm_reg->ccm_reg->SpdifClk;
	ccm_reg->ccm_reg_backup.Usb	      	= ccm_reg->ccm_reg->Usb;

	// do not touch dram related config.
	//  ccm_reg->ccm_reg_backup.PllDdrCfg	= ccm_reg->ccm_reg->PllDdrCfg;
	//  ccm_reg->ccm_reg_backup.MbusResetReg	= ccm_reg->ccm_reg->MbusResetReg;
	//  ccm_reg->ccm_reg_backup.DramCfg		= ccm_reg->ccm_reg->DramCfg;
	//  ccm_reg->ccm_reg_backup.DramGate     	= ccm_reg->ccm_reg->DramGate;

	ccm_reg->ccm_reg_backup.Lcd0Ch0   	= ccm_reg->ccm_reg->Lcd0Ch0;

	ccm_reg->ccm_reg_backup.CsiMisc      	= ccm_reg->ccm_reg->CsiMisc;
	ccm_reg->ccm_reg_backup.Csi0      	= ccm_reg->ccm_reg->Csi0;

	//mbus? function?
	//ccm_reg->ccm_reg_backup.MBus0    	= ccm_reg->ccm_reg->MBus0;

	ccm_reg->ccm_reg_backup.PllLock		= ccm_reg->ccm_reg->PllLock;
	ccm_reg->ccm_reg_backup.Pll1Lock	= ccm_reg->ccm_reg->Pll1Lock;

	ccm_reg->ccm_reg_backup.AhbReset0	= ccm_reg->ccm_reg->AhbReset0;
	ccm_reg->ccm_reg_backup.AhbReset1	= ccm_reg->ccm_reg->AhbReset1;
	ccm_reg->ccm_reg_backup.Apb1Reset	= ccm_reg->ccm_reg->Apb1Reset;

	return 0;
}

__s32 mem_ccu_restore(struct ccm_state *ccm_reg)
{
	//bias:
	ccm_reg->ccm_reg->PllAudioBias          = ccm_reg->ccm_reg_backup.PllAudioBias;
	ccm_reg->ccm_reg->PllDeBias             = ccm_reg->ccm_reg_backup.PllDeBias;
	ccm_reg->ccm_reg->PllVideo1Bias         = ccm_reg->ccm_reg_backup.PllVideo1Bias;

	//tune&pattern:
	ccm_reg->ccm_reg->PllAudioPattern		= ccm_reg->ccm_reg_backup.PllAudioPattern;

	//cfg
	ccm_reg->ccm_reg->Pll2Ctl      	= ccm_reg->ccm_reg_backup.Pll2Ctl;
	ccm_reg->ccm_reg->Pll9Ctl     	= ccm_reg->ccm_reg_backup.Pll9Ctl;
	ccm_reg->ccm_reg->Pll10Ctl    	= ccm_reg->ccm_reg_backup.Pll10Ctl;

	change_runtime_env();
	delay_us(1);

	//notice: be care to cpu, axi restore sequence!.
	ccm_reg->ccm_reg->SysClkDiv    	= ccm_reg->ccm_reg_backup.SysClkDiv;
	ccm_reg->ccm_reg->Ahb1Div  	= ccm_reg->ccm_reg_backup.Ahb1Div;
	delay_us(1);
	ccm_reg->ccm_reg->Apb2Div   	= ccm_reg->ccm_reg_backup.Apb2Div;

	/*actually, the gating & reset ctrl reg
	** should not affect corresponding module's recovery.
	*/
	//first, reset, then, gating.
	ccm_reg->ccm_reg->AhbReset0	= ccm_reg->ccm_reg_backup.AhbReset0;
	ccm_reg->ccm_reg->AhbReset1	= ccm_reg->ccm_reg_backup.AhbReset1;
	ccm_reg->ccm_reg->Apb1Reset	= ccm_reg->ccm_reg_backup.Apb1Reset;

	ccm_reg->ccm_reg->Nand0		= ccm_reg->ccm_reg_backup.Nand0;

	ccm_reg->ccm_reg->Sd0	    	= ccm_reg->ccm_reg_backup.Sd0;
	ccm_reg->ccm_reg->Sd1  		= ccm_reg->ccm_reg_backup.Sd1;
	ccm_reg->ccm_reg->Sd2    	= ccm_reg->ccm_reg_backup.Sd2;

	ccm_reg->ccm_reg->Spi0      	= ccm_reg->ccm_reg_backup.Spi0;
	ccm_reg->ccm_reg->Spi1      	= ccm_reg->ccm_reg_backup.Spi1;

	ccm_reg->ccm_reg->I2s0       	= ccm_reg->ccm_reg_backup.I2s0;
	ccm_reg->ccm_reg->I2s1       	= ccm_reg->ccm_reg_backup.I2s1;

	ccm_reg->ccm_reg->SpdifClk	= ccm_reg->ccm_reg_backup.SpdifClk;
	ccm_reg->ccm_reg->Usb	      	= ccm_reg->ccm_reg_backup.Usb;
#if 0
        ccm_reg->ccm_reg->DramCfg		= ccm_reg->ccm_reg_backup.DramCfg;
	ccm_reg->ccm_reg_backup.PllDdrCfg       = ccm_reg->ccm_reg->PllDdrCfg;
        ccm_reg->ccm_reg_backup.MbusResetReg    = ccm_reg->ccm_reg->MbusResetReg;
	ccm_reg->ccm_reg->DramGate		= ccm_reg->ccm_reg_backup.DramGate;
#endif

	ccm_reg->ccm_reg->Lcd0Ch0   	= ccm_reg->ccm_reg_backup.Lcd0Ch0;

	ccm_reg->ccm_reg->Csi0      	= ccm_reg->ccm_reg_backup.Csi0;
	//ccm_reg->ccm_reg->MBus0    	= ccm_reg->ccm_reg_backup.MBus0;

	ccm_reg->ccm_reg->PllLock	= ccm_reg->ccm_reg_backup.PllLock;
	ccm_reg->ccm_reg->Pll1Lock	= ccm_reg->ccm_reg_backup.Pll1Lock;

	change_runtime_env();
	delay_us(1);
	ccm_reg->ccm_reg->AhbGate0     	= ccm_reg->ccm_reg_backup.AhbGate0;
	ccm_reg->ccm_reg->AhbGate1     	= ccm_reg->ccm_reg_backup.AhbGate1;
	ccm_reg->ccm_reg->Apb1Gate     	= ccm_reg->ccm_reg_backup.Apb1Gate;
	delay_us(10);

	return 0;
}
#endif

#if defined(CONFIG_ARCH_SUN9IW1)
/*
*********************************************************************************************************
*                                       MEM CCU INITIALISE
*
* Description: mem interrupt initialise.
*
* Arguments  : none.
*
* Returns    : 0/-1;
*********************************************************************************************************
*/
__s32 mem_ccu_save(struct ccm_state *ccm_reg)
{
	ccm_reg->ccm_reg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);
	ccm_reg->ccm_mod_reg = (__ccmu_mod_reg_list_t *)IO_ADDRESS(SUNXI_CCM_MOD_PBASE);


	//module regs
	ccm_reg->ccm_mod_reg_backup.nand0_sclk0_cfg 	= ccm_reg->ccm_mod_reg->nand0_sclk0_cfg 	;
	ccm_reg->ccm_mod_reg_backup.nand0_sclk1_cfg     = ccm_reg->ccm_mod_reg->nand0_sclk1_cfg         ;
	ccm_reg->ccm_mod_reg_backup.nand1_sclk0_cfg 	= ccm_reg->ccm_mod_reg->nand1_sclk0_cfg 	;
	ccm_reg->ccm_mod_reg_backup.nand1_sclk1_cfg 	= ccm_reg->ccm_mod_reg->nand1_sclk1_cfg 	;
	ccm_reg->ccm_mod_reg_backup.sd0_clk         	= ccm_reg->ccm_mod_reg->sd0_clk         	;
	ccm_reg->ccm_mod_reg_backup.sd1_clk         	= ccm_reg->ccm_mod_reg->sd1_clk         	;
	ccm_reg->ccm_mod_reg_backup.sd2_clk         	= ccm_reg->ccm_mod_reg->sd2_clk         	;
	ccm_reg->ccm_mod_reg_backup.sd3_clk         	= ccm_reg->ccm_mod_reg->sd3_clk         	;
	ccm_reg->ccm_mod_reg_backup.ts_clk          	= ccm_reg->ccm_mod_reg->ts_clk          	;
	ccm_reg->ccm_mod_reg_backup.ss_clk          	= ccm_reg->ccm_mod_reg->ss_clk          	;
	ccm_reg->ccm_mod_reg_backup.spi0_clk            = ccm_reg->ccm_mod_reg->spi0_clk                ;
	ccm_reg->ccm_mod_reg_backup.spi1_clk        	= ccm_reg->ccm_mod_reg->spi1_clk        	;
	ccm_reg->ccm_mod_reg_backup.spi2_clk        	= ccm_reg->ccm_mod_reg->spi2_clk        	;
	ccm_reg->ccm_mod_reg_backup.spi3_clk        	= ccm_reg->ccm_mod_reg->spi3_clk        	;
	ccm_reg->ccm_mod_reg_backup.daudio0_clk     	= ccm_reg->ccm_mod_reg->daudio0_clk     	;
	ccm_reg->ccm_mod_reg_backup.daudio1_clk     	= ccm_reg->ccm_mod_reg->daudio1_clk     	;
	ccm_reg->ccm_mod_reg_backup.spdif_clk           = ccm_reg->ccm_mod_reg->spdif_clk               ;
	ccm_reg->ccm_mod_reg_backup.usbphy0_cfg         = ccm_reg->ccm_mod_reg->usbphy0_cfg             ;
	ccm_reg->ccm_mod_reg_backup.mdfs_clk            = ccm_reg->ccm_mod_reg->mdfs_clk                ;
	ccm_reg->ccm_mod_reg_backup.dram_cfg            = ccm_reg->ccm_mod_reg->dram_cfg                ;
	ccm_reg->ccm_mod_reg_backup.de_sclk_cfg         = ccm_reg->ccm_mod_reg->de_sclk_cfg             ;
	ccm_reg->ccm_mod_reg_backup.edp_sclk_cfg    	= ccm_reg->ccm_mod_reg->edp_sclk_cfg    	;
	ccm_reg->ccm_mod_reg_backup.mp_clk          	= ccm_reg->ccm_mod_reg->mp_clk          	;
	ccm_reg->ccm_mod_reg_backup.lcd0_clk            = ccm_reg->ccm_mod_reg->lcd0_clk                ;
	ccm_reg->ccm_mod_reg_backup.lcd1_clk            = ccm_reg->ccm_mod_reg->lcd1_clk                ;
	ccm_reg->ccm_mod_reg_backup.mipi_dsi_clk0   	= ccm_reg->ccm_mod_reg->mipi_dsi_clk0   	;
	ccm_reg->ccm_mod_reg_backup.mipi_dsi_clk1   	= ccm_reg->ccm_mod_reg->mipi_dsi_clk1   	;
	ccm_reg->ccm_mod_reg_backup.hdmi_sclk           = ccm_reg->ccm_mod_reg->hdmi_sclk               ;
	ccm_reg->ccm_mod_reg_backup.hdmi_slow_clk0  	= ccm_reg->ccm_mod_reg->hdmi_slow_clk0  	;
	ccm_reg->ccm_mod_reg_backup.mipi_csi_cfg    	= ccm_reg->ccm_mod_reg->mipi_csi_cfg    	;
	ccm_reg->ccm_mod_reg_backup.csi_isp_clk     	= ccm_reg->ccm_mod_reg->csi_isp_clk     	;
	ccm_reg->ccm_mod_reg_backup.csi0_mclk       	= ccm_reg->ccm_mod_reg->csi0_mclk       	;
	ccm_reg->ccm_mod_reg_backup.csi1_mclk       	= ccm_reg->ccm_mod_reg->csi1_mclk       	;
	ccm_reg->ccm_mod_reg_backup.fd_clk          	= ccm_reg->ccm_mod_reg->fd_clk          	;
	ccm_reg->ccm_mod_reg_backup.ve_clk          	= ccm_reg->ccm_mod_reg->ve_clk          	;
	ccm_reg->ccm_mod_reg_backup.avs_clk         	= ccm_reg->ccm_mod_reg->avs_clk         	;
	ccm_reg->ccm_mod_reg_backup.gpu_core_clk    	= ccm_reg->ccm_mod_reg->gpu_core_clk    	;
	ccm_reg->ccm_mod_reg_backup.gpu_mem_clk     	= ccm_reg->ccm_mod_reg->gpu_mem_clk     	;
	ccm_reg->ccm_mod_reg_backup.gpu_axi_clk         = ccm_reg->ccm_mod_reg->gpu_axi_clk             ;
	ccm_reg->ccm_mod_reg_backup.sata_clk            = ccm_reg->ccm_mod_reg->sata_clk                ;
	ccm_reg->ccm_mod_reg_backup.ac97_clk        	= ccm_reg->ccm_mod_reg->ac97_clk        	;
	ccm_reg->ccm_mod_reg_backup.mipi_hsi_clk    	= ccm_reg->ccm_mod_reg->mipi_hsi_clk    	;
	ccm_reg->ccm_mod_reg_backup.gp_adc          	= ccm_reg->ccm_mod_reg->gp_adc          	;
	ccm_reg->ccm_mod_reg_backup.cir_tx_clk      	= ccm_reg->ccm_mod_reg->cir_tx_clk      	;

	//module clk
	ccm_reg->ccm_mod_reg_backup.ahb0_gating     	= ccm_reg->ccm_mod_reg->ahb0_gating     	;
	ccm_reg->ccm_mod_reg_backup.ahb1_gating         = ccm_reg->ccm_mod_reg->ahb1_gating             ;
	ccm_reg->ccm_mod_reg_backup.ahb2_gating     	= ccm_reg->ccm_mod_reg->ahb2_gating     	;
	ccm_reg->ccm_mod_reg_backup.apb0_gating     	= ccm_reg->ccm_mod_reg->apb0_gating     	;
	ccm_reg->ccm_mod_reg_backup.apb1_gating     	= ccm_reg->ccm_mod_reg->apb1_gating     	;
	ccm_reg->ccm_mod_reg_backup.ahb0_rst            = ccm_reg->ccm_mod_reg->ahb0_rst                ;
	ccm_reg->ccm_mod_reg_backup.ahb1_rst        	= ccm_reg->ccm_mod_reg->ahb1_rst        	;
	ccm_reg->ccm_mod_reg_backup.ahb2_rst        	= ccm_reg->ccm_mod_reg->ahb2_rst        	;
	ccm_reg->ccm_mod_reg_backup.apb0_rst        	= ccm_reg->ccm_mod_reg->apb0_rst        	;
	ccm_reg->ccm_mod_reg_backup.apb1_rst            = ccm_reg->ccm_mod_reg->apb1_rst 		;

	return 0;
}

__s32 mem_ccu_restore(struct ccm_state *ccm_reg)
{
	//first, reset, then, gating.
	ccm_reg->ccm_mod_reg->ahb0_rst                = ccm_reg->ccm_mod_reg_backup.ahb0_rst            ;
	ccm_reg->ccm_mod_reg->ahb1_rst        	      = ccm_reg->ccm_mod_reg_backup.ahb1_rst        	;
	ccm_reg->ccm_mod_reg->ahb2_rst        	      = ccm_reg->ccm_mod_reg_backup.ahb2_rst        	;
	ccm_reg->ccm_mod_reg->apb0_rst        	      = ccm_reg->ccm_mod_reg_backup.apb0_rst        	;
	ccm_reg->ccm_mod_reg->apb1_rst                = ccm_reg->ccm_mod_reg_backup.apb1_rst            ;

	ccm_reg->ccm_mod_reg->nand0_sclk0_cfg 	      = ccm_reg->ccm_mod_reg_backup.nand0_sclk0_cfg 	;
	ccm_reg->ccm_mod_reg->nand0_sclk1_cfg         = ccm_reg->ccm_mod_reg_backup.nand0_sclk1_cfg     ;
	ccm_reg->ccm_mod_reg->nand1_sclk0_cfg 	      = ccm_reg->ccm_mod_reg_backup.nand1_sclk0_cfg 	;
	ccm_reg->ccm_mod_reg->nand1_sclk1_cfg 	      = ccm_reg->ccm_mod_reg_backup.nand1_sclk1_cfg 	;
	ccm_reg->ccm_mod_reg->sd0_clk         	      = ccm_reg->ccm_mod_reg_backup.sd0_clk         	;
	ccm_reg->ccm_mod_reg->sd1_clk         	      = ccm_reg->ccm_mod_reg_backup.sd1_clk         	;
	ccm_reg->ccm_mod_reg->sd2_clk         	      = ccm_reg->ccm_mod_reg_backup.sd2_clk         	;
	ccm_reg->ccm_mod_reg->sd3_clk         	      = ccm_reg->ccm_mod_reg_backup.sd3_clk         	;
	ccm_reg->ccm_mod_reg->ts_clk          	      = ccm_reg->ccm_mod_reg_backup.ts_clk          	;
	ccm_reg->ccm_mod_reg->ss_clk          	      = ccm_reg->ccm_mod_reg_backup.ss_clk          	;
	ccm_reg->ccm_mod_reg->spi0_clk                = ccm_reg->ccm_mod_reg_backup.spi0_clk            ;
	ccm_reg->ccm_mod_reg->spi1_clk        	      = ccm_reg->ccm_mod_reg_backup.spi1_clk        	;
	ccm_reg->ccm_mod_reg->spi2_clk        	      = ccm_reg->ccm_mod_reg_backup.spi2_clk        	;
	ccm_reg->ccm_mod_reg->spi3_clk        	      = ccm_reg->ccm_mod_reg_backup.spi3_clk        	;
	ccm_reg->ccm_mod_reg->daudio0_clk     	      = ccm_reg->ccm_mod_reg_backup.daudio0_clk     	;
	ccm_reg->ccm_mod_reg->daudio1_clk     	      = ccm_reg->ccm_mod_reg_backup.daudio1_clk     	;
	ccm_reg->ccm_mod_reg->spdif_clk               = ccm_reg->ccm_mod_reg_backup.spdif_clk           ;
	ccm_reg->ccm_mod_reg->usbphy0_cfg             = ccm_reg->ccm_mod_reg_backup.usbphy0_cfg         ;
	ccm_reg->ccm_mod_reg->mdfs_clk                = ccm_reg->ccm_mod_reg_backup.mdfs_clk            ;
	//ccm_reg->ccm_mod_reg->dram_cfg                = ccm_reg->ccm_mod_reg_backup.dram_cfg            ;
	ccm_reg->ccm_mod_reg->de_sclk_cfg             = ccm_reg->ccm_mod_reg_backup.de_sclk_cfg         ;
	ccm_reg->ccm_mod_reg->edp_sclk_cfg    	      = ccm_reg->ccm_mod_reg_backup.edp_sclk_cfg    	;
	ccm_reg->ccm_mod_reg->mp_clk          	      = ccm_reg->ccm_mod_reg_backup.mp_clk          	;
	ccm_reg->ccm_mod_reg->lcd0_clk                = ccm_reg->ccm_mod_reg_backup.lcd0_clk            ;
	ccm_reg->ccm_mod_reg->lcd1_clk                = ccm_reg->ccm_mod_reg_backup.lcd1_clk            ;
	ccm_reg->ccm_mod_reg->mipi_dsi_clk0   	      = ccm_reg->ccm_mod_reg_backup.mipi_dsi_clk0   	;
	ccm_reg->ccm_mod_reg->mipi_dsi_clk1   	      = ccm_reg->ccm_mod_reg_backup.mipi_dsi_clk1   	;
	ccm_reg->ccm_mod_reg->hdmi_sclk               = ccm_reg->ccm_mod_reg_backup.hdmi_sclk           ;
	ccm_reg->ccm_mod_reg->hdmi_slow_clk0  	      = ccm_reg->ccm_mod_reg_backup.hdmi_slow_clk0  	;
	ccm_reg->ccm_mod_reg->mipi_csi_cfg    	      = ccm_reg->ccm_mod_reg_backup.mipi_csi_cfg    	;
	ccm_reg->ccm_mod_reg->csi_isp_clk     	      = ccm_reg->ccm_mod_reg_backup.csi_isp_clk     	;
	ccm_reg->ccm_mod_reg->csi0_mclk       	      = ccm_reg->ccm_mod_reg_backup.csi0_mclk       	;
	ccm_reg->ccm_mod_reg->csi1_mclk       	      = ccm_reg->ccm_mod_reg_backup.csi1_mclk       	;
	ccm_reg->ccm_mod_reg->fd_clk          	      = ccm_reg->ccm_mod_reg_backup.fd_clk          	;
	ccm_reg->ccm_mod_reg->ve_clk          	      = ccm_reg->ccm_mod_reg_backup.ve_clk          	;
	ccm_reg->ccm_mod_reg->avs_clk         	      = ccm_reg->ccm_mod_reg_backup.avs_clk         	;
	ccm_reg->ccm_mod_reg->gpu_core_clk    	      = ccm_reg->ccm_mod_reg_backup.gpu_core_clk    	;
	ccm_reg->ccm_mod_reg->gpu_mem_clk     	      = ccm_reg->ccm_mod_reg_backup.gpu_mem_clk     	;
	ccm_reg->ccm_mod_reg->gpu_axi_clk             = ccm_reg->ccm_mod_reg_backup.gpu_axi_clk         ;
	ccm_reg->ccm_mod_reg->sata_clk                = ccm_reg->ccm_mod_reg_backup.sata_clk            ;
	ccm_reg->ccm_mod_reg->ac97_clk        	      = ccm_reg->ccm_mod_reg_backup.ac97_clk        	;
	ccm_reg->ccm_mod_reg->mipi_hsi_clk    	      = ccm_reg->ccm_mod_reg_backup.mipi_hsi_clk    	;
	ccm_reg->ccm_mod_reg->gp_adc          	      = ccm_reg->ccm_mod_reg_backup.gp_adc          	;
	ccm_reg->ccm_mod_reg->cir_tx_clk      	      = ccm_reg->ccm_mod_reg_backup.cir_tx_clk      	;

	//second, module clk related.
	change_runtime_env();
	delay_us(1);
	ccm_reg->ccm_mod_reg->ahb0_gating     	      = ccm_reg->ccm_mod_reg_backup.ahb0_gating     	;
	ccm_reg->ccm_mod_reg->ahb1_gating             = ccm_reg->ccm_mod_reg_backup.ahb1_gating         ;
	ccm_reg->ccm_mod_reg->ahb2_gating     	      = ccm_reg->ccm_mod_reg_backup.ahb2_gating     	;
	ccm_reg->ccm_mod_reg->apb0_gating     	      = ccm_reg->ccm_mod_reg_backup.apb0_gating     	;
	ccm_reg->ccm_mod_reg->apb1_gating     	      = ccm_reg->ccm_mod_reg_backup.apb1_gating     	;
	

	return 0;
}
#endif
