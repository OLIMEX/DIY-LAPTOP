
##
## Makefile for Sunxi Secure Boot
##



include $(SPLDIR)/config.mk

LIB	:= $(obj)libgeneric.o

SOBJS += switch2ns.o

COBJS += exec.o
COBJS += malloc.o
COBJS += check.o
COBJS += nand_misc.o
COBJS += mmu.o
COBJS += common.o

SOBJS   := $(SOBJS)
COBJS	:= $(COBJS)
SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS) $(SOBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
