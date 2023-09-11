#!/bin/bash


if [ "$1x" == "--gitx" ]; then
        if [ "$2x" == "x" ]; then
                git clone -b 4.0.2 https://github.com/LoupVaillant/Monocypher.git monocypher
        else
                git clone -b 4.0.2 $2 monocypher
        fi
        exit 0
fi


if [ "${SRC_DIR}x" == "x" ]; then
        echo "Please specify SRC_DIR=<source directory>"
        exit -1
fi

cp -Ra ${SRC_DIR} ./monocypher
