/*
 *  arch/arm/mach-sunxi/arisc/include/arisc_message_manager.h
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

#ifndef __ARISC_MESSAGE_MANAGER_H
#define __ARISC_MESSAGE_MANAGER_H

/**
 * initialize message manager.
 * @para:  none.
 *
 * returns:  OK if initialize succeeded, others if failed.
 */
int arisc_message_manager_init(void *addr, uint32_t size);

/**
 * exit message manager.
 * para:  none.
 *
 * returns:  OK if exit succeeded, others if failed.
 */
int arisc_message_manager_exit(void);

/**
 * allocate one message frame. mainly use for send message by message-box,
 * the message frame allocate form messages pool shared memory area.
 * @para:  none.
 *
 * returns:  the pointer of allocated message frame, NULL if failed;
 */
struct arisc_message *arisc_message_allocate(unsigned int msg_attr);

/**
 * free one message frame. mainly use for process message finished,
 * free it to messages pool or add to free message queue.
 * @pmessage:  the pointer of free message frame.
 *
 * returns:  none.
 */
void arisc_message_free(struct arisc_message *pmessage);

/**
 * notify system that one message coming.
 * @pmessage : the pointer of coming message frame.
 *
 * returns:  OK if notify succeeded, other if failed.
 */
int arisc_message_coming_notify(struct arisc_message *pmessage);

int arisc_semaphore_used_num_query(void);

struct arisc_message *arisc_message_map_to_cpux(uint32_t addr);
uint32_t arisc_message_map_to_cpus(struct arisc_message *message);
int arisc_message_valid(struct arisc_message *pmessage);

#endif  /* __MESSAGE_MANAGER_H */
