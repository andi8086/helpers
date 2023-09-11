CFLAGS =\
	-I../external/monocypher/src


x3dh: x3dh.c xsign.c
	gcc $(CFLAGS) -o $@ $^ -lmonocypher -L.
	LD_LIBRARY_PATH=. ./x3dh
