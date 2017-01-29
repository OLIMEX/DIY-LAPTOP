/**************************************************************************
*  AW5306_Drv.h
* 
*  AW5306 Driver code version 1.0
* 
*  Create Date : 2012/06/25
* 
*  Modify Date : 
*
*  Create by   : wuhaijun
* 
**************************************************************************/

#ifndef AW5306_DRV_H

#define AW5306_DRV_H

//#define WATER_PROOF						//water solution

#define	MAX_POINT					5

#define NUM_TX						21		// TX number of TOUCH IC
#define NUM_RX						12	  	// RX number of TOUCH IC

//#define NEWBASE_PROCESS			//new base process need test!!!

#define ABS(X)                  ((X > 0) ? (X) : (-X))


typedef enum{
	RawDataMode = 0,
	DeltaMode,
	MonitorMode	
}enumWorkMode;

typedef enum{
	BASE_INITIAL,
	BASE_FAST_TRACE,
	BASE_STABLE,
	TEMP_DRIFT
} CompensateMode;

typedef struct {
	unsigned short  Base[NUM_TX][NUM_RX];	
	unsigned short	ChipBase[NUM_TX][NUM_RX];
	signed char   Flag[NUM_TX][NUM_RX];	
	signed char		BaseCnt[NUM_TX][NUM_RX];
	unsigned char   CompensateFlag;    			
	unsigned char   TraceTempIncCnt;   			
	unsigned char   TraceTempDecCnt;   			
	unsigned char   CompensateStateFrameCnt;		
	short 	        LastMaxDiff;              		
	CompensateMode  CompensateState;  
	unsigned int	InitialFrameCnt;
	unsigned char	PosBigAreaTouchFlag;	
	unsigned char	NegBigAreaTouchFlag;
	unsigned char	BigAreaFirstFlag;				
	unsigned char	BigAreaChangeFlag;	
	unsigned short	BigTouchFrame;				
	unsigned short	FrameCnt;
	unsigned char	LongStableCnt;
	unsigned char   PosPeakCnt;							
	unsigned char   NegPeakCnt;	
	unsigned char 	PeakCheckFrameCnt;
	unsigned char	BaseFrozen;
	unsigned char   PosPeakCompensateCnt[MAX_POINT];
	unsigned char   NegPeakCompensateCnt[MAX_POINT];
}STRUCTBASE;

typedef struct {
	unsigned char   Peak[MAX_POINT][2];		
	unsigned char 	LastPeak[MAX_POINT][2];	
	unsigned char   NegPeak[MAX_POINT][2];	
	unsigned char   CurrentPointNum;						
	unsigned char   CurrentNegPointNum;					
	unsigned char   LastPointNum;
}STRUCTPEAK;

typedef	struct {
	unsigned short	X,Y;						// X,Y coordinate
	unsigned char	PointID;			// Assigned point ID
	unsigned char	Event;				// Event of current point
}STRUCTPOINT;

typedef	struct {
	STRUCTPOINT 	PointInfo[MAX_POINT];
	STRUCTPOINT		RptPoint[MAX_POINT];
	unsigned char 	PointNum;
	unsigned char 	LastPointNum;
	unsigned char 	NegPointNum;
	unsigned char	FilterPointCnt;
	unsigned char 	FirstLiftUpFlag;
	unsigned char 	TouchStatus;
	unsigned char	PointHoldCnt[MAX_POINT];
	unsigned char	PointPressCnt[MAX_POINT];

}STRUCTFRAME;

typedef struct {
	unsigned char fileflag[14];
	unsigned char TXOFFSET[(NUM_TX+1)/2];
	unsigned char RXOFFSET[(NUM_RX+1)/2];
	unsigned char TXCAC[NUM_TX];
	unsigned char RXCAC[NUM_RX];
	unsigned char TXGAIN[NUM_TX];
	short SOFTOFFSET[NUM_TX][NUM_RX];
}STRUCTCALI;

void AW5306_TP_Init(void);
void AW5306_TP_Reinit(void);
void AW5306_Sleep(void);
char AW5306_TouchProcess(void);
void AW5306_ChargeMode(char mode);
unsigned char AW5306_GetPointNum(void);
unsigned char AW5306_GetPeakNum(void);
char AW5306_GetPoint(int *x,int *y, int *id, int *event,char Index);
void AW5306_GetBase(unsigned short *data, char x,char y);
void AW5306_GetDiff(short *data, char x,char y);
char AW5306_GetPeak(unsigned char *x,unsigned char *y,unsigned char Index);
char AW5306_GetNegPeak(unsigned char *x,unsigned char *y,unsigned char Index);
char AW5306_GetCalcPoint(unsigned short *x,unsigned short *y,unsigned char Index);
char AW5306_CLB(void);
void AW5306_CLB_GetCfg(void);
void AW5306_CLB_WriteCfg(void);
void TP_Force_Calibration(void);
void FreqScan(unsigned char BaseFreq);



#endif
