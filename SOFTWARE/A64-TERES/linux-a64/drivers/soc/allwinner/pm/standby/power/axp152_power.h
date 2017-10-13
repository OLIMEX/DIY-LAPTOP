/*
 * axp192 for standby driver
 *
 * Copyright (C) 2015 allwinnertech Ltd.
 * Author: Ming Li <liming@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __AXP152_POWER_H__
#define __AXP152_POWER_H__

/* Unified sub device IDs for AXP */
/* LDO0 For RTCLDO ,LDO1-3 for ALDO,LDO*/
enum
{
	AXP152_ID_DCDC1 = 0,
	AXP152_ID_DCDC2,
	AXP152_ID_DCDC3,
	AXP152_ID_DCDC4,
	AXP152_ID_ALDO1,
	AXP152_ID_ALDO2,
	AXP152_ID_DLDO1,
	AXP152_ID_DLDO2,
	AXP152_ID_LDOIO0,
	AXP152_ID_LDO0,
	AXP152_ID_RTC,
	AXP152_ID_MAX,
};

#define AXP152_ADDR                 (0x34)

/*For ajust axp15-reg only*/
#define POWER15_STATUS              (0x00)
#define POWER15_LDO0OUT_VOL         (0x15)
#define POWER15_LDO34OUT_VOL        (0x28)
#define POWER15_LDO5OUT_VOL         (0x29)
#define POWER15_LDO6OUT_VOL         (0x2A)
#define POWER15_GPIO0_VOL           (0x96)
#define POWER15_DC1OUT_VOL          (0x26)
#define POWER15_DC2OUT_VOL          (0x23)
#define POWER15_DC3OUT_VOL          (0x27)
#define POWER15_DC4OUT_VOL          (0x2B)
#define POWER15_LDO0_CTL            (0x15)
#define POWER15_LDO3456_DC1234_CTL  (0x12)
#define POWER15_GPIO0_CTL           (0x90)
#define POWER15_GPIO1_CTL           (0x91)
#define POWER15_GPIO2_CTL           (0x92)
#define POWER15_GPIO3_CTL           (0x93)
#define POWER15_GPIO0123_SIGNAL     (0x97)
#define POWER15_DCDC_MODESET        (0x80)
#define POWER15_DCDC_FREQSET        (0x37)

/* AXP15 Regulator Registers */
#define AXP152_LDO0                 POWER15_LDO0OUT_VOL
#define AXP152_RTC		    POWER15_STATUS
#define AXP152_ANALOG1		    POWER15_LDO34OUT_VOL
#define AXP152_ANALOG2              POWER15_LDO34OUT_VOL
#define AXP152_DIGITAL1             POWER15_LDO5OUT_VOL
#define AXP152_DIGITAL2             POWER15_LDO6OUT_VOL
#define AXP152_LDOIO0               POWER15_GPIO0_VOL

#define AXP152_DCDC1                POWER15_DC1OUT_VOL
#define AXP152_DCDC2                POWER15_DC2OUT_VOL
#define AXP152_DCDC3                POWER15_DC3OUT_VOL
#define AXP152_DCDC4                POWER15_DC4OUT_VOL

#define AXP152_LDO0EN		    POWER15_LDO0_CTL                    //REG[15H]
#define AXP152_RTCLDOEN		    POWER15_STATUS                      //REG[00H]
#define AXP152_ANALOG1EN            POWER15_LDO3456_DC1234_CTL          //REG[12H]
#define AXP152_ANALOG2EN            POWER15_LDO3456_DC1234_CTL
#define AXP152_DIGITAL1EN           POWER15_LDO3456_DC1234_CTL
#define AXP152_DIGITAL2EN           POWER15_LDO3456_DC1234_CTL
#define AXP152_LDOI0EN              POWER15_GPIO2_CTL                   //REG[92H]

#define AXP152_DCDC1EN              POWER15_LDO3456_DC1234_CTL
#define AXP152_DCDC2EN              POWER15_LDO3456_DC1234_CTL
#define AXP152_DCDC3EN              POWER15_LDO3456_DC1234_CTL
#define AXP152_DCDC4EN              POWER15_LDO3456_DC1234_CTL

#define AXP152_BUCKMODE             POWER15_DCDC_MODESET                 //REG[80H]
#define AXP152_BUCKFREQ             POWER15_DCDC_FREQSET                 //REG[37H]


struct axp_regulator_info {
	int	enable_reg;
	int	enable_bit;
};
#endif  /* __AXP152_POWER_H__ */

