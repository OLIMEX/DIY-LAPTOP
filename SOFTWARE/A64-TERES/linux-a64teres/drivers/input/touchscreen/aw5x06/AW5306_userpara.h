#ifndef AW5306_USERPARA_H

#define AW5306_USERPARA_H

typedef struct {
	unsigned char	TX_LOCAL;	//					15		//TX number of TP
	unsigned char	RX_LOCAL;	//					10		//RX number of TP
	unsigned char	TX_ORDER[22];	// TX ORDER
	unsigned char	RX_ORDER[12];	// RX mapping in inverted order
	unsigned char	RX_START;	//RX START LINE
	unsigned char	HAVE_KEY_LINE;	// 0: no KEY line, 1: have key line on TX line TX_LOCAL-1
	unsigned char	KeyLineValid[16];

	unsigned short	MAPPING_MAX_X;	//   320
	unsigned short	MAPPING_MAX_Y;	//   460

	unsigned short	GainClbDeltaMin;	// Expected minimum delta for GAIN calibration
	unsigned short	GainClbDeltaMax;	// Expected maximum delta for GAIN calibration
	unsigned short	KeyLineDeltaMin;
	unsigned short	KeyLineDeltaMax;
	unsigned short	OffsetClbExpectedMin;	// Expected minimum data for OFFSET calibration
	unsigned short	OffsetClbExpectedMax;	// Expected minimum data for OFFSET calibration
	unsigned short	RawDataDeviation;	// Maximum deviation in a frame
	unsigned short	CacMultiCoef;

	unsigned short	RawDataCheckMin;
	unsigned short	RawDataCheckMax;

	unsigned short  FLYING_TH;
	unsigned short 	MOVING_TH;
	unsigned short 	MOVING_ACCELER;

	unsigned char	PEAK_TH;
	unsigned char	GROUP_TH;
	unsigned char	BIGAREA_TH;
	unsigned char	BIGAREA_CNT;
	unsigned char	BIGAREA_FRESHCNT;

	unsigned char	CACULATE_COEF;
	
	unsigned char 	FIRST_CALI;
	unsigned char	RAWDATA_DUMP_SWITCH;
	unsigned char	MULTI_SCANFREQ;
	unsigned char	BASE_FREQ;
	unsigned char	FREQ_OFFSET;
	
	unsigned char	ESD_PROTECT;
	
	unsigned char	MARGIN_COMPENSATE;
	unsigned char	MARGIN_COMP_DATA_UP;
	unsigned char	MARGIN_COMP_DATA_DOWN;
	unsigned char	MARGIN_COMP_DATA_LEFT;
	unsigned char	MARGIN_COMP_DATA_RIGHT;
	
	unsigned char	POINT_RELEASEHOLD;
	unsigned char	MARGIN_RELEASEHOLD;
	unsigned char	POINT_PRESSHOLD;
	unsigned char	KEY_PRESSHOLD;
	
	unsigned char	PEAK_ROW_COMPENSATE;
	unsigned char	PEAK_COL_COMPENSATE;
	unsigned char	PEAK_COMPENSATE_COEF;
	
	unsigned char	LCD_NOISE_PROCESS;
	unsigned char	LCD_NOISETH;
	
	unsigned char	FALSE_PEAK_PROCESS;
	unsigned char	FALSE_PEAK_TH;
	
	unsigned char	STABLE_DELTA_X;
	unsigned char	STABLE_DELTA_Y;
	
	unsigned char	DEBUG_LEVEL;
	
	unsigned char	FAST_FRAME;
	unsigned char	SLOW_FRAME;
	
	unsigned char	GAIN_CLB_SEPERATE;

}AW5306_UCF;

void AW5306_User_Init(void);
void AW5306_User_Cfg1(void);

#endif
