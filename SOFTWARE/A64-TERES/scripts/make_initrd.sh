#!/bin/sh
#
# Simple script to create a small busybox based initrd. It requires a compiled
# busybox static binary. You can also use any other prebuilt initrd.
#
# Please note that some prebuilt initrds might be inappropriate, for example,
# the ones from the Debian Installer will start an install process, so don't
# use those.
#
# Run this script with fakeroot or as root.

set -e

if [ "$(id -u)" -ne "0" ]; then
	exec fakeroot $0 $@
fi

BUSYBOX="../busybox"

TEMP=$(mktemp -d)
TEMPFILE=$(mktemp)

mkdir -p $TEMP/bin
cp -va $BUSYBOX/busybox $TEMP/bin

cd $TEMP
mkdir dev proc sys tmp sbin
mknod dev/console c 5 1
cat > $TEMP/init <<'EOF'
#!/bin/busybox sh

# Install busybox
/bin/busybox --install -s

# Mount the /proc and /sys filesystems.
mount -t proc none /proc
mount -t sysfs none /sys
mount -t devtmpfs none /dev

cmdline() {
        local value
        value=" $(cat /proc/cmdline) "
        value="${value##* $1=}"
        value="${value%% *}"
        [ "$value" != "" ] && echo "$value"
}

realboot() {
        echo "Rootfs: $1";
        # Mount real root.
        mkdir -p /mnt/root
        mount -o rw "$1" /mnt/root

        if [ -x /mnt/root/sbin/init -o -h /mnt/root/sbin/init ]; then
                # Cleanup.
                umount /proc
                umount /sys
                umount /dev

                # Boot the real system.
                exec switch_root /mnt/root /sbin/init
        else
                umount /mnt/root
        fi
}

runshell() {
        echo "Dropping to a shell."
        echo
        setsid cttyhack /bin/sh
}

find_parition_by_value() {
        echo `blkid | tr -d '"' | grep "$1" | cut -d ':' -f 1 | head -n 1`
}

boot() {
        echo "Kernel params: `cat /proc/cmdline`"
        local i=5
        local kernel_root_param=$(cmdline root)

        while [ "$i" -ge 1 ]; do
                echo "Waiting for root system $kernel_root_param, countdown : $i";
                local root=`find_parition_by_value $kernel_root_param`
                if [ -e "$root" ]; then
                        realboot $root;
                fi;

                i=$(( $i - 1 ));
                sleep 5;
        done;

        # Default rootfs - sd partition 2
        realboot /dev/mmcblk0p2;
        runshell;
}
boot;
EOF
chmod 755 $TEMP/init

find . | cpio -H newc -o > $TEMPFILE

cd -

cat $TEMPFILE | gzip >initrd.gz

rm $TEMPFILE
rm -rf $TEMP
sync
