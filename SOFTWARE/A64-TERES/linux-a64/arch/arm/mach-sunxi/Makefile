obj-$(CONFIG_ARCH_SUNXI) += sunxi.o

ifeq ($(CONFIG_SMP), y)
obj-y += headsmp.o platsmp.o
obj-$(CONFIG_HOTPLUG_CPU) += hotplug.o
endif
