# Makefile to create libnetprot.a and libnetcrypto.a
all: libnetprot.a libnetcrypto.a libnetprot.so libnetcrypto.so

NETPROT_OBJs = \
	net_api.o \
	net_core.o \
	tcp_client.o \
	tcp_server.o \
	net_msg.o

CRYPTO_OBJs = \
	net_crypto.o

vpath %.c tcp/

-include $(NETPROT_OBJs:.o=.d)

ifeq ($(BUILD_PLATFORM),Win32)
	CRYPTO_OBJs += arc4win.o
endif

CFLAGS =\
	-fPIC \
	-MMD \
	-Itcp/ \
	-I. \
	-I../external/zlib/ \
	-I../external/monocypher/src

include detect_os.mk

%.o: %.c
	gcc $(CFLAGS) -o $@ -c $<

libnetprot.a: $(NETPROT_OBJs)
	ar crs $@ $^

libnetprot.so: $(NETPROT_OBJs)
	gcc -shared -o $@ $^

libnetcrypto.a: $(CRYPTO_OBJs)
	ar crs $@ $^

libnetcrypto.so: $(CRYPTO_OBJs)
	gcc -shared -o $@ $^
