/*
 * drivers/staging/android/ion/sunxi/ion_sunxi.h
 *
 * Copyright(c) 2013-2015 Allwinnertech Co., Ltd.
 *      http://www.allwinnertech.com
 *
 * Author: liugang <liugang@allwinnertech.com>
 *
 * sunxi ion header file
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __ION_SUNXI_H
#define __ION_SUNXI_H

/**
 * ion_client_create() -  allocate a client and returns it
 * @name:		used for debugging
 */
struct ion_client *sunxi_ion_client_create(const char *name);
					 

#endif
