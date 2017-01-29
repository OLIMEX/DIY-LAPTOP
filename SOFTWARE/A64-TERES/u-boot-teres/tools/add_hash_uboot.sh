#!/bin/bash
set -e

BUILD_FILE=u-boot.bin
BUILD_MODE=uboot

show_help()
{
    printf "\n add_hash.sh - add git log hash value into uboot,boot0,sboot,fes \n"
    echo " eg :"
    echo " ./add_hash.sh -f input_file -m file_flag "
    echo " file_flag = uboot or boot0 or sboot"
    printf "\n\n"
}


build_uboot()
{
    dd if=./${BUILD_FILE} of=./uboot_back bs=1536 count=1 status=noxfer 2> /dev/null
    dd if=./cur.log  of=./uboot_back ibs=64 conv=notrunc,sync oflag=append obs=64 count=1 status=noxfer 2> /dev/null
    dd if=./${BUILD_FILE} of=./uboot_back ibs=1600 obs=1600 conv=notrunc oflag=append skip=1 status=noxfer 2> /dev/null
    mv uboot_back ${BUILD_FILE}
}


do_common()
{
    if [ "x${BUILD_MODE}" = "xuboot" ] ; then
        echo " build_uboot "
        build_uboot
    else
        echo "build none"
    fi
}
while getopts f:m: OPTION
do
    case $OPTION in
        f)
            BUILD_FILE=$OPTARG
            ;;
        m)
            BUILD_MODE=$OPTARG
            ;;
        *)
            show_help
            exit
            ;;
    esac
done

do_common
