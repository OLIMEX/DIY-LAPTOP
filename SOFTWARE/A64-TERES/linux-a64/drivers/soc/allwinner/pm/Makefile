#use "-Os" flags.
use-flags := -Os
#Don't use "-O2" flags.
not-use-flags := -O2
KBUILD_CFLAGS := $(filter-out $(not-use-flags), $(KBUILD_CFLAGS))
KBUILD_CFLAGS += $(use-flags)
GCOV_PROFILE := n

subdir- += standby
obj-y	 += pm_tmp.o 

pm_tmp-y :=	mem_mapping.o               \
		mem_divlibc.o               \
		pm_of.o                     \
		mem_int.o                   \
		mem_clk.o                   \
		mem_key.o                   \
		mem_ir.o                   \
		mem_usb.o                   \
		mem_tmr.o                   \
		mem_tmstmp.o                \
		pm_debug.o                  \
		pm_errcode.o                  \
		pm_debug_secure.o           \
		mem_hwspinlock.o            \
		pm.o                        \
		aw_pm.o			    \
		aw_pwr_dm.o                 

ifneq ($(strip $(CONFIG_ARCH_SUN8IW10)),)
pm_tmp-y +=	mem_ccu.o                   \
		mem_gpio.o                  \
		mem_twi.o                   \
		mem_timing.o                \
		mem_serial.o                \
		extended_standby-sun8iw6.o  \
		pm_debug_sun8i50i.o         \
		mem_sram.o                  \
		standby.o
endif

ifneq ($(strip $(CONFIG_ARCH_SUN8IW6) $(CONFIG_ARCH_SUN8IW8) $(CONFIG_ARCH_SUN50IW1P1)),)
pm_tmp-y +=	extended_standby-sun8iw6.o
pm_tmp-y +=	pm_debug_io.o
pm_tmp-y +=	pm_debug_sun8i50i.o
endif

ifneq ($(strip $(CONFIG_ARCH_SUN8IW10)),)
$(obj)/mem_divlibc.o              : $(obj)/pm_tmp_xx.o
$(obj)/mem_clk.o                  : $(obj)/pm_tmp_xx.o
$(obj)/mem_gpio.o                 : $(obj)/pm_tmp_xx.o
$(obj)/mem_serial.o               : $(obj)/pm_tmp_xx.o
$(obj)/mem_timing.o               : $(obj)/pm_tmp_xx.o
$(obj)/mem_int.o                  : $(obj)/pm_tmp_xx.o

$(obj)/pm_tmp_xx.o		: $(obj)/standby/standby.code FORCE
$(obj)/standby.o		: $(obj)/standby/standby.code FORCE

$(obj)/standby/standby.code: FORCE
	$(Q)$(MAKE) $(build)=$(obj)/standby
endif
	
