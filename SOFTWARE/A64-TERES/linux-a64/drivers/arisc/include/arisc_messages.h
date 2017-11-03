/*
 *  arch/arm/mach-sunxi/arisc/include/arisc_messages.h
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


#ifndef __ARISC_MESSAGES_H__
#define __ARISC_MESSAGES_H__

#include <linux/arisc/arisc.h>

/* message states */
#define ARISC_MESSAGE_FREED         (0x0)   /* freed state       */
#define ARISC_MESSAGE_ALLOCATED     (0x1)   /* allocated state   */
#define ARISC_MESSAGE_INITIALIZED   (0x2)   /* initialized state */
#define ARISC_MESSAGE_RECEIVED      (0x3)   /* received state    */
#define ARISC_MESSAGE_PROCESSING    (0x4)   /* processing state  */
#define ARISC_MESSAGE_PROCESSED     (0x5)   /* processed state   */
#define ARISC_MESSAGE_FEEDBACKED    (0x6)   /* feedback state    */

/* call back struct */
typedef struct arisc_msg_cb
{
	arisc_cb_t   handler;
	void        *arg;
} arisc_msg_cb_t;

#ifdef CONFIG_ARCH_SUN50I
/*
 * the structure of message frame,
 * this structure will transfer between arisc and ac327.
 * sizeof(struct message) : 128Byte.
 */
typedef struct arisc_message
{
	volatile unsigned char           state;       /* identify the used status of message frame */
	volatile unsigned char           attr;        /* message attribute : SYN OR ASYN           */
	volatile unsigned char           type;        /* message type : DVFS_REQ                   */
	volatile unsigned char           result;      /* message process result                    */
	volatile unsigned char           reserved[4]; /* reserved for 8byte align */
	volatile struct arisc_message   *next;        /* pointer of next message frame             */
	volatile struct arisc_msg_cb     cb;          /* the callback function and arg of message  */
	volatile void                   *private;     /* message private data                      */
	volatile unsigned int            paras[22];   /* the parameters of message                 */
} arisc_message_t;
#else
/*
 * the structure of message frame,
 * this structure will transfer between arisc and ac327.
 * sizeof(struct message) : 64Byte.
 */
typedef struct arisc_message
{
	volatile unsigned char           state;     /* identify the used status of message frame */
	volatile unsigned char           attr;      /* message attribute : SYN OR ASYN           */
	volatile unsigned char           type;      /* message type : DVFS_REQ                   */
	volatile unsigned char           result;    /* message process result                    */
	volatile struct arisc_message   *next;      /* pointer of next message frame             */
	volatile struct arisc_msg_cb         cb;        /* the callback function and arg of message  */
	volatile void                   *private;   /* message private data                      */
	volatile unsigned int                paras[11]; /* the parameters of message                 */
} arisc_message_t;
#endif

#endif  /* __ARISC_MESSAGES_H */
