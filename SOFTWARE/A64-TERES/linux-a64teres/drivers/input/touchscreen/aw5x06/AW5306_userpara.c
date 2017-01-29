#include "AW5306_Reg.h"
#include "AW5306_Drv.h"
#include <linux/string.h>
#include "AW5306_userpara.h"
#include <linux/kernel.h>

#define	POS_PRECISION				64

extern int screen_max_x;
extern int screen_max_y;
extern AW5306_UCF	AWTPCfg;
extern STRUCTCALI	AW_Cali;
extern char AW5306_WorkMode;

extern void AW5306_CLB_WriteCfg(void);
extern int AW_I2C_WriteByte(unsigned char addr, unsigned char data);
extern unsigned char AW_I2C_ReadByte(unsigned char addr);
extern unsigned char AW_I2C_ReadXByte( unsigned char *buf, unsigned char addr, unsigned short len);
extern unsigned char AW5306_RAWDATACHK(void);

const STRUCTCALI Default_Cali1 = 
{
	"AWINIC TP CALI",
	//{0x33,0x23,0x22,0x22,0x22,0x22,0x22,0x02,0x22,0x22},       //TXOFFSET
	{0x32,0x32,0x23,0x32,0x33,0x33,0x33,0x03,0x22,0x22},       //TXOFFSET
        //{0x9A,0xA9,0xAA,0xA9,0x9B,0x00},                             //RXOFFSET
        {0x35,0x44,0x55,0x54,0x34,0x00},                             //RXOFFSET
        //{0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c},//TXCAC
        {0x2C,0x2B,0x2B,0x2A,0x2A,0x2C,0x2C,0x2C,0x2C,0x2C,0x2D,0x2D,0x2D,0x2D,0x31,0x2C,0x2C,0x2C,0x2C,0x2C},//TXCAC
        //{0x3d,0x3c,0x3c,0x3c,0x3e,0x3a,0x3a,0x3e,0x3c,0x3b,0x3c,0x3c},//RXCAC
        {0x84,0x84,0x82,0x82,0x80,0x86,0x86,0x80,0x8C,0x82,0x84,0x84},//RXCAC
        //{0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x2e,0x2e,0x0e,0x0e,0x0e,0x0e,0x0e},//TXGAIN
        {0x88,0x88,0x88,0x88,0x88,0x68,0x68,0x68,0x68,0x68,0x48,0x48,0x48,0x48,0x28,0x08,0x08,0x08,0x08,0x08},//TXGAIN
};

 AW5306_UCF Default_UCF = 
{
	15,	//TX_NUM
	10,	//RX_NUM
	{4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,2,1,0,0},	//TX_ORDER
	{0,1,2,3,4,5,6,7,8,9,10,11},	//RX_ORDER
	0,	//RX_START
	0,	//HAVE_KEY_LINE
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},	//KeyLineValid

	480,	//MAPPING_MAX_X
	800,	//MAPPING_MAX_Y
	
	700,	//GainClbDeltaMin
	800,	//GainClbDeltaMax
	500,	//KeyLineDeltaMin
	600,	//KeyLineDeltaMax
	8300,	//OffsetClbExpectedMin
	8500,	//OffsetClbExpectedMax
	300,	//RawDataDeviation
	6,		//CacMultiCoef

	7000,	//RawDataCheckMin
	10000,	//RawDataCheckMax

	200,	//FLYING_TH
	150,	//MOVING_TH
	60,		//MOVING_ACCELER
	
	60,		//PEAK_TH
	70,		//GROUP_TH
	80,		//BIGAREA_TH
	25,		//BIGAREA_CNT
	100,	//BIGAREA_FRESHCNT

	1,		//CACULATE_COEF
	
	1,	//FIRST_CALI
	0,	//RAWDATA_DUMP_SWITCH
	
	0,	//MULTI_SCANFREQ
	9,	//BASE_FREQ
	0x83,	//FREQ_OFFSET
	
	1,	//ESD_PROTECT
	
	0,	//MARGIN_COMPENSATE
	0,	//MARGIN_COMP_DATA_UP
	0,	//MARGIN_COMP_DATA_DOWN
	0,	//MARGIN_COMP_DATA_LEFT
	0,	//MARGIN_COMP_DATA_RIGHT
	
	0,	//POINT_RELEASEHOLD
	1,	//MARGIN_RELEASEHOLD
	0,	//POINT_PRESSHOLD
	1,	//KEY_PRESSHOLD
	
	0,	//PEAK_ROW_COMPENSATE
	1,	//PEAK_COL_COMPENSATE
	3,	//PEAK_COMPENSATE_COEF
	
	0,	//LCD_NOISE_PROCESS
	50,	//LCD_NOISETH
	
	0,	//FALSE_PEAK_PROCESS
	100,	//FALSE_PEAK_TH
	
	2,	//STABLE_DELTA_X
	2,	//STABLE_DELTA_Y

	0,	//DEBUG_LEVEL
	
	50,	//FAST_FRAME
	20,	//SLOW_FRAME
	
	0	//GAIN_CLB_SEPERATE
	
};

void AW5306_User_Cfg1(void)
{
	unsigned char i;
		
	for(i=0;i<AWTPCfg.TX_LOCAL;i++)
	{
		AW_I2C_WriteByte(SA_TX_INDEX0+i,AWTPCfg.TX_ORDER[i]);			//TX REVERT
	}

	AW_I2C_WriteByte(SA_TX_NUM,AWTPCfg.TX_LOCAL);
	AW_I2C_WriteByte(SA_RX_NUM,AWTPCfg.RX_LOCAL);

	if(1 == AWTPCfg.MULTI_SCANFREQ)
	{
		AW_I2C_WriteByte(SA_SCANFREQ1,AWTPCfg.BASE_FREQ);
		AW_I2C_WriteByte(SA_SCANFREQ2,AWTPCfg.BASE_FREQ);
		AW_I2C_WriteByte(SA_SCANFREQ3,AWTPCfg.BASE_FREQ);
	}
	else
	{
		AW_I2C_WriteByte(SA_SCANFREQ1,AWTPCfg.BASE_FREQ);		//3-5
	}
	//AW_I2C_WriteByte(SA_TCLKDLY,1);
	AW_I2C_WriteByte(SA_RX_START,AWTPCfg.RX_START);
	AW_I2C_WriteByte(SA_SCANTIM,4);		// set to 32 TX cycles mode

	AW_I2C_WriteByte(SA_PAGE,1);
	AW_I2C_WriteByte(SA_CHAMPCFG,0x2b);	//
	AW_I2C_WriteByte(SA_OSCCFG1,AWTPCfg.FREQ_OFFSET);			//
	AW_I2C_WriteByte(SA_PAGE,0);

	AW5306_CLB_WriteCfg();	
}

void AW5306_User_Init(void)
{
	unsigned char ret;

	ret = 0;
	
	if(screen_max_x == 1024 && screen_max_y ==600) {
		Default_UCF.MAPPING_MAX_X = 600;
		Default_UCF.MAPPING_MAX_Y = 1024;
	}
	AW5306_WorkMode = DeltaMode;	//DeltaMode: chip output delta data  RawDataMode: chip output rawdata

	memcpy(&AWTPCfg,&Default_UCF,sizeof(AW5306_UCF));
	memcpy(&AW_Cali,&Default_Cali1,sizeof(STRUCTCALI));		//load default cali value

	AW5306_User_Cfg1();

	ret = AW5306_RAWDATACHK();
	
}
