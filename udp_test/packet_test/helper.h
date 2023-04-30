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
#include <time.h>
#include <semaphore.h>

#define PORT 8081
#define HELLO 1
#define HELLO_ACK 2
#define REQUEST 3
#define REPLY 4

#define FILE_NAME "/process.hosts"
#define MAX_STR_LEN 16
#define NUM_HOSTS 4

typedef unsigned long int u_long;
typedef unsigned short int u_short;

typedef struct msg_packet
{
  unsigned short type;
  unsigned short seq;
  unsigned int hostid;            // this is optional because you can obtain the host (sender) information from the ip header, we add this field for the convenience.
  unsigned short vector_time[NUM_HOSTS]; // support upto to 10 hosts
} msg_pkt;

typedef struct ds_lock
{
  int socket_fd;
  struct sockaddr_in s_in, from;
  short unsigned should_listen;
  char *file_name;
  char *self_ip_addr;
  char *ip_addrs[NUM_HOSTS];
  unsigned short active_hosts;
  int thread_ret;
  sem_t semaphore;
  int shared;

  pthread_t tid;
} ds_lock;

int init(ds_lock *);
int lock(ds_lock *);
int destroy(ds_lock *);