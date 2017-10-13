#!/bin/sh
#
# Simple script to create a disk image which boots to U-Boot on Pine64.
#
# This script uses boot0 binary blob (as extracted from the Pine64 Android
# image) together with a correctly prefixed U-Boot and A DOS partition table
# to create a bootable SDcard image for the Pine64. If a Kernel and DTB is
# found in ../kernel, it is added as well.
#
# U-Boot tree:
# - https://github.com/longsleep/u-boot-pine64/tree/pine64-hacks
#
# Build the U-Boot tree and assemble it with ATF, SCP and FEX and put the
# resulting u-boot-with-dtb.bin file into the ../build directory. The
# u-boot-postprocess script provides an easy way to do all that.
#

set -e

out="$1"
disk_size="$2"
kernel_tarball="$3"
model="$4"
BOOTLOGO="../blobs/bootlogo.bmp"
BATTERY="../blobs/bat"
BOOTLOGO_TARGET="bootlogo.bmp"
BATTERY_TARGET="bat"

if [ -z "$out" ]; then
	echo "Usage: $0 <image-file.img> [disk size in MiB] [<kernel-tarball>] [teres]"
	exit 1
fi

if [ "$(id -u)" -ne "0" ]; then
	echo "This script requires root."
	exit 1
fi

if [ -z "$disk_size" ]; then
	disk_size=100 #MiB
fi

if [ "$disk_size" -lt 60 ]; then
	echo "Disk size must be at least 60 MiB"
	exit 2
fi

#if [ -z "$model" = "" ]; then
    model="teres"
#fi

echo "Creating $model image $out of size $disk_size MiB ..."

suffix=""
if [ -n "$model" ]; then
    suffix="-$model"
fi
#u-boot-sun50iw1p1-secure-with-teres-dtb.bin
boot0="../build/boot0_teres.bin"
uboot="../build/u-boot-sun50iw1p1-secure-with-teres-dtb.bin"
kernel="../build"

temp=$(mktemp -d)

cleanup() {
	if [ -d "$temp" ]; then
		rm -rf "$temp"
	fi
}
trap cleanup EXIT

if [ -n "$kernel_tarball" -a "$kernel_tarball" != "-" ]; then
	echo "Using Kernel from $kernel_tarball ..."
	tar -C $temp -xJf "$kernel_tarball"
	kernel=$temp/boot
	mv $temp/boot/uEnv.txt.in $temp/boot/uEnv.txt
fi

boot0_position=8      # KiB
uboot_position=19096  # KiB
part_position=20480   # KiB
boot_size=50          # MiB

# Create beginning of disk
dd if=/dev/zero bs=1M count=$((part_position/1024)) of="$out"
dd if="$boot0" conv=notrunc bs=1k seek=$boot0_position of="$out"
dd if="$uboot" conv=notrunc bs=1k seek=$uboot_position of="$out"

# Create boot file system (VFAT)
dd if=/dev/zero bs=1M count=${boot_size} of=${out}1
mkfs.vfat -n BOOT ${out}1

# Add boot support if there
if [ -e "${kernel}/a64/Image" -a -e "${kernel}/a64/a64-olinuxino.dtb" ]; then
	mcopy -sm -i ${out}1 ${kernel}/a64 ::
	mcopy -m -i ${out}1 ${kernel}/initrd.img :: || true
	mcopy -m -i ${out}1 ${kernel}/uEnv.txt :: || true
	mcopy -m -i ${out}1 ${BOOTLOGO} :: || true 
	mcopy -sm -i ${out}1 ${BATTERY} ::
fi
dd if=${out}1 conv=notrunc oflag=append bs=1M seek=$((part_position/1024)) of="$out"
rm -f ${out}1

# Create additional ext4 file system for rootfs
dd if=/dev/zero bs=1M count=$((disk_size-boot_size-part_position/1024)) of=${out}2
mkfs.ext4 -F -b 4096 -E stride=2,stripe-width=1024 -L rootfs ${out}2
dd if=${out}2 conv=notrunc oflag=append bs=1M seek=$((part_position/1024+boot_size)) of="$out"
rm -f ${out}2

# Add partition table
cat <<EOF | fdisk "$out"
o
n
p
1
$((part_position*2))
+${boot_size}M
t
c
n
p
2
$((part_position*2 + boot_size*1024*2))

t
2
83
w
EOF

sync

echo "Done - image created: $out"
