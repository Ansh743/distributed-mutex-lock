#include <signal.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ifaddrs.h>
#include <unistd.h>

#define PORT 8080
#define HELLO       1
#define HELLO_ACK   2
#define REQUEST     3
#define REPLY       4

#define FILE_NAME "/process.hosts"
#define MAX_STR_LEN 16
#define NUM_HOSTS 10

typedef unsigned long int u_long;
typedef unsigned short int u_short;


typedef struct msg_packet {
  unsigned short type; 
  unsigned short seq;
  unsigned int hostid; // this is optional because you can obtain the host (sender) information from the ip header, we add this fied for the convenience.
  unsigned short vector_time[10]; // support upto to 10 hosts
} msg_pkt;

void printsin(struct sockaddr_in *sin, char *m1, char *m2);

/*
  Get the `process.hosts` file name
*/
char* get_file_path();

char *write_ip_to_file(char* filename);
int read_ip_from_file(const char* filename, char** strings);
