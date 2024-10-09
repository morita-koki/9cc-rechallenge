CFLAGS=-std=c11 -g -static

SRC:= $(wildcard src/*.c)

9cc: $(SRC) src/9cc.h
	$(CC) -o 9cc $(SRC) $(CFLAGS)

test: 9cc
	scripts/test.sh

clean:
	rm -f 9cc *.o *~ tmp* *.s

format:
	scripts/format.sh

.PHONY: test clean format