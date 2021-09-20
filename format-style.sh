#!/bin/sh

cf="clang-format"
which $cf 2> /dev/null || cf="/mingw64/bin/clang-format"

opts="--style=file -i"

find . -name '*.[ch]' \
        -not -path "./external/*" \
        -prune -exec $cf $opts {} \;

