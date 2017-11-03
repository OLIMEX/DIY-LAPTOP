ifneq ($(LICHEE_KDIR),)
KERNEL_DIR = $(LICHEE_KDIR)
GPU_BUILD_TYPE = release
endif

include $(KERNEL_DIR)/.config

##############################################################
##################     Set SYSTEM_TYPE      ##################
##############################################################
ifneq ($(LICHEE_PLATFORM),)
SYSTEM_TYPE = $(LICHEE_PLATFORM)
endif

##############################################################
##################   Set ARCH and CROSS_COMPILE  #############
##############################################################
ifdef CONFIG_ARM
export ARCH ?= arm
export CROSS_COMPILE ?= arm-linux-gnueabi-
endif
	
ifdef CONFIG_ARM64
export ARCH ?= arm64
export CROSS_COMPILE ?= aarch64-linux-gnu-
endif

##############################################################
##################     Set GPU_TYPE         ##################
##############################################################
ifneq ($(LICHEE_PLATFORM),)
#####################################
#For mali400 Platform
#####################################
ifdef CONFIG_ARCH_SUN8IW3P1
GPU_TYPE = mali400
endif

ifdef CONFIG_ARCH_SUN8IW5P1
GPU_TYPE = mali400
endif

ifdef CONFIG_ARCH_SUN8IW7P1
GPU_TYPE = mali400
endif

ifdef CONFIG_ARCH_SUN50IW1P1
GPU_TYPE = mali400
endif

#####################################
#For sgx544 Platform
#####################################
ifdef CONFIG_ARCH_SUN8IW6P1
GPU_TYPE = sgx544
endif

endif

##############################################################
#############  Set GPU_BUILD_DIR and GPU_KO_NAME ##############
##############################################################
#####################################
#For mali400 Platform
#####################################
ifeq ($(GPU_TYPE),mali400)
GPU_BUILD_DIR = mali400/kernel_mode/driver/src/devicedrv/mali
GPU_KO_NAME   = $(GPU_BUILD_DIR)/mali.ko
ifdef CONFIG_OF
export USING_DT = 1
endif
export USING_UMP=0
export KDIR=$(KERNEL_DIR)
endif

#####################################
#For sgx544 Platform
#####################################
ifeq ($(GPU_TYPE),sgx544)
GPU_BUILD_DIR = sgx544/$(SYSTEM_TYPE)/kernel_mode/eurasia_km/eurasiacon/build/linux2/sunxi_android
GPU_KO_NAME   = sgx544/$(SYSTEM_TYPE)/kernel_mode/eurasia_km/eurasiacon/binary2_sunxi_$(SYSTEM_TYPE)_$(GPU_BUILD_TYPE)/target/pvrsrvkm.ko
GPU_CLEAN_DIR = sgx544/$(SYSTEM_TYPE)/kernel_mode/eurasia_km/eurasiacon/binary2_sunxi_$(SYSTEM_TYPE)_$(GPU_BUILD_TYPE)
ifdef CONFIG_OF
export USING_DT = 1
endif
export KERNELDIR=$(KERNEL_DIR)
endif

##############################################################

all: build install

build:
	$(MAKE) -j -C $(GPU_BUILD_DIR) BUILD=$(GPU_BUILD_TYPE) KERNELDIR=$(KERNEL_DIR)

install:
	if [ $(LICHEE_MOD_DIR) ]; then \
		cp -u $(GPU_KO_NAME) $(LICHEE_MOD_DIR); \
	fi
	if [ -z $(LICHEE_MOD_DIR) ]; then \
		cp -u $(GPU_KO_NAME) $(GPU_TARGET_OUT_DIR)/module; \
	fi

clean:
	$(MAKE) -C $(GPU_BUILD_DIR) clean
	if [ $(GPU_CLEAN_DIR) ]; then \
		rm -rf $(GPU_CLEAN_DIR); \
	fi
