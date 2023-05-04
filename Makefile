# banker: banker.c 
# 	gcc -o banker banker.c 
CC=gcc
CFLAGS=-g -Wall -lpthread

banker: banker.o dsm_lock_api.o
	$(CC) -o banker banker.o dsm_lock_api.o $(CFLAGS)

clean:
	rm -f *.o banker *.dat *.log