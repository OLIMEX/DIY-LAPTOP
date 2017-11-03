#
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(SPLDIR)/config.mk
LIB	=  $(obj)libsource_spl.o

COBJS	+= rtc_region.o
COBJS	+= timer_spl.o
COBJS	+= gpio_spl.o
COBJS	+= serial_spl.o
COBJS	+= clock_spl.o

#COBJS-$(CONFIG_XXXX)	+= xxxx.o

SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))



all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

depend:	$(obj).depend
.PHONY: depend

# defines $(obj).depend target
include $(SRCTREE)/rules.mk
