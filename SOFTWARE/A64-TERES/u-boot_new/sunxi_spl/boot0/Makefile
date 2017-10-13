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

TOOLS_DIR  := $(TOPDIR)/tools

BOOT0_LDSCRIPT := $(TOPDIR)/sunxi_spl/boot0/main/boot0.lds

# We want the final binaries in this directory
obj := $(OBJTREE)/sunxi_spl/boot0/

BOOT0_HEAD  := sunxi_spl/boot0/boot0_head.o
START       := sunxi_spl/boot0/boot0_entry.o

LIBS-y += sunxi_spl/boot0/spl/libsource_spl.o
LIBS-y += sunxi_spl/boot0/main/libmain.o
LIBS-y += sunxi_spl/boot0/libs/libgeneric.o
LIBS-y += sunxi_spl/spl/lib/libgeneric.o
LIBS-y += arch/$(ARCH)/cpu/$(CPU)/$(SOC)/dram/libdram.o
LIBS-$(CONFIG_SUNXI_CHIPID) += arch/$(ARCH)/cpu/$(CPU)/$(SOC)/dram/libchipid.o
LIBS := $(addprefix $(OBJTREE)/,$(sort $(LIBS-y)))

LIBNAND-$(CONFIG_STORAGE_MEDIA_NAND) += sunxi_spl/boot0/load_nand/libloadnand.o
LIBNAND-$(CONFIG_STORAGE_MEDIA_NAND) += arch/$(ARCH)/cpu/$(CPU)/$(SOC)/nand/libnand.o

LIBNAND := $(addprefix $(OBJTREE)/,$(sort $(LIBNAND-y)))

LIBMMC-$(CONFIG_STORAGE_MEDIA_MMC) += sunxi_spl/boot0/load_mmc/libloadmmc.o
LIBMMC-$(CONFIG_STORAGE_MEDIA_MMC) += arch/$(ARCH)/cpu/$(CPU)/$(SOC)/mmc/libmmc.o

LIBMMC := $(addprefix $(OBJTREE)/,$(sort $(LIBMMC-y)))


LIBSPINOR-$(CONFIG_STORAGE_MEDIA_SPINOR) += sunxi_spl/boot0/load_spinor/libloadspinor.o
LIBSPINOR-$(CONFIG_STORAGE_MEDIA_SPINOR) += arch/$(ARCH)/cpu/$(CPU)/$(SOC)/spinor/libspinor.o

LIBSPINOR := $(addprefix $(OBJTREE)/,$(sort $(LIBSPINOR-y)))

__LIBS := $(subst $(obj),,$(LIBS))



# Special flags for CPP when processing the linker script.
# Pass the version down so we can handle backwards compatibility
# on the fly.
LDPPFLAGS += \
	-include $(TOPDIR)/include/u-boot/u-boot.lds.h \
	-DBOOT0ADDR=$(CONFIG_BOOT0_RUN_ADDR)	 \
	$(shell $(LD) --version | \
	  sed -ne 's/GNU ld version \([0-9][0-9]*\)\.\([0-9][0-9]*\).*/-DLD_MAJOR=\1 -DLD_MINOR=\2/p')

ALL-$(CONFIG_STORAGE_MEDIA_NAND)  += $(obj)boot0_nand.bin 
ALL-$(CONFIG_STORAGE_MEDIA_MMC)  += $(obj)boot0_sdcard.bin
ALL-$(CONFIG_STORAGE_MEDIA_SPINOR)  += $(obj)boot0_spinor.bin


all:	 $(ALL-y)

$(obj)boot0_nand.bin:	$(obj)boot0_nand.axf
	@echo bootaddr is $(CONFIG_BOOT0_RUN_ADDR)
	$(OBJCOPY) $(OBJCFLAGS) -O binary  $< $@

$(obj)boot0_nand.axf:  $(LIBS) $(LIBNAND) $(obj)boot0.lds
	$(LD) $(LIBS) $(LIBNAND) $(PLATFORM_LIBGCC) $(LDFLAGS) -T$(obj)boot0.lds -o boot0_nand.axf -Map boot0_nand.map

$(obj)boot0_sdcard.bin:	$(obj)boot0_sdcard.axf
	$(OBJCOPY) $(OBJCFLAGS) -O binary  $< $@

$(obj)boot0_sdcard.axf:  $(LIBS) $(LIBMMC) $(obj)boot0.lds
	$(LD) $(LIBS) $(LIBMMC)  $(PLATFORM_LIBGCC) $(LDFLAGS) -T$(obj)boot0.lds -o boot0_sdcard.axf -Map boot0_sdcard.map

$(obj)boot0_spinor.bin:	$(obj)boot0_spinor.axf
	$(OBJCOPY) $(OBJCFLAGS) -O binary  $< $@

$(obj)boot0_spinor.axf:  $(LIBS) $(LIBSPINOR) $(obj)boot0.lds
	$(LD) $(LIBS) $(LIBSPINOR)  $(PLATFORM_LIBGCC) $(LDFLAGS) -T$(obj)boot0.lds -o boot0_spinor.axf -Map boot0_spinor.map

$(LIBS): depend
	$(MAKE) -C $(SRCTREE)$(dir $(subst $(OBJTREE),,$@))
	
$(LIBNAND): depend
	$(MAKE) -C $(SRCTREE)$(dir $(subst $(OBJTREE),,$@))

$(LIBMMC): depend
	$(MAKE) -C $(SRCTREE)$(dir $(subst $(OBJTREE),,$@))

$(LIBSPINOR): depend
	$(MAKE) -C $(SRCTREE)$(dir $(subst $(OBJTREE),,$@))

$(obj)boot0.lds: $(BOOT0_LDSCRIPT)
	@$(CPP) $(ALL_CFLAGS) $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P - <$^ >$@

depend: .depend
#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude .depend

#########################################################################
