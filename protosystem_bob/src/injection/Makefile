KERNEL = $(strip $(shell uname -r))
#INCLUDE = -I /usr/src/linux-headers-$(KERNEL)/include
CC_OPTS = -O3 -Wall -Wno-unknown-pragmas
CC = gcc

CFLAGS = $(CC_OPTS) $(INCLUDE)

ALL = start_connect random_packets rv tx 

all: $(ALL)

clean:
	rm $(ALL)

start_connect: start_connect.c
	$(CC) $(CFLAGS) -o $@ $^ -Wl,-Bstatic -lorcon -Wl,-Bdynamic -lm -lrt -lpcap

random_packets: random_packets.c util.o
	$(CC) $(CFLAGS) -o $@ $^ -Wl,-Bstatic -lorcon -Wl,-Bdynamic -lm -lrt -lpcap

rv:rv.c
	$(CC) $(CFLAGS) -o $@ $^ -Wl,-Bstatic -lorcon -Wl,-Bdynamic -lm -lrt -lpcap

tx:tx.c
	$(CC) $(CFLAGS) -o $@ $^ -Wl,-Bstatic -lorcon -Wl,-Bdynamic -lm -lrt -lpcap

util.o: util.c util.h Makefile
