/*
 *  drivers/arisc/interfaces/arisc_dram_crc.c
 *
 * Copyright (c) 2012 Allwinner.
 * 2012-05-01 Written by sunny (sunny@allwinnertech.com).
 * 2012-10-01 Written by superm (superm@allwinnertech.com).
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

#include "../arisc_i.h"

/**
 * set arisc debug dram crc paras.
 * @dram_crc_en: arisc debug dram crc enable or disable;
 * @dram_crc_srcaddr: source address of dram crc area
 * @dram_crc_len: lenght of dram crc area
 *
 * return: 0 - set arisc debug dram crc paras successed, !0 - set arisc debug dram crc paras failed;
 */
int arisc_set_dram_crc_paras(unsigned int dram_crc_en, unsigned int dram_crc_srcaddr, unsigned int dram_crc_len)
{
	int result;

	result = invoke_scp_fn_smc(ARM_SVC_ARISC_SET_DEBUG_DRAM_CRC_PARAS, (u64)dram_crc_en, (u64)dram_crc_srcaddr, (u64)dram_crc_len);

	return result;
}

