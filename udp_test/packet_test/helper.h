#include <signal.h>
#include <errno.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT 8080
#define REQUEST     1
#define REPLY       2
#define HELLO       12
#define HELLO_ACK   13


typedef unsigned long int u_long;
typedef unsigned short int u_short;


typedef struct msg_packet {
  unsigned short type; 
  unsigned short seq;
  unsigned int hostid; // this is optional because you can obtain the host (sender) information from the ip header, we add this fied for the convenience.
  unsigned short vector_time[10]; // support upto to 10 hosts
} msg_pkt;
