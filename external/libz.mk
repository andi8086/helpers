all: libz.a


LIBZ_OBJs =\
	   adler32.o \
	   crc32.o \
	   deflate.o \
	   infback.o \
	   inffast.o \
	   inflate.o \
	   inftrees.o \
	   trees.o \
	   zutil.o


vpath %.c zlib/


CFLAGS =\
	-Izlib/

%.o: %.c
	gcc $(CFLAGS) -o $@ -c $<

libz.a: $(LIBZ_OBJs)
	ar crs $@ $^

