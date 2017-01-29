/*
 * Battery charger driver for AW-POWERS
 *
 * Copyright (C) 2014 ALLWINNERTECH.
 *  Ming Li <liming@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "axp-cfg.h"
#ifdef CONFIG_AW_AXP81X
#include "axp81x-sply.h"
#include "axp81x-common.h"
static const struct axp_config_info *axp_config = &axp81x_config;
#endif

struct axp_adc_res adc;

s32 axp_read_bat_cap(void)
{
	s32 bat_cap;
	u8 tmp;

	axp_read(axp_charger->master, AXP_CAP,&tmp);
	bat_cap	= (s32)	(tmp & 0x7F);
	return bat_cap;
}
EXPORT_SYMBOL_GPL(axp_read_bat_cap);

static inline s32 axp_vts_to_temp(s32 data, const struct axp_config_info *axp_config)
{
	 s32 temp;
	 if (data < 80)
		 return 30;
	 else if (data < axp_config->pmu_bat_temp_para16)
		 return 80;
	 else if (data <= axp_config->pmu_bat_temp_para15) {
		 temp = 70 + (axp_config->pmu_bat_temp_para15-data)*10/(axp_config->pmu_bat_temp_para15-axp_config->pmu_bat_temp_para16);
	 } else if (data <= axp_config->pmu_bat_temp_para14) {
		 temp = 60 + (axp_config->pmu_bat_temp_para14-data)*10/(axp_config->pmu_bat_temp_para14-axp_config->pmu_bat_temp_para15);
	 } else if (data <= axp_config->pmu_bat_temp_para13) {
		 temp = 55 + (axp_config->pmu_bat_temp_para13-data)*5/(axp_config->pmu_bat_temp_para13-axp_config->pmu_bat_temp_para14);
	 } else if (data <= axp_config->pmu_bat_temp_para12) {
		 temp = 50 + (axp_config->pmu_bat_temp_para12-data)*5/(axp_config->pmu_bat_temp_para12-axp_config->pmu_bat_temp_para13);
	 } else if (data <= axp_config->pmu_bat_temp_para11) {
		 temp = 45 + (axp_config->pmu_bat_temp_para11-data)*5/(axp_config->pmu_bat_temp_para11-axp_config->pmu_bat_temp_para12);
	 } else if (data <= axp_config->pmu_bat_temp_para10) {
		 temp = 40 + (axp_config->pmu_bat_temp_para10-data)*5/(axp_config->pmu_bat_temp_para10-axp_config->pmu_bat_temp_para11);
	 } else if (data <= axp_config->pmu_bat_temp_para9) {
		 temp = 30 + (axp_config->pmu_bat_temp_para9-data)*10/(axp_config->pmu_bat_temp_para9-axp_config->pmu_bat_temp_para10);
	 } else if (data <= axp_config->pmu_bat_temp_para8) {
		 temp = 20 + (axp_config->pmu_bat_temp_para8-data)*10/(axp_config->pmu_bat_temp_para8-axp_config->pmu_bat_temp_para9);
	 } else if (data <= axp_config->pmu_bat_temp_para7) {
		 temp = 10 + (axp_config->pmu_bat_temp_para7-data)*10/(axp_config->pmu_bat_temp_para7-axp_config->pmu_bat_temp_para8);
	 } else if (data <= axp_config->pmu_bat_temp_para6) {
		 temp = 5 + (axp_config->pmu_bat_temp_para6-data)*5/(axp_config->pmu_bat_temp_para6-axp_config->pmu_bat_temp_para7);
	 } else if (data <= axp_config->pmu_bat_temp_para5) {
		 temp = 0 + (axp_config->pmu_bat_temp_para5-data)*5/(axp_config->pmu_bat_temp_para5-axp_config->pmu_bat_temp_para6);
	 } else if (data <= axp_config->pmu_bat_temp_para4) {
		 temp = -5 + (axp_config->pmu_bat_temp_para4-data)*5/(axp_config->pmu_bat_temp_para4-axp_config->pmu_bat_temp_para5);
	 } else if (data <= axp_config->pmu_bat_temp_para3) {
		 temp = -10 + (axp_config->pmu_bat_temp_para3-data)*5/(axp_config->pmu_bat_temp_para3-axp_config->pmu_bat_temp_para4);
	 } else if (data <= axp_config->pmu_bat_temp_para2) {
		 temp = -15 + (axp_config->pmu_bat_temp_para2-data)*5/(axp_config->pmu_bat_temp_para2-axp_config->pmu_bat_temp_para3);
	 } else if (data <= axp_config->pmu_bat_temp_para1) {
		 temp = -25 + (axp_config->pmu_bat_temp_para1-data)*10/(axp_config->pmu_bat_temp_para1-axp_config->pmu_bat_temp_para2);
	 } else
		 temp = -25;
    return temp;
}

static inline s32 axp_vts_to_mV(u16 reg)
{
    return ((s32)(((reg >> 8) << 4 ) | (reg & 0x000F))) * 800 / 1000;
}

static inline s32 axp_vbat_to_mV(u16 reg)
{
	 return ((s32)((( reg >> 8) << 4 ) | (reg & 0x000F))) * 1100 / 1000;
}

static inline s32 axp_ocvbat_to_mV(u16 reg)
{
	 return ((s32)((( reg >> 8) << 4 ) | (reg & 0x000F))) * 1100 / 1000;
}

static inline s32 axp_vdc_to_mV(u16 reg)
{
	 return ((s32)(((reg >> 8) << 4 ) | (reg & 0x000F))) * 1700 / 1000;
}

static inline s32 axp_ibat_to_mA(u16 reg)
{
	 return ((s32)(((reg >> 8) << 4 ) | (reg & 0x000F))) ;
}

static inline s32 axp_icharge_to_mA(u16 reg)
{
	 return ((s32)(((reg >> 8) << 4 ) | (reg & 0x000F)));
}

static inline s32 axp_iac_to_mA(u16 reg)
{
	 return ((s32)(((reg >> 8) << 4 ) | (reg & 0x000F))) * 625 / 1000;
}

static inline s32 axp_iusb_to_mA(u16 reg)
{
	 return ((s32)(((reg >> 8) << 4 ) | (reg & 0x000F))) * 375 / 1000;
}

static inline void axp_read_adc(struct axp_charger *charger,
  struct axp_adc_res *adc)
{
	u8 tmp[8];

	adc->vac_res = 0;
	adc->iac_res = 0;
	adc->vusb_res = 0;
	adc->iusb_res = 0;
	axp_reads(charger->master,AXP_OCVBATH_RES,2,tmp);
	adc->ocvbat_res = ((u16) tmp[0] << 8 )| tmp[1];
	axp_reads(charger->master,AXP_VTS_RES,2,tmp);
	adc->ts_res = ((u16) tmp[0] << 8 )| tmp[1];

	axp_reads(charger->master,AXP_VBATH_RES,6,tmp);
	adc->vbat_res = ((u16) tmp[0] << 8 )| tmp[1];
	adc->ichar_res = ((u16) tmp[2] << 8 )| tmp[3];
	adc->idischar_res = ((u16) tmp[4] << 8 )| tmp[5];
}

u64 axp_read_power_sply(void)
{
	u16 tmp;
	int disvbat;
	int disibat;
	u64 power_sply = 0;

	axp_read_adc(axp_charger, &adc);
	tmp = adc.vbat_res;
	disvbat = axp_vbat_to_mV(tmp);
	tmp = adc.idischar_res;
	disibat = axp_ibat_to_mA(tmp);
	printk(KERN_ERR "vbat = %d mV, disibat=%d mA\n", disvbat, disibat);
	power_sply = disvbat * disibat;
	if (0 != power_sply)
		power_sply = power_sply/1000;
	return power_sply;
}
EXPORT_SYMBOL(axp_read_power_sply);

void axp_battery_update_vol(struct axp_charger * charger)
{
	u8 val, temp_val[2];
	s32 ocv_percentage = 0, coulumb_percentage = 0, rest_vol = 0;

	axp_reads(charger->master,0xe4,2,temp_val);
	ocv_percentage = temp_val[0] & 0x7f;
	coulumb_percentage = temp_val[1] & 0x7f;
	axp_read(charger->master, AXP_CAP,&val);
	rest_vol = (s32) (val & 0x7F);

	DBG_PSY_MSG(DEBUG_SPLY, "AXP rest_vol = %d\n", rest_vol);
	DBG_PSY_MSG(DEBUG_SPLY, "Axp OCV_percentage = %d\n", ocv_percentage);
	DBG_PSY_MSG(DEBUG_SPLY, "Axp Coulumb_percentage = %d\n", coulumb_percentage);

	spin_lock(&charger->charger_lock);
	charger->rest_vol = rest_vol;
	if ((axp_config->ocv_coulumb_100) && (100 == ocv_percentage) && (100 == coulumb_percentage)) {
		charger->rest_vol = 100;
	}
	if((charger->bat_det == 0) || (charger->rest_vol == 127)){
		charger->rest_vol = 100;
	}
	spin_unlock(&charger->charger_lock);

	DBG_PSY_MSG(DEBUG_SPLY, "charger->rest_vol = %d\n",charger->rest_vol);
	return;
}

void axp_charger_update_state(struct axp_charger *charger)
{
	u8 val[2];
	u16 tmp;

	/* wait for the stability of ACIN valid and VBUS valid */
	msleep(2);
	axp_reads(charger->master,AXP_CHARGE_STATUS,2,val);
	tmp = (val[1] << 8 )+ val[0];
    DBG_PSY_MSG(DEBUG_CHG, "Axp read again charger->ext_valid: %d\n", \
                                                   charger->ext_valid);
	spin_lock(&charger->charger_lock);
	charger->is_on = (val[1] & AXP_IN_CHARGE) ? 1 : 0;
	charger->fault = val[1];
	charger->bat_det = (tmp & AXP_STATUS_BATEN)?1:0;
	charger->ac_det = (tmp & AXP_STATUS_ACEN)?1:0;
	charger->usb_det = (tmp & AXP_STATUS_USBEN)?1:0;
	charger->ext_valid = (tmp & (AXP_STATUS_USBVA |AXP_STATUS_ACVA))?1:0;
	charger->in_short = (tmp& AXP_STATUS_ACUSBSH)?1:0;
	if(!charger->in_short) {
		charger->ac_valid = (tmp & AXP_STATUS_ACVA)?1:0;
	}
	charger->bat_current_direction = (tmp & AXP_STATUS_BATCURDIR)?1:0;
	charger->batery_active = (tmp & AXP_STATUS_BATINACT)?1:0;
	charger->int_over_temp = (tmp & AXP_STATUS_ICTEMOV)?1:0;
	spin_unlock(&charger->charger_lock);
	axp_read(charger->master,AXP_CHARGE_CONTROL1,val);
	spin_lock(&charger->charger_lock);
	charger->charge_on = ((val[0] >> 7) & 0x01);
	spin_unlock(&charger->charger_lock);
}

void axp_charger_update(struct axp_charger *charger, const struct axp_config_info *axp_config)
{
	u16 tmp;
	u8 val[2];
	s32 bat_temp_mv;

	charger->adc = &adc;
	axp_read_adc(charger, &adc);
	tmp = charger->adc->vbat_res;

	spin_lock(&charger->charger_lock);
	charger->vbat = axp_vbat_to_mV(tmp);
	spin_unlock(&charger->charger_lock);

	//tmp = charger->adc->ichar_res + charger->adc->idischar_res;
	spin_lock(&charger->charger_lock);
	charger->ibat = axp_icharge_to_mA(charger->adc->ichar_res)-axp_ibat_to_mA(charger->adc->idischar_res);
	tmp = 00;///qin
	charger->vac = axp_vdc_to_mV(tmp);
	tmp = 00;
	charger->iac = axp_iac_to_mA(tmp);
	tmp = 00;
	charger->vusb = axp_vdc_to_mV(tmp);
	tmp = 00;
	charger->iusb = axp_iusb_to_mA(tmp);
	spin_unlock(&charger->charger_lock);

	axp_reads(charger->master,AXP_INTTEMP,2,val);
	//DBG_PSY_MSG("TEMPERATURE:val1=0x%x,val2=0x%x\n",val[1],val[0]);
	tmp = (val[0] << 4 ) + (val[1] & 0x0F);

	spin_lock(&charger->charger_lock);
	charger->ic_temp = (s32) tmp *1063/10000  - 2667/10;
	charger->disvbat =  charger->vbat;
	charger->disibat =  axp_ibat_to_mA(charger->adc->idischar_res);
	spin_unlock(&charger->charger_lock);

	tmp = charger->adc->ocvbat_res;
	spin_lock(&charger->charger_lock);
	charger->ocv = axp_ocvbat_to_mV(tmp);
	spin_unlock(&charger->charger_lock);

	tmp = charger->adc->ts_res;
	bat_temp_mv = axp_vts_to_mV(tmp);
	spin_lock(&charger->charger_lock);
	charger->bat_temp = axp_vts_to_temp(bat_temp_mv, axp_config);
	spin_unlock(&charger->charger_lock);
}

