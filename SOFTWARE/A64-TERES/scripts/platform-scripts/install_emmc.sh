#!/bin/bash 
# Init
FILE="/tmp/out.$$"
GREP="/bin/grep"
#....
# Make sure only root can run our script


if [ "$(id -u)" -ne "0" ]; then
	echo "This script requires root."
	exit 1
fi
#get boot device 
BOOT_PARTITION=`df -P / | tail -n 1 | awk '/.*/ { print substr($1,0,13); }'`
EMMC_DEVICE="/dev/"`cat /proc/partitions | awk '{if ($4 ~ "boot")  print substr($4,0,8); }' | head -1`
BLOBS_LOCATION="/blobs"
boot0_position=8      # KiB
uboot_position=19096  # KiB
part_position=20480   # KiB
boot_size=50          # MiB
dir="/usr/local/sbin"
tmp_dir="/tmp"
boot0=${dir}/"blobs/boot0_teres.bin"
uboot=${dir}/"blobs/u-boot_emmc.bin"
EMMC_DIALOG_OPTION=$EMMC_DEVICE"(eMMC)"
if [ "$BOOT_DEVICE" == "/dev/mmcblk0" ]; then
  echo "You should boot from SD card"
  exit 1
fi

function repartition_emmc {
exec 3>&1;

ans=$(dialog --backtitle "Device" \
  --radiolist "Select device:" 10 80 4 \
	$EMMC_DEVICE $EMMC_DIALOG_OPTION on \
        $BOOT_PARTITION $BOOT_PARTITION off \
2>&1 1>&3 )
exec 3>&-;

echo $ans

exitstatus=$?

if [ $exitstatus = 1 ] || [ $exitstatus = 255 ] ; then
    echo "Exit"
    exit;
fi

umount ${ans}* 2>&1

dd if=/dev/zero bs=1M count=$((part_position/1024)) of="$ans"
echo "Extract u-boot and boot0 drom boot partition"
dd if=$BOOT_PARTITION of=${tmp_dir}/boot0.bin bs=8k skip=1 count=4
dd if=$BOOT_PARTITION of=${tmp_dir}/u-boot.bin bs=8k skip=2387 count=120
dd if=${tmp_dir}/boot0.bin conv=notrunc bs=1k seek=$boot0_position of="$ans"
dd if=${tmp_dir}/u-boot.bin conv=notrunc bs=1k seek=$uboot_position of="$ans"
rm ${tmp_dir}/boot0.bin ${tmp_dir}/u-boot.bin
cat <<EOF | fdisk "$ans"
o
n
p
1
40960
+50M
t
c
n
p
2
143360

t
2
83
w
EOF
mkfs.vfat -n BOOT ${ans}p1 2>&1
mkfs.ext4 -q -F -b 4096 -E stride=2,stripe-width=1024 -L rootfs ${ans}p2 2>&1
sync
}

function copy_files() {
umount ${ans}*
mount ${ans}p1 /mnt
rsync -aAXv  --info=progress2  --no-relative  /boot/* /mnt
umount /mnt/
mount ${ans}p2 /mnt
rsync -aAXv  --info=progress2 --exclude={"/dev/*","/proc/*","/sys/*","/tmp/*","/run/*","/mnt/*","/media/*","/lost+found"} / /mnt \

cat > /mnt/etc/fstab <<EOF
# <file system>	<dir>	<type>	<options>			<dump>	<pass>
/dev/mmcblk0p1	/boot	vfat	defaults			0		2
/dev/mmcblk0p2	/		ext4	defaults,noatime	0		1
EOF

sync

}

dialog --title "Confirmation" \
--backtitle "eMMC Installer" \
--yesno "This script will reformat yuor internal eMMC Device. Are you sure ?" 7 60

response=$?


case "$response" in

0)  repartition_emmc
    copy_files
    ;;
1)  echo  "Exit"
    exit
    ;;
255)  echo  "Esc detected"
    exit
    ;;
*) echo "exit"
   ;;
esac




# ...
