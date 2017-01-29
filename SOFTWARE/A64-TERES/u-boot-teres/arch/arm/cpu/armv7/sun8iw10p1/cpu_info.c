/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <boot_type.h>


int print_boot_type(void)
{
	sunxi_boot_type_t type;

	puts("BOOT:  ");

	type = uboot_spare_head.boot_data.storage_type;
	switch (type) {
	case SUNXI_BOOT_TYPE_MMC0:
		puts("MMC0\n");
		break;
	case SUNXI_BOOT_TYPE_NAND:
		puts("NAND\n");
		break;
	case SUNXI_BOOT_TYPE_MMC2:
		puts("MMC2\n");
		break;
	case SUNXI_BOOT_TYPE_SPI:
		puts("SPI\n");
		break;
	case SUNXI_BOOT_TYPE_NULL:
		/* fall through */
	default:
		puts("ERROR\n");
		break;
	}

	return 0;
}

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	tick_printf("CPU:   SUNXI Family\n");
	return 0;
}
#endif
