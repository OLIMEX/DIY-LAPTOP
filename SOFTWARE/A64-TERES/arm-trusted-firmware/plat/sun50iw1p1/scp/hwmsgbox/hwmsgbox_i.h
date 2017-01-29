/*
 *  arch/arm/mach-sunxi/arisc/hwmsgbox/hwmsgbox_i.h
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

#ifndef __ARISC_HWMSGBOX_I_H
#define __ARISC_HWMSGBOX_I_H

#include "../include/arisc_includes.h"

//hardware message-box register list
#define	AW_MSGBOX_CTRL_REG(m)		(0x0000 + (0x4 * (m>>2)))
#define AW_MSGBOX_IRQ_EN_REG(u)		(0x0040 + (0x20* u))
#define	AW_MSGBOX_IRQ_STATUS_REG(u)	(0x0050 + (0x20* u))
#define AW_MSGBOX_FIFO_STATUS_REG(m)	(0x0100 + (0x4 * m))
#define AW_MSGBOX_MSG_STATUS_REG(m)	(0x0140 + (0x4 * m))
#define AW_MSGBOX_MSG_REG(m)		(0x0180 + (0x4 * m))
#define AW_MSGBOX_DEBUG_REG		(0x01c0)

/* local functions */
int arisc_hwmsgbox_clear_receiver_pending(int queue, int user);
int arisc_hwmsgbox_query_receiver_pending(int queue, int user);
int arisc_hwmsgbox_enable_receiver_int(int queue, int user);
int arisc_hwmsgbox_set_receiver(int queue, int user);
int arisc_hwmsgbox_set_transmitter(int queue, int user);
int arisc_hwmsgbox_wait_message_feedback(struct arisc_message *pmessage);
int arisc_hwmsgbox_message_feedback(struct arisc_message *pmessage);
int arisc_message_valid(struct arisc_message *pmessage);

#endif  /* __ARISC_HWMSGBOX_I_H */
