
##
## Makefile for Sunxi Secure Boot
##



include $(SPLDIR)/config.mk

LIB	:= $(obj)libmain.o

START	:= brom.o

HEAD    := sbromsw_head.o
DATA 	:= sbromsw_hash.o

COBJS-y += sbrom_main.o
COBJS-y += sbromsw_toc1.o

SRCS	:= $(START:.o=.S) $(COBJS:.o=.c) $(HEAD:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS) $(COBJS-y) $(SOBJS))
HEAD	:= $(addprefix $(obj),$(HEAD) $(START))
DATA	:= $(addprefix $(obj),$(DATA))

all:	 $(obj).depend $(HEAD) $(START) $(LIB) $(DATA)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
