CFLAGS=-std=c11 -g -static

SRC:= $(wildcard src/*.c)

9cc: $(SRC) src/9cc.h
	$(CC) -o 9cc $(SRC) $(CFLAGS)

test-basic: 9cc
	scripts/test.sh basic

test-advance: 9cc
	scripts/test.sh advance

test-all: 9cc
	scripts/test.sh basic
	scripts/test.sh advance

clean:
	rm -f 9cc *.o *~ tmp* *.s

format:
	scripts/format.sh

.PHONY: test clean format