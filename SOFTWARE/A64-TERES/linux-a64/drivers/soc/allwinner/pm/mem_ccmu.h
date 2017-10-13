/*
 *  arch/arm/mach-sunxi/pm/mem_ccmu.h
 *
 * Copyright 2012 (c) njubietech.
 * gq.yang (yanggq@njubietech.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __MEM_CCMU_H__
#define __MEM_CCMU_H__

#if defined CONFIG_ARCH_SUN8IW1P1
#include "mem_ccmu-sun8iw1p1.h"
struct ccm_state{
	__ccmu_reg_list_t	*ccm_reg;
	__ccmu_reg_list_t	ccm_reg_backup;
	
};
#elif defined CONFIG_ARCH_SUN8IW3P1
#include "mem_ccmu-sun8iw3p1.h"
struct ccm_state{
	__ccmu_reg_list_t	*ccm_reg;
	__ccmu_reg_list_t	ccm_reg_backup;
	
};
#elif defined CONFIG_ARCH_SUN8IW5P1
#include "mem_ccmu-sun8iw5p1.h"
struct ccm_state{
	__ccmu_reg_list_t	*ccm_reg;
	__ccmu_reg_list_t	ccm_reg_backup;
	
};
#elif defined CONFIG_ARCH_SUN8IW6P1
#include "mem_ccmu-sun8iw6p1.h"
struct ccm_state{
	__ccmu_reg_list_t	*ccm_reg;
	__ccmu_reg_list_t	ccm_reg_backup;
	
};
#elif defined CONFIG_ARCH_SUN9IW1P1
#include "mem_ccmu-sun9iw1p1.h"
struct ccm_state{
	__ccmu_reg_list_t		*ccm_reg;
	__ccmu_reg_list_t		ccm_reg_backup;
	__ccmu_mod_reg_list_t		*ccm_mod_reg;
	__ccmu_mod_reg_list_t		ccm_mod_reg_backup;
	
};
#elif defined CONFIG_ARCH_SUN8IW8P1
#include "mem_ccmu-sun8iw8p1.h"
struct ccm_state{
	__ccmu_reg_list_t	*ccm_reg;
	__ccmu_reg_list_t	ccm_reg_backup;

};
#elif defined CONFIG_ARCH_SUN8IW10P1
#include "mem_ccmu-sun8iw10p1.h"
struct ccm_state{
	__ccmu_reg_list_t	*ccm_reg;
	__ccmu_reg_list_t	ccm_reg_backup;

};
#elif defined CONFIG_ARCH_SUN50IW1P1
#include "mem_ccmu-sun50iw1p1.h"
struct ccm_state{
	__ccmu_reg_list_t	*ccm_reg;
	__ccmu_reg_list_t	ccm_reg_backup;

};
#else
#error "please select a platform\n"
#endif

#endif  // #ifndef __MEM_CCMU_H__

