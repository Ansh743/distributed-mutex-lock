#include "helper.h"

int socket_fd;
struct sockaddr_in s_in, from;
short unsigned should_listen = 1;
char *file_name;
char *self_ip_addr;
char *ip_addrs[NUM_HOSTS];
unsigned short active_hosts;

extern char *get_file_path();
extern char *write_ip_to_file(char *);
extern int read_ip_from_file(const char *filename, char **strings);

/*
        Sets up the socket
*/
void setup_socket()
{
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0)
    {
        perror("recv_udp:socket");
        exit(1);
    }

    bzero((char *)&s_in, sizeof(s_in));
    s_in.sin_family = (short)AF_INET;
    s_in.sin_addr.s_addr = htonl(INADDR_ANY);
    s_in.sin_port = htons((u_short)PORT);
    printsin(&s_in, "RECV_UDP", "Local socket is:");
    fflush(stdout);

    if (bind(socket_fd, (struct sockaddr *)&s_in, sizeof(s_in)) < 0)
    {
        perror("recv_udp:bind");
        exit(1);
    }
}

int register_self()
{
    file_name = get_file_path();
    self_ip_addr = write_ip_to_file(file_name);
    if (self_ip_addr == NULL)
        return -1;
    return 0;
}

void clean_up()
{
    should_listen = 0;
    free(file_name);
    for (int i = 0; i < active_hosts; i++)
    {
        free(ip_addrs[i]);
    }
}

void *network_thread(void *arg)
{
    // code to receive incoming messages using recv() call
    int cc, count = 0;
    socklen_t fsize;
    char out_file[21];
    strcat(out_file, self_ip_addr);
    strcat(out_file, ".out");

    msg_pkt packet;
    for (;;)
    {
        if (should_listen == 0)
            break;
        fsize = sizeof(from);
        cc = recvfrom(socket_fd, &packet, sizeof(msg_pkt), 0, (struct sockaddr *)&from, (socklen_t *)&fsize);
        if (cc < 0)
            perror("network_thread:recvfrom");
        FILE *fp = fopen(out_file, "a+");
        if (fp == NULL)
        {
            perror("fopen");
            return NULL;
        }
        fprintf(fp, "From %s: packet type = %d\n", inet_ntoa(from.sin_addr), packet.type);
        fclose(fp);
        if(packet.type == HELLO_ACK)
        {
            count++;
            if(count >= active_hosts-1)
            {
                break;
            }
        }
        packet.type = HELLO_ACK;
        cc = sendto(socket_fd, &packet, sizeof(packet), 0, (struct sockaddr *)&from,
                    sizeof(from));
        if (cc < 0)
            perror("network_thread:sendto");
        fflush(stdout);
    }
    return (0);
}

int main()
{
    setup_socket();
    register_self();

    pthread_t tid;
    pthread_create(&tid, NULL, network_thread, NULL);

    // main program loop to handle user input
    char op;
    msg_pkt packet;
    int cc;
    struct sockaddr_in dest;
    struct hostent *gethostbyname(), *hostptr;

    printf("Do you want to send HELLO? (y/n): ");
    scanf("%c", &op);
    if (op == 'y')
    {
        active_hosts = read_ip_from_file(file_name, ip_addrs);
        for (int i = 0; i < active_hosts; i++)
        {
            if (strcmp(ip_addrs[i], self_ip_addr) != 0)
            {
                printf("Sending HELLO to %d: %s\n", i, ip_addrs[i]);
                if ((hostptr = gethostbyname(ip_addrs[i])) == NULL)
                {
                    fprintf(stderr, "send_udp: invalid host name, %s\n", ip_addrs[i]);
                    exit(1);
                }
                dest.sin_family = AF_INET;
                bcopy(hostptr->h_addr_list[0], (char *)&dest.sin_addr, hostptr->h_length);
                dest.sin_port = htons((u_short)PORT);

                packet.type = HELLO;

                cc = sendto(socket_fd, &packet, sizeof(packet), 0, (struct sockaddr *)&dest,
                            sizeof(dest));
                if (cc < 0)
                {
                    perror("send_udp:sendto");
                    exit(1);
                }
            }
        }
    }
    
    pthread_join(tid, NULL);
    // code to clean up and exit the program
    clean_up();
}
