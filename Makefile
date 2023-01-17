CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

cc0: $(OBJS)
		$(CC) -o cc0 $(OBJS) $(LDFALGS)

$(OBJS): cc0.h



test: cc0
	./test.sh

clean:
		rm -f cc0 *.o *~ tmp* a.out

.PHONY: test clean
