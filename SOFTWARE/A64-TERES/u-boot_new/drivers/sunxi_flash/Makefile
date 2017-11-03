#
# (C) Copyright 2007-2015
# Allwinner Technology Co., Ltd. <www.allwinnertech.com>
# Jerry Wang <wangflord@allwinnertech.com>
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
# GNU General Public License for more details.
#
# SPDX-License-Identifier:	GPL-2.0
#

ifndef  CONFIG_SUNXI_SPINOR_PLATFORM
obj-y	+= sunxi_flash.o nand.o sdmmc.o
else
obj-y   += sunxi_nor_flash.o
endif
