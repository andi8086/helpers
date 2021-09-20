test: test.c
	gcc -g -I. -I.. -L.. ../infix2rpn.o ../ops.o ../stackq.o ../regex.o -lpcre2 test.c -o test
	./test
