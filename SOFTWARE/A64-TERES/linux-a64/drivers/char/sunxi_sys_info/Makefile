#
# Makefile for sunxi system information driver
#

obj-$(CONFIG_SUNXI_SYS_INFO) += sysinfo.o

sysinfo-y += sunxi-sysinfo.o

ifdef CONFIG_SUNXI_SMC
sysinfo-y += sunxi-smc.o smc_call.o
endif

 #ccflags-y += -DDEBUG

