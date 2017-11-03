//*****************************************************************************
//	Allwinner Technology, All Right Reserved. 2006-2010 Copyright (c)
//
//	File: 				mctl_hal.c
//
//	Description:  This file implements basic functions for AW1667 DRAM controller
//		 
//	History:
//				2013/09/18		Golden			0.10	Initial version
//*****************************************************************************
#include <common.h>
#include "mctl_hal.h"
#include "mctl_reg.h"
#include "mctl_par.h"

unsigned int delay[4];
void local_delay (unsigned int n)
{
	while(n--);
}
void paraconfig(unsigned int *para, unsigned int mask, unsigned int value)
{
	*para &= ~(mask);
	*para |= value;
}

#ifdef AUTO_SET_DRAM_TEST_PARA
void auto_set_test_para(__dram_para_t *para)
{
	//u32 val;

#if defined SOURCE_CLK_USE_PLL_PERI0 || defined DRAMC_PLL_PERI0_TEST
	para->dram_tpr13 &= ~(0x3<<22);	//PLL_PERIO
	para->dram_tpr13 |= (0x1<<22);
#endif

#if defined SOURCE_CLK_USE_DDR_PLL0 || defined DRAMC_DDR_PLL0_TEST
	para->dram_tpr13 &= ~(0x3<<22);	//USE DDR_PLL0
	para->dram_tpr13 |= (0x1<<22);
#endif

#if defined SOURCE_CLK_USE_DDR_PLL1 || defined  DRAMC_DDR_PLL1_TEST
	para->dram_tpr13 &= ~(0x3<<22);	//USE DDR_PLL1
	para->dram_tpr13 |= (0x0<<22);
#endif

#if defined DRAMC_PHY_BIST_TEST
	para->dram_tpr13 &= ~(0x7<<2);	//use dqs traing mode
	para->dram_tpr13 |= (1<<2);

	para->dram_tpr13 &= ~(0x1<<5);	//use 1T mode
	para->dram_tpr13 |= (0x1<<5);
#endif

#if defined DRAMC_USE_AUTO_DQS_GATE_PD_MODE || defined DRAMC_AUTO_DQS_GATE_PD_MODE_TEST || defined DRAMC_MDFS_DFS_TEST		//use auto gating pull down mode
	para->dram_tpr13 &= ~(0x7<<2);
	para->dram_tpr13 |= (0<<2);
#endif

#if defined DRAMC_USE_DQS_GATING_MODE || defined DRAMC_AUTO_DQS_GATE_PU_MODE_TEST		//use gate training mode
	para->dram_tpr13 &= ~(0x7<<2);
	para->dram_tpr13 |= (1<<2);
#endif

#if defined DRAMC_USE_AUTO_DQS_GATE_PU_MODE || defined DRAMC_AUTO_DQS_GATE_PU_MODE_TEST		//use auto gating pull up mode
	para->dram_tpr13 &= ~(0x7<<2);
	para->dram_tpr13 |= (2<<2);
#endif

#ifdef DRAMC_USE_1T_MODE
	para->dram_tpr13 &= ~(0x1<<5);	//use 1T mode
	para->dram_tpr13 |= (0x1<<5);
#endif

#ifdef DRAMC_HALF_DQ_TEST					//use half dq mode,page_size/2
	para->dram_para2 |= 1;
	val =((para->dram_para1 >> 16) & 0xf)/2;
	para->dram_para1 = (para->dram_para1 & (~(0xf << 16))) | val;
#endif
#ifdef DRAMC_MDFS_CFS_TEST
	para->dram_tpr13 |= (0x1<<10);
#endif

}
#endif
//***********************************************************************************************
//	void auto_set_timing_para(__dram_para_t *para)
//
//  Description:	auto set the timing para base on the DRAM Frequency in structure
//
//	Arguments:		DRAM parameter
//
//	Return Value:	None
//***********************************************************************************************
#ifndef FPGA_PLATFORM
void auto_set_timing_para(__dram_para_t *para)
{
	unsigned int  ctrl_freq;//half speed mode :ctrl_freq=1/2 ddr_fre
	unsigned int  type;
	unsigned int  reg_val       =0;
	unsigned int  mr0 	        =0x0;
	unsigned int  mr1	        =0x0;
	unsigned int  mr2 	        =0x0;
	unsigned int  mr3 	        =0x0;
	unsigned int  tdinit0       = 0;
	unsigned int  tdinit1       = 0;
	unsigned int  tdinit2       = 0;
	unsigned int  tdinit3       = 0;
	unsigned char t_rdata_en    = 1;    //ptimg0
	unsigned char wr_latency    = 1;	//ptimg0
	unsigned char tcl 			= 3;	//6
	unsigned char tcwl			= 3;	//6
	unsigned char tmrw			= 0;	//0
	unsigned char tmrd			= 2;	//4;
	unsigned char tmod			= 6;	//12;
	unsigned char tccd			= 2;	//4;
	unsigned char tcke			= 2;	//3;
	unsigned char trrd			= 3;	//6;
	unsigned char trcd			= 6;	//11;
	unsigned char trc			= 20;	//39;
	unsigned char tfaw			= 16;	//32;
	unsigned char tras			= 14;	//28;
	unsigned char trp			= 6;	//11;
	unsigned char twtr			= 3;	//6;
	unsigned char twr			= 8;	//15；
	unsigned char trtp			= 3;	//6;
	unsigned char txp			= 10;	//20;
	unsigned short trefi		= 98;	//195;
	unsigned short trfc		    = 128;
	unsigned char twtp			= 12;	//24;	//write to pre_charge
	unsigned char trasmax		= 27;	//54;	//54*1024ck
	unsigned char twr2rd		= 8;	//16;
	unsigned char trd2wr		= 4;	//7;
	unsigned char tckesr		= 3;	//5;
	unsigned char tcksrx		= 4;	//8;
	unsigned char tcksre		= 4;	//8;
	unsigned char tdqsckmax	= 0;
	unsigned char tdqsck	= 0;
	ctrl_freq = para->dram_clk/2;	//Controller work in half rate mode
	type      = para->dram_type;
	//add the time user define
	if(para->dram_tpr13&0x2)
	{
		tcl = ( (para->dram_tpr0 >> 0) & 0x1f );
		tcwl = ( (para->dram_tpr0 >> 5) & 0x1f );
		trcd = ( (para->dram_tpr0 >> 10) & 0x1f );
		trrd = ( (para->dram_tpr0 >> 15) & 0x1f );
		tfaw = ( (para->dram_tpr0 >> 20) & 0x1f );
		tccd = ( (para->dram_tpr0 >> 25) & 0x1f );
		tras = ( (para->dram_tpr1 >> 0) & 0x1f );
		trp = ( (para->dram_tpr1 >> 5) & 0x1f );
		twr = ( (para->dram_tpr1 >> 10) & 0x1f );
		trtp = ( (para->dram_tpr1 >> 15) & 0x1f );
		twtr = ( (para->dram_tpr1 >> 20) & 0x1f );
		trefi = ( (para->dram_tpr2 >> 0) & 0xfff );	//after 32 divider
		wr_latency= ( (para->dram_tpr2 >> 21) & 0x1f );
		t_rdata_en= ( (para->dram_tpr2 >> 26) & 0x1f );
		trc = tras + trp;
	}//add finish
	else //use auto set timing
	{
		switch(type)
		{
		case 3://DDR3
	#ifdef DRAM_TYPE_DDR3
			tmrw=0;
			tmrd=2;
			tmod=6;
			tccd=2;//4
			tcke=3;
			tcksrx=5;
			tcksre=5;
			tckesr=tcke+1;
			txp	= 4;
			trasmax = 0x1b;	  //unit in 1024
	#if 1
			if(!(para->dram_tpr13&0x2))
			{//auto calculate the time according to JEDEC
				if(para->dram_clk <= 400)
				{
					tcl		= 3;	//CL=6
					tcwl	= 3;	//CWL=5
					mr0 	= 0x420;//CL=6,WR=6
					mr2     = 0;    //CWL=5
					t_rdata_en  =1; //(CL-4(3))/2
					wr_latency  =1;
				}
				else if(para->dram_clk <= 533)
				{
					tcl		= 4;	//CL   8
					tcwl	= 3;	//CWL  6
					mr0 	= 0x840;//CL=8,WR=8
					mr2     = 8;	//CWL=6
					t_rdata_en  =2;
					wr_latency  =1;
				}
				else if(para->dram_clk <= 667)
				{
					tcl		= 5;	//CL    10
					tcwl	= 4;	//CWL   7
					mr0 	= 0xa60;//CL=10,WR=10
					mr2     = 0x10; //CWL=7
					t_rdata_en  =3;
					wr_latency  =2;
				}
				else if(para->dram_clk <= 800)
				{
					tcl		= 6;	//CL   11
					tcwl	= 4;	//CWL  8
					mr0 	= 0xc70;//CL=11,WR=12
					mr2     = 0x18; //CWL=8
					t_rdata_en  =4;
					wr_latency  =2;
				}
			trrd=(6*ctrl_freq)/1000 + ( ( ((10*ctrl_freq)%1000) != 0) ? 1 :0);
			if(trrd<2) trrd=2;	//max(4ck,10ns(6ns))
			trcd= (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);//15ns(10ns)
			trp = (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);//15ns(10ns)
			trefi	= ( (7800*ctrl_freq)/1000 + ( ( ((7800*ctrl_freq)%1000) != 0) ? 1 :0) )/32;//7800ns
			tras	= (35*ctrl_freq)/1000 + ( ( ((35*ctrl_freq)%1000) != 0) ? 1 :0);	//35ns;
	//		txp		= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
	//		if(txp<2) txp = 2;//max(3ck,7.5ns)
			trtp	= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
			if(trtp<2) trtp=2;	//max(4ck,7.5ns)
			trc	= (53*ctrl_freq)/1000 + ( ( ((53*ctrl_freq)%1000) != 0) ? 1 :0);	//52.5ns(45ns)
	//		tcksrx= (10*ctrl_freq)/1000 + ( ( ((10*ctrl_freq)%1000) != 0) ? 1 :0);	//10ns;
	//		if(tcksrx<3) tcksrx=3;//max(5ck,10ns)
	//		tcksre= (10*ctrl_freq)/1000 + ( ( ((10*ctrl_freq)%1000) != 0) ? 1 :0);	//10ns;
	//		if(tcksre<3) tcksrx=3;//max(5ck,10ns)
			twtr= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
			if(twtr<2) twtr=2;	//max(4ck,7,5ns)
			twr= (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);	//15ns;
			tfaw= (40*ctrl_freq)/1000 + ( ( ((50*ctrl_freq)%1000) != 0) ? 1 :0);	//50ns;
			tdinit0	= (500*para->dram_clk) + 1;	//500us
			tdinit1	= (360*para->dram_clk)/1000 + 1;	//360ns
			tdinit2	= (200*para->dram_clk) + 1;	//200us
			tdinit3	= (1*para->dram_clk) + 1;	//1us
			}
	#endif
			twtp=tcwl+2+twr;//WL+BL/2+tWR
			twr2rd= tcwl+2+twtr;//WL+BL/2+tWTR
			trd2wr= tcl+2+1-tcwl;//RL+BL/2+2-WL
	#endif
			break;
		case 6 ://LPDDR2
	#ifdef DRAM_TYPE_LPDDR2
			tcke	= 2;//3;
			tcksrx	= 1;//2;
			tcksre	= 1;//2;
			tmrw	= 3;//5;
			mr3     = 4;
			tccd	= 1;//1
			txp     = 4;
			trasmax = 0x1b;
	#if 0
			if(!(para->dram_tpr13&0x2))
			{//auto calculate the time according to JEDEC
				if(para->dram_clk <= 200)
				{
					tcl		= 2;	//CL=3
					tcwl	= 1;	//CWL=1
					mr1     = 0x22; //WR=3
					mr2     = 0x1;  //CL=3,CWL=1
					t_rdata_en  =1;
					wr_latency  =1;
				}
				else if(para->dram_clk <= 267)
				{
					tcl		= 2;	//CL   4
					tcwl	= 1;	//CWL  2
					mr1 	= 0x62; //WR=5
					mr2     = 0x2;	//CL=4,CWL=2
					t_rdata_en  =1;
					wr_latency  =1;
				}
				else if(para->dram_clk <= 333)
				{
					tcl		= 3;	//CL   5
					tcwl	= 1;	//CWL  2
					mr1 	= 0x62; //WR=5
					mr2     = 0x3;	//CL=5,CWL=2
					t_rdata_en  =1;
					wr_latency  =1;
				}
				else if(para->dram_clk <= 400)
				{
					tcl		= 3;	//CL   6
					tcwl	= 2;	//CWL  3
					mr1 	= 0x82; //WR=6
					mr2     = 0x4;	//CL=6,CWL=3
					t_rdata_en  =1;
					wr_latency  =1;
				}
				else if(para->dram_clk <= 467)
				{
					tcl		= 4;	//CL    7
					tcwl	= 2;	//CWL   4
					mr1 	= 0xc2; //WR=8
					mr2     = 0x5;  //CL=7,CWL=4
					t_rdata_en  =3;
					wr_latency  =1;
				}
				else if(para->dram_clk <= 533)
				{
					tcl		= 4;	//CL   8
					tcwl	= 2;	//CWL  4
					mr1 	= 0xc2;//WR=8
					mr2     = 0x6; //CL=8,CWL=4
					t_rdata_en  =3;
					wr_latency  =1;
				}
				trefi	= ( (3900*ctrl_freq)/1000 + ( ( ((3900*ctrl_freq)%1000) != 0) ? 1 :0) )/32;	//3.9us when chip density more than 2Gb
				tras	= (42*ctrl_freq)/1000 + ( ( ((42*ctrl_freq)%1000) != 0) ? 1 :0);	//42ns min;
				if(tras<2) tras	= 2;
				tckesr	= (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);	//15ns;
				if(tckesr<2) tckesr = 2;
	//			txp		= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
	//			if(txp<1) txp = 1;
				trtp	= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
				if(trtp<1) trtp	= 1;
				trcd	= (24*ctrl_freq)/1000 + ( ( ((24*ctrl_freq)%1000) != 0) ? 1 :0);	//24ns;
				if(trcd<2) trcd	= 2;
				trp		= (27*ctrl_freq)/1000 + ( ( ((27*ctrl_freq)%1000) != 0) ? 1 :0);	//trpab 27ns;
				if(trp<2) trp = 2;
				twr		= (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);	//15ns;
				if(twr<2) twr = 2;
				trrd	= (10*ctrl_freq)/1000 + ( ( ((10*ctrl_freq)%1000) != 0) ? 1 :0);	//10ns;
				if(trrd<1) trrd	= 1;
				twtr	= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
				if(twtr<1) twtr	= 1;
				if(para->dram_clk<=166)
					tfaw	= (60*ctrl_freq)/1000 + ( ( ((60*ctrl_freq)%1000) != 0) ? 1 :0);	//60ns;
				else
					tfaw	= (50*ctrl_freq)/1000 + ( ( ((50*ctrl_freq)%1000) != 0) ? 1 :0);	//50ns;
				if(tfaw<4) tfaw	= 4;
				tdinit0	= (200*para->dram_clk) + 1;	//200us
				tdinit1	= (100*para->dram_clk)/1000 + 1;	//100ns
				tdinit2	= (11*para->dram_clk) + 1;	//11us
				tdinit3	= (1*para->dram_clk) + 1;	//1us
			}
	#endif
			trc		= tras + trp;	//tras+ trpab
			twtp	= tcwl + 2 + twr + 1;	// CWL+BL/2+tWR
			trd2wr	= tcl + 2 + 5 - tcwl + 1;//5?
			twr2rd	= tcwl + 2 + 1 + twtr;//wl+BL/2+1+tWTR??
	#endif
			break;
		case 7 ://LPDDR3
	#ifdef DRAM_TYPE_LPDDR3
			tccd	= 2;//4;
			tmrw	= 5;//10;
			tcksrx	= 1;//2;
			tcksre	= 1;//2;
			txp     = 4;
			trasmax = 0x1b;
			mr3     = 0x2;
	#if 0   //simulation no need
			if(!(para->dram_tpr13&0x2))
			{
				if(para->dram_clk <= 200)
				{
					tcl		= 2;	//CL=3
					tcwl	= 1;	//CWL=1
					mr1     = 0x23; //WR=3
					mr2     = 0x1;  //CL=3,CWL=1
					t_rdata_en  =1;
					wr_latency  =1;
				}
				else if(para->dram_clk <= 533)
				{
					tcl		= 4;	//CL   8
					tcwl	= 2;	//CWL  4
					mr1 	= 0xc3; //WR=8
					mr2     = 0x6;	//CL=8,CWL=4
					t_rdata_en  =3;
					wr_latency  =1;
				}
				tcke	= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
				if(tcke<2)	tcke = 2;
				tckesr	= (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);	//15ns;
				if(tckesr<2) tckesr = 2;
	//			txp		= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
	//			if(txp<2) txp = 2;
				trtp	= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
				if(trtp<2) trtp=2;	//4ck min
				trcd	= (24*ctrl_freq)/1000 + ( ( ((24*ctrl_freq)%1000) != 0) ? 1 :0);	//24ns;
				if(trcd<2) trcd	= 2;
				trp		= (27*ctrl_freq)/1000 + ( ( ((27*ctrl_freq)%1000) != 0) ? 1 :0);	//trpab 27ns;
				if(trp<2) trp = 2;
				tras	= (42*ctrl_freq)/1000 + ( ( ((42*ctrl_freq)%1000) != 0) ? 1 :0);	//42ns min;
				if(tras<2) tras	= 2;
				twr		= (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);	//15ns;
				if(twr<2) twr = 2;
				twtr	= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
				if(twtr<2) twtr=2;	//4ck min
				trrd	= (10*ctrl_freq)/1000 + ( ( ((10*ctrl_freq)%1000) != 0) ? 1 :0);	//10ns;
				if(trrd<2) trrd	= 2;	//for burst length = 8
				tfaw	= (50*ctrl_freq)/1000 + ( ( ((50*ctrl_freq)%1000) != 0) ? 1 :0);	//50ns;
				if(tfaw<4) tfaw	= 4;
				trefi	= ( (3900*ctrl_freq)/1000 + ( ( ((3900*ctrl_freq)%1000) != 0) ? 1 :0) )/32;	//3.9us
				tdinit0	= (200*para->dram_clk) + 1;	//200us
				tdinit1	= (100*para->dram_clk)/1000 + 1;	//100ns
				tdinit2	= (11*para->dram_clk) + 1;	//11us
				tdinit3	= (1*para->dram_clk) + 1;	//1us
			}
	#endif
			trc		= tras + trp;	//tras+ trpab
			twtp	= tcwl + 4 + twr + 1;	// CWL+BL/2+tWR
			trd2wr	= tcl + 4 + 5 - tcwl + 1;	//13;
			twr2rd	= tcwl + 4 + 1 + twtr;
	#endif
			break;
		default:
			break;
		}
	}

	if(!(para->dram_tpr13&0x2))
	{
		//assign the value back to the DRAM structure
		para->dram_tpr0 = (tcl<<0) | (tcwl<<5) | (trcd<<10) | (trrd<<15) | (tfaw<<20) | (tccd<<25);
		para->dram_tpr1 = (tras<<0) | (trp<<5) | (twr<<10) | (trtp<<15) | (twtr<<20) | (tdqsckmax<<25) | (tdqsck<<28);
		para->dram_tpr2 = (trefi<<0) | (trfc<<12) | (wr_latency<<21) | (t_rdata_en<<26);
		para->dram_mr0  =  mr0;
		para->dram_mr1  =  mr1;
		para->dram_mr2  =  mr2;
		para->dram_mr3  =  mr3;
		para->dram_tpr3 =  (tdinit0<<0)|(tdinit1<<20);
		para->dram_tpr4 =  (tdinit2<<0)|(tdinit3<<20);
	}
	//set work mode register
	reg_val=mctl_read_w(MC_WORK_MODE);
	reg_val &=~((0xfff<<12)|(0xf<<0));
	if(type==6)
		reg_val |=(0x2<<20);
	else
		reg_val |=(0x4<<20);
	reg_val |= ((( ( ((para->dram_para1)>>20) & 0xff) - 1) & 0xf) << 4);//Row number
	reg_val |= ((para->dram_type & 0x07)<<16);//DRAM type
	reg_val |= (( ( (para->dram_para2) & 0x01 )? 0x0:0x1) << 12);	//DQ width
	reg_val |= ( (para->dram_para2)>>12 & 0x03 );	//rank
	reg_val |= ((((para->dram_para1)>>28) & 0x01) << 2);//BANK
	reg_val |= (((para->dram_tpr13>>5)&0x1)<<19);//2T or 1T
	reg_val |= (((para->dram_tpr13>>6)&0x1)<<15);//address map mode
	mctl_write_w(reg_val,MC_WORK_MODE);
	//set mode register
	mctl_write_w((para->dram_mr0),DRAM_MR0);
	mctl_write_w((para->dram_mr1),DRAM_MR1);
	mctl_write_w((para->dram_mr2),DRAM_MR2);
	mctl_write_w((para->dram_mr3),DRAM_MR3);
	//set dram timing
	reg_val= (twtp<<24)|(tfaw<<16)|(trasmax<<8)|(tras<<0);
	mctl_write_w(reg_val,DRAMTMG0);//DRAMTMG0
	reg_val= (txp<<16)|(trtp<<8)|(trc<<0);
	mctl_write_w(reg_val,DRAMTMG1);//DRAMTMG1
	reg_val= (tcwl<<24)|(tcl<<16)|(trd2wr<<8)|(twr2rd<<0);
	mctl_write_w(reg_val,DRAMTMG2);//DRAMTMG2
	reg_val= (tmrw<<16)|(tmrd<<12)|(tmod<<0);
	mctl_write_w(reg_val,DRAMTMG3);//DRAMTMG3
	reg_val= (trcd<<24)|(tccd<<16)|(trrd<<8)|(trp<<0);
	mctl_write_w(reg_val,DRAMTMG4);//DRAMTMG4
	reg_val= (tcksrx<<24)|(tcksre<<16)|(tckesr<<8)|(tcke<<0);
	mctl_write_w(reg_val,DRAMTMG5);//DRAMTMG5
	//set phy interface time
	reg_val=(0x2<<24)|(t_rdata_en<<16)|(0x1<<8)|(wr_latency<<0);
	mctl_write_w(reg_val,PITMG0);	//PHY interface write latency and read latency configure
	//set phy time  simulation use
	mctl_write_w(0x01e007c3,PTR0);
	mctl_write_w(0x00170023,PTR1);
	mctl_write_w(0x00800800,PTR3);
	if(para->dram_type==7)
		mctl_write_w(0x32034156,PTR4);//1us LPDDR3 must set this value
	else
		mctl_write_w(0x01000500,PTR4);
	//set refresh timing

}
void auto_set_dram_para(__dram_para_t *para)
{
	u32 ret_val=2;
	ret_val=para->dram_type;
	switch(ret_val)
	{
		case 2: //ddr2
#ifdef DRAM_TYPE_DDR2
			para->dram_zq			= 0xbbbb77;
			para->dram_odt_en       = 1;
			para->dram_para1		= 0x10D41000;
			para->dram_para2		= 0x1000;//half dq 1001;default 1000
			para->dram_mr0			= 0xa63;
			para->dram_mr1			= 0x0;
			para->dram_mr2			= 0x0;
			para->dram_mr3			= 0x0;
			para->dram_tpr0 		= 0x04911063;
			para->dram_tpr1 		= 0x00211089;
			para->dram_tpr2 		= 0x0428003A;
			para->dram_tpr3 		= 0x0AD3A981;
			para->dram_tpr4 		= 0x1E117701;
			para->dram_tpr5          = 0x0;
			para->dram_tpr6          = 0x0;
			para->dram_tpr7          = 0x0;
			para->dram_tpr8          = 0x0;
			para->dram_tpr9          = 0x0;
			para->dram_tpr10       	 = 192;
			para->dram_tpr13       	 = 0x443;//default2T:0x47   1T:0x27    Rank, Bank, Row, Column :0x47
#endif
			break;
		case 3: //ddr3
#ifdef DRAM_TYPE_DDR3
			para->dram_zq			= 0xbbbb77;
			para->dram_odt_en       = 0;
			para->dram_para1		= 0x11081000;//COL:11bit  ROW:16bit
			para->dram_para2		= 0x1000;//half dq 1001;default 1000
			para->dram_mr0			= 0x840;
			para->dram_mr1			= 0x0;
			para->dram_mr2			= 0x8;
			para->dram_mr3			= 0x0;
			para->dram_tpr0 		= 0x04911064;
			para->dram_tpr1 		= 0x00211089;
			para->dram_tpr2 		= 0x0828003A;
			para->dram_tpr3 		= 0x0AD3A981;
			para->dram_tpr4 		= 0x1E117701;
			para->dram_tpr5          = 0x0;
			para->dram_tpr6          = 0x0;
			para->dram_tpr7          = 0x0;
			para->dram_tpr8          = 0x0;
			para->dram_tpr9          = 0x0;
			para->dram_tpr10       	 = 192;
			para->dram_tpr13       	 = 0x27;//default2T:0x47   1T:0x67    Rank,Row,Bank,Column :0x07
#endif
			break;
		case 6:
#ifdef DRAM_TYPE_LPDDR2
			para->dram_zq			= 0xbbbb77;
			para->dram_odt_en       = 1;
			para->dram_para1		= 0x10F40000;//according to design
			para->dram_para2		= 0x1000;
			para->dram_mr0			= 0x0;
			para->dram_mr1			= 0xc2;
			para->dram_mr2			= 0x6;
			para->dram_mr3			= 0x4;
			para->dram_tpr0 		= 0x02C19844;
			para->dram_tpr1 		= 0x002110EB;
			para->dram_tpr2 		= 0x0C28001D;
			para->dram_tpr3 		= 0x03117701;
			para->dram_tpr4 		= 0x1E1014A1;
			para->dram_tpr5          = 0x0;
			para->dram_tpr6          = 0x0;
			para->dram_tpr7          = 0x0;
			para->dram_tpr8          = 0x0;
			para->dram_tpr9          = 0x0;
			para->dram_tpr10       	 = 192;
			para->dram_tpr13       	 = 0x443;
#endif
			break;
		case 7:
#ifdef DRAM_TYPE_LPDDR3
			para->dram_zq			= 0xbbbb77;
			para->dram_odt_en       = 1;
			para->dram_para1		= 0x10E40000;//according to design
			para->dram_para2		= 0x1000;
			para->dram_mr0			= 0x0;
			para->dram_mr1			= 0xc3;
			para->dram_mr2			= 0x6;
			para->dram_mr3			= 0x2;
			para->dram_tpr0 		= 0x04C19844;
			para->dram_tpr1 		= 0x002110EB;
			para->dram_tpr2 		= 0x0C28001D;
			para->dram_tpr3 		= 0x03117701;
			para->dram_tpr4 		= 0x1E1014A1;
			para->dram_tpr5          = 0x0;
			para->dram_tpr6          = 0x0;
			para->dram_tpr7          = 0x0;
			para->dram_tpr8          = 0x0;
			para->dram_tpr9          = 0x0;
			para->dram_tpr10       	 = 192;
			para->dram_tpr13       	 = 0x443;//default2T:0x47   1T:0x27    Rank, Bank, Row, Column :0x47
#endif
			break;
		default:
			break;
	}
}
#endif
//***********************************************************************************************
//	unsigned int ccm_set_dram_div(u32 div)
//
//  Description:	set-dram-div
//
//	Return Value:	None
//***********************************************************************************************
#ifndef FPGA_PLATFORM
unsigned int ccm_set_dram_div(u32 div)
{
	unsigned int  reg_val,ret;
	ret=div-1;
	reg_val =(ret|(0x1<<16));
	mctl_write_w(reg_val,_CCM_DRAMCLK_CFG_REG);
	while(mctl_read_w(_CCM_DRAMCLK_CFG_REG) & (0x1<<16));
	local_delay(100);
	return 0;
}
//***********************************************************************************************
//	unsigned int ccm_set_pll_ddr_clk(u32 pll_clk)
//
//  Description:	set-pll-ddr0-clk
//
//	Return Value:	None
//***********************************************************************************************
unsigned int ccm_set_pll_ddr0_clk(u32 pll_clk)
{
	unsigned int n, k, m = 1,rval;
	unsigned int div;
	unsigned int mod2, mod3;
	unsigned int min_mod = 0;

	div = pll_clk/24;
	if (div <= 32) {
		n = div;
		k = 1;
	} else {
		/* when m=1, we cann't get absolutely accurate value for follow clock:
		 * 840(816), 888(864),
		 * 984(960)
		 */
		mod2 = div&1;
		mod3 = div%3;
		min_mod = mod2;
		k = 2;
		if (min_mod > mod3) {
			min_mod = mod3;
			k = 3;
		}
		n = div / k;
	}
	rval = mctl_read_w(_CCM_PLL_DDR0_REG);
	rval &= ~((0x1f << 8) | (0x3 << 4) | (0x3 << 0));
	rval = (1U << 31)  | ((n-1) << 8) | ((k-1) << 4) | (m-1);
	mctl_write_w(rval, _CCM_PLL_DDR0_REG);
	mctl_write_w(rval|(1U << 20), _CCM_PLL_DDR0_REG);
	local_delay(20);
	return 24 * n * k / m;
}



unsigned int ccm_set_pll_ddr1_clk(u32 pll_clk)
{
	u32 rval;
	u32 div;

	div = pll_clk/24;
	if (div < 12) //no less than 288M
	{
		div=12;
	}
	rval = mctl_read_w(_CCM_PLL_DDR1_REG);
	rval &= ~(0x3f<<8);
	rval |=(((div-1)<<8)|(0x1U<<31));
	mctl_write_w(rval, _CCM_PLL_DDR1_REG);
	mctl_write_w(rval|(1U << 30), _CCM_PLL_DDR1_REG);
	local_delay(1);
	return 24*div;
}

#endif
//***********************************************************************************************
//	unsigned int mctl_sys_init(__dram_para_t *para)
//
//  Description:	set pll and dram clk
//
//	Arguments:		DRAM parameter
//
//	Return Value:	None
//***********************************************************************************************
unsigned int mctl_sys_init(__dram_para_t *para)
{
#ifndef FPGA_PLATFORM
	unsigned int reg_val = 0;
	//trun off mbus clk gate
	reg_val = mctl_read_w(MBUS_CLK_CTL_REG);
	reg_val &=~(1U<<31);
	writel(reg_val, MBUS_CLK_CTL_REG);
	//mbus reset
	reg_val = mctl_read_w(MBUS_RESET_REG);
	reg_val &=~(1U<<31);
	writel(reg_val, MBUS_RESET_REG);
	// DISABLE DRAMC BUS GATING
	reg_val = mctl_read_w(BUS_CLK_GATE_REG0);
	reg_val &= ~(1U<<14);
	writel(reg_val, BUS_CLK_GATE_REG0);
	//DRAM BUS reset
	reg_val = mctl_read_w(BUS_RST_REG0);
	reg_val &= ~(1U<<14);
	writel(reg_val, BUS_RST_REG0);
	//disable pll-ddr0
	reg_val = mctl_read_w(_CCM_PLL_DDR0_REG);
	reg_val &=~(1U<<31);
	writel(reg_val, _CCM_PLL_DDR0_REG);
	local_delay(10);//1us ic
	//controller reset
	reg_val = mctl_read_w(_CCM_DRAMCLK_CFG_REG);
	reg_val &= ~(0x1U<<31);
	mctl_write_w(reg_val,_CCM_DRAMCLK_CFG_REG);
	local_delay(10);//1us ic
	//set pll
	reg_val = mctl_read_w(_CCM_DRAMCLK_CFG_REG);
	reg_val &= ~(0x3<<20);
#ifdef DRAMC_PLL_PERI0_TEST
	reg_val = (0x2<<20);
#else
#ifdef	DRAMC_MDFS_CFS_TEST
	para->dram_tpr13 |= 0x1;
#endif
	if(para->dram_tpr13 & (0x1<<10))	//ddr_pll1  CFS mode
	{
		ccm_set_pll_ddr1_clk((para->dram_clk)*2);
		reg_val |= (0x1<<20);
	}
	else
	{
		ccm_set_pll_ddr0_clk((para->dram_clk)*2);
		reg_val &= ~(0x3<<20);
#ifdef DRAMC_MDFS_DFS_TEST
		reg_val &= ~(0x3<<20);
		ccm_set_pll_ddr0_clk((para->dram_clk)*2);
		ccm_set_pll_ddr1_clk((para->dram_clk)*2/2);	//for DFS clk source changing
#endif
	}
#endif
	writel(reg_val, _CCM_DRAMCLK_CFG_REG);
	//Setup DRAM Clock
	ccm_set_dram_div(1);

	//release DRAM ahb BUS RESET
	reg_val = mctl_read_w(BUS_RST_REG0);
	reg_val |= (1U<<14);
	writel(reg_val, BUS_RST_REG0);
	//open AHB gating
	reg_val = mctl_read_w(BUS_CLK_GATE_REG0);
	reg_val |= (1U<<14);
	writel(reg_val, BUS_CLK_GATE_REG0);
	//release DRAM mbus RESET
	reg_val = mctl_read_w(MBUS_RESET_REG);
	reg_val |=(1U<<31);
	writel(reg_val, MBUS_RESET_REG);
	//open mbus gating
	reg_val = mctl_read_w(MBUS_CLK_CTL_REG);
	reg_val |=(1U<<31);
	writel(reg_val, MBUS_CLK_CTL_REG);
#ifdef DRAMC_REFRESH_TEST
	//set to one rank mode before global clock enable for refresh test,liuke edit 20140902
	reg_val = mctl_read_w(MC_WORK_MODE);
	reg_val &= ~(0x1<<0);
	mctl_write_w(reg_val,MC_WORK_MODE);
	//enable dramc clk
	mctl_write_w(0xffffffff,MC_CLKEN);	//--------------------------------------------------------------------------------------
	para->dram_para2 |= 1<<12;
#else
	mctl_write_w(0xffffffff,MC_CLKEN);
#endif

#else
	unsigned int reg_val = 0;
	reg_val = mctl_read_w(_CCM_DRAMCLK_CFG_REG);
	reg_val &= ~(0x1U<<31);
	mctl_write_w(reg_val,_CCM_DRAMCLK_CFG_REG);

	mctl_write_w(0x25ffff,MC_CLKEN);
#endif

	return DRAM_RET_OK;
}
//***********************************************************************************************
//	void mctl_com_init(__dram_para_t *para)
//
//  Description:	set ddr para and enable clk
//
//	Arguments:		DRAM parameter
//
//	Return Value:	None
//***********************************************************************************************
void mctl_com_init(__dram_para_t *para)
{
	unsigned int reg_val;
	reg_val = mctl_read_w(MC_WORK_MODE);
	reg_val &= ~(0xffffff);
	if(para->dram_type==6)
		reg_val |=(0x2<<20);
	else
		reg_val |=(0x4<<20);
	reg_val |= ((para->dram_type & 0x07)<<16);//DRAM type
	reg_val |= ((((para->dram_para1)>>28) & 0x01) << 2);//BANK
	reg_val |= ((( ( ((para->dram_para1)>>20) & 0xff) - 1) & 0xf) << 4);//Row number
	reg_val |= (( ( (para->dram_para2) & 0x01 )? 0x0:0x1) << 12);	//DQ width
	reg_val |= ( (para->dram_para2)>>12 & 0x03 );	//rank
	reg_val |= (((para->dram_tpr13>>5)&0x1)<<19);//2T or 1T
	reg_val |= (((para->dram_tpr13>>6)&0x1)<<15);//address map mode

	//pattern_goto(0x73);
	//if half DQ(para->dram_para2==1),pagesize should half
	switch( ((para->dram_para2)&0x01) ?( (((para->dram_para1)>>16)& 0xf)>>1 ) : (((para->dram_para1)>>16)& 0xf) )	//MCTL_PAGE_SIZE
	{//************************************IC should not half, have auto scan******************************************
	case 8:
		reg_val |= 0xA << 8;
		break;
	case 4:
		reg_val |= 0x9 << 8;
		break;
	case 2:
		reg_val |= 0x8 << 8;
		break;
	case 1:
		reg_val |= 0x7 << 8;
		break;
	default:
		reg_val |= 0x6 <<8;
		break;
	}
	mctl_write_w(reg_val,MC_WORK_MODE);


}

#ifndef FPGA_PLATFORM
unsigned int mctl_channel_init(unsigned int ch_index,__dram_para_t *para)
{
	unsigned int reg_val = 0,ret_val,pad_hold_flag,value;
	unsigned int dqs_gating_mode =0;
	dqs_gating_mode = (para->dram_tpr13>>2)&0x7;
#ifdef DRAMC_MASTER_TEST
	dqs_gating_mode = 1;	//gate training mode
#endif
#if defined DRAMC_NORMAL_STANDBY_TEST
	dqs_gating_mode = 0;	//gate training mode
#endif
	auto_set_timing_para(para);
//****************************************************************************************************//
//setting VTC\ODT

	//pattern_goto(0x30);

	if(para->dram_clk<240)
	{
		//disable VTC
		reg_val=mctl_read_w(PGCR0);
		reg_val &=~(0x3f<<0);
		mctl_write_w(reg_val,PGCR0);
		//ODT set   add an external control
		mctl_write_w(0,ODTMAP);
		reg_val=mctl_read_w(DXnGCR0(0));
		reg_val&=~(0x1<<1);
		mctl_write_w(reg_val,DXnGCR0(0));

		reg_val=mctl_read_w(DXnGCR0(1));
		reg_val&=~(0x1<<1);
		mctl_write_w(reg_val,DXnGCR0(1));

		reg_val=mctl_read_w(DXnGCR0(2));
		reg_val&=~(0x1<<1);
		mctl_write_w(reg_val,DXnGCR0(2));

		reg_val=mctl_read_w(DXnGCR0(3));
		reg_val&=~(0x1<<1);
		mctl_write_w(reg_val,DXnGCR0(3));
	}
	else
	{
		//enable VTC
		reg_val=mctl_read_w(PGCR0);
		reg_val |=(0x3f<<0);
		mctl_write_w(reg_val,PGCR0);
		//odt set add a external control
		reg_val=mctl_read_w(DXnGCR0(0));
		reg_val|=(0x1<<1);
		mctl_write_w(reg_val,DXnGCR0(0));

		reg_val=mctl_read_w(DXnGCR0(1));
		reg_val|=(0x1<<1);
		mctl_write_w(reg_val,DXnGCR0(1));

		reg_val=mctl_read_w(DXnGCR0(2));
		reg_val|=(0x1<<1);
		mctl_write_w(reg_val,DXnGCR0(2));

		reg_val=mctl_read_w(DXnGCR0(3));
		reg_val|=(0x1<<1);
		mctl_write_w(reg_val,DXnGCR0(3));
	}

	if((para->dram_odt_en)==1)//open odt
	{
		reg_val=mctl_read_w(DXnGCR0(0));
		reg_val&=~(0x3<<4);
		mctl_write_w(reg_val,DXnGCR0(0));

		reg_val=mctl_read_w(DXnGCR0(1));
		reg_val&=~(0x3<<4);
		mctl_write_w(reg_val,DXnGCR0(1));

		reg_val=mctl_read_w(DXnGCR0(2));
		reg_val&=~(0x3<<4);
		mctl_write_w(reg_val,DXnGCR0(2));

		reg_val=mctl_read_w(DXnGCR0(3));
		reg_val&=~(0x3<<4);
		mctl_write_w(reg_val,DXnGCR0(3));
	}
	else//close odt
	{
		reg_val=mctl_read_w(DXnGCR0(0));
		reg_val&=~(0x3<<4);
		reg_val|=(0x2<<4);
		mctl_write_w(reg_val,DXnGCR0(0));

		reg_val=mctl_read_w(DXnGCR0(1));
		reg_val&=~(0x3<<4);
		reg_val|=(0x2<<4);
		mctl_write_w(reg_val,DXnGCR0(1));

		reg_val=mctl_read_w(DXnGCR0(2));
		reg_val&=~(0x3<<4);
		reg_val|=(0x2<<4);
		mctl_write_w(reg_val,DXnGCR0(2));

		reg_val=mctl_read_w(DXnGCR0(3));
		reg_val&=~(0x3<<4);
		reg_val|=(0x2<<4);
		mctl_write_w(reg_val,DXnGCR0(3));
	}
//****************************************************************************************************//
//set DQS Gating mode: default close
	switch(dqs_gating_mode)
	{
		case 0://close DQS gating--auto gating pull down
			reg_val=mctl_read_w(PGCR2);
			reg_val|=(0x3<<6);
			mctl_write_w(reg_val,PGCR2);
			break;
		case 1://open DQS gating
			reg_val=mctl_read_w(PGCR2);
			reg_val&=~(0x3<<6);
			mctl_write_w(reg_val,PGCR2);

			reg_val=mctl_read_w(DQSGMR);
			reg_val&=~((0x1<<8)|(0x7));
			mctl_write_w(reg_val,DQSGMR);
			break;
		case 2://auto gating pull up
			reg_val=mctl_read_w(PGCR2);
			reg_val&=~(0x3<<6);
			reg_val|=(0x2<<6);
			mctl_write_w(reg_val,PGCR2);

			ret_val =((mctl_read_w(DRAMTMG2)>>16)&0x1f)-2;
			reg_val=mctl_read_w(DQSGMR);
			reg_val&=~((0x1<<8)|(0x7));
			reg_val|=((0x1<<8)|(ret_val));
			mctl_write_w(reg_val,DQSGMR);

			reg_val =mctl_read_w(DXCCR);//dqs pll up
			reg_val |=(0x1<<27);
			reg_val &=~(0x1U<<31);
			mctl_write_w(reg_val,DXCCR);
			break;
		default:
			break;
	}
//****************************************************************************************************//
//setting half DQ
	if((para->dram_para2)&0x01)
	{
		mctl_write_w(0,DXnGCR0(2));
		mctl_write_w(0,DXnGCR0(3));
	}
//****************************************************************************************************//
//map cs1/cke1/odt1 for rank0
//set to 1ranks
#ifdef DRAMC_MAP_RANK1_FOR_RANK0_TEST
	para->dram_para2 &= ~(0x1<<12);
	reg_val=mctl_read_w(MC_WORK_MODE);
	reg_val |= 0x1<<25;
	mctl_write_w(reg_val,MC_WORK_MODE);  //two rank
#endif
//****************************************************************************************************//
//data training configuration
	if((para->dram_para2>>12)&0x1)
	{
		reg_val=mctl_read_w(DTCR);
		reg_val&=(0xfU<<28);
		reg_val|=0x03000081;
		mctl_write_w(reg_val,DTCR);  //two rank
	}
	else
	{
		reg_val=mctl_read_w(DTCR);
		reg_val&=(0xfU<<28);
		reg_val|=0x01000081;
		mctl_write_w(reg_val,DTCR);  //one rank
	}
#ifdef DRAMC_USE_MPR_TEST
	reg_val =mctl_read_w(DTCR);
	reg_val|=(0x1<<6);
	mctl_write_w(reg_val,DTCR);
#endif
//****************************************************************************************************//
//debug auto enter self-refresh and power down and ck gating
	reg_val =0;
	if(para->dram_tpr13 & (1<<12))
		reg_val |=(0x1<<3);
	if(para->dram_tpr13 & (1<<13))
		reg_val |=(0x1<<0);
	mctl_write_w(reg_val,PWRCTL);
//****************************************************************************************************//
//release the controller reset
	reg_val = mctl_read_w(_CCM_DRAMCLK_CFG_REG);
	reg_val |= 0x1U<<31;
	mctl_write_w(reg_val,_CCM_DRAMCLK_CFG_REG);
	local_delay(100);
//****************************************************************************************************//

#ifdef  DRAMC_TIMING_TEST
	//DRAMTMG3 max timing test
	reg_val=mctl_read_w(DRAMTMG3);
	reg_val|=( (0x3ff)|(0x7<<12)|(0x3ff<<16)  );
	mctl_write_w(reg_val,DRAMTMG3);
#endif

	pad_hold_flag = (mctl_read_w(0x01F01400 + 0x110) & 0x1);
if(pad_hold_flag ==0)
{
	//pattern_goto(0x80);
	//set zq para   CA
	reg_val = mctl_read_w(ZQPR);
	reg_val &= ~(0x000000ff);
	reg_val |= ( (para->dram_zq) & 0xff );
	mctl_write_w(reg_val,ZQPR);
	//set zq para   DX0/1
	reg_val = mctl_read_w(ZQPR);
	reg_val &= ~(0x0000ff00);
	reg_val |= ( ((para->dram_zq)>>8 ) & 0xff)<<8;
	mctl_write_w(reg_val,ZQPR);
	//set zq para   DX2/3
	reg_val = mctl_read_w(ZQPR);
	reg_val &= ~(0x00ff0000);
	reg_val |= ( ((para->dram_zq)>>16 ) & 0xff)<<16;
	mctl_write_w(reg_val,ZQPR);
	//	mctl_write_w(ZQPR , 0x004010d02);
	//accord to DQS gating mode to choose value of pir  default:1f3
#ifdef DRAMC_MASTER_TEST
	reg_val=0xfff3;
#else

#ifdef DRAMC_PHY_BIST_TEST
	reg_val=0x40000461;
	if(dqs_gating_mode==1)
	{
		value = mctl_read_w(PGCR1);
		value|=(0x1u<<31);
		mctl_write_w(value,PGCR1);
	}
#else
	reg_val=0x1f3;
#endif
#endif

#ifndef DRAMC_PHY_BIST_TEST
	ret_val =(mctl_read_w(PGCR2)>>6)&0x3;
	if(ret_val==0)//open gating need DQS gating training
		reg_val|=(1<<10);  //DQS gating training
	if(para->dram_type==6||para->dram_type==7||para->dram_type==2)
		reg_val&=~(0x1<<7);//Dram reset DDR3 only
	if(para->dram_type==6||para->dram_type==7||para->dram_type==2)
		reg_val&=~(0x1<<11);//Write Leveling Adjust DDR3 only
	if(para->dram_type==6||para->dram_type==7||para->dram_type==2)
		reg_val&=~(0x1<<9);//Write Leveling DDR3 only
#endif
	//pattern_goto(0x31);
	mctl_write_w(reg_val,PIR );	//for fast simulation
	//pattern_goto(0x32);
	while((mctl_read_w(PGSR0 )&0x1) != 0x1);	//for fast simulation
	while((mctl_read_w(STATR )&0x1) != 0x1);
	//set PGCR3,CKE polarity
	mctl_write_w(0x00aa0060,PGCR3);

#ifdef DRAMC_PHY_BIST_TEST
	value = mctl_read_w(PGCR1);
	value&=(~(0x1u<<31));
	mctl_write_w(value,PGCR1);
#endif

	return (1);
}
else
{
#ifdef DDR_TEST
	//pattern_goto(0x31);
	mctl_write_w(0x40000071,PIR);//phy reset\phy init and DDLC
	while((mctl_read_w(PGSR0 )&0x3) != 0x3);//waiting for pll lock and init done
	while((mctl_read_w(STATR )&0x1) != 0x1);
	//set PGCR3
	mctl_write_w(0x00aa0060,PGCR3);
	mctl_self_refresh_entry(0);
	mctl_pad_release();
#ifdef DRAMC_SUPER_STANDBY_ZQ_CALIBRATION_TEST
	//set zq para   CA
	reg_val = mctl_read_w(ZQPR);
	reg_val &= ~(0x000000ff);
	reg_val |= ( (para->dram_zq) & 0xff );
	mctl_write_w(reg_val,ZQPR);
	//set zq para   DX0/1
	reg_val = mctl_read_w(ZQPR);
	reg_val &= ~(0x0000ff00);
	reg_val |= ( ((para->dram_zq)>>8 ) & 0xff)<<8;
	mctl_write_w(reg_val,ZQPR);
	//set zq para   DX2/3
	reg_val = mctl_read_w(ZQPR);
	reg_val &= ~(0x00ff0000);
	reg_val |= ( ((para->dram_zq)>>16 ) & 0xff)<<16;
	mctl_write_w(reg_val,ZQPR);

	reg_val=0x3;
	mctl_write_w(reg_val,PIR);//ZQ
	while((mctl_read_w(PGSR0 )&0x1) != 0x1);//waiting for training finish
#endif
	mctl_self_refresh_exit(0);//dram exit self-refresh for training
	ret_val =(mctl_read_w(PGCR2)>>6)&0x3;
	if(ret_val==0)//open gating need DQS gating training
	{
		reg_val=0x40000401;
		mctl_write_w(reg_val,PIR);//phy reset\phy init and DDLC
		while((mctl_read_w(PGSR0 )&0x1) != 0x1);//waiting for training finish
	}
	mctl_write_w(0xffffffff,MC_MAER);	//master access enable
	return (1);
#endif
}



}
#else
unsigned int mctl_channel_init(unsigned int ch_index,__dram_para_t *para)
{
	u32 reg_val;
#if 1       //DDR2---col 10,row 14,bank 3,rank 1
	mctl_write_w(0x4219D5,MC_WORK_MODE);//map0;default 0x4288D5--16bit  /0x0x4299D5--32bit
	mctl_write_w(mctl_read_w(_CCM_DRAMCLK_CFG_REG)|(0x1U<<31),_CCM_DRAMCLK_CFG_REG);
	mctl_write_w(0x00070005,RFSHTMG);
	mctl_write_w(0xa63,DRAM_MR0);
	mctl_write_w(0x00,DRAM_MR1);
	mctl_write_w(0,DRAM_MR2);
	mctl_write_w(0,DRAM_MR3);
	mctl_write_w(0x01e007c3,PTR0);
	mctl_write_w(0x00170023,PTR1);
	mctl_write_w(0x00800800,PTR3);
	mctl_write_w(0x01000500,PTR4);
	mctl_write_w(0x01000081,DTCR);
	mctl_write_w(0x03808620,PGCR1);
	mctl_write_w(0x02010101,PITMG0);
	mctl_write_w(0x06021b02,DRAMTMG0);
	mctl_write_w(0x00020102,DRAMTMG1);
	mctl_write_w(0x03030306,DRAMTMG2);
	mctl_write_w(0x00002006,DRAMTMG3);
	mctl_write_w(0x01020101,DRAMTMG4);
	mctl_write_w(0x05010302,DRAMTMG5);
#else  //DDR3
	mctl_write_w(0x004318e4,MC_WORK_MODE);
	mctl_write_w(mctl_read_w(_CCM_DRAMCLK_CFG_REG)|(0x1U<<31),_CCM_DRAMCLK_CFG_REG);
	mctl_write_w(0x00070005,RFSHTMG);
	mctl_write_w(0x420,DRAM_MR0);
	mctl_write_w(0,DRAM_MR1);
	mctl_write_w(0,DRAM_MR2);
	mctl_write_w(0,DRAM_MR3);
	mctl_write_w(0x01e007c3,PTR0);
	mctl_write_w(0x00170023,PTR1);
	mctl_write_w(0x00800800,PTR3);
	mctl_write_w(0x01000500,PTR4);
	mctl_write_w(0x01000081,DTCR);
	mctl_write_w(0x03808620,PGCR1);
	mctl_write_w(0x02010101,PITMG0);
	mctl_write_w(0x0b091b0b,DRAMTMG0);
	mctl_write_w(0x00040310,DRAMTMG1);
	mctl_write_w(0x03030308,DRAMTMG2);
	mctl_write_w(0x00002007,DRAMTMG3);
	mctl_write_w(0x04020204,DRAMTMG4);
	mctl_write_w(0x05050403,DRAMTMG5);
	reg_val = mctl_read_w(MC_CLKEN);
	reg_val |= (0x3<<20);
	mctl_write_w(reg_val,MC_CLKEN);
#endif


	reg_val = 0x000183;		//PLL enable, PLL6 should be dram_clk/2
	mctl_write_w(reg_val,PIR);	//for fast simulation
	while((mctl_read_w(PGSR0 )&0x1) != 0x1);	//for fast simulation
	while((mctl_read_w(STATR )&0x1) != 0x1);	//init done
//	para->dram_tpr13&=~(0x1<<0);//to detect DRAM space

	reg_val = mctl_read_w(MC_CCR);
	reg_val|=(0x1U)<<31;
	mctl_write_w(reg_val,MC_CCR);
	local_delay(20);

	mctl_write_w(0x00aa0060,PGCR3);//

	reg_val = mctl_read_w(RFSHCTL0);
	reg_val|=(0x1U)<<31;
	mctl_write_w(reg_val,RFSHCTL0);
	local_delay(200);
	reg_val = mctl_read_w(RFSHCTL0);
	reg_val&=~(0x1U<<31);
	mctl_write_w(reg_val,RFSHCTL0);
	local_delay(200);

	reg_val = mctl_read_w(MC_CCR);
	reg_val|=(0x1U)<<31;
	mctl_write_w(reg_val,MC_CCR);
	local_delay(20);

	return (mctl_soft_training());

}
#endif


//*****************************************************************************
//	unsigned int mctl_init_chip()
//  Description:	DRAM Controller Initialize Procession and size confirm
//
//	Arguments:		None
//  
//	Return Value:	others: Success		0: Fail
//*****************************************************************************

/*
**********************************************************************************************************************
*                                               GET DRAM SIZE
*
* Description: Get DRAM Size in MB unit;
*
* Arguments  : None
*
* Returns    : 32/64/128/...
*
* Notes      :
*
**********************************************************************************************************************
*/
unsigned int DRAMC_get_dram_size(void)
{
	unsigned int reg_val;
	unsigned int dram_size;
	unsigned int temp;

    reg_val = mctl_read_w(MC_WORK_MODE);

    temp = (reg_val>>8) & 0xf;	//page size code
    dram_size = (temp - 6);	//(1<<dram_size) * 512Bytes

    temp = (reg_val>>4) & 0xf;	//row width code
    dram_size += (temp + 1);	//(1<<dram_size) * 512Bytes

    temp = (reg_val>>2) & 0x3;	//bank number code
    dram_size += (temp + 2);	//(1<<dram_size) * 512Bytes

    temp = reg_val & 0x3;	//rank number code
    dram_size += temp;	//(1<<dram_size) * 512Bytes

    dram_size = dram_size - 11;	//(1<<dram_size)MBytes

    return (1<<dram_size);
}


void auto_detect_rank_dq(__dram_para_t *para)
{
	unsigned int reg_val;
	unsigned int byte,col;
	//-----------------------------------detect dq-------------------------------------------
	reg_val = mctl_read_w(MC_WORK_MODE);
	reg_val|=(0x1<<12);//full DQ mode--32bit
	mctl_write_w(reg_val,MC_WORK_MODE);

	reg_val = mctl_read_w(PGCR0);
	reg_val|=(0x1<<25);//enable read Time-out
	mctl_write_w(reg_val,PGCR0);
	local_delay(10);

	mctl_write_w(0x12345678,0x40000000);

	if(mctl_read_w(0x40000000)==0)
	{
		paraconfig(&(para->dram_para2), 0xfU<<0, 1);//half DQ
	}
	else
	{
		paraconfig(&(para->dram_para2), 0xfU<<0, 0);//half DQ
	}

	reg_val = mctl_read_w(PGCR0);
	reg_val&=~(0x1<<26);//reset PHY FIFO
	mctl_write_w(reg_val,PGCR0);
	local_delay(10);
	reg_val|=(0x1<<26);//clear reset PHY FIFO
	mctl_write_w(reg_val,PGCR0);

	//-----------------------------------detect rank-------------------------------------------
	reg_val = mctl_read_w(MC_WORK_MODE);
	reg_val |= (( ( (para->dram_para2) & 0x01 )? 0x0:0x1) << 12);	//DQ width
	reg_val |=(0x1<<15);//Row, Rank, Bank, Column
	reg_val |=(0x1);//two rank
	paraconfig(&reg_val,0xf<<8,0x6<<8);//page_size is 512B
	mctl_write_w(reg_val,MC_WORK_MODE);

	if(((para->dram_para2)&0xf) == 1)
	{
		byte=1;//half dq--16 bit
	}
	else
	{
		byte=2;//full dq--32bit
	}

	col=(byte)?((byte-1)?7:8):9 ;

	mctl_write_w(0x12345678,0x40000000+(0x1<<(col+byte+3)) );//write rank 1

	if(mctl_read_w(0x40000000+(0x1<<(col+byte+3)))==0)
	{
		paraconfig(&(para->dram_para2), 0xfU<<12, 0<<12);//one rank
	}
	else
	{
		paraconfig(&(para->dram_para2), 0xfU<<12, 1<<12);//two rank
	}

	reg_val = mctl_read_w(PGCR0);
	reg_val&=~(0x1<<26);//reset PHY FIFO
	mctl_write_w(reg_val,PGCR0);
	local_delay(10);
	reg_val&=~(0x1<<25);//disable read Time-out
	reg_val|=(0x1<<26);//clear reset PHY FIFO
	mctl_write_w(reg_val,PGCR0);
	return;

}

void auto_detect_dram_size(__dram_para_t *para)
{
	unsigned int i=0,j=0;
	unsigned int reg_val=0,ret=0,cnt=0;
	for(i=0;i<64;i++)
	{
		mctl_write_w((i%2)?(0x40000000 + 4*i):(~(0x40000000 + 4*i)),0x40000000 + 4*i);
	}
	reg_val=mctl_read_w(MC_WORK_MODE);
	paraconfig(&reg_val,0xf<<8,0x6<<8);//512B
	reg_val|=(0x1<<15);
	reg_val|=(0xf<<4);//16 row
	mctl_write_w(reg_val,MC_WORK_MODE);
	//row detect
	for(i=11;i<=16;i++)
	{
		ret = 0x40000000 + (1<<(i+9));//row-column
		cnt = 0;
		for(j=0;j<64;j++)
		{
			if(mctl_read_w(0x40000000 + j*4) == mctl_read_w(ret + j*4))
			{
				cnt++;
			}
		}
		if(cnt == 64)
		{
			break;
		}
	}
	if(i >= 16)
		i = 16;
	paraconfig(&(para->dram_para1), 0xffU<<20, i<<20);//row width confirmed
	//pagesize(column)detect
	reg_val=mctl_read_w(MC_WORK_MODE);
	paraconfig(&reg_val,0xf<<4,0xa<<4);//11rows
	paraconfig(&reg_val,0xf<<8,0xa<<8);//8KB
	mctl_write_w(reg_val,MC_WORK_MODE);
	for(i=9;i<=13;i++)
	{
		ret = 0x40000000 + (0x1U<<i);//column
		cnt = 0;
		for(j=0;j<64;j++)
		{
			if(mctl_read_w(0x40000000 + j*4) == mctl_read_w(ret + j*4))
			{
				cnt++;
			}
		}
		if(cnt == 64)
		{
			break;
		}
	}
	if(i >= 13)
		i = 13;
	if(i==9)
		i = 0;
	else
		i = (0x1U<<(i-10));
	paraconfig(&(para->dram_para1), 0xfU<<16, i<<16);//pagesize confirmed


}

#ifdef FPGA_PLATFORM

unsigned int mctl_soft_training(void)
{
	int i, j;
	unsigned int k;
	const unsigned int words[64] = {	0x12345678, 0xaaaaaaaa, 0x55555555, 0x00000000, 0x11223344, 0xffffffff, 0x55aaaa55, 0xaa5555aa,
								0x23456789, 0x18481113, 0x01561212, 0x12156156, 0x32564661, 0x61532544, 0x62658451, 0x15564795,
								0x10234567, 0x54515152, 0x33333333, 0xcccccccc, 0x33cccc33, 0x3c3c3c3c, 0x69696969, 0x15246412,
								0x56324789, 0x55668899, 0x99887744, 0x00000000, 0x33669988, 0x66554477, 0x5555aaaa, 0x54546212,
								0x21465854, 0x66998877, 0xf0f0f0f0, 0x0f0f0f0f, 0x77777777, 0xeeeeeeee, 0x3333cccc, 0x52465621,
								0x24985463, 0x22335599, 0x78945623, 0xff00ff00, 0x00ff00ff, 0x55aa55aa, 0x66996699, 0x66544215,
								0x54484653, 0x66558877, 0x36925814, 0x58694712, 0x11223344, 0xffffffff, 0x96969696, 0x65448861,
								0x48898111, 0x22558833, 0x69584701, 0x56874123, 0x11223344, 0xffffffff, 0x99669966, 0x36544551};

	for(i=0;i<4;i++)
		delay[i]=0;
	for(i=0; i<0x10; i++)
		{
			for(j=0; j<0x4; j++)
			{
				mctl_write_w(((3-j)<<20)|((0xf-i)<<16)|0x400f,MCTL_CTL_BASE+0xc);
				for(k=0; k<0x10; k++);
				for(k=0; k<(1<<10); k++)
				{
					mctl_write_w(words[k%64],DRAM_BASE_ADDR+(k<<2));
				}

				for(k=0; k<(1<<10); k++)
				{
					if(words[k%64] != mctl_read_w(DRAM_BASE_ADDR+(k<<2)))
					break;
				}

				if(k==(1<<10))
				{
					delay[j]=((3-j)<<20)|((0xf-i)<<16)|0x400f;
				}
			}
		}

	if(delay[0]!=0)
	{
		mctl_write_w(delay[0],MCTL_CTL_BASE+0xc);
	}
	else if(delay[1]!=0)
	{
		mctl_write_w(delay[1],MCTL_CTL_BASE+0xc);
	}
	else if(delay[2]!=0)
	{
		mctl_write_w(delay[2],MCTL_CTL_BASE+0xc);
	}
	else if(delay[3]!=0)
	{
		mctl_write_w(delay[3],MCTL_CTL_BASE+0xc);
	}

		return 1;
}
#endif



//*****************************************************************************
//	signed int init_DRAM(int type)
//  Description:	System init dram
//
//	Arguments:		type:	0: no lock		1: get the fixed parameters & auto detect & lock
//
//	Return Value:	0: fail
//					others: pass
//*****************************************************************************
signed int init_DRAM(int type, __dram_para_t *para)
{
	unsigned int ret_val=0;
	//unsigned int reg_val=0;
	mctl_sys_init(para);
#if defined DRAMC_COM_REG_TEST || defined DRAMC_CTR_REG_TEST || defined DRAMC_PROC_REG_TEST
	return 0;
#endif
	ret_val=mctl_channel_init(0,para);
	if(ret_val==0)
		return 0;
	//----------------------------------------------------------------------------------------------------------------------------------------
#ifndef FPGA_PLATFORM
	reg_val = mctl_read_w(MC_CCR);
	reg_val|=(0x1U)<<31;//after initil before write or read must write 1 ~~31bit
	mctl_write_w(reg_val,MC_CCR);
	local_delay(20);
#endif
	//----------------------------------------------------------------------------------------------------------------------------------------

#ifdef DRAMC_SIZE_DETECT
	auto_detect_rank_dq(para);

//	if(!(para->dram_tpr13&0x1))
//		auto_detect_dram_size(para);
#endif

#ifndef FPGA_PLATFORM
	mctl_com_init(para);
#endif
	ret_val= DRAMC_get_dram_size();
	para->dram_para1 &= ~0xffff;
	para->dram_para1|=(ret_val);
	return ret_val;
}

//*****************************************************************************
//	unsigned int mctl_init()

//  Description:	FOR SD SIMULATION
//
//	Arguments:		None
//
//	Return Value:	1: Success		0: Fail
//*****************************************************************************
unsigned int mctl_init(void)
{
	signed int ret_val=0;

	__dram_para_t dram_para;
	//set the parameter
#ifdef DRAMC_MR_TEST
	dram_para.dram_clk			= 796;	//default:480
#else
	dram_para.dram_clk			= 480;	//default:480
#endif
#if defined DRAM_TYPE_DDR3
	dram_para.dram_type			= 3;	//dram_type	DDR2: 2	DDR3: 3	LPDDR2: 6  LPDDR3: 7
#elif defined DRAM_TYPE_DDR2
	dram_para.dram_type			= 2;	//dram_type	DDR2: 2	DDR3: 3	LPDDR2: 6  LPDDR3: 7
#elif defined DRAM_TYPE_LPDDR2
	dram_para.dram_type			= 6;	//dram_type	DDR2: 2	DDR3: 3	LPDDR2: 6  LPDDR3: 7
#else
	dram_para.dram_type			= 7;	//dram_type	DDR2: 2	DDR3: 3	LPDDR2: 6  LPDDR3: 7
#endif

#ifndef FPGA_PLATFORM
	auto_set_dram_para(&dram_para);
#endif
#ifdef AUTO_SET_DRAM_TEST_PARA
	auto_set_test_para(&dram_para);		//added by liuke for aw1689 simulation,20140829
#endif
	ret_val = init_DRAM(0, &dram_para);//signed int init_DRAM(int type, void *para);
	return ret_val;
}

