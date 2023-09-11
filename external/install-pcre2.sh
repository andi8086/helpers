#!/bin/bash

declare -a FILES=(pcre2_auto_possess.c \
       pcre2_compile.c \
       pcre2_config.c \
       pcre2_context.c \
       pcre2_convert.c \
       pcre2_dfa_match.c \
       pcre2_error.c \
       pcre2_extuni.c \
       pcre2_find_bracket.c \
       pcre2_jit_compile.c \
       pcre2_maketables.c \
       pcre2_match.c \
       pcre2_match_data.c \
       pcre2_newline.c \
       pcre2_ord2utf.c \
       pcre2_pattern_info.c \
       pcre2_script_run.c \
       pcre2_serialize.c \
       pcre2_string_utils.c \
       pcre2_study.c \
       pcre2_substitute.c \
       pcre2_substring.c \
       pcre2_tables.c \
       pcre2_ucd.c \
       pcre2_valid_utf.c \
       pcre2_xclass.c \
       pcre2_internal.h \
       pcre2_ucp.h \
       pcre2_intmodedep.h \
       pcre2_jit_match.c \
       pcre2_jit_misc.c)


if [ "$1x" == "--gitx" ]; then
        set -e
        if [ "$2x" == "x" ]; then
                git clone -b pcre2-10.38 https://github.com/PCRE2Project/pcre2.git pcre2-git
        else
                git clone -b pcre2-10.38 $2 pcre2-git
        fi
        PCRE2_DIR="./pcre2-git"
        set +e
fi


if [ "${PCRE2_DIR}x" == "x" ]; then
        echo "Please specify PCRE2_DIR=<source directory>"
        exit -1
fi

mkdir -p pcre2/src

for i in ${FILES[@]}
do
        cp ${PCRE2_DIR}/src/$i ./pcre2/src/
done

cp ${PCRE2_DIR}/src/config.h.generic ./pcre2/src/config.h
cp ${PCRE2_DIR}/src/pcre2.h.generic ./pcre2/src/pcre2.h
cp ${PCRE2_DIR}/src/pcre2_chartables.c.dist ./pcre2/src/pcre2_chartables.c
cp ${PCRE2_DIR}/LICENCE ./pcre2/src/LICENSE

rm -rf "./pcre2-git"
