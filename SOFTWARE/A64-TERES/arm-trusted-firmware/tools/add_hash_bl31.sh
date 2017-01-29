#!/bin/bash
set -e

BUILD_FILE=bl31.bin
BUILD_MODE=bl31

show_help()
{
    printf "\n add_hash.sh - add git log hash value into uboot,boot0,sboot,fes \n"
    echo " eg :"
    echo " ./add_hash.sh -f input_file -m file_flag "
    echo " file_flag = uboot or boot0 or sboot"
    printf "\n\n"
}


build_bl31()
{
    dd if=./${BUILD_FILE} of=./bl31_back bs=48 count=1 status=noxfer 2> /dev/null
    dd if=./cur.log  of=./bl31_back ibs=64 conv=notrunc,sync oflag=append obs=64 count=1 status=noxfer 2> /dev/null
    dd if=./${BUILD_FILE} of=./bl31_back ibs=112 obs=112 conv=notrunc oflag=append skip=1 status=noxfer 2> /dev/null
    mv bl31_back ${BUILD_FILE}
}


do_common()
{
    if [ "x${BUILD_MODE}" = "xbl31" ] ; then
        echo " add commit info for bl31 "
        build_bl31
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
