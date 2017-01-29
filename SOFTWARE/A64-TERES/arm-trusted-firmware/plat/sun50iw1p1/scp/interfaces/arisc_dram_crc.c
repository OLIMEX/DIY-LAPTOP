/*
 *  drivers/arisc/interfaces/arisc_dram_crc.c
 *
 * Copyright (c) 2012 Allwinner.
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
	struct arisc_message *pmessage;

	ARISC_INF("en:%x, src:%x len:%x\n", dram_crc_en, dram_crc_srcaddr, dram_crc_len);

	/* allocate a message frame */
	pmessage = arisc_message_allocate(0);
	if (pmessage == NULL) {
		ARISC_ERR("allocate message for seting dram crc paras request failed\n");
		return -ENOMEM;
	}

	/* initialize message */
	pmessage->type     = ARISC_SET_DEBUG_DRAM_CRC_PARAS;
	pmessage->paras[0] = dram_crc_en;
	pmessage->paras[1] = dram_crc_srcaddr;
	pmessage->paras[2] = dram_crc_len;
	pmessage->state    = ARISC_MESSAGE_INITIALIZED;

	/* send set debug level request to arisc */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);

	return 0;
}

