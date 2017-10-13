

# Load generated board configuration
include $(OBJTREE)/include/autoconf.mk

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc-4.7
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
LDR		= $(CROSS_COMPILE)ldr
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump

##########################################################
COMPILEINC :=  -isystem $(shell dirname `$(CC)  -print-libgcc-file-name`)/include
SPLINCLUDE    := \
		-I$(SRCTREE)/include \
		-I$(SRCTREE)/arch/arm/include \
		-I$(SPLDIR)/include           \
		-I$(SRCTREE)/include/openssl

PLATFORM_RELFLAGS +=  -march=armv7-a

 COMM_FLAGS := -nostdinc  $(COMPILEINC) \
	-g  -Os   -fno-common -msoft-float -mfpu=neon  \
	-fno-builtin -ffreestanding \
	-D__KERNEL__  \
	-DCONFIG_ARM -D__ARM__ \
	-D__NEON_SIMD__  \
	-mabi=aapcs-linux \
	-mthumb-interwork \
	-fno-stack-protector \
	-Wall \
	-Wstrict-prototypes \
	-Wno-format-security \
	-Wno-format-nonliteral \
	-pipe




C_FLAGS += $(SPLINCLUDE)   $(COMM_FLAGS)
S_FLAGS += $(SPLINCLUDE)   -D__ASSEMBLY__  $(COMM_FLAGS)
#LDFLAGS += --gap-fill=0xff
###########################################################

###########################################################
PLATFORM_LIBGCC = -L $(shell dirname `$(CC) $(CFLAGS) -print-libgcc-file-name`) -lgcc
export PLATFORM_LIBGCC
###########################################################

# Allow boards to use custom optimize flags on a per dir/file basis
ALL_AFLAGS = $(AFLAGS)  $(PLATFORM_RELFLAGS) $(S_FLAGS)
ALL_CFLAGS = $(CFLAGS)  $(PLATFORM_RELFLAGS) $(C_FLAGS)
export ALL_CFLAGS ALL_AFLAGS


$(obj)%.o:	%.S
	@$(CC)  $(ALL_AFLAGS) -o $@ $< -c
	@echo " CC      "$< ...
$(obj)%.o:	%.c
	@$(CC)  $(ALL_CFLAGS) -o $@ $< -c
	@echo " CC      "$< ...

#########################################################################

# If the list of objects to link is empty, just create an empty built-in.o
cmd_link_o_target = $(if $(strip $1),\
		      @$(LD) $(LDFLAGS) -r -o $@ $1,\
		      rm -f $@; $(AR) rcs $@ )

#########################################################################
