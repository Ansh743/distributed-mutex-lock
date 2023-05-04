CC=gcc
CFLAGS=-g -Wall -lpthread

all: recv send send_recv

recv: recv.o
	$(CC) -o recv recv.o

send: send.o
	$(CC) -o send send.o

send_recv: send_recv.o helper.o
	$(CC) -o send_recv send_recv.o helper.o $(CFLAGS)

clean:
	rm -f *.o recv send send_recv *.out
