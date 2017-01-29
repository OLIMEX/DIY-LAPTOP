#
# Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# Neither the name of ARM nor the names of its contributors may be used
# to endorse or promote products derived from this software without specific
# prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

# Shared memory may be allocated at the top of Trusted SRAM (tsram) or at the
# base of Trusted SRAM (tdram)
SUNXI_SHARED_DATA_LOCATION	:=	tdram
ifeq (${SUNXI_SHARED_DATA_LOCATION}, tsram)
  SUNXI_SHARED_DATA_LOCATION_ID := SUNXI_IN_TRUSTED_SRAM
else ifeq (${SUNXI_SHARED_DATA_LOCATION}, tdram)
  SUNXI_SHARED_DATA_LOCATION_ID := SUNXI_IN_TRUSTED_DRAM
else
  $(error "Unsupported SUNXI_SHARED_DATA_LOCATION value")
endif

# On FVP, the TSP can execute either from Trusted SRAM or Trusted DRAM.
# Trusted SRAM is the default.
SUNXI_TSP_RAM_LOCATION	:=	tdram
ifeq (${SUNXI_TSP_RAM_LOCATION}, tsram)
  SUNXI_TSP_RAM_LOCATION_ID := SUNXI_IN_TRUSTED_SRAM
else ifeq (${SUNXI_TSP_RAM_LOCATION}, tdram)
  SUNXI_TSP_RAM_LOCATION_ID := SUNXI_IN_TRUSTED_DRAM
else
  $(error "Unsupported SUNXI_TSP_RAM_LOCATION value")
endif

ifeq (${SUNXI_SHARED_DATA_LOCATION}, tsram)
  ifeq (${SUNXI_TSP_RAM_LOCATION}, tdram)
    $(error Shared data in Trusted SRAM and TSP in Trusted DRAM is not supported)
  endif
endif

# Process flags
$(eval $(call add_define,SUNXI_SHARED_DATA_LOCATION_ID))
$(eval $(call add_define,SUNXI_TSP_RAM_LOCATION_ID))

PLAT_INCLUDES		:=	-Iplat/sun50iw1p1/include/


PLAT_BL_COMMON_SOURCES	:= lib/aarch64/xlat_tables.c			\
				plat/common/aarch64/plat_common.c		\
				plat/sun50iw1p1/drivers/uart/uart.c		\
				plat/sun50iw1p1/drivers/gpio/gpio.c		



BL31_SOURCES		+=	drivers/arm/cci400/cci400.c \
				drivers/arm/gic/gic_v2.c			\
				lib/cpus/aarch64/aem_generic.S			\
				lib/cpus/aarch64/cortex_a53.S			\
				lib/cpus/aarch64/cortex_a57.S			\
				plat/common/aarch64/platform_mp_stack.S		\
				plat/sun50iw1p1/bl31_sunxi_setup.c			\
				plat/sun50iw1p1/plat_pm.c				\
				plat/sun50iw1p1/plat_gic.c				\
				plat/sun50iw1p1/mhu.c				\
				plat/sun50iw1p1/scpi.c				\
				plat/sun50iw1p1/sunxi_security.c				\
				plat/sun50iw1p1/sunxi_cpu_ops.c				\
				plat/sun50iw1p1/plat_topology.c				\
				plat/sun50iw1p1/aarch64/plat_helpers.S			\
				plat/sun50iw1p1/aarch64/sunxi_common.c			\
				services/arm/arm_svc_setup.c	

include plat/${PLAT}/scp/arisc.mk
