PROGNAME = btrb_test

tests: btrb_test
	./$(PROGNAME)

TEST = btrb.o \
       btrb_compact.o \
       btrb_test.o

clean:
	rm -rf $(TEST)
	rm -rf $(PROGNAME)
	rm -rf $(PROGNAME).exe

%.o: %.c
	gcc -g -c $<

$(PROGNAME): $(TEST)
	gcc -g $^ -o $@
