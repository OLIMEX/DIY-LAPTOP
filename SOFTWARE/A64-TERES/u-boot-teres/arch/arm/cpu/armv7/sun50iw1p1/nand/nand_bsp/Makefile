
##
## Makefile for Sunxi Secure Boot
##



include $(TOPDIR)/config.mk

LIB	:= $(obj)libnand.o

COBJS += nand_for_boot0/nand_for_boot0.o  nand_for_boot0/osal/nand_osal_boot0.o

COBJS += nfc_for_boot0/src/nand_scan_for_boot.o  nfc_for_boot0/src/nfc_for_boot0.o  nfc_for_boot0/src/nand_simple_for_boot0.o


COBJS	:= $(COBJS)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
