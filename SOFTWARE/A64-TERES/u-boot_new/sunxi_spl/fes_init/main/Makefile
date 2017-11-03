
##
## Makefile for Sunxi Secure Boot
##



include $(SPLDIR)/config.mk

LIB	:= $(obj)libmain.o

HEAD    := fes_head.o

START	:= fes1_entry.o

COBJS   += fes1_main.o

SRCS	:= $(START:.o=.S) $(COBJS:.o=.c) $(HEAD:.o=.c)

OBJS =  $(COBJS)

all:	 .depend $(HEAD) $(START) $(LIB)


$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
