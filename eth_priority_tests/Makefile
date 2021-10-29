
CC = gcc
CFLAGS = -lpthread -I . -g0


jammer: 
	$(CC) -o jammer.out jammer.c $(CFLAGS)

sink: 
	$(CC) -o sink.out sink.c $(CFLAGS)

source: 
	$(CC) -o source.out source.c $(CFLAGS)


all: jammer sink source

clean:
	rm -f jammer.out sink.out source.out