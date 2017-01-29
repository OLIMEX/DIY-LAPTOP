/*
**********************************************************************************************************************
*											        eGon
*						           the Embedded GO-ON Bootloader System
*									       eGON arm boot sub-system
*
*						  Copyright(C), 2006-2010, SoftWinners Microelectronic Co., Ltd.
*                                           All Rights Reserved
*
* File    : nand_osal_boot0.c
*
* By      : Jerry
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/
#include "../../nand_common.h"

extern int printf(const char *fmt, ...);
#define NAND_Print(fmt, args...)        printf(fmt,##args)

#define get_wvalue(addr)	(*((volatile unsigned long  *)(addr)))
#define put_wvalue(addr, v)	(*((volatile unsigned long  *)(addr)) = (unsigned long)(v))

u32 _get_pll4_periph1_clk(void)
{
#if 1
	return 24000000;
#else
	u32 n,div1,div2;
	u32 rval;

	rval = get_wvalue((0x06000000 + 0xC)); //read pll4-periph1 register

	n = (0xff & (rval >> 8));
	div1 = (0x1 & (rval >> 16));
	div2 = (0x1 & (rval >> 18));

	rval = 24000000 * n / (div1+1) / (div2+1);;

	return rval; //24000000 * n / (div1+1) / (div2+1);
#endif
}

__u32 _Getpll6Clk(void)
{
	return 600;
}

#ifndef CONFIG_ARCH_SUN9IW1P1
int NAND_ClkRequest(__u32 nand_index)
{
	__u32 cfg;
	__u32 m, n;
	__u32 clk_src;

	clk_src = 0;
	if(nand_index == 0)
	{
		//10M
		if(clk_src == 0)
		{
			m = 0;
			n = 0;
		}
		else
		{
			m = 14;
			n = 1;
		}


		/*set nand clock gate on*/
		cfg = 0;

		/*gate on nand clock*/
		cfg |= (1U << 31);
		/*take pll6 as nand src block*/
		cfg |=  ((clk_src&0x3) << 24);

		//set divn = 0
		cfg |= (n&0x3)<<16;
		cfg |= (m&0xf)<<0;

		*(volatile __u32 *)(0x01c20000 + 0x80) = cfg;

		//open ahb
		cfg = *(volatile __u32 *)(0x01c20000 + 0x60);
		cfg |= (0x1<<13);
		*(volatile __u32 *)(0x01c20000 + 0x60) = cfg;

		//reset
		cfg = *(volatile __u32 *)(0x01c20000 + 0x2c0);
		cfg &= (~(0x1<<13));
		*(volatile __u32 *)(0x01c20000 + 0x2c0) = cfg;

		cfg = *(volatile __u32 *)(0x01c20000 + 0x2c0);
		cfg |= (0x1<<13);
		*(volatile __u32 *)(0x01c20000 + 0x2c0) = cfg;

		cfg = *(volatile __u32 *)(0x01c20000 + 0x2c0);
		cfg &= (~(0x1<<13));
		*(volatile __u32 *)(0x01c20000 + 0x2c0) = cfg;

		cfg = *(volatile __u32 *)(0x01c20000 + 0x2c0);
		cfg |= (0x1<<13);
		*(volatile __u32 *)(0x01c20000 + 0x2c0) = cfg;

		NAND_Print("NAND_ClkRequest, nand_index: 0x%x\n", nand_index);
		NAND_Print("Reg 0x01c20080: 0x%x\n", *(volatile __u32 *)(0x01c20080));
		NAND_Print("Reg 0x01c20060: 0x%x\n", *(volatile __u32 *)(0x01c20060));
		NAND_Print("Reg 0x01c202c0: 0x%x\n", *(volatile __u32 *)(0x01c202c0));
	}
	else
	{
		//10M
		if(clk_src == 0)
		{
			m = 0;
			n = 0;
		}
		else
		{
			m = 14;
			n = 1;
		}

		/*set nand clock gate on*/
		cfg = 0;

		/*gate on nand clock*/
		cfg |= (1U << 31);
		/*take pll6 as nand src block*/
		cfg |=  ((clk_src&0x3) << 24);
		//set divn = 0
		cfg |= (n&0x3)<<16;
		cfg |= (m&0xf)<<0;

		*(volatile __u32 *)(0x01c20000 + 0x84) = cfg;

		//open ahb
		cfg = *(volatile __u32 *)(0x01c20000 + 0x60);
		cfg |= (0x1<<12);
		*(volatile __u32 *)(0x01c20000 + 0x60) = cfg;

		//open reset
		cfg = *(volatile __u32 *)(0x01c20000 + 0x2c0);
		cfg &= (~(0x1<<12));
		*(volatile __u32 *)(0x01c20000 + 0x2c0) = cfg;

		cfg = *(volatile __u32 *)(0x01c20000 + 0x2c0);
		cfg |= (0x1<<12);
		*(volatile __u32 *)(0x01c20000 + 0x2c0) = cfg;

		cfg = *(volatile __u32 *)(0x01c20000 + 0x2c0);
		cfg &= (~(0x1<<12));
		*(volatile __u32 *)(0x01c20000 + 0x2c0) = cfg;

		cfg = *(volatile __u32 *)(0x01c20000 + 0x2c0);
		cfg |= (0x1<<12);
		*(volatile __u32 *)(0x01c20000 + 0x2c0) = cfg;

		NAND_Print("NAND_ClkRequest, nand_index: 0x%x\n", nand_index);
		NAND_Print("Reg 0x01c20084: 0x%x\n", *(volatile __u32 *)(0x01c20084));
		NAND_Print("Reg 0x01c20060: 0x%x\n", *(volatile __u32 *)(0x01c20060));
		NAND_Print("Reg 0x01c202c0: 0x%x\n", *(volatile __u32 *)(0x01c202c0));
	}


	return 0;
}
#else
int NAND_ClkRequest(__u32 nand_index)
{
	u32 reg_val;
	u32 dclk_src_sel, dclk, cclk_src_sel, cclk;
	u32 sclk0_src_sel, sclk0, sclk0_src, sclk0_pre_ratio_n, sclk0_src_t, sclk0_ratio_m;
	u32 sclk1_src_sel, sclk1, sclk1_src, sclk1_pre_ratio_n, sclk1_src_t, sclk1_ratio_m;
	u32 sclk0_reg_adr, sclk1_reg_adr;

	/*
		1. release ahb reset and open ahb clock gate
	*/
	if (nand_index == 0) {
		// reset
		reg_val = *(volatile __u32 *)(0x06000400 + 0x1A0);
		reg_val &= (~(0x1U<<13));
		reg_val |= (0x1U<<13);
		*(volatile __u32 *)(0x06000400 + 0x1A0) = reg_val;
		// ahb clock gate
		reg_val = *(volatile __u32 *)(0x06000400 + 0x180);
		reg_val &= (~(0x1U<<13));
		reg_val |= (0x1U<<13);
		*(volatile __u32 *)(0x06000400 + 0x180) = reg_val;
	} else if (nand_index == 1) {
		// reset
		reg_val = *(volatile __u32 *)(0x06000400 + 0x1A0);
		reg_val &= (~(0x1U<<12));
		reg_val |= (0x1U<<12);
		*(volatile __u32 *)(0x06000400 + 0x1A0) = reg_val;
		// ahb clock gate
		reg_val = *(volatile __u32 *)(0x06000400 + 0x180);
		reg_val &= (~(0x1U<<12));
		reg_val |= (0x1U<<12);
		*(volatile __u32 *)(0x06000400 + 0x180) = reg_val;
	} else {
		NAND_Print("NAND_ClkRequest error--1, wrong nand index: %d\n", nand_index);
		return -1;
	}


	/*
		2. configure ndfc's sclk0 and sclk1
	*/
	////////////////////////////////////////////////
	dclk_src_sel = 0;
	dclk = 12;
	cclk_src_sel = 0;
	cclk = 24;
	////////////////////////////////////////////////

	if (nand_index == 0) {
		sclk0_reg_adr = (0x06000400 + 0x0); //CCM_NAND0_CLK0_REG;
		sclk1_reg_adr = (0x06000400 + 0x4); //CCM_NAND0_CLK1_REG;
	} else if (nand_index == 1) {
		sclk0_reg_adr = (0x06000400 + 0x8); //CCM_NAND1_CLK0_REG;
		sclk1_reg_adr = (0x06000400 + 0xC); //CCM_NAND1_CLK1_REG;
	} else {
		NAND_Print("NAND_ClkRequest error--2, wrong nand index: %d\n", nand_index);
		return -1;
	}

	/*close dclk and cclk*/
	if ((dclk == 0) && (cclk == 0))
	{
		reg_val = get_wvalue(sclk0_reg_adr);
		reg_val &= (~(0x1U<<31));
		put_wvalue(sclk0_reg_adr, reg_val);

		reg_val = get_wvalue(sclk1_reg_adr);
		reg_val &= (~(0x1U<<31));
		put_wvalue(sclk1_reg_adr, reg_val);
		return 0;
	}

	sclk0_src_sel = dclk_src_sel;
	sclk0 = dclk*2; //set sclk0 to 2*dclk.
	sclk1_src_sel = cclk_src_sel;
	sclk1 = cclk;

	if(sclk0_src_sel == 0x0) {
		//osc pll
        sclk0_src = 24;
	} else {
		//pll6
		sclk0_src = _get_pll4_periph1_clk()/1000000;
	}

	if(sclk1_src_sel == 0x0) {
		//osc pll
        sclk1_src = 24;
	} else {
		//pll6
		sclk1_src = _get_pll4_periph1_clk()/1000000;
	}

	//////////////////// sclk0: 2*dclk
	//sclk0_pre_ratio_n
	sclk0_pre_ratio_n = 3;
	if(sclk0_src > 4*16*sclk0)
		sclk0_pre_ratio_n = 3;
	else if (sclk0_src > 2*16*sclk0)
		sclk0_pre_ratio_n = 2;
	else if (sclk0_src > 1*16*sclk0)
		sclk0_pre_ratio_n = 1;
	else
		sclk0_pre_ratio_n = 0;

	sclk0_src_t = sclk0_src>>sclk0_pre_ratio_n;

	//sclk0_ratio_m
	sclk0_ratio_m = (sclk0_src_t/(sclk0)) - 1;
    if( sclk0_src_t%(sclk0) )
    	sclk0_ratio_m +=1;


	//////////////// sclk1: cclk
	//sclk1_pre_ratio_n
	sclk1_pre_ratio_n = 3;
	if(sclk1_src > 4*16*sclk1)
		sclk1_pre_ratio_n = 3;
	else if (sclk1_src > 2*16*sclk1)
		sclk1_pre_ratio_n = 2;
	else if (sclk1_src > 1*16*sclk1)
		sclk1_pre_ratio_n = 1;
	else
		sclk1_pre_ratio_n = 0;

	sclk1_src_t = sclk1_src>>sclk1_pre_ratio_n;

	//sclk1_ratio_m
	sclk1_ratio_m = (sclk1_src_t/(sclk1)) - 1;
    if( sclk1_src_t%(sclk1) )
    	sclk1_ratio_m +=1;

	/////////////////////////////// close clock
	reg_val = get_wvalue(sclk0_reg_adr);
	reg_val &= (~(0x1U<<31));
	put_wvalue(sclk0_reg_adr, reg_val);

	reg_val = get_wvalue(sclk1_reg_adr);
	reg_val &= (~(0x1U<<31));
	put_wvalue(sclk1_reg_adr, reg_val);


	///////////////////////////////configure
	//sclk0 <--> 2*dclk
	reg_val = get_wvalue(sclk0_reg_adr);
	//clock source select
	reg_val &= (~(0x3<<24));
	reg_val |= (sclk0_src_sel&0x3)<<24;
	//clock pre-divide ratio(N)
	reg_val &= (~(0x3<<16));
	reg_val |= (sclk0_pre_ratio_n&0x3)<<16;
	//clock divide ratio(M)
	reg_val &= ~(0xf<<0);
	reg_val |= (sclk0_ratio_m&0xf)<<0;
	put_wvalue(sclk0_reg_adr, reg_val);

	//sclk1 <--> cclk
	reg_val = get_wvalue(sclk1_reg_adr);
	//clock source select
	reg_val &= (~(0x3<<24));
	reg_val |= (sclk1_src_sel&0x3)<<24;
	//clock pre-divide ratio(N)
	reg_val &= (~(0x3<<16));
	reg_val |= (sclk1_pre_ratio_n&0x3)<<16;
	//clock divide ratio(M)
	reg_val &= ~(0xf<<0);
	reg_val |= (sclk1_ratio_m&0xf)<<0;
	put_wvalue(sclk1_reg_adr, reg_val);


	/////////////////////////////// open clock
	reg_val = get_wvalue(sclk0_reg_adr);
	reg_val |= 0x1U<<31;
	put_wvalue(sclk0_reg_adr, reg_val);

	reg_val = get_wvalue(sclk1_reg_adr);
	reg_val |= 0x1U<<31;
	put_wvalue(sclk1_reg_adr, reg_val);

	return 0;
}
#endif

void NAND_ClkRelease(__u32 nand_index)
{
    return ;
}

/*
**********************************************************************************************************************
*
*             NAND_GetCmuClk
*
*  Description:
*
*
*  Parameters:
*
*
*  Return value:
*
*
**********************************************************************************************************************
*/
#ifndef CONFIG_ARCH_SUN9IW1P1
int NAND_SetClk(__u32 nand_index, __u32 nand_clock)
{
	__u32 edo_clk;
	__u32 cfg;
	__u32 m = 0, n = 0;
	__u32 clk_src;


	clk_src = 0;

	if(clk_src == 0)
	{
	}
	else
	{
		edo_clk = nand_clock * 2;
		if(edo_clk <= 20)  //10M
		{
			n =  1;
			m = 14;
		}
		else if((edo_clk >20)&&(edo_clk <= 40))  //20M
		{
			n =  0;
			m = 14;
		}
		else if((edo_clk >40)&&(edo_clk <= 50))  //25M
		{
			n =  0;
			m = 11;
		}
		else if((edo_clk >50)&&(edo_clk <= 60))  //30M
		{
			n = 0;
			m = 9;
		}
		else //40M
		{
			n = 0;
			m = 7;
		}
	}


	if(nand_index == 0)
	{
		/*set nand clock*/
		/*set nand clock gate on*/
		cfg = *(volatile __u32 *)(0x01c20000 + 0x80);
		cfg &= (~(0x03 << 16));
		cfg &= (~(0xf));
		cfg |= ((n&0x3)<<16);
		cfg |= ((m&0xf));

		*(volatile __u32 *)(0x01c20000 + 0x80) = cfg;
		NAND_Print("NAND_SetClk, nand_index: 0x%x\n", nand_index);
		NAND_Print("Reg 0x01c20080: 0x%x\n", *(volatile __u32 *)(0x01c20080));

	}
	else
	{
		/*set nand clock*/
		/*set nand clock gate on*/
		cfg = *(volatile __u32 *)(0x01c20000 + 0x84);
		cfg &= (~(0x03 << 16));
		cfg &= (~(0xf));
		cfg |= ((n&0x3)<<16);
		cfg |= ((m&0xf));

		*(volatile __u32 *)(0x01c20000 + 0x84) = cfg;
		NAND_Print("NAND_SetClk, nand_index: 0x%x\n", nand_index);
		NAND_Print("Reg 0x01c20084: 0x%x\n", *(volatile __u32 *)(0x01c20084));

	}

	return 0;
}
#else
int NAND_SetClk(__u32 nand_index, __u32 nand_clock)
{
	u32 reg_val;
	u32 dclk_src_sel, dclk, cclk_src_sel, cclk;
	u32 sclk0_src_sel, sclk0, sclk0_src, sclk0_pre_ratio_n, sclk0_src_t, sclk0_ratio_m;
	u32 sclk1_src_sel, sclk1, sclk1_src, sclk1_pre_ratio_n, sclk1_src_t, sclk1_ratio_m;
	u32 sclk0_reg_adr, sclk1_reg_adr;

	////////////////////////////////////////////////
	dclk_src_sel = 0;
	dclk = nand_clock;
	cclk_src_sel = 0;
	cclk = nand_clock*2;
	////////////////////////////////////////////////

	if (nand_index == 0) {
		sclk0_reg_adr = (0x06000400 + 0x0); //CCM_NAND0_CLK0_REG;
		sclk1_reg_adr = (0x06000400 + 0x4); //CCM_NAND0_CLK1_REG;
	} else if (nand_index == 1) {
		sclk0_reg_adr = (0x06000400 + 0x8); //CCM_NAND1_CLK0_REG;
		sclk1_reg_adr = (0x06000400 + 0xC); //CCM_NAND1_CLK1_REG;
	} else {
		return -1;
	}

	/*close dclk and cclk*/
	if ((dclk == 0) && (cclk == 0))
	{
		reg_val = get_wvalue(sclk0_reg_adr);
		reg_val &= (~(0x1U<<31));
		put_wvalue(sclk0_reg_adr, reg_val);

		reg_val = get_wvalue(sclk1_reg_adr);
		reg_val &= (~(0x1U<<31));
		put_wvalue(sclk1_reg_adr, reg_val);
		return 0;
	}

	sclk0_src_sel = dclk_src_sel;
	sclk0 = dclk*2; //set sclk0 to 2*dclk.
	sclk1_src_sel = cclk_src_sel;
	sclk1 = cclk;

	if(sclk0_src_sel == 0x0) {
		//osc pll
        sclk0_src = 24;
	} else {
		//pll6
		sclk0_src = _get_pll4_periph1_clk()/1000000;
	}

	if(sclk1_src_sel == 0x0) {
		//osc pll
        sclk1_src = 24;
	} else {
		//pll6
		sclk1_src = _get_pll4_periph1_clk()/1000000;
	}

	//////////////////// sclk0: 2*dclk
	//sclk0_pre_ratio_n
	sclk0_pre_ratio_n = 3;
	if(sclk0_src > 4*16*sclk0)
		sclk0_pre_ratio_n = 3;
	else if (sclk0_src > 2*16*sclk0)
		sclk0_pre_ratio_n = 2;
	else if (sclk0_src > 1*16*sclk0)
		sclk0_pre_ratio_n = 1;
	else
		sclk0_pre_ratio_n = 0;

	sclk0_src_t = sclk0_src>>sclk0_pre_ratio_n;

	//sclk0_ratio_m
	sclk0_ratio_m = (sclk0_src_t/(sclk0)) - 1;
    if( sclk0_src_t%(sclk0) )
    	sclk0_ratio_m +=1;


	//////////////// sclk1: cclk
	//sclk1_pre_ratio_n
	sclk1_pre_ratio_n = 3;
	if(sclk1_src > 4*16*sclk1)
		sclk1_pre_ratio_n = 3;
	else if (sclk1_src > 2*16*sclk1)
		sclk1_pre_ratio_n = 2;
	else if (sclk1_src > 1*16*sclk1)
		sclk1_pre_ratio_n = 1;
	else
		sclk1_pre_ratio_n = 0;

	sclk1_src_t = sclk1_src>>sclk1_pre_ratio_n;

	//sclk1_ratio_m
	sclk1_ratio_m = (sclk1_src_t/(sclk1)) - 1;
    if( sclk1_src_t%(sclk1) )
    	sclk1_ratio_m +=1;

	/////////////////////////////// close clock
	reg_val = get_wvalue(sclk0_reg_adr);
	reg_val &= (~(0x1U<<31));
	put_wvalue(sclk0_reg_adr, reg_val);

	reg_val = get_wvalue(sclk1_reg_adr);
	reg_val &= (~(0x1U<<31));
	put_wvalue(sclk1_reg_adr, reg_val);


	///////////////////////////////configure
	//sclk0 <--> 2*dclk
	reg_val = get_wvalue(sclk0_reg_adr);
	//clock source select
	reg_val &= (~(0x3<<24));
	reg_val |= (sclk0_src_sel&0x3)<<24;
	//clock pre-divide ratio(N)
	reg_val &= (~(0x3<<16));
	reg_val |= (sclk0_pre_ratio_n&0x3)<<16;
	//clock divide ratio(M)
	reg_val &= ~(0xf<<0);
	reg_val |= (sclk0_ratio_m&0xf)<<0;
	put_wvalue(sclk0_reg_adr, reg_val);

	//sclk1 <--> cclk
	reg_val = get_wvalue(sclk1_reg_adr);
	//clock source select
	reg_val &= (~(0x3<<24));
	reg_val |= (sclk1_src_sel&0x3)<<24;
	//clock pre-divide ratio(N)
	reg_val &= (~(0x3<<16));
	reg_val |= (sclk1_pre_ratio_n&0x3)<<16;
	//clock divide ratio(M)
	reg_val &= ~(0xf<<0);
	reg_val |= (sclk1_ratio_m&0xf)<<0;
	put_wvalue(sclk1_reg_adr, reg_val);


	/////////////////////////////// open clock
	reg_val = get_wvalue(sclk0_reg_adr);
	reg_val |= 0x1U<<31;
	put_wvalue(sclk0_reg_adr, reg_val);

	reg_val = get_wvalue(sclk1_reg_adr);
	reg_val |= 0x1U<<31;
	put_wvalue(sclk1_reg_adr, reg_val);

	return 0;
}
#endif

int NAND_GetClk(__u32 nand_index)
{
	__u32 pll6_clk;
	__u32 cfg;
	__u32 nand_max_clock;
	__u32 m, n;

	if(nand_index == 0)
	{
		/*set nand clock*/
	    pll6_clk = _Getpll6Clk();

	    /*set nand clock gate on*/
		cfg = *(volatile __u32 *)(0x01c20000 + 0x80);
	    m = ((cfg)&0xf) +1;
	    n = ((cfg>>16)&0x3);
	    nand_max_clock = pll6_clk/(2*(1<<n)*(m+1));
		NAND_Print("NAND_GetClk, nand_index: 0x%x, nand_clk: %dM\n", nand_index, nand_max_clock);
		NAND_Print("Reg 0x01c20080: 0x%x\n", *(volatile __u32 *)(0x01c20080));


	}
	else
	{
		/*set nand clock*/
	    pll6_clk = _Getpll6Clk();

	    /*set nand clock gate on*/
		cfg = *(volatile __u32 *)(0x01c20000 + 0x84);
	    m = ((cfg)&0xf) +1;
	    n = ((cfg>>16)&0x3);
	    nand_max_clock = pll6_clk/(2*(1<<n)*(m+1));
		NAND_Print("NAND_GetClk, nand_index: 0x%x, nand_clk: %dM\n", nand_index, nand_max_clock);
		NAND_Print("Reg 0x01c20084: 0x%x\n", *(volatile __u32 *)(0x01c20084));
	}

	return nand_max_clock;
}

#ifndef CONFIG_ARCH_SUN9IW1P1
void NAND_PIORequest(__u32 nand_index)
{
	__u32 cfg;

	if(nand_index == 0)
	{
		*(volatile __u32 *)(0x01c20800 + 0x48) = 0x22222222;
		*(volatile __u32 *)(0x01c20800 + 0x4c) = 0x22222222;
		*(volatile __u32 *)(0x01c20800 + 0x50) = 0x00000222;
		cfg = *(volatile __u32 *)(0x01c20800 + 0x54);
		cfg &= (~0xfff);
		cfg |= 0x222;
		*(volatile __u32 *)(0x01c20800 + 0x54) = cfg;
		//NAND_Print("NAND_PIORequest, nand_index: 0x%x\n", nand_index);
		//NAND_Print("Reg 0x01c20848: 0x%x\n", *(volatile __u32 *)(0x01c20848));
		//NAND_Print("Reg 0x01c2084c: 0x%x\n", *(volatile __u32 *)(0x01c2084c));
		//NAND_Print("Reg 0x01c20850: 0x%x\n", *(volatile __u32 *)(0x01c20850));
		//NAND_Print("Reg 0x01c20854: 0x%x\n", *(volatile __u32 *)(0x01c20854));
	}
	else if(nand_index == 1)
	{
		*(volatile __u32 *)(0x01c20800 + 0x50) = 0x33333333;
		*(volatile __u32 *)(0x01c20800 + 0xfc) = 0x22222222;
		cfg = *(volatile __u32 *)(0x01c20800 + 0x100);
		cfg &= (~0xf);
		cfg |= (0x2);
		*(volatile __u32 *)(0x01c20800 + 0x100) = cfg;
		cfg = *(volatile __u32 *)(0x01c20800 + 0x108);
		cfg &= (~0xfU<<20);
		cfg |= (0x22<<20);
		*(volatile __u32 *)(0x01c20800 + 0x108) = cfg;

		//NAND_Print("NAND_PIORequest, nand_index: 0x%x\n", nand_index);
		//NAND_Print("Reg 0x01c20850: 0x%x\n", *(volatile __u32 *)(0x01c20850));
		//NAND_Print("Reg 0x01c208fc: 0x%x\n", *(volatile __u32 *)(0x01c208fc));
		//NAND_Print("Reg 0x01c20900: 0x%x\n", *(volatile __u32 *)(0x01c20900));
		//NAND_Print("Reg 0x01c20908: 0x%x\n", *(volatile __u32 *)(0x01c20908));
	}
	else
	{

	}

}
#else
void NAND_PIORequest(__u32 nand_index)
{
	__u32 cfg;

	if(nand_index == 0)
	{
		*(volatile __u32 *)(0x06000800 + 0x48) = 0x22222222;
		*(volatile __u32 *)(0x06000800 + 0x4c) = 0x22222222;
		cfg = *(volatile __u32 *)(0x06000800 + 0x50);
		cfg &= (~0xfff);
		cfg |= 0x222;
		*(volatile __u32 *)(0x06000800 + 0x50) = cfg;

		//pull-up/down
		*(volatile __u32 *)(0x06000800 + 0x64) = 0x00005140;
		cfg = *(volatile __u32 *)(0x06000800 + 0x68);
		cfg &= (~0xfff);
		cfg |= 0x014;
		*(volatile __u32 *)(0x06000800 + 0x68) = cfg;

		*(volatile __u32 *)(0x06000800 + 0x308) = 0xa;

		//NAND_Print("NAND_PIORequest, nand_index: 0x%x\n", nand_index);
		//NAND_Print("Reg 0x06000848: 0x%x\n", *(volatile __u32 *)(0x06000848));
		//NAND_Print("Reg 0x0600084c: 0x%x\n", *(volatile __u32 *)(0x0600084c));
		//NAND_Print("Reg 0x06000850: 0x%x\n", *(volatile __u32 *)(0x06000850));
		//NAND_Print("Reg 0x06000864: 0x%x\n", *(volatile __u32 *)(0x06000864));
		//NAND_Print("Reg 0x06000868: 0x%x\n", *(volatile __u32 *)(0x06000868));
		//NAND_Print("Reg 0x06000b08: 0x%x\n", *(volatile __u32 *)(0x06000b08));
	}
	else if(nand_index == 1)
	{
		*(volatile __u32 *)(0x06000800 + 0x120) = 0x22222222;
		*(volatile __u32 *)(0x06000800 + 0x124) = 0x22222222;
		cfg = *(volatile __u32 *)(0x06000800 + 0x128);
		cfg &= (~0xfff);
		cfg |= 0x222;
		*(volatile __u32 *)(0x06000800 + 0x128) = cfg;

		//pull-up/down
		*(volatile __u32 *)(0x06000800 + 0x13c) = 0x00005140;
		*(volatile __u32 *)(0x06000800 + 0x140) = 0x014;

		//NAND_Print("NAND_PIORequest, nand_index: 0x%x\n", nand_index);
		//NAND_Print("Reg 0x06000920: 0x%x\n", *(volatile __u32 *)(0x06000920));
		//NAND_Print("Reg 0x06000924: 0x%x\n", *(volatile __u32 *)(0x06000924));
		//NAND_Print("Reg 0x06000928: 0x%x\n", *(volatile __u32 *)(0x06000928));
		//NAND_Print("Reg 0x0600093c: 0x%x\n", *(volatile __u32 *)(0x0600093c));
		//NAND_Print("Reg 0x06000940: 0x%x\n", *(volatile __u32 *)(0x06000940));
	}
	else
	{
		NAND_Print("NAND_PIORequest error, wrong nand_index: 0x%x\n", nand_index);
	}

}
#endif

void NAND_PIORelease(__u32 nand_index)
{
	return;
}

void NAND_EnRbInt(void)
{
	return ;
}


void NAND_ClearRbInt(void)
{
	return ;
}

int NAND_WaitRbReady(void)
{
	return 0;
}

int NAND_WaitDmaFinish(void)
{
	return 0;
}

void NAND_RbInterrupt(void)
{
	return ;
}
/*
************************************************************************************************************
*
*                                             OSAL_malloc
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ： 这是一个虚假的malloc函数，目的只是提供这样一个函数，避免编译不通过
*               本身不提供任何的函数功能
*
************************************************************************************************************
*/
void* NAND_Malloc(unsigned int Size)
{
	//return (void *)malloc(Size);
	return (void *)CONFIG_SYS_SDRAM_BASE;
}
/*
************************************************************************************************************
*
*                                             OSAL_free
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ： 这是一个虚假的free函数，目的只是提供这样一个函数，避免编译不通过
*               本身不提供任何的函数功能
*
************************************************************************************************************
*/
void NAND_Free(void *pAddr, unsigned int Size)
{
    //return free(pAddr);
}

void *NAND_IORemap(unsigned int base_addr, unsigned int size)
{
    return (void *)base_addr;
}
/*
**********************************************************************************************************************
*
*             OSAL_printf
*
*  Description:  用户可以自行设定是否需要打印
*
*
*  Parameters:
*
*
*  Return value:
*
*
**********************************************************************************************************************
*/
#if 0
__s32 NAND_Print(const char * str, ...)
{
	printf(str);
    return 0;
}
#endif
