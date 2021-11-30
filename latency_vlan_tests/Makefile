
CC = gcc
CFLAGS = -lpthread  -g0 -I./libs

libs: 
	$(CC) -c helpers.c $(CFLAGS)


eth_jammer: libs
	$(CC) -o eth_jammer.out eth_jammer.c helpers.o $(CFLAGS)

udp_jammer: libs
	$(CC) -o udp_jammer.out udp_jammer.c helpers.o $(CFLAGS)

eth_sink: libs
	$(CC) -o eth_sink.out eth_sink.c helpers.o $(CFLAGS)

eth_source: libs
	$(CC) -o eth_source.out eth_source.c helpers.o $(CFLAGS)

udp_sink: libs
	$(CC) -o udp_sink.out udp_sink.c helpers.o $(CFLAGS)

udp_source: libs
	$(CC) -o udp_source.out udp_source.c helpers.o $(CFLAGS)

tcp_sink: libs
	$(CC) -o tcp_sink.out tcp_sink.c helpers.o $(CFLAGS)

tcp_source: libs
	$(CC) -o tcp_source.out tcp_source.c helpers.o $(CFLAGS)


all: eth_jammer udp_jammer eth_sink eth_source udp_sink udp_source tcp_sink tcp_source 

clean:
	rm -f helpers.o eth_jammer.out udp_jammer.out eth_sink.out eth_source.out udp_sink.out udp_source.out tcp_sink.out tcp_source.out