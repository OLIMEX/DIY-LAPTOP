/*
 *  drivers/arisc/interfaces/arisc_standby.c
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
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include <linux/sys_config.h>

/* record super-standby wakeup event */
static unsigned long dram_crc_error = 0;
static unsigned long dram_crc_total_count = 0;
static unsigned long dram_crc_error_count = 0;


/**
 * query super-standby wakeup source.
 * @para:  point of buffer to store wakeup event informations.
 *
 * return: result, 0 - query successed,
 *                !0 - query failed;
 */
int arisc_query_wakeup_source(u32 *event)
{
	int result;

	/* send message use hwmsgbox */
	result = invoke_scp_fn_smc(ARM_SVC_ARISC_QUERY_WAKEUP_SRC_REQ, virt_to_phys(event), 0, 0);

	return result;
}
EXPORT_SYMBOL(arisc_query_wakeup_source);

/*
 * query super-standby infoation.
 * @para:  point of array to store power states informations during sst.
 * @op: 0:read, 1:set
 *
 * return: result, 0 - query successed,
 *                !0 - query failed;
 */
int arisc_query_standby_power_cfg(struct standby_info_para *para)
{
	memcpy((void *)para, (void *)&arisc_powchk_back, sizeof(struct standby_info_para));

	return 0;
}
EXPORT_SYMBOL(arisc_query_standby_power_cfg);

/*
 * query super-standby infoation.
 * @para:  point of array to store power states informations during sst.
 * @op: 0:read, 1:set
 *
 * return: result, 0 - query successed,
 *                !0 - query failed;
 */
int arisc_query_set_standby_info(struct standby_info_para *para, arisc_rw_type_e op)
{
	int result;
	u32 temp_paras[22];

	/* check standby_info_para size valid or not */
	if (sizeof(struct standby_info_para) > sizeof(temp_paras)) {
		ARISC_ERR("standby info parameters number too long\n");
		return -EINVAL;
	}

	/* initialize message */
	if (ARISC_WRITE == op) {
		memcpy((void *)temp_paras, (const void *)para, sizeof(struct standby_info_para));
		memcpy((void *)&arisc_powchk_back, (const void *)para, sizeof(struct standby_info_para));
		/* send query sst info request to arisc */
		/* FIXME: if the runtime sever enable the mmu & dcache,
	 	 * should not use flush cache here.
	 	 */
	}


	/* send message use hwmsgbox */
	result = invoke_scp_fn_smc(ARM_SVC_ARISC_STANDBY_INFO_REQ, virt_to_phys(temp_paras), op, 0);
	if (ARISC_READ == op) {
		memcpy((void *)para, (void *)temp_paras, sizeof(struct standby_info_para));
	}

	return result;
}
EXPORT_SYMBOL(arisc_query_set_standby_info);

/*
 * query super-standby dram crc result.
 * @para:  point of buffer to store dram crc result informations.
 *
 * return: result, 0 - query successed,
 *                !0 - query failed;
 */
int arisc_query_dram_crc_result(unsigned long *perror, unsigned long *ptotal_count,
	unsigned long *perror_count)
{
	*perror = dram_crc_error;
	*ptotal_count = dram_crc_total_count;
	*perror_count = dram_crc_error_count;

	return 0;
}
EXPORT_SYMBOL(arisc_query_dram_crc_result);

int arisc_set_dram_crc_result(unsigned long error, unsigned long total_count,
	unsigned long error_count)
{
	dram_crc_error = error;
	dram_crc_total_count = total_count;
	dram_crc_error_count = error_count;

	return 0;
}
EXPORT_SYMBOL(arisc_set_dram_crc_result);
