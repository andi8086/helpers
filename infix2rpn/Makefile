# target libpcre2.a provided by libpcre2.mk
all: libpcre2.a itest

vpath %.c ../external/pcre2/src
vpath %.c ../stackq
vpath %.c ../regex

include ../external/libpcre2.mk

CFLAGS = \
	-g \
	-DPCRE2_CODE_UNIT_WIDTH=8 \
	-DHAVE_CONFIG_H \
	-DPCRE2_EXP_DECL= \
	-DPCRE2_EXP_DEFN= \
	-I.. \
        -I../external/pcre2/src

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

itest: main.o infix2rpn.o ops.o stackq.o regex.o
	gcc $(CFLAGS) $^ -L. -lpcre2  -o $@
	$(MAKE) -C test

