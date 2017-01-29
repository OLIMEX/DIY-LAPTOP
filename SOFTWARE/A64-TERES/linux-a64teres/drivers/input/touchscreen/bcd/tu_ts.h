#ifndef _LINUX_TU_I2C_MTOUCH_H
#define _LINUX_TU_I2C_MTOUCH_H

//Device Parameter Setting
#define TU_I2C_NAME		"tu_ts"
#define DEV_I2C_ADDRESS 	0x5F
#define CHANNEL_X_SIZE 	15
#define CHANNEL_Y_SIZE 	10
#define MAX_POINT_SIZE 	5


//Auto Gen. Parameters
#define REPORT_BUF_SIZE	64						//(3+(MAX_POINT_SIZE*5))
#define AA_X_SIZE 			((CHANNEL_X_SIZE-1)<<6)	//Touch Resolution
#define AA_Y_SIZE 			((CHANNEL_Y_SIZE-1)<<6)	//Touch Resolution
#define AA_MAX_X			AA_X_SIZE-1				//Touch Max X
#define AA_MAX_Y			AA_Y_SIZE-1				//Touch Max Y

//#define I2C_NAME_SIZE		20

#define COMMAND_COUNT	5
#define NORM_CMD_LENG	4

static u_int8_t command_list[COMMAND_COUNT][NORM_CMD_LENG] = 
{
	{0x0E, 0x13, 0x00, 0x00},	//sleep
	{0x0E, 0x01, 0x00, 0x00},	//Resume
	{0x0E, 0x03, 0x00, 0x00},	//Disable Touch
	{0x0E, 0x01, 0x00, 0x00},	//Enable Touch
	{0x0E, 0x12, 0x00, 0x00}	//Chip Reset
};



enum tu_registers {

	TU_RMOD = 0x0,		//0xb1
	TU_KEY_CODE,		        //0x00
	TU_POINTS,		        //Number of touch points

	TU_1_POS_X_LOW,
	TU_1_POS_X_HI,
	TU_1_POS_Y_LOW,
	TU_1_POS_Y_HI,
	TU_1_ID_STATUS,		

	TU_2_POS_X_LOW,
	TU_2_POS_X_HI,
	TU_2_POS_Y_LOW,
	TU_2_POS_Y_HI,
	TU_2_ID_STATUS,

	TU_3_POS_X_LOW,
	TU_3_POS_X_HI,
	TU_3_POS_Y_LOW,
	TU_3_POS_Y_HI,
	TU_3_ID_STATUS,

	TU_4_POS_X_LOW,
	TU_4_POS_X_HI,
	TU_4_POS_Y_LOW,
	TU_4_POS_Y_HI,
	TU_4_ID_STATUS,

	TU_5_POS_X_LOW,
	TU_5_POS_X_HI,
	TU_5_POS_Y_LOW,
	TU_5_POS_Y_HI,
	TU_5_ID_STATUS,

	TU_6_POS_X_LOW,
	TU_6_POS_X_HI,
	TU_6_POS_Y_LOW,
	TU_6_POS_Y_HI,
	TU_6_ID_STATUS,

	TU_7_POS_X_LOW,
	TU_7_POS_X_HI,
	TU_7_POS_Y_LOW,
	TU_7_POS_Y_HI,
	TU_7_ID_STATUS,

	TU_8_POS_X_LOW,
	TU_8_POS_X_HI,
	TU_8_POS_Y_LOW,
	TU_8_POS_Y_HI,
	TU_8_ID_STATUS,

	TU_9_POS_X_LOW,
	TU_9_POS_X_HI,
	TU_9_POS_Y_LOW,
	TU_9_POS_Y_HI,
	TU_9_ID_STATUS,

	TU_10_POS_X_LOW,
	TU_10_POS_X_HI,
	TU_10_POS_Y_LOW,
	TU_10_POS_Y_HI,
	TU_10_ID_STATUS,

	TU_DATA_SIZE
};

enum tu_key_code {
	TOUCH_KEY_REL = 0x0,
	TOUCH_KEY_HOME,
	TOUCH_KEY_BACK,
	TOUCH_KEY_MENU,
	TOUCH_KEY_4_RESERVED,
	TOUCH_KEY_CALL,
	TOUCH_KEY_6_RESERVED,
	TOUCH_KEY_VOL_UP,
	TOUCH_KEY_VOL_DOWN,
};

#endif 	/* _LINUX_TU_I2C_MTOUCH_H */



