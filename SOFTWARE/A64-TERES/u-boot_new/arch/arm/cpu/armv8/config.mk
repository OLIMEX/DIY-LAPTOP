#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
#
# SPDX-License-Identifier:	GPL-2.0+
#

CROSS_COMPILE := $(src)/../armv8_toolchain/gcc-linaro-aarch64-linux-gnu-4.9-2014.09_linux/bin/aarch64-linux-gnu-

PLATFORM_RELFLAGS += -fno-common -ffixed-x18 -Werror

PF_CPPFLAGS_ARMV8 := $(call cc-option, -march=armv8-a)
PF_NO_UNALIGNED := $(call cc-option, -mstrict-align)
PLATFORM_CPPFLAGS += $(PF_CPPFLAGS_ARMV8)
PLATFORM_CPPFLAGS += $(PF_NO_UNALIGNED)
