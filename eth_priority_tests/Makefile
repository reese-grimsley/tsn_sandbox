
CC = gcc
CFLAGS = -lpthread  -g0 -I=../libs

libs: 
	$(CC) -c helpers.c $(CFLAGS)


jammer: libs
	$(CC) -o jammer.out jammer.c helpers.o $(CFLAGS)

sink: libs
	$(CC) -o sink.out sink.c helpers.o $(CFLAGS)

source: libs
	$(CC) -o source.out source.c helpers.o $(CFLAGS)


all: jammer sink source

clean:
	rm -f jammer.out sink.out source.out