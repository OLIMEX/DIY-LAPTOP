#ifndef _LINUX_AXP81X_REGU_H_
#define _LINUX_AXP81X_REGU_H_
/*Schematic_name	regulator_ID	virtual_consumer_name	max_voltage 	min_voltage   step
------------------------------------------------------------------------------------------------
*     DCDC1		axp81x_dcdc1	 reg-23-cs-dcdc1	1600mV		3400mV		100mV
*     DCDC2		axp81x_dcdc2	 reg-23-cs-dcdc2	500/1220mV	1200mV/1300mV	10/20mV
*     DCDC3		axp81x_dcdc3	 reg-23-cs-dcdc3	500/1220mV	1200mV/1300mV	10/20mV
*     DCDC4		axp81x_dcdc4	 reg-23-cs-dcdc4	500/1220mV	1200mV/1300mV	10/20mV
*     DCDC5		axp81x_dcdc5	 reg-23-cs-dcdc5	800/1140mV	1120mV/1840mV	10/20mV
*     DCDC6		axp81x_dcdc6	 reg-23-cs-dcdc6	600/1120mV	1100mV/1520mV	10/20mV
*     DCDC7		axp81x_dcdc7	 reg-23-cs-dcdc7	600/1120mV	1100mV/1520mV	10/20mV
*     RTC		axp81x_aldo1	 reg-23-cs-rtc		3000mV           3000mV		0mV
*     ALDO1		axp81x_aldo1	 reg-23-cs-aldo1	3300mV           700mV		100mV
*     ALDO2		axp81x_aldo2	 reg-23-cs-aldo2	3300mV           700mV		100mV
*     ALDO3		axp81x_aldo3	 reg-23-cs-aldo3	3300mV           700mV		100mV
*     DLDO1		axp81x_dldo1	 reg-23-cs-dldo1	3300mV           700mV		100mV
*     DLDO2		axp81x_dldo2	 reg-23-cs-dldo2	3400mV/4200mV    700mV/3600mV	100mV/200mV
*     DLDO3		axp81x_dldo3	 reg-23-cs-dldo3	3300mV           700mV		100mV
*     DLDO4		axp81x_dldo4	 reg-23-cs-dldo4	3300mV           700mV		100mV
*     ELDO1		axp81x_eldo1	 reg-23-cs-eldo1	1900mV           700mV		50mV
*     ELDO2		axp81x_eldo2	 reg-23-cs-eldo2	1900mV           700mV		50mV
*     ELDO3		axp81x_eldo3	 reg-23-cs-eldo3	1900mV           700mV		50mV
*     FLDO1		axp81x_fldo1	 reg-23-cs-fldo1	1450mV           700mV		50mV
*     FLDO2		axp81x_fldo2	 reg-23-cs-fldo2	1450mV           700mV		50mV
*     FLDO3		axp81x_fldo3	 reg-23-cs-fldo3
*/

/* Unified sub device IDs for AXP */
/* LDO0 For RTCLDO ,LDO1-3 for ALDO,LDO*/
enum {
	AXP81X_ID_LDO1,   //RTC
	AXP81X_ID_LDO2,   //ALDO1
	AXP81X_ID_LDO3,   //ALDO2
	AXP81X_ID_LDO4,   //ALDO3
	AXP81X_ID_LDO5,   //DLDO1
	AXP81X_ID_LDO6,   //DLDO2
	AXP81X_ID_LDO7,   //DLDO3
	AXP81X_ID_LDO8,   //DLDO4
	AXP81X_ID_LDO9,   //ELDO1
	AXP81X_ID_LDO10,  //ELDO2
	AXP81X_ID_LDO11,  //ELDO3
	AXP81X_ID_LDO12,  //FLDO1
	AXP81X_ID_LDO13,  //FLDO2
	AXP81X_ID_LDO14,  //FLDO3
	AXP81X_ID_SW0,   //DC1SW
	AXP81X_ID_LDOIO0 = AXP_LDOIO_ID_START,
	AXP81X_ID_LDOIO1,
	AXP81X_ID_DCDC1 = AXP_DCDC_ID_START,
	AXP81X_ID_DCDC2,
	AXP81X_ID_DCDC3,
	AXP81X_ID_DCDC4,
	AXP81X_ID_DCDC5,
	AXP81X_ID_DCDC6,
	AXP81X_ID_DCDC7,
	AXP81X_ID_SUPPLY,
};

/* AXP81X Regulator Registers */
#define AXP81X_LDO1		AXP81X_STATUS
#define AXP81X_LDO2		AXP81X_ALDO1OUT_VOL
#define AXP81X_LDO3	        AXP81X_ALDO2OUT_VOL
#define AXP81X_LDO4	        AXP81X_ALDO3OUT_VOL
#define AXP81X_LDO5	        AXP81X_DLDO1OUT_VOL
#define AXP81X_LDO6	        AXP81X_DLDO2OUT_VOL
#define AXP81X_LDO7	        AXP81X_DLDO3OUT_VOL
#define AXP81X_LDO8	        AXP81X_DLDO4OUT_VOL
#define AXP81X_LDO9	        AXP81X_ELDO1OUT_VOL
#define AXP81X_LDO10		AXP81X_ELDO2OUT_VOL
#define AXP81X_LDO11		AXP81X_ELDO3OUT_VOL
#define AXP81X_LDO12	        AXP81X_FLDO1OUT_VOL
#define AXP81X_LDO13		AXP81X_FLDO2OUT_VOL
#define AXP81X_LDO14		AXP81X_FLDO3OUT_VOL
#define AXP81X_DCDC1	        AXP81X_DC1OUT_VOL
#define AXP81X_DCDC2	        AXP81X_DC2OUT_VOL
#define AXP81X_DCDC3	        AXP81X_DC3OUT_VOL
#define AXP81X_DCDC4	        AXP81X_DC4OUT_VOL
#define AXP81X_DCDC5	        AXP81X_DC5OUT_VOL
#define AXP81X_DCDC6	        AXP81X_DC6OUT_VOL
#define AXP81X_DCDC7	        AXP81X_DC7OUT_VOL
#define AXP81X_LDOIO0		AXP81X_GPIO0LDOOUT_VOL
#define AXP81X_LDOIO1		AXP81X_GPIO1LDOOUT_VOL
#define AXP81X_DC1SW		AXP81X_STATUS

#define AXP81X_LDO1EN		AXP81X_STATUS
#define AXP81X_LDO2EN		AXP81X_LDO_DC_EN3
#define AXP81X_LDO3EN		AXP81X_LDO_DC_EN3
#define AXP81X_LDO4EN		AXP81X_LDO_DC_EN3
#define AXP81X_LDO5EN		AXP81X_LDO_DC_EN2
#define AXP81X_LDO6EN		AXP81X_LDO_DC_EN2
#define AXP81X_LDO7EN		AXP81X_LDO_DC_EN2
#define AXP81X_LDO8EN		AXP81X_LDO_DC_EN2
#define AXP81X_LDO9EN		AXP81X_LDO_DC_EN2
#define AXP81X_LDO10EN		AXP81X_LDO_DC_EN2
#define AXP81X_LDO11EN		AXP81X_LDO_DC_EN2
#define AXP81X_LDO12EN		AXP81X_LDO_DC_EN3
#define AXP81X_LDO13EN		AXP81X_LDO_DC_EN3
#define AXP81X_LDO14EN		AXP81X_LDO_DC_EN3
#define AXP81X_DCDC1EN		AXP81X_LDO_DC_EN1
#define AXP81X_DCDC2EN		AXP81X_LDO_DC_EN1
#define AXP81X_DCDC3EN		AXP81X_LDO_DC_EN1
#define AXP81X_DCDC4EN		AXP81X_LDO_DC_EN1
#define AXP81X_DCDC5EN		AXP81X_LDO_DC_EN1
#define AXP81X_DCDC6EN		AXP81X_LDO_DC_EN1
#define AXP81X_DCDC7EN		AXP81X_LDO_DC_EN1
#define AXP81X_LDOIO0EN		AXP81X_GPIO0_CTL
#define AXP81X_LDOIO1EN		AXP81X_GPIO1_CTL
#define AXP81X_DC1SWEN		AXP81X_LDO_DC_EN2

extern struct axp_funcdev_info *axp81x_regu_init(void);
extern void axp81x_regu_exit(void);

#endif
