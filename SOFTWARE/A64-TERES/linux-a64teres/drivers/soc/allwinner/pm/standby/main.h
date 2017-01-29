#ifndef _MAIN_H
#define _MAIN_H

/*
 * Copyright (c) 2011-2015 yanggq.young@allwinnertech.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

#include "power/standby_power.h"
#include "standby_twi.h"
#include "standby_clock.h"
#include "standby_debug.h"
#include "../pm_debug.h"
#include "../mem_timing.h"
#include "../mem_int.h"
#include "standby_divlib.h"

typedef enum POWER_SCENE_FLAGS
{
	TALKING_STANDBY_FLAG           = (1<<0x0),
	USB_STANDBY_FLAG               = (1<<0x1),
	MP3_STANDBY_FLAG               = (1<<0x2),
	SUPER_STANDBY_FLAG             = (1<<0x3),
	NORMAL_STANDBY_FLAG            = (1<<0x4),
	GPIO_STANDBY_FLAG              = (1<<0x5),
	MISC_STANDBY_FLAG              = (1<<0x6),
	BOOT_FAST_STANDBY_FLAG         = (1<<0x7),
	MISC1_STANDBY_FLAG             = (1<<0x8),
	GPIO_HOLD_STANDBY_FLAG         = (1<<0x9),
	DYNAMIC_STANDBY_FLAG           = (1<<0xa),
	USB_OHCI_STANDBY_FLAG          = (1<<0xb),
	USB_EHCI_STANDBY_FLAG          = (1<<0xc)
} power_scene_flags;

#endif /*_MAIN_H*/

