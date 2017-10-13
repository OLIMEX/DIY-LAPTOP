/*
*********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2006-2011, kevin.z China
*                                             All Rights Reserved
*
* File    : standby_power.h
* By      : kevin.z
* Version : v1.0
* Date    : 2011-5-31 14:34
* Descript:
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/
#ifndef __AXP_POWER_H__
#define __AXP_POWER_H__

#define AXP_LDO(_pmic, _id, min, max, step1, vreg, shift, nbits, ereg, ebit, switch_vol, step2, new_level, mode_addr, freq_addr) \
{									\
	.min_uV		= (min) * 1000,					\
	.max_uV		= (max) * 1000,					\
	.step1_uV	= (step1) * 1000,				\
	.vol_reg	= _pmic##_##vreg,				\
	.vol_shift	= (shift),					\
	.vol_nbits	= (nbits),					\
	.enable_reg	= _pmic##_##ereg,				\
	.enable_bit	= (ebit),					\
	.switch_uV	= (switch_vol)*1000,				\
	.step2_uV	= (step2)*1000,					\
	.new_level_uV	= (new_level)*1000,				\
}

#define AXP_BUCK(_pmic, _id, min, max, step1, vreg, shift, nbits, ereg, ebit, switch_vol, step2, new_level, mode_addr, freq_addr) \
{									\
	.min_uV		= (min) * 1000,					\
	.max_uV		= (max) * 1000,					\
	.step1_uV	= (step1) * 1000,				\
	.vol_reg	= _pmic##_##vreg,				\
	.vol_shift	= (shift),					\
	.vol_nbits	= (nbits),					\
	.enable_reg	= _pmic##_##ereg,				\
	.enable_bit	= (ebit),					\
	.switch_uV	= (switch_vol)*1000,				\
	.step2_uV	= (step2)*1000,					\
	.new_level_uV	= (new_level)*1000,				\
	.mode_reg	= mode_addr,					\
	.freq_reg	= freq_addr,					\
}

#define AXP_DCDC(_pmic, _id, min, max, step1, vreg, shift, nbits, ereg, ebit, switch_vol, step2, new_level, mode_addr, freq_addr) \
{									\
	.min_uV		= (min) * 1000,					\
	.max_uV		= (max) * 1000,					\
	.step1_uV	= (step1) * 1000,				\
	.vol_reg	= _pmic##_##vreg,				\
	.vol_shift	= (shift),					\
	.vol_nbits	= (nbits),					\
	.enable_reg	= _pmic##_##ereg,				\
	.enable_bit	= (ebit),					\
	.switch_uV	= (switch_vol)*1000,				\
	.step2_uV	= (step2)*1000,					\
	.new_level_uV	= (new_level)*1000,				\
	.mode_reg	= mode_addr,					\
	.freq_reg	= freq_addr,					\
}

#define AXP_SW(_pmic, _id, min, max, step1, vreg, shift, nbits, ereg, ebit, switch_vol, step2, new_level, mode_addr, freq_addr) \
{									\
	.min_uV		= (min) * 1000,					\
	.max_uV		= (max) * 1000,					\
	.step1_uV	= (step1) * 1000,				\
	.vol_reg	= _pmic##_##vreg,				\
	.vol_shift	= (shift),					\
	.vol_nbits	= (nbits),					\
	.enable_reg	= _pmic##_##ereg,				\
	.enable_bit	= (ebit),					\
	.switch_uV	= (switch_vol)*1000,				\
	.step2_uV	= (step2)*1000,					\
	.new_level_uV	= (new_level)*1000,				\
	.mode_reg	= mode_addr,					\
	.freq_reg	= freq_addr,					\
}

#define AXP_REGU_ATTR(_name)						\
{									\
	.attr = { .name = #_name,.mode = 0644 },			\
	.show =  _name##_show,						\
	.store = _name##_store,						\
}

struct axp_regulator_info {
	int	min_uV;
	int	max_uV;
	int	step1_uV;
	int	vol_reg;
	int	vol_shift;
	int	vol_nbits;
	int	enable_reg;
	int	enable_bit;
	int	switch_uV;
	int	step2_uV;
	int	new_level_uV;
	int	mode_reg;
	int	freq_reg;
};

#endif
