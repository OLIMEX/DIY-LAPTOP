#makefile for standby.bin
always  := standby.code
targets := standby.elf

GCOV_PROFILE=no

#use "-Os" flags.
#Don't use "-O2" flags.
KBUILD_CFLAGS := -g -c -nostdlib -march=armv7-a -D__LINUX_ARM_ARCH__=7 -marm -fno-unwind-tables -fno-asynchronous-unwind-tables -mlittle-endian -O2 -mno-unaligned-access

#Include the cur dir.
KBUILD_CPPFLAGS += -I.

LD_FLAGS = -static
LIBS =

INCLUDE   = -I. \
       -I$(KDIR)/include \
       -I$(KDIR)/drivers/soc/allwinner/standby/include \
       -I$(KDIR)/drivers/soc/allwinner/pm

subdir- += brom
$(obj)/brom.o : $(obj)/brom/resumes.code FORCE
obj-y   += brom.o

standby-y := common.o                    \
             standby_twi.o               \
             power/axp152_power.o        \
             power/axp192_power.o        \
             power/standby_power.o       \
             standby_clock.o             \
             standby_debug.o             \
             standby_divlib.o            \
             ./../mem_divlibc.o          \
             ./../mem_clk.o              \
             ./../mem_timing.o           \
             ./../mem_serial.o           \
             ./../mem_printk.o           \
             main.o

standby-y := $(addprefix $(obj)/,$(standby-y))

$(obj)/standby.code: $(obj)/standby.elf FORCE
	$(Q)$(CROSS_COMPILE)objcopy -O binary $(obj)/standby.elf $(obj)/standby.code
#$(call if_changed,objcopy)
	rm -rf *.o $(obj)/./*.o

$(obj)/standby.elf: $(obj)/standby.xn $(standby-y)
	$(Q)$(CROSS_COMPILE)ld -T $(obj)/standby.xn $(LD_FLAGS) $(LIBS) -EL  $(standby-y) -o $(obj)/standby.elf -Map $(obj)/standby.map
	$(Q)$(CROSS_COMPILE)objdump -D $(obj)/standby.elf > $(obj)/standby.lst
#$(call if_changed,ld)

$(obj)/brom/resumes.code: FORCE
	$(Q)$(MAKE) $(build)=$(obj)/brom

clean-files += standby.code standby.elf
