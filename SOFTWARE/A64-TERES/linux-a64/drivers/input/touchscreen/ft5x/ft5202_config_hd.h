#ifndef __FT5X02_CONFIG_H__ 
#define __FT5X02_CONFIG_H__ 
/*FT5X02 config*/


#define FT5X02_FIRMWARE_ID			14
#define FT5X02_OTP_PARAM_ID			0
#define FT5X02_CUSTOMER_ID			121
#define FT5X02_CHIPER_ID			2
#define FT5X02_RESOLUTION_X			600
#define FT5X02_RESOLUTION_Y			1024
#define FT5X02_LEMDA_X			57//45//48
#define FT5X02_LEMDA_Y			65//51//53
#define FT5X02_KX			242//194//191
#define FT5X02_KY			274//214//213
#define FT5X02_DIRECTION			1
#define FT5X02_KX_LR			457//366//391
#define FT5X02_KY_UD			523//408//430
#define FT5X02_POINTS_SUPPORTED			5
#define FT5X02_THGROUP			100
#define FT5X02_THPEAK			60
#define FT5X02_THDIFF			2560
#define FT5X02_MAX_TOUCH_VALUE			1200
#define FT5X02_DRAW_LINE_TH			250
#define FT5X02_PWMODE_CTRL			1
#define FT5X02_PERIOD_ACTIVE			16
#define FT5X02_TIME_ENTER_MONITOR			10
#define FT5X02_PERIOD_MONITOR			40
#define FT5X02_FILTER_FRAME_NOISE			3
#define FT5X02_POWERNOISE_FILTER_TH			60
#define FT5X02_DIFFDATA_HADDLE_VALUE			-100
#define FT5X02_FACE_DETECT_MODE			0
#define FT5X02_FACE_DETECT_STATISTICS_TX_NUM			3
#define FT5X02_FACE_DETECT_PRE_VALUE			20
#define FT5X02_FACE_DETECT_NUM			10
#define FT5X02_FACE_DETECT_LAST_TIME			1000
#define FT5X02_BIGAREA_PEAK_VALUE_MIN			255
#define FT5X02_BIGAREA_DIFF_VALUE_OVER_NUM			30
#define FT5X02_BIGAREA_POINT_AUTO_CLEAR_TIME			3000
#define FT5X02_ABNORMAL_DIFF_VALUE			60
#define FT5X02_ABNORMAL_DIFF_NUM			10
#define FT5X02_ABNORMAL_DIFF_LAST_FRAME			30
#define FT5X02_START_RX			0
#define FT5X02_ADC_TARGET			8500
#define FT5X02_ESD_FILTER_FRAME			1
#define FT5X02_MODE			1
#define FT5X02_PMODE			0
#define FT5X02_ERR			0
#define FT5X02_AUTO_CLB_MODE			255
#define FT5X02_STATE			1
#define FT5X02_HIGH_SPEED_TH			183
#define FT5X02_MID_SPEED_TH			28
#define FT5X02_STATIC_TH			158
#define FT5X02_THFALSE_TOUCH_PEAK			246


unsigned char g_ft5x02_tx_num = 15;
unsigned char g_ft5x02_rx_num = 10;
unsigned char g_ft5x02_gain = 10;
unsigned char g_ft5x02_voltage = 3;
unsigned char g_ft5x02_scanselect = 8;
unsigned char g_ft5x02_tx_order[] = {14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
unsigned char g_ft5x02_tx_offset = 1;
unsigned char g_ft5x02_tx_cap[] = {50,45,35,33,33,33,33,30,30,29,29,29,27,27,25};
unsigned char g_ft5x02_rx_order[] = {9,8,7,6,5,4,3,2,1,0};
unsigned char g_ft5x02_rx_offset[] = {136,152,136,136,136};
unsigned char g_ft5x02_rx_cap[] = {100,100,100,100,100,100,100,100,100,100};


#endif
