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

#ifndef __AXP192_POWER_H__
#define __AXP192_POWER_H__

/* Unified sub device IDs for AXP */
/* LDO0 For RTCLDO ,LDO1-3 for ALDO,LDO*/
enum {
	AXP19_ID_DCDC1 = 0,
	AXP19_ID_DCDC2,
	AXP19_ID_DCDC3,
	AXP19_ID_LDO1,   //RTC
	AXP19_ID_LDO2,   //ALDO1
	AXP19_ID_LDO3,   //ALDO2
	AXP19_ID_LDO4,   //ALDO3
	AXP19_ID_MAX,
};

#define AXP192_ADDR                 (0x34)

#define POWER19_STATUS              (0x00)
#define POWER19_LDO3_DC2_CTL        (0x10)
#define POWER19_LDO24_DC13_CTL      (0x12)
#define POWER19_DC2OUT_VOL          (0x23)
#define POWER19_LDO3_DC2_DVM        (0x25)
#define POWER19_DC1OUT_VOL          (0x26)
#define POWER19_DC3OUT_VOL          (0x27)
#define POWER19_LDO24OUT_VOL        (0x28)
#define POWER19_LDO3OUT_VOL         (0x29)
#define POWER19_DCDC_MODESET        (0x80)
#define POWER19_DCDC_FREQSET        (0x37)

/* AXP19 Regulator Registers */
#define AXP19_LDO1              POWER19_STATUS
#define AXP19_LDO2              POWER19_LDO24OUT_VOL
#define AXP19_LDO3              POWER19_LDO3OUT_VOL
#define AXP19_LDO4              POWER19_LDO24OUT_VOL
#define AXP19_DCDC1             POWER19_DC1OUT_VOL
#define AXP19_DCDC2             POWER19_DC2OUT_VOL
#define AXP19_DCDC3             POWER19_DC3OUT_VOL

#define AXP19_LDO1EN            POWER19_STATUS
#define AXP19_LDO2EN            POWER19_LDO24_DC13_CTL
#define AXP19_LDO3EN            POWER19_LDO3_DC2_CTL
#define AXP19_LDO4EN            POWER19_LDO24_DC13_CTL
#define AXP19_DCDC1EN           POWER19_LDO24_DC13_CTL
#define AXP19_DCDC2EN           POWER19_LDO3_DC2_CTL
#define AXP19_DCDC3EN           POWER19_LDO24_DC13_CTL

#define AXP19_DCDC_MODESET      POWER19_DCDC_MODESET
#define AXP19_DCDC_FREQSET      POWER19_DCDC_FREQSET

#endif  /* __AXP192_POWER_H__ */


