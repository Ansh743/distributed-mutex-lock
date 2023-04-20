CC=gcc
CFLAGS=-g -Wall

all: recv send send_recv

recv: recv.o
	$(CC) -o recv recv.o

send: send.o
	$(CC) -o send send.o

send_recv: send_recv.o
	$(CC) -o send_recv send_recv.o

clean:
	rm -f *.o recv_udp send fancy_recv_udp
