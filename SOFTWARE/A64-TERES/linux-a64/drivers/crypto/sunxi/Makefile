#
# Makefile for the SUNXI SS controller
#

obj-$(CONFIG_CRYPTO_SUNXI) += ss.o

ss-y += sunxi_ss.o

ifdef CONFIG_ARCH_SUN50I
	SUNXI_SS_VER = v3
endif

ss-y += sunxi_ss_proc_comm.o $(SUNXI_SS_VER)/sunxi_ss_reg.o $(SUNXI_SS_VER)/sunxi_ss_proc.o

EXTRA_CFLAGS += -Idrivers/crypto/sunxi/$(SUNXI_SS_VER)

#ccflags-y += -DDEBUG
