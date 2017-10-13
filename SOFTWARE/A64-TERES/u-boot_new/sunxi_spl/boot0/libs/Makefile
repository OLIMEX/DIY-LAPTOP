
##
## Makefile for Sunxi Secure Boot
##



include $(SPLDIR)/config.mk

LIB	:= $(lib)libgeneric.o

COBJS += check.o
COBJS += nand_misc.o
COBJS += mmu.o
COBJS += eabi_compat.o
#COBJS += jmp.o
COBJS += common.o

SOBJS += jmpto64.o

SOBJS   := $(SOBJS)
COBJS	:= $(COBJS)
SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS) $(SOBJS))

all:	$(obj).depend $(LIB)

$(LIB):	 $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
