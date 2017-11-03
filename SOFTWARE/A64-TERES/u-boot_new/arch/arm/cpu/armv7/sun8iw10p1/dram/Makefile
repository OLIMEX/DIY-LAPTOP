
##
## Makefile for Sunxi Secure Boot
##



include $(TOPDIR)/config.mk

all:
ifeq ($(notdir $(shell find ./ -name lib-dram)), lib-dram)

	make -C lib-dram
else
	@echo "libdram exist"

endif
	cp ./libdram ./libdram.o
#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################

