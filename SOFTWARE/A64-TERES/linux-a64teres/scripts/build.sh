#!/bin/bash
set -e

#Setup common variables
export ARCH=arm
if [ -n "`echo ${LICHEE_CHIP} | grep "sun5[0-9]i"`" ]; then
    export ARCH=arm64
fi
export CROSS_COMPILE=${ARCH}-linux-gnueabi-
if [ -n "${LICHEE_TOOLCHAIN_PATH}" \
	-a -d "${LICHEE_TOOLCHAIN_PATH}" ]; then
    GCC=$(find ${LICHEE_TOOLCHAIN_PATH} -perm /a+x -a -regex '.*-gcc');
    export CROSS_COMPILE="${GCC%-*}-";
elif [ -n "${LICHEE_CROSS_COMPILER}" ]; then
    export CROSS_COMPILE="${LICHEE_CROSS_COMPILER}-"
fi

export AS=${CROSS_COMPILE}as
export LD=${CROSS_COMPILE}ld
export CC=${CROSS_COMPILE}gcc
export AR=${CROSS_COMPILE}ar
export NM=${CROSS_COMPILE}nm
export STRIP=${CROSS_COMPILE}strip
export OBJCOPY=${CROSS_COMPILE}objcopy
export OBJDUMP=${CROSS_COMPILE}objdump
export LOCALVERSION=""
export MKBOOTIMG=${LICHEE_TOOLS_DIR}/pack/pctools/linux/android/mkbootimg

KERNEL_VERSION=`make -s kernelversion -C ./`
LICHEE_KDIR=`pwd`
LICHEE_MOD_DIR=${LICHEE_KDIR}/output/lib/modules/${KERNEL_VERSION}
export LICHEE_KDIR

update_kern_ver()
{
    if [ -r include/generated/utsrelease.h ]; then
        KERNEL_VERSION=`cat include/generated/utsrelease.h |awk -F\" '{print $2}'`
    fi
    LICHEE_MOD_DIR=${LICHEE_KDIR}/output/lib/modules/${KERNEL_VERSION}
}

show_help()
{
    printf "
    Build script for Lichee platform

    Invalid Options:

    help         - show this help
    kernel       - build kernel
    modules      - build kernel module in modules dir
    clean        - clean kernel and modules

    "
}

NAND_ROOT=${LICHEE_KDIR}/modules/nand

build_nand_lib()
{
	echo "build nand library ${NAND_ROOT}/${LICHEE_CHIP}/lib"
	if [ -d ${NAND_ROOT}/${LICHEE_CHIP}/lib ]; then
		echo "build nand library now"
	make -C modules/nand/${LICHEE_CHIP}/lib clean	2>/dev/null
	make -C modules/nand/${LICHEE_CHIP}/lib lib install
	else
		echo "build nand with existing library"
	fi
}

build_gpu_sun8i()
{
    echo "$0"
}

build_gpu_sun8iw6()
{
    echo "$0"
}

build_gpu_sun9iw1()
{
    echo "$0"
}

build_gpu()
{
	if [ "x${LICHEE_PLATFORM}" = "xandroid" ] ; then
	{
        unset OUT
        unset TOP
	}
	fi
	
	make -C modules/gpu LICHEE_MOD_DIR=${LICHEE_MOD_DIR} LICHEE_KDIR=${LICHEE_KDIR}
}

clean_gpu_sun9iw1()
{
    echo "$0"
}

clean_gpu_sun8iw6()
{
    echo "$0"
}

clean_gpu()
{
    echo "Nothing to do."
}


build_kernel()
{
    echo "Building kernel"

    cd ${LICHEE_KDIR}

    rm -rf output/
    echo "${LICHEE_MOD_DIR}"
    mkdir -p ${LICHEE_MOD_DIR}

    # We need to copy rootfs files to compile kernel for linux image
	echo "lichee_chip = $LICHEE_CHIP"
	if [ "${LICHEE_CHIP}" = "sun8iw10p1" ] || [ "${LICHEE_CHIP}" = "sun8iw11p1" ]; then
		echo "cp rootfs_32bit.cpio.gz"
		cp -f rootfs_32bit.cpio.gz output/rootfs.cpio.gz
	else
		cp -f rootfs.cpio.gz output/
	fi

    if [ ! -f .config ] ; then
        printf "\n\033[0;31;1mUsing default config ${LICHEE_KERN_DEFCONF} ...\033[0m\n\n"
        cp arch/${ARCH}/configs/${LICHEE_KERN_DEFCONF} .config
    fi

    if [ "x$SUNXI_CHECK" = "x1" ];then
        SUNXI_SPARSE_CHECK=1
        SUNXI_SMATCH_CHECK=1
        SUNXI_STACK_CHECK=1
    fi
    if [ "x$SUNXI_SPARSE_CHECK" = "x" ] && [ "x$SUNXI_SMATCH_CHECK" = "x" ];then
        make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} -j${LICHEE_JLEVEL} all modules
    else
        if [ "x$SUNXI_SPARSE_CHECK" = "x1" ] && [ -f ../tools/codecheck/sparse/sparse ];then
            echo "\n\033[0;31;1mBuilding Round for sparse check ${KERNEL_CFG}...\033[0m\n\n"
            make clean
            make CHECK="../tools/codecheck/sparse/sparse" ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} -j${LICHEE_JLEVEL} C=2 all modules 2>&1|tee output/build_sparse.txt
            cat output/build_sparse.txt|egrep -w '(warn|error|warning)' >output/warn_sparse.txt
        fi
        if [ "x$SUNXI_SMATCH_CHECK" = "x1" ]&& [ -f ../tools/codecheck/smatch/smatch ];then
            echo "\n\033[0;31;1mBuilding Round for smatch check ${KERNEL_CFG}...\033[0m\n\n"
            make clean
            make CHECK="../tools/codecheck/smatch/smatch --full-path --no-data -p=kkernel" ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} -j${LICHEE_JLEVEL} C=2 all modules 2>&1|tee output/build_smatch.txt
            cat output/build_smatch.txt|egrep -w '(warn|error|warning)' >output/warn_smatch.txt
        fi
    fi
    if [ "x$SUNXI_STACK_CHECK" = "x1" ];then
        make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} -j${LICHEE_JLEVEL} checkstack 2>&1|tee output/warn_stack.txt
    fi
    update_kern_ver

    #The Image is origin binary from vmlinux.
    if [ -f arch/${ARCH}/boot/Image ]; then
        cp -vf arch/${ARCH}/boot/Image output/bImage
    fi

    if [ -f arch/${ARCH}/boot/zImage ] || [ -f arch/${ARCH}/boot/uImage ]; then
        cp -vf arch/${ARCH}/boot/[zu]Image output/
    fi

    if [ -f arch/${ARCH}/boot/Image.gz ]; then
        cp -vf arch/${ARCH}/boot/Image.gz output/
    fi

    cp .config output/

    tar -jcf output/vmlinux.tar.bz2 vmlinux
    for file in $(find drivers sound crypto block fs security net -name "*.ko"); do
        cp $file ${LICHEE_MOD_DIR}
    done
    cp -f Module.symvers ${LICHEE_MOD_DIR}

}

build_modules()
{

	echo "Building modules"
	
	if [ ! -f include/generated/utsrelease.h ]; then
		printf "Please build kernel first!\n"
		exit 1
	fi
	
	update_kern_ver
	build_nand_lib
	make -C modules/nand LICHEE_MOD_DIR=${LICHEE_MOD_DIR} LICHEE_KDIR=${LICHEE_KDIR} \
		CONFIG_CHIP_ID=${CONFIG_CHIP_ID} install
       make -C modules/aw_schw LICHEE_MOD_DIR=${LICHEE_MOD_DIR} LICHEE_KDIR=${LICHEE_KDIR} \
               CONFIG_CHIP_ID=${CONFIG_CHIP_ID} install
	
	echo "lichee_chip = $LICHEE_CHIP"
	if [ "${LICHEE_CHIP}" = "sun8iw10p1" ] || [ "${LICHEE_CHIP}" = "sun8iw11p1" ]; then
		echo "no gpu modules."	
	else
		build_gpu
	fi
}

regen_rootfs_cpio()
{
    echo "regenerate rootfs cpio"

	cd ${LICHEE_KDIR}/output
	if [ -x "../scripts/build_rootfs.sh" ]; then
		../scripts/build_rootfs.sh e ./rootfs.cpio.gz > /dev/null
	else
		echo "No such file: scripts/build_rootfs.sh"
		exit 1
	fi

	mkdir -p ./skel/lib/modules/${KERNEL_VERSION}

	if [ -e ${LICHEE_MOD_DIR}/nand.ko ]; then
		cp ${LICHEE_MOD_DIR}/nand.ko ./skel/lib/modules/${KERNEL_VERSION}
		if [ $? -ne 0 ]; then
			echo "copy nand module error: $?"
			exit 1
		fi
	fi
	ko_file=`find ./skel/lib/modules/$KERNEL_VERSION/ -name *.ko`
	if [ ! -z "$ko_file" ]; then
	        ${STRIP} -d ./skel/lib/modules/$KERNEL_VERSION/*.ko
	fi
	rm -f rootfs.cpio.gz
	../scripts/build_rootfs.sh c rootfs.cpio.gz > /dev/null
	rm -rf skel
	cd - > /dev/null
}

build_ramfs()
{
	local bss_sz=0;
	local CHIP="";

	local BIMAGE="output/bImage";
	local RAMDISK="output/rootfs.cpio.gz";
	local BASE="";
	local RAMDISK_OFFSET="";
	local KERNEL_OFFSET="";

	# update rootfs.cpio.gz with new module files
	regen_rootfs_cpio

	CHIP=`echo ${LICHEE_CHIP} | sed -e 's/.*\(sun[0-9x]*i\).*/\1/g'`;

	if [ "${CHIP}" = "sun9i" ]; then
		BASE="0x20000000";
		KERNEL_OFFSET="0x8000";
	elif [ "${CHIP}" = "sun50i" ]; then
		BASE="0x41000000";
		KERNEL_OFFSET="0x80000";
	else
		BASE="0x40000000";
		KERNEL_OFFSET="0x8000";
	fi

	if [ -f vmlinux ]; then
		bss_sz=`${CROSS_COMPILE}readelf -S vmlinux | \
			awk  '/\.bss/ {print strtonum("0x"$5)+strtonum("0x"$6)}'`;
	fi
	#bss_sz=`du -sb ${BIMAGE} | awk '{printf("%u", $1)}'`;
	#
	# If the size of bImage larger than 16M, will offset 0x02000000
	#
	if [ ${bss_sz} -gt $((16#1000000)) ]; then
		RAMDISK_OFFSET="0x02000000";
	else
		RAMDISK_OFFSET="0x01000000";
	fi

	${MKBOOTIMG} --kernel ${BIMAGE} \
		--ramdisk ${RAMDISK} \
		--board ${CHIP} \
		--base ${BASE} \
		--kernel_offset ${KERNEL_OFFSET} \
		--ramdisk_offset ${RAMDISK_OFFSET} \
		-o output/boot.img
	
	# If uboot use *bootm* to boot kernel, we should use uImage.
	echo build_ramfs
	echo "Copy boot.img to output directory ..."
	cp output/boot.img ${LICHEE_PLAT_OUT}
	cp output/vmlinux.tar.bz2 ${LICHEE_PLAT_OUT}

	if [ ! -f output/arisc ]; then
		echo "arisc" > output/arisc
	fi
	cp output/arisc    ${LICHEE_PLAT_OUT}

	if [ ! -f arch/${ARCH}/boot/dts/${LICHEE_CHIP}-${LICHEE_BOARD}.dtb ]; then
		echo "sunxi.dtb" > output/sunxi.dtb
	else
		cp arch/${ARCH}/boot/dts/${LICHEE_CHIP}-${LICHEE_BOARD}.dtb output/sunxi.dtb
	fi
	cp output/sunxi.dtb    ${LICHEE_PLAT_OUT}
}

gen_output()
{
    if [ "x${LICHEE_PLATFORM}" = "xandroid" ] ; then
        echo "Copy modules to target ..."
        rm -rf ${LICHEE_PLAT_OUT}/lib
        cp -rf ${LICHEE_KDIR}/output/* ${LICHEE_PLAT_OUT}
        return
    fi
    echo "$0"
}

clean_kernel()
{
	echo "Cleaning kernel ..."
	make ARCH=${ARCH} clean
	rm -rf output/*
	echo
}

clean_modules()
{
    echo "Cleaning modules ..."
    clean_gpu
	echo
}

#####################################################################
#
#                      Main Runtine
#
#####################################################################

#LICHEE_ROOT=`(cd ${LICHEE_KDIR}/..; pwd)`
#export PATH=${LICHEE_ROOT}/buildroot/output/external-toolchain/bin:${LICHEE_ROOT}/tools/pack/pctools/linux/android:$PATH
#if [ x$2 = x ];then
#	echo Error! you show pass chip name as param 2
#	exit 1
#else
#	chip_name=$2
#	platform_name=${chip_name:0:5}
#fi

case "$1" in
    kernel)
        build_kernel
        ;;
    modules)
        build_modules
        ;;
    clean)
        clean_kernel
        clean_modules
        ;;
    *)
        build_kernel
        build_modules
        build_ramfs
        gen_output
        echo -e "\n\033[0;31;1m${LICHEE_CHIP} compile Kernel successful\033[0m\n\n"
        ;;
esac

