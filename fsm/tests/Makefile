all: test ctest

CFLAGS += -DX_MUTEX_NO_THREAD_YIELD \
	  -DX_SIGNAL_NO_THREAD_YIELD

test: test.c ../fsm.c
	gcc -g $(CFLAGS) -std=c11 -o $@ $^
	- ./test


ctest: ctest.c ../cfsm.c ../../mutex/xmutex.c
	gcc -g $(CFLAGS) -std=c11 -o $@ $^
	- ./ctest
