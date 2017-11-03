/*
 * include/arisc.h
 *
 * Copyright 2015-2016 (c) Allwinner.
 * superm (superm@allwinnertech.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#ifndef	__ARISC_H__
#define	__ARISC_H__

int  sunxi_arisc_probe(void);
int sunxi_arisc_wait_ready(void);

#endif /* __ARISC_H__ */
