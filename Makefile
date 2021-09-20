all: tests


tests: btrees fsm heapm lua perf crc base64


btrees:
	$(MAKE) -C $@ -B

fsm:
	$(MAKE) -C $@/tests -B

heapm:
	$(MAKE) -C $@ -B

lua:
	$(MAKE) -C $@ -B

perf:
	$(MAKE) -C $@/tests -B

crc:
	$(MAKE) -C $@ -B

base64:
	$(MAKE) -C $@/tests -B
