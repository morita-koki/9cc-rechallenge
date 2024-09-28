CFLAGS=-std=c11 -g -static

9cc: ./src/9cc.c
	$(CC) -o 9cc ./src/9cc.c $(CFLAGS)

test: 9cc
	scripts/test.sh

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: test clean