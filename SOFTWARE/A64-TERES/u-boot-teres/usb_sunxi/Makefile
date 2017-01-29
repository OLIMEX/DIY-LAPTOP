#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

#obj-y += usb_efex_test.o
obj-y += usb_mass.o
obj-y += usb_efex.o
obj-y += usb_fastboot.o
obj-y += usb_burn.o
obj-y += usb_base_common.o
obj-y += buf_queue.o
obj-y += efex_queue.o
obj-$(CONFIG_SUNXI_HDCP_IN_SECURESTORAGE) += key_deal.o


obj-$(CONFIG_ARCH_SUN50IW1P1)   += usb_base_sun50iw1p1.o
obj-$(CONFIG_ARCH_SUN50IW1P1)   += usb_dma_control.o

obj-$(CONFIG_ARCH_SUN8IW10P1)   += usb_base_sun8iw10p1.o
obj-$(CONFIG_ARCH_SUN8IW10P1)   += usb_dma_control.o

#########################################################################
