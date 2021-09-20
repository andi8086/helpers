all: test

CFLAGS += -DX_MUTEX_NO_THREAD_YIELD \
	  -DX_SIGNAL_NO_THREAD_YIELD

test: test.c ../perf.c ../../mutex/xmutex.c
	gcc -g $(CFLAGS) -std=c11 -lm -o $@ $^
	- ./test
