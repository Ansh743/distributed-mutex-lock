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
#define MAX_IP_ADDR_LENGTH 17
#define MAX_FILE_NAME_LENGTH 17

typedef unsigned long int u_long;
typedef unsigned short int u_short;
typedef unsigned char smallint;

typedef struct msg_packet
{
  u_short type;
  u_short seq;
  u_short lamport_clock;
} msg_pkt;

typedef struct ip_id
{
  char ip[MAX_IP_ADDR_LENGTH];
  smallint id;
} ip_id;

/**
 * @brief A structure to store all the distributed lock related variables
 */
typedef struct ds_lock
{
  int socket_fd; /**< The socket file descriptor. */
  struct sockaddr_in s_in, from;
  smallint should_listen : 1;
  smallint in_cs : 1;
  smallint requesting : 1;
  smallint active_hosts;
  char *file_name;
  char *self_ip_addr;
  char *ip_addrs[NUM_HOSTS];
  int thread_ret;
  sem_t semaphore;
  smallint shared;
  smallint d_array[NUM_HOSTS];
  ip_id ip_ids[NUM_HOSTS];
  pthread_t tid;
  u_short lamport_clock;
} ds_lock;

/**
 * @brief Initialize the struct ds_lock
 * @param ds_lock*
 * @return 0 if successful and -1 if unsuccessful
 */
int init(ds_lock *);

/**
 * @brief Sends a REQUEST to all other process and wait for REPLY
 * @param ds_lock*
 * @return 0 if successful and -1 if unsuccessful
 */
int lock(ds_lock *);

/**
 * @brief Unlock the distributed lock and send REPLY to processes in deferred array
 * @param ds_lock*
 * @return 0 if successful and -1 if unsuccessful
 */
int unlock(ds_lock *);

/**
 * @brief Safely dispose the ds_lock structure pointed by ds_lock*
 * @param ds_lock*
 * @return 0 if successful and -1 if unsuccessful
 */
int destroy(ds_lock *);