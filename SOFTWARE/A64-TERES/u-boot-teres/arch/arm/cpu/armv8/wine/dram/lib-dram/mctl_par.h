#ifndef   MCTL_PAR_H
#define   MCTL_PAR_H

//DDR2_32B				DDR2_32B_128Mx8x4
//DDR3_32B				DDR3_32B_256Mx8x4

//#ifdef SYSTEM_SIMULATION
//#define DRAM_2T_ENABLE				1
//#define DRAM_DUAL_CS_ENABLE		1
//#else
//#define DRAM_2T_ENABLE				1
//#define	DRAM_DUAL_CS_ENABLE		0
//#endif

//******************************************************************************
//DDR2 (x16)
//******************************************************************************
#ifdef DDR2_FPGA_S2C_32B
//DDR2 128Mx8 (128M Byte), total 1GB
#define MCTL_DDR_TYPE			2				//1: DDR, 2: DDR2, 3: DDR3
#define MCTL_BANK_SIZE			8
#define MCTL_ROW_WIDTH			14
#define MCTL_BUS_WIDTH			32
#define MCTL_PAGE_SIZE			4				//unit in KByte for one rank
#define BURST_LENGTH			4				//DDR2 burst length
#define MCTL_RANK_NUM			1				//rank number
#endif

//*****************************************************************************
//DDR2 SDRAM(x32)
//*****************************************************************************
#ifdef DDR2_FPGA_S2C
//DDR2 128Mx8 (128M Byte), total 1GB
#define MCTL_DDR_TYPE			2				//1: DDR, 2: DDR2, 3: DDR3
#define MCTL_BANK_SIZE			8
#define MCTL_ROW_WIDTH			14
#define MCTL_BUS_WIDTH			32
#define MCTL_PAGE_SIZE			4				//unit in KByte for one rank
#define BURST_LENGTH			4				//DDR2 burst length
#define MCTL_RANK_NUM			1				//rank number
#endif

#ifdef DDR2_FPGA_S2C_2C
//DDR2 128Mx8 (128M Byte), total 512MB
#define MCTL_DDR_TYPE			2				//1: DDR, 2: DDR2, 3: DDR3
#define MCTL_BANK_SIZE			8
#define MCTL_ROW_WIDTH			14
#define MCTL_BUS_WIDTH			16
#define MCTL_PAGE_SIZE			2				//unit in KByte for one rank
#endif

#ifdef DDR2_32B
//DDR2 128Mx8 (128M Byte)
#define MCTL_DDR_TYPE			2				//1: DDR, 2: DDR2, 3: DDR3
#define MCTL_BANK_SIZE			8
#define MCTL_ROW_WIDTH			14
#define MCTL_BUS_WIDTH			32
#define MCTL_PAGE_SIZE			4				//unit in KByte for one rank
#endif

//*****************************************************************************
//DDR3 SDRAM(x32)
//*****************************************************************************
#ifdef DDR3_32B
//DDR3 512Mx8 (512M Byte)
#define MCTL_DDR_TYPE			3				//1: DDR, 2: DDR2, 3: DDR3
#define MCTL_BANK_SIZE			8
#define MCTL_ROW_WIDTH			16
#define MCTL_BUS_WIDTH			32
#define MCTL_PAGE_SIZE			4				//unit in KByte for one rank
#endif

#ifdef DDR3_32B_4GB
//DDR3 1024Mx32 (4GB Byte)
#define MCTL_DDR_TYPE			3				//1: DDR, 2: DDR2, 3: DDR3
#define MCTL_BANK_SIZE			8
#define MCTL_ROW_WIDTH			16
#define MCTL_BUS_WIDTH			32

#ifdef HALF_DQ_TEST
#define MCTL_PAGE_SIZE			4				//unit in KByte for one rank
#else
#define MCTL_PAGE_SIZE			8				//unit in KByte for one rank
#endif

#endif


#ifdef DDR3_32B_128M8
//DDR3 128Mx8 (128M Byte)
#define MCTL_DDR_TYPE			3				//1: DDR, 2: DDR2, 3: DDR3
#define MCTL_BANK_SIZE			8
#define MCTL_ROW_WIDTH			14
#define MCTL_BUS_WIDTH			32
#define MCTL_PAGE_SIZE			4				//unit in KByte for one rank
#endif


#ifdef DDR3_32B_512M8
//DDR3 128Mx8 (128M Byte)
#define MCTL_DDR_TYPE			3				//1: DDR, 2: DDR2, 3: DDR3
#define MCTL_BANK_SIZE			8
#define MCTL_ROW_WIDTH			16
#define MCTL_BUS_WIDTH			32
#define MCTL_PAGE_SIZE			4				//unit in KByte for one rank (for debug)
#endif

//*****************************************************************************
//LPDDR2 SDRAM(x32)
//*****************************************************************************
#ifdef LPDDR2_FPGA_S2C_2CS_2CH
//LPDDR2 128Mx32 (512M Byte), total 2GB
#define MCTL_DDR_TYPE			6				//1: DDR, 2: DDR2, 3: DDR3
#define MCTL_BANK_SIZE			8
#define MCTL_ROW_WIDTH			14
#define MCTL_BUS_WIDTH			32
#define MCTL_PAGE_SIZE			4				//unit in KByte for one rank
#endif

#ifdef LPDDR2_32B
//LPDDR2 128Mx32 (512MB)
#define MCTL_DDR_TYPE			6				//1: DDR, 2: DDR2, 3: DDR3
#define MCTL_BANK_SIZE			8
#define MCTL_ROW_WIDTH			14
#define MCTL_BUS_WIDTH			32
#define MCTL_PAGE_SIZE			4				//unit in KByte for one rank
#endif

#if (MCTL_DDR_TYPE==2)		//DDR2-800

#define MCTL_TREFI			7800  //max. in ns
#define MCTL_TMRD			2
#define MCTL_TRFC			400			//min. in ns
#define MCTL_TRP			6
#define MCTL_TPREA			1
#define MCTL_TRTW			2
#define MCTL_TAL			0
#define MCTL_TCL			6
#define MCTL_TCWL			5
#define MCTL_TRAS			18
#define MCTL_TRC			24
#define MCTL_TRCD			6
#define MCTL_TRRD			4
#define MCTL_TRTP			3
#define MCTL_TWR			6
#define MCTL_TWTR			4
#define MCTL_TEXSR			200
#define MCTL_TXP			2
#define MCTL_TXPDLL			6
#define MCTL_TZQCS			0
#define MCTL_TZQCSI			0
#define MCTL_TDQS			1
#define MCTL_TCKSRE			0
#define MCTL_TCKSRX			0
#define MCTL_TCKE			3
#define MCTL_TMOD			0
#define MCTL_TRSTL			0
#define MCTL_TZQCL			0
#define MCTL_TMRR			2
#define MCTL_TCKESR			2
#define MCTL_TDPD			0

#define MCTL_MR0			0xa63
#define MCTL_MR1			0x0
#define MCTL_MR2			0x0
#define MCTL_MR3			0x0
#define MCTL_PITMG0         0x02010101

#elif(MCTL_DDR_TYPE==3)		//DDR3-1333
#define MCTL_TREFI		7800   	//max. in ns
#define MCTL_TMRD			4
#define MCTL_TRFC			350				//min. in ns
#define MCTL_TRP			9
#define MCTL_TPREA		0
#define MCTL_TRTW			2
#define MCTL_TAL			0
#define MCTL_TCL			9
#define MCTL_TCWL			8
#define MCTL_TRAS			24
#define MCTL_TRC			33
#define MCTL_TRCD			9
#define MCTL_TRRD			4
#define MCTL_TRTP			5
#define MCTL_TWR			10
#define MCTL_TWTR			5
#define MCTL_TEXSR			512
#define MCTL_TXP			5
#define MCTL_TXPDLL			16
#define MCTL_TZQCS			64
#define MCTL_TZQCSI			0
#define MCTL_TDQS			1
#define MCTL_TCKSRE			7
#define MCTL_TCKSRX			7
#define MCTL_TCKE			4
#define MCTL_TMOD			12
#define MCTL_TRSTL			80
#define MCTL_TZQCL			512
#define MCTL_TMRR			2
#define MCTL_TCKESR			5
#define MCTL_TDPD			0

#define MCTL_MR0			0x1a50
#define MCTL_MR1			0x0
#define MCTL_MR2			0x18
#define MCTL_MR3			0x0

#elif(MCTL_DDR_TYPE==5)		//LPDDR

#elif(MCTL_DDR_TYPE==6)		//LPDDR2-800

#define MCTL_TREFI			3900
#define MCTL_TMRD			2
#define MCTL_TRFC			52
#define MCTL_TRP			8
#define MCTL_TPREA			0
#define MCTL_TRTW			2
#define MCTL_TAL			0
#define MCTL_TCL			6
#define MCTL_TCWL			4
#define MCTL_TRAS			18
#define MCTL_TRC			27
#define MCTL_TRCD			8
#define MCTL_TRRD			4
#define MCTL_TRTP			3
#define MCTL_TWR			6
#define MCTL_TWTR			3
#define MCTL_TEXSR			200
#define MCTL_TXP			3
#define MCTL_TXPDLL			6
#define MCTL_TZQCS			0
#define MCTL_TZQCSI			0
#define MCTL_TDQS			1
#define MCTL_TCKSRE			5
#define MCTL_TCKSRX			5
#define MCTL_TCKE			3
#define MCTL_TMOD			0
#define MCTL_TRSTL			0
#define MCTL_TZQCL			0
#define MCTL_TMRR			2
#define MCTL_TCKESR			6
#define MCTL_TDPD			0

#define MCTL_MR0			0x0
#define MCTL_MR1			0x92
#define MCTL_MR2			0x4
#define MCTL_MR3			0x2

#else              //LPDDR3, #elif(MCTL_DDR_TYPE==7)

#define MCTL_TREFI			3900
#define MCTL_TMRD			2
#define MCTL_TRFC			52
#define MCTL_TRP			8
#define MCTL_TPREA			0
#define MCTL_TRTW			2
#define MCTL_TAL			0
#define MCTL_TCL			6
#define MCTL_TCWL			4
#define MCTL_TRAS			18
#define MCTL_TRC			27
#define MCTL_TRCD			8
#define MCTL_TRRD			4
#define MCTL_TRTP			3
#define MCTL_TWR			6
#define MCTL_TWTR			3
#define MCTL_TEXSR			200
#define MCTL_TXP			3
#define MCTL_TXPDLL			6
#define MCTL_TZQCS			0
#define MCTL_TZQCSI			0
#define MCTL_TDQS			1
#define MCTL_TCKSRE			5
#define MCTL_TCKSRX			5
#define MCTL_TCKE			3
#define MCTL_TMOD			0
#define MCTL_TRSTL			0
#define MCTL_TZQCL			0
#define MCTL_TMRR			2
#define MCTL_TCKESR			6
#define MCTL_TDPD			0

#define MCTL_MR0			0x0
#define MCTL_MR1			0x92
#define MCTL_MR2			0x4
#define MCTL_MR3			0x2

#endif

#endif  //MCTL_PAR_H
