#makefile for resume.bin

always  := resumes.code
targets := resumes.elf

#use "-Os" flags.
#Don't use "-O2" flags.
KBUILD_CFLAGS := -g -c -nostdlib -march=armv7-a -marm -fno-unwind-tables -fno-asynchronous-unwind-tables -mlittle-endian -O2 --min_array_alignment=4 --no_unaligned_access

#Include the cur dir.
KBUILD_CPPFLAGS += -I.

LD_FLAGS = -static
LIBS =

INCLUDE   = -I. \
       -I$(KDIR)/include \
       -I$(KDIR)/drivers/soc/allwinner/standby/include

resumes-y :=   resumes.o        \
               resume_head.o

resumes-y := $(addprefix $(obj)/,$(resumes-y))

$(obj)/resumes.code: $(obj)/resumes.bin
	$(Q)$(obj)/gen_check_code $(obj)/resumes.bin $(obj)/resumes.code

$(obj)/resumes.bin: $(obj)/resumes.elf FORCE
	$(Q)$(CROSS_COMPILE)objcopy -O binary $(obj)/resumes.elf $(obj)/resumes.bin

ifneq ($(strip $(CONFIG_ARCH_SUN8I)),)
$(obj)/resumes.elf: $(obj)/sun8i_brom_scatter.scat $(resumes-y)
	$(Q)$(CROSS_COMPILE)ld -T $(obj)/sun8i_brom_scatter.scat  -EL  $(resumes-y) -o $(obj)/resumes.elf  -Map $(obj)/resumes.map
	$(Q)$(CROSS_COMPILE)objdump -D $(obj)/resumes.elf > $(obj)/resumes.lst
endif

clean-files += resumes.code resumes.elf
