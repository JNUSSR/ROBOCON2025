#!/bin/bash

#获得该文件的位置
echo $0 | grep -q $0
if [ $? -eq 0 ]; then
        cd "$(dirname "$BASH_SOURCE")"
        CUR_FILE=$(pwd)/$(basename "$BASH_SOURCE")
        CUR_DIR=$(dirname "$CUR_FILE")
        cd - > /dev/null
else
        if [ ${0:0:1} = "/" ]; then
                CUR_FILE=$0
        else
                CUR_FILE=$(pwd)/$0
        fi
        CUR_DIR=$(dirname "$CUR_FILE")
fi
#去掉路径中的相对路径，如a/..b/c
cd "$CUR_DIR"
CUR_DIR=$PWD
cd - > /dev/null

echo $CUR_DIR
export BERXEL_SDK_LIBRARY=$CUR_DIR
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CUR_DIR/libs
