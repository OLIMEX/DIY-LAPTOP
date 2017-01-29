/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 * Aneesh V <aneesh@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef __ARM_MODE_H__
#define __ARM_MODE_H__

#define  ARMV7_USR_MODE           0x10
#define  ARMV7_FIQ_MODE           0x11
#define  ARMV7_IRQ_MODE           0x12
#define  ARMV7_SVC_MODE           0x13
#define  ARMV7_MON_MODE        	  0x16
#define  ARMV7_ABT_MODE           0x17
#define  ARMV7_UND_MODE           0x1b
#define  ARMV7_SYSTEM_MODE        0x1f
#define  ARMV7_MODE_MASK          0x1f
#define  ARMV7_FIQ_MASK           0x40
#define  ARMV7_IRQ_MASK           0x80

#endif
