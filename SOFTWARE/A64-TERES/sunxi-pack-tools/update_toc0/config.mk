
##
## Makefile for Sunxi Secure Boot
##

#########################################################################
# clean the slate ...
PLATFORM_RELFLAGS =
PLATFORM_CPPFLAGS =
PLATFORM_LDFLAGS =

#########################################################################

HOSTCFLAGS	= -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer \
		  $(HOSTCPPFLAGS)
HOSTSTRIP	= strip

#
# Include the make variables (CC, etc...)
#

CROSS_COMPILE ?=

AS	= $(CROSS_COMPILE)as
LD	= $(CROSS_COMPILE)ld
CC	= $(CROSS_COMPILE)gcc
CPP	= $(CC) -E
AR	= $(CROSS_COMPILE)ar
NM	= $(CROSS_COMPILE)nm
LDR	= $(CROSS_COMPILE)ldr
STRIP	= $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
RANLIB	= $(CROSS_COMPILE)RANLIB

ifneq (,$(findstring s,$(MAKEFLAGS)))
ARFLAGS = cr
else
ARFLAGS = crv
endif

CPPFLAGS := $(DBGFLAGS) $(OPTFLAGS) $(RELFLAGS)

CPPFLAGS =

CFLAGS := $(CPPFLAGS) -Wall -Wstrict-prototypes

CFLAGS += $(call cc-option,-fno-stack-protector)
# Some toolchains enable security related warning flags by default,
# but they don't make much sense in the u-boot world, so disable them.

INCLUDES  =  -I$(OBJTREE)/include	\
			 -I$(COMMONDIR)/include

CPPFLAGS += $(INCLUDES)
CFLAGS += $(INCLUDES)

# $(CPPFLAGS) sets -g, which causes gcc to pass a suitable -g<format>
# option to the assembler.
AFLAGS_DEBUG :=


AFLAGS := $(AFLAGS_DEBUG) -D__ASSEMBLY__ $(CPPFLAGS)

LDFLAGS += $(PLATFORM_LDFLAGS)
#########################################################################

export	HOSTCC HOSTCFLAGS HOSTLDFLAGS PEDCFLAGS HOSTSTRIP CROSS_COMPILE \
	AS LD CC CPP AR NM STRIP OBJCOPY OBJDUMP MAKE
export	CONFIG_SYS_TEXT_BASE PLATFORM_CPPFLAGS PLATFORM_RELFLAGS CPPFLAGS CFLAGS AFLAGS

#########################################################################

# Allow boards to use custom optimize flags on a per dir/file basis
BCURDIR = $(subst $(SRCTREE)/,,$(CURDIR:$(obj)%=%))
ALL_AFLAGS = $(AFLAGS) $(AFLAGS_$(BCURDIR)/$(@F)) $(AFLAGS_$(BCURDIR))
ALL_CFLAGS = $(CFLAGS) $(CFLAGS_$(BCURDIR)/$(@F)) $(CFLAGS_$(BCURDIR))
$(obj)%.s:	%.S
	@$(CPP) $(ALL_AFLAGS) -o $@ $<
	@echo " CPP     "$< ...
$(obj)%.o:	%.S
	@$(CC)  $(ALL_AFLAGS) -o $@ $< -c
	@echo " CC      "$< ...
$(obj)%.o:	%.c
	@$(CC)  $(ALL_CFLAGS) -o $@ $< -c
	@echo " CC      "$< ...
$(obj)%.i:	%.c
	@$(CPP) $(ALL_CFLAGS) -o $@ $< -c
	@echo " CPP     "$< ...
$(obj)%.s:	%.c
	@$(CC)  $(ALL_CFLAGS) -o $@ $< -c -S
	@echo " CPP     "$< ...

#########################################################################

# If the list of objects to link is empty, just create an empty built-in.o
cmd_link_o_target = $(if $(strip $1),\
		      @$(LD) $(LDFLAGS) -r -o $@ $1,\
		      rm -f $@; $(AR) rcs $@ )

#########################################################################
