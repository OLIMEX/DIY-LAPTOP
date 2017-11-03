#ifndef __LCD_EDP_ANX9804_H__
#define __LCD_EDP_ANX9804_H__

#include "panels.h"
/**********************************************************************
 *
 *  anx9804 function
 *
 **********************************************************************/

//define
/***************************************************************/
//  DEV_ADDR = 0x7A or 0x7E , HDMI mode registers
#define HDMI_RST_REG				0x00

#define HDMI_SYS_STATUS				0x01
#define HDMI_SYS_STATE_PLL_LOCK		0x40
#define HDMI_SYS_STATE_HP			0x08//bit position
#define HDMI_SYS_STATE_RSV_DET		0x01//bit position
#define HDMI_SYS_STATE_CLK_DET		0x02


#define HDMI_SYS_CTRL1				0x02
#define HDMI_MODE_ENABLE			0x02//bit position
#define HDMI_VID_STATUS_VID_STABLE  0x01//bit position

#define HDMI_VID_CTRL1				0x03
#define HDMI_VID_CTRL2				0x04
#define HDMI_VID_CAP_CTRL			0x05

#define HDMI_AUDIO_CTRL				0x08


#define HDMI_AUDIO_CTRL1			0x09
#define HDMI_AUDIO_EN				0x80//bit position
#define HDMI_PD_RING_OSC			0x40//bit position

#define HDMI_PLL_MISC_CTRL1 		0x38
#define HDMI_PLL_MISC_CTRL2 		0x39

#define HDMI_DDC_SLV_ADDR_REG 		0x40
#define HDMI_DDC_SLV_SEGADDR_REG 	0x41
#define HDMI_DDC_SLV_OFFADDR_REG 	0x42

#define HDMI_DDC_ACC_CMD_REG 		0x43
#define HDMI_DDC_ACC_ABORT_CURRENT      0x00 // bit position
#define HDMI_DDC_ACC_DDC_WRITE          0x02 // bit position
#define HDMI_DDC_ACC_CLEAR_FIFO         0x05// bit position
#define HDMI_DDC_ACC_I2C_RST            0x06// bit position

#define HDMI_DDC_ACCNUM0_REG 		0x44
#define HDMI_DDC_ACCNUM1_REG 		0x45

#define HDMI_DDC_ACC_DDC_READ 		0x01

#define HDMI_DDC_CHSTATUS_REG 				0x46
#define HDMI_DDC_CHSTATUS_DDCERR	 		0x80	// bit position
#define HDMI_DDC_CHSTATUS_DDC_OCCUPY	 	0x40	// bit position
#define HDMI_DDC_CHSTATUS_FIFO_FULL			0x20	// bit position
#define HDMI_DDC_CHSTATUS_FIFO_EMPT	    	0x10	// bit position
#define HDMI_DDC_CHSTATUS_NOACK	 			0x08	// bit position
#define HDMI_DDC_CHSTATUS_FIFO_RD			0x04	// bit position
#define HDMI_DDC_CHSTATUS_FIFO_WR			0x02	// bit position
#define HDMI_DDC_CHSTATUS_INPRO				0x01	// bit position


#define HDMI_DDC_FIFO_ACC_REG 		0x47
#define HDMI_DDC_FIFOCNT_REG 		0x48

#define HDMI_TMDS_CH0_REG			0x61
#define HDMI_TMDS_CH1_REG			0x62
#define HDMI_TMDS_CH2_REG			0x63
#define HDMI_TMDS_CH3_REG			0x64


#define HDMI_TMDS_CLKCH_CONFIG_REG	0x64
#define HDMI_TMDS_CLKCH_MUTE	 	0x40	// bit position

#define HDMI_CHIP_DEBUG_CTRL1       0x67
#define HDMI_FORCE_HOTPLUG          0x01//bit position 


#define HDMI_INFO_PKTCTRL1_REG  	0x70
#define HDMI_INFO_PKTCTRL1_SPD_RPT 	 	0x80	// bit position
#define HDMI_INFO_PKTCTRL1_SPD_EN	 	0x40	// bit position
#define HDMI_INFO_PKTCTRL1_AVI_RPT		0x20	// bit position
#define HDMI_INFO_PKTCTRL1_AVI_EN	    0x10	// bit position
#define HDMI_INFO_PKTCTRL1_GCP_RPT	 	0x08	// bit position
#define HDMI_INFO_PKTCTRL1_GCP_EN		0x04	// bit position
#define HDMI_INFO_PKTCTRL1_ACR_NEW		0x02	// bit position
#define HDMI_INFO_PKTCTRL1_ACR_EN		0x01	// bit position

#define HDMI_INFO_PKTCTRL2_REG  	0x71
#define HDMI_INFO_PKTCTRL2_AIF_RPT		0x02	// bit position
#define HDMI_INFO_PKTCTRL2_AIF_EN		0x01	// bit position



#define HDMI_ACR_N1_SW_REG  			0x72
#define HDMI_ACR_N2_SW_REG  			0x73
#define HDMI_ACR_N3_SW_REG  			0x74

#define HDMI_GNRL_CTRL_PKT_REG  		0x7c
#define HDMI_GNRL_CTRL_CLR_ColorDepth_EN		0x40   //bit position  Guochuncheng
#define HDMI_GNRL_CTRL_CLR_AVMUTE		0x02	// bit position
#define HDMI_GNRL_CTRL_SET_AVMUTE		0x01	// bit position

#define TX_PLL_CTRL_REG1				0xE4
#define Chip_PLL_MISC_REG2				0x39



//End for DEV_addr 0x7A/0x7E

/***************************************************************/
//  DEV_ADDR = 0x70 or 0x78 , Displayport mode and HDCP registers
#define DP_TX_HDCP_STATUS							  				0x00
#define DP_TX_HDCP_AUTH_PASS						  			0x02//bit position

#define DP_TX_HDCP_CONTROL_0_REG                  		0x01
#define DP_TX_HDCP_CONTROL_0_STORE_AN            0x80//bit position
#define DP_TX_HDCP_CONTROL_0_RX_REPEATER   	0x40//bit position
#define DP_TX_HDCP_CONTROL_0_RE_AUTH              0x20//bit position
#define DP_TX_HDCP_CONTROL_0_SW_AUTH_OK       0x10//bit position
#define DP_TX_HDCP_CONTROL_0_HARD_AUTH_EN   0x08//bit position
#define DP_TX_HDCP_CONTROL_0_HDCP_ENC_EN      0x04//bit position
#define DP_TX_HDCP_CONTROL_0_BKSV_SRM_PASS  0x02//bit position
#define DP_TX_HDCP_CONTROL_0_KSVLIST_VLD        0x01//bit position


#define DP_TX_HDCP_CONTROL_1_REG                  		0x02
#define DP_TX_HDCP_CONTROL_1_DDC_NO_STOP      			0x20//bit position
#define DP_TX_HDCP_CONTROL_1_DDC_NO_ACK        			0x10//bit position
#define DP_TX_HDCP_CONTROL_1_EDDC_NO_ACK          		0x08//bit position
#define DP_TX_HDCP_CONTROL_1_HDCP_EMB_SCREEN_EN   		0x04//bit position
#define DP_TX_HDCP_CONTROL_1_RCV_11_EN                  0x02//bit position
#define DP_TX_HDCP_CONTROL_1_HDCP_11_EN           		0x01//bit position

#define DP_TX_HDCP_LINK_CHK_FRAME_NUM				 	0x03
#define DP_TX_HDCP_CONTROL_2_REG						0x04

#define DP_TX_HDCP_AKSV0								0x05
#define DP_TX_HDCP_AKSV1								0x06
#define DP_TX_HDCP_AKSV2								0x07
#define DP_TX_HDCP_AKSV3								0x08
#define DP_TX_HDCP_AKSV4								0x09

#define HDCP_DEBUG_CONTROL   						    0x1D


#define DP_TX_SYS_CTRL1_REG           					0x80
#define DP_TX_SYS_CTRL1_PD_IO         					0x80    // bit position
#define DP_TX_SYS_CTRL1_PD_VID        					0x40    // bit position
#define DP_TX_SYS_CTRL1_PD_LINK       					0x20    // bit position
#define DP_TX_SYS_CTRL1_PD_TOTAL      					0x10    // bit position
#define DP_TX_SYS_CTRL1_MODE_SEL      					0x08    // bit position
#define DP_TX_SYS_CTRL1_DET_STA       					0x04    // bit position
#define DP_TX_SYS_CTRL1_FORCE_DET     					0x02    // bit position
#define DP_TX_SYS_CTRL1_DET_CTRL      					0x01    // bit position

#define DP_TX_SYS_CTRL2_REG           					0x81
// #define DP_TX_SYS_CTRL2_ENHANCED 	  					0x08	  //bit position
#define DP_TX_SYS_CTRL2_CHA_STA       					0x04    // bit position
#define DP_TX_SYS_CTRL2_FORCE_CHA     					0x02    // bit position
#define DP_TX_SYS_CTRL2_CHA_CTRL      					0x01    // bit position

#define DP_TX_SYS_CTRL3_REG           					0x82
#define DP_TX_SYS_CTRL3_HPD_STATUS    					0x40    // bit position
#define DP_TX_SYS_CTRL3_F_HPD         					0x20    // bit position
#define DP_TX_SYS_CTRL3_HPD_CTRL      					0x10    // bit position
#define DP_TX_SYS_CTRL3_STRM_VALID    					0x04    // bit position
#define DP_TX_SYS_CTRL3_F_VALID       					0x02    // bit position
#define DP_TX_SYS_CTRL3_VALID_CTRL    					0x01    // bit position

#define DP_TX_SYS_CTRL4_REG			  					0x83
#define DP_TX_SYS_CTRL4_ENHANCED 	  					0x08//bit position

#define DP_TX_VID_CTRL				  					0x84

#define DP_TX_AUD_CTRL									0x86
#define DP_TX_AUD_CTRL_AUD_EN							0x01

#ifndef M_VID_0
#define M_VID_0 0XC0
#define M_VID_1 0XC1
#define M_VID_2 0XC2
#define N_VID_0 0XC3
#define N_VID_1 0XC4
#define N_VID_2 0XC5
#endif

#define DP_TX_PKT_EN_REG              					0x90
#define DP_TX_PKT_AUD_UP								0x80  // bit position
#define DP_TX_PKT_AVI_UD              					0x40  // bit position
#define DP_TX_PKT_MPEG_UD             					0x20  // bit position    
#define DP_TX_PKT_SPD_UD              					0x10  // bit position   
#define DP_TX_PKT_AUD_EN								0x08  // bit position=
#define DP_TX_PKT_AVI_EN              					0x04  // bit position          
#define DP_TX_PKT_MPEG_EN             					0x02  // bit position     
#define DP_TX_PKT_SPD_EN              					0x01  // bit position       


#define DP_TX_HDCP_CTRL 												0x92

#define DP_TX_LINK_BW_SET_REG         				  		0xA0
#define DP_TX_LANE_COUNT_SET_REG      				  	0xA1
#define DP_TX_TRAINING_PTN_SET_REG                   0xA2
#define DP_TX_TRAINING_LANE0_SET_REG                 				0xA3
#define DP_TX_TRAINING_LANE0_SET_MAX_PRE_REACH        0x20        // bit position
#define DP_TX_TRAINING_LANE0_SET_MAX_DRIVE_REACH     0x04        // bit position

#define DP_TX_TRAINING_LANE1_SET_REG                0xA4
#define DP_TX_TRAINING_LANE2_SET_REG                0xA5
#define DP_TX_TRAINING_LANE3_SET_REG                0xA6

#define DP_TX_LINK_TRAINING_CTRL_REG                0xA8
#define DP_TX_LINK_TRAINING_CTRL_EN                 0x01        // bit position


#define DP_TX_DEBUG_1_REG							0xB0
#define DP_TX_DEBUG_1_PLL_LOCK						0x10//bit position

#define DP_TX_LINK_TEST_COUNT                     0xC0
#define DP_TX_LINK_TEST_RATE                     0xBB

#define DP_TX_LINK_DEBUG_REG                        0xB8
#define DP_TX_DIS_AUTO_RST_ENCODER					0x04		//bit 2
#define DP_TX_LINK_DEBUG_INSERT_ER                  0x02        // bit position
#define DP_TX_LINK_DEBUG_PRBS31_EN                  0x01        // bit position

#define DP_TX_SINK_STATUS_REG                       0xBE
#define DP_TX_SINK_STATUS_SINK_STATUS_1          	0x02        // bit position
#define DP_TX_SINK_STATUS_SINK_STATUS_0          	0x01        // bit position


#define DP_TX_PLL_CTRL_REG											0xC7	
#define DP_TX_PLL_CTRL_PLL_PD           						0x80        // bit position
#define DP_TX_PLL_CTRL_PLL_RESET        					0x40        // bit position 
#define DP_TX_PLL_CTRL_CPREG_BLEED      					0x08        // bit position 

#define DP_TX_ANALOG_POWER_DOWN_REG                   			0xC8
#define DP_TX_ANALOG_POWER_DOWN_MACRO_PD              	0x20        // bit position 
#define DP_TX_ANALOG_POWER_DOWN_AUX_PD                		0x10        // bit position 
#define DP_TX_ANALOG_POWER_DOWN_CH3_PD                		0x08        // bit position 
#define DP_TX_ANALOG_POWER_DOWN_CH2_PD                		0x04        // bit position 
#define DP_TX_ANALOG_POWER_DOWN_CH1_PD                		0x02        // bit position 
#define DP_TX_ANALOG_POWER_DOWN_CH0_PD                		0x01        // bit position 


#define DP_TX_ANALOG_TEST_REG                         		0xC9
#define DP_TX_ANALOG_TEST_MACRO_RST                   				0x20       // bit position 
#define DP_TX_ANALOG_TEST_PLL_TEST                    				0x10       // bit position 
#define DP_TX_ANALOG_TEST_CH3_TEST                    				0x08       // bit position 
#define DP_TX_ANALOG_TEST_CH2_TEST                    				0x04       // bit position 
#define DP_TX_ANALOG_TEST_CH1_TEST                    				0x02       // bit position 
#define DP_TX_ANALOG_TEST_CH0_TEST                    				0x01       // bit position 

#define DP_TX_FIFO_THRESHOLD                            0xCC

#define DP_TX_GNS_CTRL_REG                            	0xCD
#define DP_TX_GNS_CTRL_VIDEO_MAP_CTRL                 	 0x02       // bit position 
#define DP_TX_GNS_CTRL_RS_CTRL                        	 0x01       // bit position 

#define DP_TX_SSC_D_VALUE                               0xD0   //guochuncheng
#define DP_TX_DOWN_SPREADING_CTRL2                      0xD1
#define DP_TX_SSC_D_CTRL                                 0x40       //bit position
#define DP_TX_FS_CTRL_TH_CTRL                            0x20       //bit position


#define DP_TX_EXTRA_ADDR_REG							0xCE

#define DP_TX_M_CALCU_CTRL								0xD8
#define M_GEN_CLK_SEL									 0x01//bit 0

#define DP_TX_I2C_STRETCH_CTRL_REG                      0xDA
#define DP_TX_AUX_STATUS            					0xE0
#define DP_TX_DEFER_CTRL_REG            				0xE2
#define DP_TX_DEFER_CTRL_DEFER_CTRL_EN  				 0x80       // bit position 

#define DP_TX_BUF_DATA_COUNT_REG						0xE4
#define DP_TX_AUX_CTRL_REG              				0xE5
#define DP_TX_AUX_ADDR_7_0_REG          				0xE6
#define DP_TX_AUX_ADDR_15_8_REG         				0xE7
#define DP_TX_AUX_ADDR_19_16_REG        				0xE8
#define DP_TX_AUX_CTRL_REG2                             0xE9

#define DP_TX_BUF_DATA_0_REG                          0xf0
#define DP_TX_BUF_DATA_1_REG                          0xf1
#define DP_TX_BUF_DATA_2_REG                          0xf2
#define DP_TX_BUF_DATA_3_REG                          0xf3
#define DP_TX_BUF_DATA_4_REG                          0xf4
#define DP_TX_BUF_DATA_5_REG                          0xf5
#define DP_TX_BUF_DATA_6_REG                          0xf6
#define DP_TX_BUF_DATA_7_REG                          0xf7
#define DP_TX_BUF_DATA_8_REG                          0xf8
#define DP_TX_BUF_DATA_9_REG                          0xf9
#define DP_TX_BUF_DATA_10_REG                         0xfa
#define DP_TX_BUF_DATA_11_REG                         0xfb
#define DP_TX_BUF_DATA_12_REG                         0xfc
#define DP_TX_BUF_DATA_13_REG                         0xfd
#define DP_TX_BUF_DATA_14_REG                         0xfe
#define DP_TX_BUF_DATA_15_REG                         0xff

//End for Address 0x70 or 0x78

/***************************************************************/
//  DEV_ADDR = 0x72 or 0x76, HDMI and Displayport registers
#define DP_TX_VND_IDL_REG             	0x00
#define DP_TX_VND_IDH_REG             	0x01
#define DP_TX_DEV_IDL_REG             	0x02
#define DP_TX_DEV_IDH_REG             	0x03
#define DP_TX_DEV_REV_REG             	0x04

#define DP_POWERD_CTRL_REG			  	0x05
#define DP_POWERD_REGISTER_REG			0x80// bit position
#define DP_POWERD_MISC_REG			  	0x40// bit position
#define DP_POWERD_IO_REG			  	0x20// bit position
#define DP_POWERD_AUDIO_REG				0x10// bit position
#define DP_POWERD_VIDEO_REG			  	0x08// bit position
#define DP_POWERD_LINK_REG			  	0x04// bit position
#define DP_POWERD_TOTAL_REG			  	0x02// bit position
#define DP_MODE_SEL_REG				  	0x01// bit position

#define DP_TX_RST_CTRL_REG            	0x06
#define DP_TX_RST_MISC_REG 			  	0x80	// bit position
#define DP_TX_RST_VIDCAP_REG		  	0x40	// bit position
#define DP_TX_RST_VIDFIF_REG          	0x20    // bit position
#define DP_TX_RST_AUDFIF_REG          	0x10    // bit position
#define DP_TX_RST_AUDCAP_REG         	0x08    // bit position
#define DP_TX_RST_HDCP_REG            	0x04    // bit position
#define DP_TX_RST_SW_RST             	0x02    // bit position
#define DP_TX_RST_HW_RST             	0x01    // bit position

#define DP_TX_RST_CTRL2_REG				0x07
#define DP_TX_RST_SSC					0x80//bit position
#define DP_TX_AC_MODE					0x40//bit position
#define DP_TX_DDC_RST					0x10//bit position
#define DP_TX_TMDS_BIST_RST				0x08//bit position
#define DP_TX_AUX_RST					0x04//bit position
#define DP_TX_SERDES_FIFO_RST			0x02//bit position
#define DP_TX_I2C_REG_RST				0x01//bit position


#define DP_TX_VID_CTRL1_REG           	0x08
#define DP_TX_VID_CTRL1_VID_EN       0x80    // bit position
#define DP_TX_VID_CTRL1_VID_MUTE   0x40    // bit position
#define DP_TX_VID_CTRL1_DE_GEN      0x20    // bit position
#define DP_TX_VID_CTRL1_DEMUX        0x10    // bit position
#define DP_TX_VID_CTRL1_IN_BIT		  	0x04    // bit position
#define DP_TX_VID_CTRL1_DDRCTRL		0x02    // bit position
#define DP_TX_VID_CTRL1_EDGE		  		0x01    // bit position

#define DP_TX_VID_CTRL2_REG           	0x09
#define DP_TX_VID_CTRL1_YCBIT_SEL  		0x04    // bit position

#define DP_TX_VID_CTRL3_REG           	0x0A

#define DP_TX_VID_CTRL4_REG           	0x0B
#define DP_TX_VID_CTRL4_E_SYNC_EN	  	0x80	  //bit position
#define DP_TX_VID_CTRL4_EX_E_SYNC    0x40    // bit position
#define DP_TX_VID_CTRL4_BIST          		0x08    // bit position
#define DP_TX_VID_CTRL4_BIST_WIDTH   0x04        // bit position

#define DP_TX_VID_CTRL5_REG           		0x0C

#define DP_TX_VID_CTRL6_REG           		0x0D
#define DP_TX_VID_UPSAMPLE					0x02//bit position

#define DP_TX_VID_CTRL7_REG           		0x0E
#define DP_TX_VID_CTRL8_REG           		0x0F
#define DP_TX_VID_CTRL9_REG           		0x10

#define DP_TX_VID_CTRL10_REG           	0x11
#define DP_TX_VID_CTRL10_INV_F         	0x08    // bit position
#define DP_TX_VID_CTRL10_I_SCAN        	0x04    // bit position
#define DP_TX_VID_CTRL10_VSYNC_POL   0x02    // bit position
#define DP_TX_VID_CTRL10_HSYNC_POL   0x01    // bit position

#define DP_TX_TOTAL_LINEL_REG         0x12
#define DP_TX_TOTAL_LINEH_REG         0x13
#define DP_TX_ACT_LINEL_REG           0x14
#define DP_TX_ACT_LINEH_REG           0x15
#define DP_TX_VF_PORCH_REG            0x16
#define DP_TX_VSYNC_CFG_REG           0x17
#define DP_TX_VB_PORCH_REG            0x18
#define DP_TX_TOTAL_PIXELL_REG        0x19
#define DP_TX_TOTAL_PIXELH_REG        0x1A
#define DP_TX_ACT_PIXELL_REG          0x1B
#define DP_TX_ACT_PIXELH_REG          0x1C
#define DP_TX_HF_PORCHL_REG           0x1D
#define DP_TX_HF_PORCHH_REG           0x1E
#define DP_TX_HSYNC_CFGL_REG          0x1F
#define DP_TX_HSYNC_CFGH_REG          0x20
#define DP_TX_HB_PORCHL_REG           0x21
#define DP_TX_HB_PORCHH_REG           0x22

#define DP_TX_VID_STATUS						0x23

#define DP_TX_TOTAL_LINE_STA_L        0x24
#define DP_TX_TOTAL_LINE_STA_H        0x25
#define DP_TX_ACT_LINE_STA_L          0x26
#define DP_TX_ACT_LINE_STA_H          0x27
#define DP_TX_V_F_PORCH_STA           0x28
#define DP_TX_V_SYNC_STA              0x29
#define DP_TX_V_B_PORCH_STA           0x2A
#define DP_TX_TOTAL_PIXEL_STA_L       0x2B
#define DP_TX_TOTAL_PIXEL_STA_H       0x2C
#define DP_TX_ACT_PIXEL_STA_L         0x2D
#define DP_TX_ACT_PIXEL_STA_H         0x2E
#define DP_TX_H_F_PORCH_STA_L         0x2F
#define DP_TX_H_F_PORCH_STA_H         0x30
#define DP_TX_H_SYNC_STA_L            0x31
#define DP_TX_H_SYNC_STA_H            0x32
#define DP_TX_H_B_PORCH_STA_L         0x33
#define DP_TX_H_B_PORCH_STA_H         0x34

#define SPDIF_AUDIO_CTRL0			0x36
#define SPDIF_AUDIO_CTRL0_SPDIF_IN  0x80 // bit position
#define SPDIF_AUDIO_SEL                       0x08

#define SPDIF_AUDIO_STATUS0			0x38
#define SPDIF_AUDIO_STATUS0_CLK_DET 0x80
#define SPDIF_AUDIO_STATUS0_AUD_DET 0x01
#define SPDIF_AUDIO_STATUS1			0x39

#define DP_TX_VIDEO_BIT_CTRL_0_REG                    0x40
#define DP_TX_VIDEO_BIT_CTRL_1_REG                    0x41
#define DP_TX_VIDEO_BIT_CTRL_2_REG                    0x42
#define DP_TX_VIDEO_BIT_CTRL_3_REG                    0x43
#define DP_TX_VIDEO_BIT_CTRL_4_REG                    0x44
#define DP_TX_VIDEO_BIT_CTRL_5_REG                    0x45
#define DP_TX_VIDEO_BIT_CTRL_6_REG                    0x46
#define DP_TX_VIDEO_BIT_CTRL_7_REG                    0x47
#define DP_TX_VIDEO_BIT_CTRL_8_REG                    0x48
#define DP_TX_VIDEO_BIT_CTRL_9_REG                    0x49
#define DP_TX_VIDEO_BIT_CTRL_10_REG                   0x4a
#define DP_TX_VIDEO_BIT_CTRL_11_REG                   0x4b
#define DP_TX_VIDEO_BIT_CTRL_12_REG                   0x4c
#define DP_TX_VIDEO_BIT_CTRL_13_REG                   0x4d
#define DP_TX_VIDEO_BIT_CTRL_14_REG                   0x4e
#define DP_TX_VIDEO_BIT_CTRL_15_REG                   0x4f
#define DP_TX_VIDEO_BIT_CTRL_16_REG                   0x50
#define DP_TX_VIDEO_BIT_CTRL_17_REG                   0x51
#define DP_TX_VIDEO_BIT_CTRL_18_REG                   0x52
#define DP_TX_VIDEO_BIT_CTRL_19_REG                   0x53
#define DP_TX_VIDEO_BIT_CTRL_20_REG                   0x54
#define DP_TX_VIDEO_BIT_CTRL_21_REG                   0x55
#define DP_TX_VIDEO_BIT_CTRL_22_REG                   0x56
#define DP_TX_VIDEO_BIT_CTRL_23_REG                   0x57
#define DP_TX_VIDEO_BIT_CTRL_24_REG                   0x58
#define DP_TX_VIDEO_BIT_CTRL_25_REG                   0x59
#define DP_TX_VIDEO_BIT_CTRL_26_REG                   0x5a
#define DP_TX_VIDEO_BIT_CTRL_27_REG                   0x5b
#define DP_TX_VIDEO_BIT_CTRL_28_REG                   0x5c
#define DP_TX_VIDEO_BIT_CTRL_29_REG                   0x5d
#define DP_TX_VIDEO_BIT_CTRL_30_REG                   0x5e
#define DP_TX_VIDEO_BIT_CTRL_31_REG                   0x5f
#define DP_TX_VIDEO_BIT_CTRL_32_REG                   0x60
#define DP_TX_VIDEO_BIT_CTRL_33_REG                   0x61
#define DP_TX_VIDEO_BIT_CTRL_34_REG                   0x62
#define DP_TX_VIDEO_BIT_CTRL_35_REG                   0x63
#define DP_TX_VIDEO_BIT_CTRL_36_REG                   0x64
#define DP_TX_VIDEO_BIT_CTRL_37_REG                   0x65
#define DP_TX_VIDEO_BIT_CTRL_38_REG                   0x66
#define DP_TX_VIDEO_BIT_CTRL_39_REG                   0x67
#define DP_TX_VIDEO_BIT_CTRL_40_REG                   0x68
#define DP_TX_VIDEO_BIT_CTRL_41_REG                   0x69
#define DP_TX_VIDEO_BIT_CTRL_42_REG                   0x6a
#define DP_TX_VIDEO_BIT_CTRL_43_REG                   0x6b
#define DP_TX_VIDEO_BIT_CTRL_44_REG                   0x6c
#define DP_TX_VIDEO_BIT_CTRL_45_REG                   0x6d
#define DP_TX_VIDEO_BIT_CTRL_46_REG                   0x6e
#define DP_TX_VIDEO_BIT_CTRL_47_REG                   0x6f

//AVI info frame
#define DP_TX_AVI_TYPE                0x70
#define DP_TX_AVI_VER                 0x71
#define DP_TX_AVI_LEN                 0x72
#define DP_TX_AVI_DB0				  0x73
#define DP_TX_AVI_DB1                 0x74
#define DP_TX_AVI_DB2               0x75
#define DP_TX_AVI_DB3               0x76
#define DP_TX_AVI_DB4               0x77
#define DP_TX_AVI_DB5               0x78
#define DP_TX_AVI_DB6               0x79
#define DP_TX_AVI_DB7               0x7A
#define DP_TX_AVI_DB8               0x7B
#define DP_TX_AVI_DB9               0x7C
#define DP_TX_AVI_DB10              0x7D
#define DP_TX_AVI_DB11              0x7E
#define DP_TX_AVI_DB12              0x7F
#define DP_TX_AVI_DB13              0x80
#define DP_TX_AVI_DB14              0x81
#define DP_TX_AVI_DB15              0x82

//Audio info frame
#define DP_TX_AUD_TYPE			 0x83
#define DP_TX_AUD_VER			 0x84
#define DP_TX_AUD_LEN			 0x85
#define DP_TX_AUD_DB0			 0x86
#define DP_TX_AUD_DB1			 0x87
#define DP_TX_AUD_DB2			 0x88
#define DP_TX_AUD_DB3			 0x89
#define DP_TX_AUD_DB4			 0x8A
#define DP_TX_AUD_DB5			 0x8B
#define DP_TX_AUD_DB6			 0x8C
#define DP_TX_AUD_DB7			 0x8D
#define DP_TX_AUD_DB8			 0x8E
#define DP_TX_AUD_DB9			 0x8F
#define DP_TX_AUD_DB10			 0x90

//SPD info frame
#define DP_TX_SPD_TYPE                0x91
#define DP_TX_SPD_VER                 0x92
#define DP_TX_SPD_LEN                 0x93
#define DP_TX_SPD_DATA0				  0x94
#define DP_TX_SPD_DATA1               0x95
#define DP_TX_SPD_DATA2               0x96
#define DP_TX_SPD_DATA3               0x97
#define DP_TX_SPD_DATA4               0x98
#define DP_TX_SPD_DATA5               0x99
#define DP_TX_SPD_DATA6               0x9A
#define DP_TX_SPD_DATA7               0x9B
#define DP_TX_SPD_DATA8               0x9C
#define DP_TX_SPD_DATA9               0x9D
#define DP_TX_SPD_DATA10              0x9E
#define DP_TX_SPD_DATA11              0x9F
#define DP_TX_SPD_DATA12              0xA0
#define DP_TX_SPD_DATA13              0xA1
#define DP_TX_SPD_DATA14              0xA2
#define DP_TX_SPD_DATA15              0xA3
#define DP_TX_SPD_DATA16              0xA4
#define DP_TX_SPD_DATA17              0xA5
#define DP_TX_SPD_DATA18              0xA6
#define DP_TX_SPD_DATA19              0xA7
#define DP_TX_SPD_DATA20              0xA8
#define DP_TX_SPD_DATA21              0xA9
#define DP_TX_SPD_DATA22              0xAA
#define DP_TX_SPD_DATA23              0xAB
#define DP_TX_SPD_DATA24              0xAC
#define DP_TX_SPD_DATA25              0xAD
#define DP_TX_SPD_DATA26              0xAE
#define DP_TX_SPD_DATA27              0xAF

//Mpeg source info frame
#define DP_TX_MPEG_TYPE               0xB0
#define DP_TX_MPEG_VER                0xB1
#define DP_TX_MPEG_LEN                0xB2
#define DP_TX_MPEG_DATA0              0xB3
#define DP_TX_MPEG_DATA1              0xB4
#define DP_TX_MPEG_DATA2              0xB5
#define DP_TX_MPEG_DATA3              0xB6
#define DP_TX_MPEG_DATA4              0xB7
#define DP_TX_MPEG_DATA5              0xB8
#define DP_TX_MPEG_DATA6              0xB9
#define DP_TX_MPEG_DATA7              0xBA
#define DP_TX_MPEG_DATA8              0xBB
#define DP_TX_MPEG_DATA9              0xBC
#define DP_TX_MPEG_DATA10             0xBD
#define DP_TX_MPEG_DATA11            0xBE
#define DP_TX_MPEG_DATA12            0xBF
#define DP_TX_MPEG_DATA13            0xC0
#define DP_TX_MPEG_DATA14            0xC1
#define DP_TX_MPEG_DATA15            0xC2
#define DP_TX_MPEG_DATA16            0xC3
#define DP_TX_MPEG_DATA17            0xC4
#define DP_TX_MPEG_DATA18            0xC5
#define DP_TX_MPEG_DATA19            0xC6
#define DP_TX_MPEG_DATA20            0xC7
#define DP_TX_MPEG_DATA21            0xC8
#define DP_TX_MPEG_DATA22            0xC9
#define DP_TX_MPEG_DATA23            0xCA
#define DP_TX_MPEG_DATA24            0xCB
#define DP_TX_MPEG_DATA25            0xCC
#define DP_TX_MPEG_DATA26            0xCD
#define DP_TX_MPEG_DATA27            0xCE


#define SSC_CTRL_REG1					 0xA7
#define GNSS_CTRL_REG				0xCD
#define ENABLE_SSC_FILTER			0x80//bit 

#define SSC_D_VALUE					 0xD0
#define SSC_CTRL_REG2					 0xD1

#define ANALOG_DEBUG_REG1			 0xDC
#define ANALOG_DEBUG_REG3			 0xDE
#define DP_TX_PLL_FILTER_CTRL1		 0xDF

#define DP_TX_PLL_FILTER_CTRL3				0xE1
#define DP_TX_PLL_FILTER_CTRL       0xE2
#define DP_TX_PLL_CTRL1				0xE4
#define DP_TX_PLL_CTRL3				0xE6
#define HDMI_TMDS_CHNL_ALIGN        0x01
//interrupt
#define DP_COMMON_INT_STATUS1     0xF1
#define DP_COMMON_INT1_PLL_LOCK_CHG 	0x40//bit position
#define DP_COMMON_INT1_SPDIF_UNSTBL 	0x10//bit position
#define DP_COMMON_INT1_VIDEO_FORMAT_CHG 0x08//bit position
#define DP_COMMON_INT1_AUDIO_CLK_CHG	0x04//bit position
#define DP_COMMON_INT1_VIDEO_CLOCK_CHG  0x02//bit position


#define DP_COMMON_INT_STATUS2	  0xF2
#define DP_COMMON_INT2_AUTHCHG	  0x02 //bit position
#define DP_COMMON_INT2_AUTHDONE	  0x01 //bit position

#define DP_COMMON_INT_STATUS3	  0xF3
#define DP_COMMON_INT3_AFIFO_UNDER	0x80//bit position
#define DP_COMMON_INT3_AFIFO_OVER	0x40//bit position

#define DP_COMMON_INT_STATUS4	    0xF4
#define DP_COMMON_INT4_PLUG      0x01   // bit position
#define DP_COMMON_INT4_HPDLOST		0x02   //bit position
#define DP_COMMON_INT4_HPD_CHANGE   0x04   //bit position



#define HDMI_INT_STATUS1		  		0xF5
#define HDMI_INTR_STATUS1_SPDIF_ERR		0x80//bit position
#define HDMI_INT_RX_SEN_CHG		  		0x04//bit position

#define HDMI_INT_STATUS2		  0xF6

#define DP_TX_INT_STATUS1		  0xF7
#define DP_TX_INT_STATUS1_HPD	  0x40 //bit position
#define DP_TX_INT_STATUS1_TRAINING_Finish       0x20   // bit position
#define DP_TX_INT_STATUS1_POLLING_ERR        0x10   // bit position

#define DP_TX_INT_SINK_CHG		  0x08//bit position

//interrupt mask
#define DP_COMMON_INT_MASK1			  0xF8
#define DP_COMMON_INT_MASK2			  0xF9
#define DP_COMMON_INT_MASK3			  0xFA
#define DP_COMMON_INT_MASK4			  0xFB
#define HDMI_INT_MASK1				  				0xFC
#define HDMI_INT_MASK2				  				0xFD
#define DP_INT_MASK					  					0xFE
#define DP_TX_INT_CTRL_REG            		0xFF
//End for dev_addr 0x72 or 0x76

/***************************************************************/
/***************************************************************/

//DPCD regs
#define DPCD_DPCD_REV                                   0x00
#define DPCD_MAX_LINK_RATE                              0x01
#define DPCD_MAX_LANE_COUNT                             0x02
#define DPCD_MAX_DOWNSPREAD                             0x03
#define DPCD_NORP                                       0x04
#define DPCD_DOWNSTREAMPORT_PRESENT                     0x05

#define DPCD_RECEIVE_PORT0_CAP_0                        0x08
#define DPCD_RECEIVE_PORT0_CAP_1                        0x09
#define DPCD_RECEIVE_PORT0_CAP_2                        0x0a
#define DPCD_RECEIVE_PORT0_CAP_3                        0x0b

#define DPCD_LINK_BW_SET                                0x00
#define DPCD_LANE_COUNT_SET                             0x01
#define DPCD_TRAINING_PATTERN_SET                       0x02
#define DPCD_TRAINNIG_LANE0_SET                         0x03
#define DPCD_TRAINNIG_LANE1_SET                         0x04
#define DPCD_TRAINNIG_LANE2_SET                         0x05
#define DPCD_TRAINNIG_LANE3_SET                         0x06
#define DPCD_DOWNSPREAD_CTRL                            0x07

#define DPCD_SINK_COUNT                                 0x00
#define DPCD_DEVICE_SERVICE_IRQ_VECTOR                  0x01
#define DPCD_LANE0_1_STATUS                             0x02
#define DPCD_LANE2_3_STATUS                             0x03
#define DPCD_LANE_ALIGN_STATUS_UPDATED                  0x04
#define DPCD_SINK_STATUS                                0x05
#define DPCD_ADJUST_REQUEST_LANE0_1                     0x06
#define DPCD_ADJUST_REQUEST_LANE2_3                     0x07
#define DPCD_TRAINING_SCORE_LANE0                       0x08
#define DPCD_TRAINING_SCORE_LANE1                       0x09
#define DPCD_TRAINING_SCORE_LANE2                       0x0a
#define DPCD_TRAINING_SCORE_LANE3                       0x0b

#define DPCD_TEST_REQUEST                               0x18
#define DPCD_TEST_LINK_RATE                             0x19

#define DPCD_TEST_LANE_COUNT                            0x20

#define DPCD_TEST_Response                              0x60
#define TEST_ACK                                                  0x01
#define DPCD_TEST_EDID_Checksum_Write                   0x04//bit position

#define DPCD_TEST_EDID_Checksum                         0x61

//---------------------------------------------------------

//void anx9804_init(__panel_para_t * info);

#endif
