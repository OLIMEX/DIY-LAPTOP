
##
## Makefile for Sunxi Secure Boot
##



include $(SPLDIR)/config.mk

LIB	:= $(obj)libloadspinor.o

COBJS-y += load_boot1_from_spinor.o

SRCS	:= $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))

all:	 $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))


#########################################################################

sinclude $(obj).depend

#########################################################################