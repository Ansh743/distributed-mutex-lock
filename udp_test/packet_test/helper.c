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

int last_id_from_file(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("fopen");
        return -1;
    }

    int last_id = -1;
    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        char ip_address[MAX_IP_ADDR_LENGTH];
        int id_ = 0;
        sscanf(line, "%s %d", ip_address, &id_);
        if (id_ > last_id)
        {
            last_id = id_;
        }
    }
    fclose(file);
    return last_id;
}

char *write_ip_to_file(char *filename)
{
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    char *ip_address;
    char id;

    if (getifaddrs(&ifap) == -1)
    {
        perror("getifaddrs");
        return NULL;
    }

    if (access(filename, F_OK) != -1)
    {
        // File exists - Read lst ID from file
        id = last_id_from_file(filename) + 1;
    }
    else
    {
        // File does not exists
        id = 0;
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
                fprintf(file, "%s %d\n", ip_address, id);
                break;
            }
        }
    }

    fclose(file);
    freeifaddrs(ifap);
    return ip_address;
}

int read_ip_from_file(const char *filename, ip_id *ip_ids_arr)
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
        char *temp = malloc(MAX_IP_ADDR_LENGTH);
        if (buf[0] == '\n' || buf[0] == '\0')
        {
            continue;
        }
        if (strlen(buf) > 0 && buf[strlen(buf) - 1] == '\n')
        {
            buf[strlen(buf) - 1] = '\0';
        }

        temp = strdup(buf);
        char *token = strtok(temp, " ");
        if (token != NULL)
        {
            // Put IP in strings array
            strcpy(ip_ids_arr[count].ip, token);

            // Put ID in arrays
            token = strtok(NULL, " ");
            if (token != NULL)
            {
                char s2[4];
                strcpy(s2, token);
                ip_ids_arr[count].id = atoi(s2);
            }
        }

        if (!ip_ids_arr[count].ip)
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
            sprintf(str1, "%d", (ds_lck->active_hosts) - 1);
            strcat(str, str1);

            write_msg("Received HELLO_ACK", str, ds_lck);
            if (count >= ds_lck->active_hosts - 1)
            {
                count = 0;
                sem_post(&ds_lck->semaphore);
                continue;
            }
        }

        if (packet.type == REPLY)
        {
            count++;
            char str1[10], str[10];

            sprintf(str, "%d", count);
            strcat(str, "/");
            sprintf(str1, "%d", (ds_lck->active_hosts) - 1);
            strcat(str, str1);

            write_msg("Received REPLY", str, ds_lck);
            if (count >= ds_lck->active_hosts - 1)
            {
                count = 0;
                sem_post(&ds_lck->semaphore);
                continue;
            }
        }

        if (ds_lck->in_cs == 1)
        {
            write_msg("Received REQUEST while locked from", inet_ntoa(ds_lck->from.sin_addr), ds_lck);

            // TODO: Put request in deferred array
            for (int i = 0; i < ds_lck->active_hosts; i++)
            {
                if (strcmp(ds_lck->ip_ids[i].ip, inet_ntoa(ds_lck->from.sin_addr)) == 0)
                {
                    ds_lck->d_array[ds_lck->ip_ids[i].id] = 1;
                }
            }
            continue;
        }

        /*
            1. If the host receives a REQUEST message and the host is not requesting, sends a REPLY immediately;
        */
        if (packet.type == REQUEST && ds_lck->requesting == 0)
        {
            /*
            Upon the receipt of a message m, process Pj
            adjusts its own local counter as
            Cj <- max{Cj , ts (m)}, after which it then executes
            the first step and delivers the message to the
            application.
            */
            if (packet.lamport_clock > ds_lck->lamport_clock)
                ds_lck->lamport_clock = packet.lamport_clock;

            write_msg("Received REQUEST from", inet_ntoa(ds_lck->from.sin_addr), ds_lck);
            packet.type = REPLY;
            write_msg("Sending REPLY to", inet_ntoa(ds_lck->from.sin_addr), ds_lck);

            cc = sendto(ds_lck->socket_fd, &packet, sizeof(packet), 0, (struct sockaddr *)&(ds_lck->from), sizeof(ds_lck->from));
            if (cc < 0)
                perror("network_thread:sendto");

            write_msg("Sent REPLY to", inet_ntoa(ds_lck->from.sin_addr), ds_lck);
            continue;
        }

        /*
            2. If the host receives a REQUEST message and the host is also requesting:
        */
        if (packet.type == REQUEST && ds_lck->requesting == 1)
        {
            /*
                a) the host has a vector time earlier than that of REQUEST, defer the REPLY and put the REQUEST in the queue;
            */
            if (ds_lck->lamport_clock <= packet.lamport_clock)
            {
                // TODO: Put REQUEST in deferred array
                for (int i = 0; i < ds_lck->active_hosts; i++)
                {
                    if (strcmp(ds_lck->ip_ids[i].ip, inet_ntoa(ds_lck->from.sin_addr)) == 0)
                    {
                        ds_lck->d_array[ds_lck->ip_ids[i].id] = 1;
                        break;
                    }
                }
                continue;
            }

            /*
                b) the host has a vector time later than that of REQUEST, sends a REPLY immediately.
            */
            if (ds_lck->lamport_clock > packet.lamport_clock)
            {
                // TODO: send REPLY
                write_msg("Received REQUEST from", inet_ntoa(ds_lck->from.sin_addr), ds_lck);
                packet.type = REPLY;
                write_msg("Sending REPLY to", inet_ntoa(ds_lck->from.sin_addr), ds_lck);

                cc = sendto(ds_lck->socket_fd, &packet, sizeof(packet), 0, (struct sockaddr *)&(ds_lck->from), sizeof(ds_lck->from));
                if (cc < 0)
                    perror("network_thread:sendto");

                write_msg("Sent REPLY to", inet_ntoa(ds_lck->from.sin_addr), ds_lck);
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
    return 0;
}

int  init(ds_lock *ds_lck)
{
    ds_lck->shared = 0;
    ds_lck->in_cs = 0;
    ds_lck->requesting = 0;
    ds_lck->lamport_clock = 0;
    for (int i = 0; i < NUM_HOSTS; i++)
    {
        ds_lck->d_array[i] = 0;
    }
    if (setup_socket(ds_lck) < 0)
    {
        perror("api:setup_socket");
        return -1;
    }
    if (register_self(ds_lck) < 0)
    {
        perror("api:register_self");
        return -1;
    }
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

    ds_lck->active_hosts = read_ip_from_file(ds_lck->file_name, ds_lck->ip_ids);
    ds_lck->requesting = 1;
    packet.lamport_clock = ds_lck->lamport_clock;

    for (i = 0; i < ds_lck->active_hosts; i++)
    {
        if (strcmp(ds_lck->ip_ids[i].ip, ds_lck->self_ip_addr) != 0)
        {
            // printf("Sending HELLO to %d: %s\n", i, ds_lck->ip_addrs[i]);
            write_msg("Sending REQUEST to", ds_lck->ip_addrs[i], ds_lck);
            if ((hostptr = gethostbyname(ds_lck->ip_ids[i].ip)) == NULL)
            {
                fprintf(stderr, "lock: invalid host name, %s\n", ds_lck->ip_ids[i].ip);
                return -1;
            }

            bcopy(hostptr->h_addr_list[0], (char *)&dest.sin_addr, hostptr->h_length);

            packet.type = REQUEST;

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
    sem_wait(&ds_lck->semaphore);
    ds_lck->in_cs = 1;
    ds_lck->requesting = 0;
    //CLOCK: Updating
    ds_lck->lamport_clock += 1;

    return 0;
}

int unlock(ds_lock *ds_lck)
{

    int cc;
    struct hostent *hostptr;
    struct sockaddr_in dest;
    msg_pkt packet;

    dest.sin_family = AF_INET;
    dest.sin_port = htons((u_short)PORT);

    ds_lck->active_hosts = read_ip_from_file(ds_lck->file_name, ds_lck->ip_ids);
    //CLOCK: Updating
    ds_lck->lamport_clock += 1;

    // TODO: Check deferred array and send REPLY to pending processes
    for (int i = 0; i < ds_lck->active_hosts; i++)
    {
        if (ds_lck->d_array[i] == 1)
        {

            printf("Send REPLY to %s\n", ds_lck->ip_ids[i].ip);
            if (strcmp(ds_lck->ip_ids[i].ip, ds_lck->self_ip_addr) != 0)
            {
                // printf("Sending HELLO to %d: %s\n", i, ds_lck->ip_addrs[i]);
                write_msg("Sending deferred REPLY to", ds_lck->ip_addrs[i], ds_lck);
                if ((hostptr = gethostbyname(ds_lck->ip_ids[i].ip)) == NULL)
                {
                    fprintf(stderr, "lock: invalid host name, %s\n", ds_lck->ip_ids[i].ip);
                    return -1;
                }

                bcopy(hostptr->h_addr_list[0], (char *)&dest.sin_addr, hostptr->h_length);

                packet.type = REPLY;

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
            ds_lck->d_array[i] = 0;
        }
    }
    ds_lck->in_cs = 0;
    printf("Clock = %d\n", ds_lck->lamport_clock);
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