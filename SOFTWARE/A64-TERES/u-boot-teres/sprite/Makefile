#
# Copyright 2000-2008
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



ifndef CONFIG_SUNXI_SPINOR_PLATFORM
obj-y	+= queue.o
obj-y += sprite_queue.o
obj-y += sprite_card.o
#obj-y += encrypt/encrypt.o
obj-y += encrypt/twofish/twofish.o
obj-y += encrypt/twofish/twofish_new.o
obj-y += sparse/sparse.o
obj-y += sprite_download.o
obj-y += sprite_verify.o
obj-y += firmware/imgdecode.o
obj-y += sprite_privatedata.o
obj-y += sprite_main.o
obj-y += sprite_erase.o
obj-y += sprite_recovery.o
obj-y += sprite_led.o
obj-y += sprite_cartoon.o
obj-y += sprite_storage_crypt.o

obj-$(CONFIG_SUNXI_SECURE_STORAGE) += sprite_secstorage.o
obj-$(CONFIG_SUNXI_SECURE_STORAGE) += sprite_secure_object.o
obj-$(CONFIG_SUNXI_SECURE_SYSTEM) += sprite_storage_crypt.o

endif
