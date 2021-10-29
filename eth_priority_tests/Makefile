
CC = gcc
CFLAGS = -lpthread -I . -g0

libs: helpers.h
	$(CC) -c helpers.c $(CFLAGS)


jammer: libs
	$(CC) -o jammer.out jammer.c $(CFLAGS)

sink: libs
	$(CC) -o sink.out sink.c $(CFLAGS)

source: libs
	$(CC) -o source.out source.c $(CFLAGS)


all: jammer sink source

clean:
	rm -f jammer.out sink.out source.out