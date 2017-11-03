
##
## Makefile for Sunxi Secure Boot
##



include $(SPLDIR)/config.mk

LIB	:= $(obj)libloadnand.o

COBJS-y += load_Boot1_from_nand.o

SRCS	:= $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))

all: 	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
