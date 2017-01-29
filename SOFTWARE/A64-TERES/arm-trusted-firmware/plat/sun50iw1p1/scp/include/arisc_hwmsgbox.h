/*
 *  arch/arm/mach-sunxi/arisc/include/arisc_hwmsgbox.h
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

#ifndef __ARISC_HWMSGBOX_H
#define __ARISC_HWMSGBOX_H

//the number of hardware message queue.
#define AW_HWMSG_QUEUE_NUMBER	(8)

//the user of hardware message queue.
typedef enum aw_hwmsg_queue_user
{
	AW_HWMSG_QUEUE_USER_ARISC,	//arisc
	AW_HWMSG_QUEUE_USER_AC327,	//cpu0
} aw_hwmsg_queue_user_e;

/**
 * initialize hwmsgbox.
 * @para:  none.
 *
 * returns:  OK if initialize hwmsgbox succeeded, others if failed.
 */
int arisc_hwmsgbox_init(void);

/**
 * exit hwmsgbox.
 * @para:  none.
 *
 * returns:  OK if exit hwmsgbox succeeded, others if failed.
 */
int arisc_hwmsgbox_exit(void);

/**
 * send one message to another processor by hwmsgbox.
 * @pmessage:  the pointer of sended message frame.
 * @timeout:   the wait time limit when message fifo is full,
 * it is valid only when parameter mode = SEND_MESSAGE_WAIT_TIMEOUT.
 *
 * returns:  OK if send message succeeded, other if failed.
 */
int arisc_hwmsgbox_send_message(struct arisc_message *pmessage, unsigned int timeout);

/**
 * Description:     query message of hwmsgbox by hand, mainly for.
 * @para:  none.
 *
 * returns:  the point of message, NULL if timeout.
 */
struct arisc_message *arisc_hwmsgbox_query_message(void);

int arisc_hwmsgbox_enable_receiver_int(int queue, int user);
int arisc_hwmsgbox_disable_receiver_int(int queue, int user);

int arisc_hwmsgbox_feedback_message(struct arisc_message *pmessage, unsigned int timeout);

int arisc_hwmsgbox_standby_resume(void);
int arisc_hwmsgbox_standby_suspend(void);

#endif  /* __ARISC_HWMSGBOX_H */
