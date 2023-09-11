#!/bin/bash


if [ "$1x" == "--gitx" ]; then
        set -e
        if [ "$2x" == "x" ]; then
                git clone -b v5.4.3 https://github.com/lua/lua.git lua
        else
                git clone -b v5.4.3 $2 lua
        fi
        LUA_DIR="./lua"
        set +e
fi


if [ "${LUA_DIR}x" == "x" ]; then
        echo "Please specify LUA_DIR=<source directory>"
        exit -1
else
        cp -R ${LUA_DIR}/ lua
fi

