CFLAGS=-std=c11 -g -static

SRC:= $(wildcard src/*.c)

9cc: $(SRC) 9cc.h
	$(CC) -o 9cc $(SRC) $(CFLAGS)

test: 9cc
	scripts/test.sh

clean:
	rm -f 9cc *.o *~ tmp*

format:
	scripts/format.sh

.PHONY: test clean format