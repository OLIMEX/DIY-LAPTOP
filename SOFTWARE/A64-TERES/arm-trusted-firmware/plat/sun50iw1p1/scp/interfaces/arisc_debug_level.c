/*
 *  drivers/arisc/interfaces/arisc_debug_level.c
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
 * set arisc debug level.
 * @level: arisc debug level;
 *
 * return: 0 - set arisc debug level successed, !0 - set arisc debug level failed;
 */
int arisc_set_debug_level(unsigned int level)
{
	int                   result;
	struct arisc_message *pmessage;

	/* allocate a message frame */
	pmessage = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);
	if (pmessage == NULL) {
		ARISC_ERR("allocate message for debug level request failed\n");
		return -ENOMEM;
	}

	/* initialize message */
	pmessage->type     = ARISC_SET_DEBUG_LEVEL;
	pmessage->state    = ARISC_MESSAGE_INITIALIZED;
	pmessage->paras[0] = level;
	pmessage->cb.handler = NULL;
	pmessage->cb.arg     = NULL;

	/* send set debug level request to arisc */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);

	/* free message */
	result = pmessage->result;
	arisc_message_free(pmessage);

	return result;
}

int arisc_set_uart_baudrate(uint32_t baudrate)
{
	int                   result;
	struct arisc_message *pmessage;

	/* allocate a message frame */
	pmessage = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);
	if (pmessage == NULL) {
		ARISC_ERR("allocate message for set baudrate request failed\n");
		return -ENOMEM;
	}

	/* initialize message */
	pmessage->type       = ARISC_SET_UART_BAUDRATE;
	pmessage->state      = ARISC_MESSAGE_INITIALIZED;
	pmessage->paras[0]   = baudrate;
	pmessage->cb.handler = NULL;
	pmessage->cb.arg     = NULL;

	/* send message use hwmsgbox */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);

	/* free message */
	result = pmessage->result;
	arisc_message_free(pmessage);

	return result;
}

int arisc_report_error_info(struct arisc_message *pmessage)
{
	uint32_t id = pmessage->paras[0];

	switch(id) {
		case ERR_NMI_INT_TIMEOUT: {
			ARISC_ERR("arisc report error info: nmi int response timeout\n");
			break;
		}
		default: {
			ARISC_ERR("invaid arisc report error infomation id:%u\n", id);
			return -EINVAL;
		}
	}

	return 0;
}

/**
 * set paras.
 *
 * return: 0 - set paras successed, !0 - set paras failed;
 */
int arisc_set_paras(void)
{
	int                   result;
	struct arisc_message *pmessage;

	/* allocate a message frame */
	pmessage = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);
	if (pmessage == NULL) {
		ARISC_ERR("allocate message for set paras request failed\n");
		return -ENOMEM;
	}

	/* initialize message */
	pmessage->type     = ARISC_SET_PARAS;
	pmessage->state    = ARISC_MESSAGE_INITIALIZED;
	pmessage->cb.handler = NULL;
	pmessage->cb.arg     = NULL;

	/* send set debug level request to arisc */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);

	/* free message */
	result = pmessage->result;
	arisc_message_free(pmessage);

	return result;
}