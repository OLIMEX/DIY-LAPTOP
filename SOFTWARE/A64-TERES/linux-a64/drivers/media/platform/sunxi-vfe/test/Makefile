CC:=~/workspace/aw1689/lichee/out/external-toolchain/gcc-aarch64/bin/aarch64-linux-gnueabihf-gcc
#LINUX_DIR = $(shell pwd)/../../../../../include/:$(shell pwd)/../../../../../arch/arm64/include
ARCH_DIR=../../../../../arch/arm64/include/
CFLAGS := -I $(LINUX_DIR)
TARGET := csi_test

.PHONY: all clean

all: $(TARGET) 

csi_test: csi_test.c 
	$(CC) $(CFLAGS) -static  $^  -o  $@

clean:
	rm -rf $(TARGET)  
