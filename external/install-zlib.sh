#!/bin/bash


if [ "$1x" == "--gitx" ]; then
        if [ "$2x" == "x" ]; then
                git clone -b v1.2.12 https://github.com/madler/zlib.git zlib
        else
                git clone -b v1.2.12 $2 zlib
        fi
        rm -f zlib/zconf.h
        cp zconf.h zlib/
        exit 0
fi


if [ "${SRC_DIR}x" == "x" ]; then
        echo "Please specify SRC_DIR=<source directory>"
        exit -1
fi

cp -Ra ${SRC_DIR} ./zlib
cp zconf.h ./zlib
