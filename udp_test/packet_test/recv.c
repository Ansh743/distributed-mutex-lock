#include "helper.h"



int main()
{
    int socket_fd, cc, fsize;
    struct sockaddr_in s_in, from;
    void printsin();

    msg_pkt packet;

    /*
       Create the socket to be used for datagram reception. Initially,
       it has no name in the internet (or any other) domain.
    */
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0)
    {
        perror("recv_udp:socket");
        exit(1);
    }


    bzero((char *)&s_in, sizeof(s_in)); /* They say you must do this    */

    s_in.sin_family = (short)AF_INET;
    s_in.sin_addr.s_addr = htonl(INADDR_ANY); /* WILDCARD */
    s_in.sin_port = htons((u_short)PORT);
    printsin(&s_in, "RECV_UDP", "Local socket is:");
    fflush(stdout);

    if (bind(socket_fd, (struct sockaddr *)&s_in, sizeof(s_in)) < 0)
    {
        perror("recv_udp:bind");
        exit(1);
    }

    for (;;)
    {
        fsize = sizeof(from);
        cc = recvfrom(socket_fd, &packet, sizeof(msg_pkt), 0, (struct sockaddr *)&from, &fsize);
        if (cc < 0)
            perror("recv_udp:recvfrom");
        printf("From %d: packet type = %d\n", packet.hostid, packet.type);
        packet.type = HELLO_ACK;
        cc = sendto(socket_fd, &packet, sizeof(packet), 0, (struct sockaddr *)&from,
                    sizeof(from));
        if (cc < 0)
        {
            perror("send_udp:sendto");
            exit(1);
        }
        fflush(stdout);
    }
    return (0);
}

void
    printsin(sin, m1, m2) struct sockaddr_in *sin;
char *m1, *m2;
{

    printf("%s %s:\n", m1, m2);
    printf("  family %d, addr %s, port %d\n", sin->sin_family,
           inet_ntoa(sin->sin_addr), ntohs((unsigned short)(sin->sin_port)));
}
