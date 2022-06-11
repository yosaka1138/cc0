CFLAGS=-std=c11 -g -static

cc0: cc0.c

test: cc0
		./test.sh

clean:
		rm -f cc0 *.o *~ tmp*

.PHONY: test clean
