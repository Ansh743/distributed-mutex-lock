#include "helper.h"

void printsin(struct sockaddr_in *sin, char *m1, char *m2)
{
    printf("%s %s:\n", m1, m2);
    printf("  family %d, addr %s, port %d\n", sin->sin_family,
           inet_ntoa(sin->sin_addr), ntohs((unsigned short)(sin->sin_port)));
}

char *get_file_path()
{
    char *home_dir = getenv("HOME");
    if (home_dir == NULL)
    {
        perror("get_file_path: HOME environment variable not set");
        return NULL;
    }

    // Allocate memory for the file path string
    char *file_path = malloc(strlen(home_dir) + strlen(FILE_NAME) + 1);
    if (file_path == NULL)
    {
        perror("get_file_path: Failed to allocate memory for file path");
        return NULL;
    }

    // Copy the home directory path and the file name into the file path string
    strcpy(file_path, home_dir);
    strcat(file_path, FILE_NAME);

    return file_path;
}

char *write_ip_to_file(char *filename)
{
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    char *ip_address;

    if (getifaddrs(&ifap) == -1)
    {
        perror("getifaddrs");
        return NULL;
    }

    FILE *file = fopen(filename, "a+");
    if (file == NULL)
    {
        perror("fopen");
        return NULL;
    }

    for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            sa = (struct sockaddr_in *)ifa->ifa_addr;
            ip_address = inet_ntoa(sa->sin_addr);

            // Ignore loopback address
            if (strcmp(ip_address, "127.0.0.1") != 0)
            {
                fprintf(file, "%s\n", ip_address);
                break;
            }
        }
    }

    fclose(file);
    freeifaddrs(ifap);
    return ip_address;
}

int read_ip_from_file(const char *filename, char **strings)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        perror("Error opening file");
        return -1;
    }

    char buf[MAX_STR_LEN + 1];
    int count = 0;
    while (fgets(buf, sizeof(buf), fp) != NULL && count < NUM_HOSTS)
    {
        if (strlen(buf) > 0 && buf[strlen(buf) - 1] == '\n')
        {
            buf[strlen(buf) - 1] = '\0';
        }
        strings[count] = strdup(buf);
        if (!strings[count])
        {
            perror("Error allocating memory for string");
            fclose(fp);
            return -1;
        }
        count++;
    }

    fclose(fp);
    return count;
}

void write_msg(char *msg, char *from, ds_lock *ds_lck)
{
    char out_file[21] = "";
    strcat(out_file, ds_lck->self_ip_addr);
    strcat(out_file, ".out");

    time_t current_time;
    struct tm *time_info;
    char time_string[9];

    time(&current_time);
    time_info = localtime(&current_time);

    strftime(time_string, sizeof(time_string), "%H:%M:%S", time_info);

    FILE *fp = fopen(out_file, "a+");
    if (fp == NULL)
    {
        perror("fopen");
    }
    else
    {
        if (from == NULL)
            fprintf(fp, "%s: %s\n", time_string, msg);
        else
            fprintf(fp, "%s: %s %s\n", time_string, msg, from);
    }
    fclose(fp);
}

void *network_thread(void *arg)
{
    // code to receive incoming messages using recv() call
    ds_lock *ds_lck;
    int cc, count = 0;
    socklen_t fsize;
    ds_lck = (ds_lock *)arg;
    ds_lck->should_listen = 1;

    write_msg("Thread started", NULL, ds_lck);

    msg_pkt packet;
    for (;;)
    {

        fsize = sizeof(ds_lck->from);

        write_msg("Listening for incoming messages...", NULL, ds_lck);

        cc = recvfrom(ds_lck->socket_fd, &packet, sizeof(msg_pkt), 0, (struct sockaddr *)&(ds_lck->from), (socklen_t *)&fsize);
        if (cc < 0)
            perror("network_thread:recvfrom");
        if (ds_lck->should_listen == 0)
            break;
        if (packet.type == HELLO)
        {
            write_msg("Received HELLO from", inet_ntoa(ds_lck->from.sin_addr), ds_lck);
            packet.type = HELLO_ACK;

            write_msg("Sending HELLO_ACK to", inet_ntoa(ds_lck->from.sin_addr), ds_lck);

            cc = sendto(ds_lck->socket_fd, &packet, sizeof(packet), 0, (struct sockaddr *)&(ds_lck->from),
                        sizeof(ds_lck->from));

            write_msg("Sent HELLO_ACK to", inet_ntoa(ds_lck->from.sin_addr), ds_lck);

            if (cc < 0)
                perror("network_thread:sendto");
            continue;
        }
        if (packet.type == HELLO_ACK)
        {
            count++;
            char str1[10], str[10];

            sprintf(str, "%d", count);
            strcat(str, "/");
            sprintf(str1, "%d", (ds_lck->active_hosts)-1);
            strcat(str, str1);

            write_msg("Received ", str, ds_lck);
            if (count >= ds_lck->active_hosts - 1)
            {
                count = 0;
                sem_post(&ds_lck->semaphore);
                continue;
            }
        }

        fflush(stdout);
    }

    write_msg("Thread exited", NULL, ds_lck);
    pthread_exit(NULL);
}

int register_self(ds_lock *ds_lck)
{
    ds_lck->file_name = get_file_path();
    ds_lck->self_ip_addr = write_ip_to_file(ds_lck->file_name);
    printf("Self ip_addr: %s\n", ds_lck->self_ip_addr);
    if (ds_lck->self_ip_addr == NULL)
        return -1;

    pthread_create(&(ds_lck->tid), NULL, network_thread, (void *)ds_lck);
    return 0;
}

int setup_socket(ds_lock *ds_lck)
{
    int status;
    ds_lck->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if ((ds_lck->socket_fd) < 0)
    {
        perror("api:socket");
        return -1;
    }

    bzero((char *)&(ds_lck->s_in), sizeof(ds_lck->s_in));
    ds_lck->s_in.sin_family = (short)AF_INET;
    ds_lck->s_in.sin_addr.s_addr = htonl(INADDR_ANY);
    ds_lck->s_in.sin_port = htons((u_short)PORT);
    printsin(&ds_lck->s_in, "RECV_UDP", "Local socket is:");

    fflush(stdout);
    status = bind(ds_lck->socket_fd, (struct sockaddr *)&(ds_lck->s_in), sizeof(ds_lck->s_in));
    if (status < 0)
    {
        perror("api:bind");
        return -1;
    }

    status = register_self(ds_lck);
    if (status < 0)
    {
        perror("api:register_self");
        return -1;
    }
    return 0;
}

int init(ds_lock *ds_lck)
{
    ds_lck->shared = 0;
    if (setup_socket(ds_lck) < 0)
        return -1;
    // pthread_join(ds_lck->tid, NULL);
    return 0;
}

int lock(ds_lock *ds_lck)
{
    int i, cc;
    struct hostent *hostptr;
    struct sockaddr_in dest;
    msg_pkt packet;

    sem_init(&ds_lck->semaphore, 0, 0);

    dest.sin_family = AF_INET;
    dest.sin_port = htons((u_short)PORT);

    ds_lck->active_hosts = read_ip_from_file(ds_lck->file_name, ds_lck->ip_addrs);
    for (i = 0; i < ds_lck->active_hosts; i++)
    {
        if (strcmp(ds_lck->ip_addrs[i], ds_lck->self_ip_addr) != 0)
        {
            printf("Sending HELLO to %d: %s\n", i, ds_lck->ip_addrs[i]);
            write_msg("Sending HELLO to", ds_lck->ip_addrs[i], ds_lck);
            if ((hostptr = gethostbyname(ds_lck->ip_addrs[i])) == NULL)
            {
                fprintf(stderr, "lock: invalid host name, %s\n", ds_lck->ip_addrs[i]);
                return -1;
            }

            bcopy(hostptr->h_addr_list[0], (char *)&dest.sin_addr, hostptr->h_length);

            packet.type = HELLO;

            if (ds_lck->socket_fd < 0)
            {
                perror("lock: invalid socket_fd\n");
                return -1;
            }
            cc = sendto(ds_lck->socket_fd, &packet, sizeof(packet), 0, (struct sockaddr *)&dest,
                        sizeof(dest));
            if (cc < 0)
            {
                perror("lock:sendto");
                return -1;
            }
        }
    }
    // TODO: Check for a condition before returning 0
    sem_wait(&ds_lck->semaphore);
    return 0;
}

int destroy(ds_lock *ds_lck)
{
    ds_lck->should_listen = 0;
    int ret = pthread_kill(ds_lck->tid, 0);
    if (ret == 0)
    {
        pthread_cancel(ds_lck->tid);
    }
    else
    {
        printf("Thread is not running.\n");
    }
    // pthread_join(ds_lck->tid, NULL);
    free(ds_lck->file_name);
    for (int i = 0; i < ds_lck->active_hosts; i++)
    {
        free(ds_lck->ip_addrs[i]);
    }
    free(ds_lck);
    return 0;
}