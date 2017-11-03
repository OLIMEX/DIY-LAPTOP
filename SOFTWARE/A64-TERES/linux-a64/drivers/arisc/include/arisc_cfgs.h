/*
 *  arch/arm/mach-sunxi/arisc/include/arisc_cfgs.h
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

#ifndef __ARISC_CFGS_H
#define __ARISC_CFGS_H

/* arisc software version number */
#if defined CONFIG_ARCH_SUN8IW1P1
#define ARISC_VERSIONS (100)
#elif defined CONFIG_ARCH_SUN8IW3P1
#define ARISC_VERSIONS (101)
#elif defined CONFIG_ARCH_SUN8IW5P1
#define ARISC_VERSIONS (102)
#elif defined CONFIG_ARCH_SUN8IW6P1
#define ARISC_VERSIONS (103)
#elif defined CONFIG_ARCH_SUN8IW7P1
#define ARISC_VERSIONS (104)
#elif defined CONFIG_ARCH_SUN8IW9P1
#define ARISC_VERSIONS (105)
#elif defined CONFIG_ARCH_SUN50IW1P1
#define ARISC_VERSIONS (110)
#elif defined CONFIG_ARCH_SUN9IW1P1
#define ARISC_VERSIONS (200)
#else
#error "please select a platform\n"
#endif

/* debugger system */
#define ARISC_DEBUG_ON
#define ARISC_DEBUG_LEVEL           (3) /* debug level */

#define ARISC_DEV_CLKSRC_NUM        (4)     /* the number of dev clocksource support */

#endif /* __ARISC_CFGS_H */
