/*
 *  arch/arm/mach-sunxi/arisc/message_manager/message_manager_i.h
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

#ifndef __ARISC_MESSAGE_MANAGER_I_H
#define __ARISC_MESSAGE_MANAGER_I_H

#include "../include/arisc_includes.h"
#include "../arisc_i.h"

#define ARISC_SEM_CACHE_MAX (8)

/*
 *the strcuture of message cache,
 *main for messages cache management.
 */
typedef struct arisc_message_cache
{
	uint32_t              number;                           /* valid message number */
	struct arisc_message *cache[ARISC_MESSAGE_CACHED_MAX];  /* message cache table */
} arisc_message_cache_t;

#endif  /* __ARISC_MESSAGE_MANAGER_I_H */
