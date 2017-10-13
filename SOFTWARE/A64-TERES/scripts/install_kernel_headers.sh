#!/bin/sh

set -e

DEST="$1"

if [ -z "$DEST" ]; then
	echo "Usage: $0 <destination-root> [linux-folder]"
	exit 1
fi

LINUX="../linux-a64"

if [ -n "$2" ]; then
	LINUX="$2"
fi

echo "Using Linux from $LINUX ..."

VERSION=$(strings $LINUX/arch/arm64/boot/Image |grep "Linux version"|awk '{print $3}')
echo "Kernel build version $VERSION ..."
if [ -z "$VERSION" ]; then
	echo "Failed to get build version, correct <linux-folder>?"
	exit 1
fi

LINUX_ARCH=arm64
CROSS_COMPILE=aarch64-linux-gnu-

cd $LINUX

TARGET="$DEST/usr/src/linux-headers-$VERSION"
mkdir -p "$TARGET"
cp -a Makefile "$TARGET"
mkdir -p "$TARGET/arch/$LINUX_ARCH"
cp -a arch/$LINUX_ARCH/Makefile "$TARGET/arch/$LINUX_ARCH"
cp -a Module.symvers "$TARGET"

# Install Kernel headers
make ARCH=$LINUX_ARCH CROSS_COMPILE=$CROSS_COMPILE headers_install INSTALL_HDR_PATH="$TARGET"

tar cfh - include | (cd "$TARGET"; umask 000; tar xsf -)
tar cfh - scripts | (cd "$TARGET"; umask 000; tar xsf -)
find . -path './scripts/*'   -prune -o -path './Documentation/*' -prune -o  \
               -path './debian/*'    -prune -o -type f                      \
               \( -name Makefile -o  -name 'Kconfig*' \) -print  |          \
                  cpio -pd --preserve-modification-time "$TARGET";

# Clean compile host executables and replace some with target arch.
find "$TARGET/scripts" -type f | while read i; do if file -b $i | egrep -q "^ELF.*executable"; then rm "$i"; fi; done
(cd scripts && ${CROSS_COMPILE}gcc kallsyms.c -o "$TARGET/scripts/kallsyms")
(cd scripts && ${CROSS_COMPILE}gcc pnmtologo.c -o "$TARGET/scripts/pnmtologo")
(cd scripts && ${CROSS_COMPILE}gcc conmakehash.c -o "$TARGET/scripts/conmakehash")
(cd scripts && ${CROSS_COMPILE}gcc bin2c.c -o "$TARGET/scripts/bin2c")
(cd scripts && ${CROSS_COMPILE}gcc recordmcount.c -o "$TARGET/scripts/recordmcount")
(cd scripts && ${CROSS_COMPILE}gcc -I../tools/include sortextable.c -o "$TARGET/scripts/sortextable")
(cd scripts && ${CROSS_COMPILE}gcc unifdef.c -o "$TARGET/scripts/unifdef")
(cd scripts/basic && ${CROSS_COMPILE}gcc fixdep.c -o "$TARGET/scripts/basic/fixdep")
(cd scripts/mod && ${CROSS_COMPILE}gcc modpost.c file2alias.c sumversion.c -o "$TARGET/scripts/mod/modpost")
(cd scripts/mod && ${CROSS_COMPILE}gcc mk_elfconfig.c -o "$TARGET/scripts/mod/mk_elfconfig")
(cd scripts/genksyms && ${CROSS_COMPILE}gcc genksyms.c parse.tab.c lex.lex.c -o "$TARGET/scripts/genksyms/genksyms")

find arch/$LINUX_ARCH/include   \
               -print | cpio -pdL --preserve-modification-time "$TARGET";

mkdir -p "$TARGET/arch/um"
cp -a arch/um/Makefile* "$TARGET/arch/um/"
mkdir -p "$TARGET/arch/$LINUX_ARCH/kernel"
cp -a arch/$LINUX_ARCH/kernel/asm-offsets.s "$TARGET/arch/$LINUX_ARCH/kernel"
rm -f "$TARGET/include/linux/version.h"
cp -a .config "$TARGET/"

echo "Done - installed Kernel headers to $DEST"
