#
# (C) Copyright 2000-2011
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2011
# Daniel Schwierzeck, daniel.schwierzeck@googlemail.com.
#
# (C) Copyright 2011
# Texas Instruments Incorporated - http://www.ti.com/
# Aneesh V <aneesh@ti.com>
#
# This file is released under the terms of GPL v2 and any later version.
# See the file COPYING in the root directory of the source tree for details.
#
# Based on top-level Makefile.
#

include $(SPLDIR)/config.mk
include $(TOPDIR)/include/autoconf.mk
include $(TOPDIR)/include/autoconf.mk.dep

CONFIG_SPL := y
export CONFIG_SPL

FES_LDSCRIPT := $(SPLDIR)/fes_init/main/fes_init.lds


# We want the final binaries in this directory
obj := $(OBJTREE)/sunxi_spl/fes_init/


LIBS-y += sunxi_spl/fes_init/spl/libsource_spl.o
LIBS-y += sunxi_spl/fes_init/main/libmain.o
LIBS-y += sunxi_spl/spl/lib/libgeneric.o
LIBS-y += arch/$(ARCH)/cpu/$(CPU)/$(SOC)/dram/libdram.o
LIBS-$(CONFIG_SUNXI_CHIPID) += arch/$(ARCH)/cpu/$(CPU)/$(SOC)/dram/libchipid.o

LIBS := $(addprefix $(OBJTREE)/,$(sort $(LIBS-y)))


# Special flags for CPP when processing the linker script.
# Pass the version down so we can handle backwards compatibility
# on the fly.
LDPPFLAGS += \
	-include $(TOPDIR)/include/u-boot/u-boot.lds.h \
	-DFES1ADDR=$(CONFIG_FES1_RUN_ADDR)	 \
	$(shell $(LD) --version | \
	  sed -ne 's/GNU ld version \([0-9][0-9]*\)\.\([0-9][0-9]*\).*/-DLD_MAJOR=\1 -DLD_MINOR=\2/p')

ALL-y	+= $(obj)fes1.bin

all: $(ALL-y)

$(obj)fes1.bin:	$(obj)fes1.axf
	$(OBJCOPY) $(OBJCFLAGS) -O binary $< $@

$(obj)fes1.axf:  $(LIBS) $(obj)fes_init.lds
	$(LD) $(LIBS) $(PLATFORM_LIBGCC) $(LDFLAGS) -T$(obj)fes_init.lds -o fes1.axf -Map fes1.map

$(LIBS): depend
	$(MAKE) -C $(SRCTREE)$(dir $(subst $(OBJTREE),,$@))

$(obj)fes_init.lds: $(FES_LDSCRIPT)
	@$(CPP) $(ALL_CFLAGS) $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P - <$^ >$@

depend:.depend

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude .depend

#########################################################################
