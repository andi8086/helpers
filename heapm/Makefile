PROGNAME = heapm_test
PROGNAME32 = heapm32_test

tests: $(PROGNAME) $(PROGNAME32)


CFLAGS += \
	  -g \
	  -DHEAPM_DEBUG \
	  -DHEAPM_MALLOC_LINE_STORE \
	  -DHEAPM_USE_MUTEX_X \
	  -DX_MUTEX_NO_THREAD_YIELD

OBJs := btrb.o \
	heapm.o \
	heapm_test.o \
	x-threads.o \
	xmutex.o

OBJs32 := btrb_compact.o \
	  heapm32.o \
	  heapm32_test.o \
	  x-threads.o \
	  xmutex.o

vpath %.c ../btrees/
vpath %.c ../mutex/
vpath %.c ../threads/

clean:
	rm -rf $(OBJs)
	rm -rf $(OBJs32)

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

$(PROGNAME): $(OBJs)
	gcc $(CFLAGS) $^ -o $@
	./$(PROGNAME)

$(PROGNAME32): $(OBJs32)
	gcc $(CFLAGS) $^ -o $@
	./$(PROGNAME32)
