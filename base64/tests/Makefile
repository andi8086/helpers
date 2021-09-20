all: tests run_tests

.PHONY: FORCE

vpath %.c ../

tests: base64.o main.o
	gcc -o $@ $^

run_tests: FORCE
	./tests
