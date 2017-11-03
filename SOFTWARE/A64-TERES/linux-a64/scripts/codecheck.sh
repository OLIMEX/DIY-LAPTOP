#!/bin/sh
do_cppcheck()
{
    if [ ! -f  $CODE_CHECK_TOOL/cppcheck/cppcheck ];then
        echo not found  $CODE_CHECK_TOOL/cppcheck/cppcheck
        return 0
    fi
    if [ "x$1" != "x" ];then
        if [ -d $1 ] || [ -f $1 ];then
            $CODE_CHECK_TOOL/cppcheck/cppcheck -j16 --force $1|egrep -w '(warn|error|warning)'
        else
            if git log --oneline -1 $1 >/dev/null 2>&1;then
                for file in `git diff --stat=150,100 ${1}^..${1}|awk '{print $1 }'`
                do
                    if [ -d $file ];then
                        $CODE_CHECK_TOOL/cppcheck/cppcheck -j16 --force $file|egrep -w '(warn|error|warning)'
                    elif [ -f $file ];then
                        file_type=${file##*.}
                        if [ "x$file_type" = "xc" ] || [ "x$file_type" = "xh" ];then
                            $CODE_CHECK_TOOL/cppcheck/cppcheck -j16 --force $file|egrep -w '(warn|error|warning)'
                        fi
                    fi
                done
            fi
        fi
    else
            $CODE_CHECK_TOOL/cppcheck/cppcheck -j16 --force ./|egrep '(warn|error|warning):'
    fi
}
do_patchcheck()
{
    if [ ! -f scripts/checkpatch.pl ];then
        echo not found scripts/checkpatch.pl
        return 0
    fi
    PATCH_CHECK="$PWD/scripts/checkpatch.pl"
    if [ "x$1" != "x" ];then
        if [ -f $1 ];then
            $PATCH_CHECK --file $1
        elif [ -d $1 ];then
            for file in `find $1 -type f -name *.[c\|h]`
            do
                    if [ -f  $file  ];then
                        echo $PATCH_CHECK -f $file
                        $PATCH_CHECK -f  $file
                    fi
            done
        else
            echo try commit $1
            if git log --oneline -1 $1 >/dev/null;then
                echo `git diff --stat=150,100 ${1}^..${1}|awk '{print $1 }'`
                for file in `git diff --stat=150,100 ${1}^..${1}|awk '{print $1 }'`
                do
                    if [ -f $file ];then
                        echo $PATCH_CHECK -f $file
                        $PATCH_CHECK -f $file
                    fi
                done
            fi
        fi
    else
        for file in `find ./ -type f -name *.[c\|h]`
        do
                if [ -f  $file  ];then
                    echo $PATCH_CHECK -f $file
                    $PATCH_CHECK -f  $file
                fi
        done
    fi
}
CURDIR=$PWD
CODE_CHECK_TOOL=../tools/codecheck
if [ $# -lt 1 ];then
    echo "usage:"
    echo "  ./script/codecheck.sh [cppcheck/patchcheck] [subdir/commit_id]"
    exit 0
fi
if [ "x$1" = "xcppcheck" ];then
    do_cppcheck $2
elif [ "x$1" = "xpatchcheck" ];then
    do_patchcheck $2
else
    echo not support command $1
fi
