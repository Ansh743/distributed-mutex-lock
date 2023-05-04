#include "helper.h"

int main(int argc,char** argv)

{
    int socket_fd, cc, fsize;
    long getpid();
    struct sockaddr_in dest, from;
    struct hostent *gethostbyname(), *hostptr;

    msg_pkt packet;


    /*
       Set up a datagram (UDP/IP) socket in the Internet domain.
       We will use it as the handle thru which we will send a
       single datagram. Note that, since this no message is ever
       addressed TO this socket, we do not bind an internet address
       to it. It will be assigned a (temporary) address when we send
       a message thru it.
    */

    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd == -1)
    {
        perror("send_udp:socket");
        exit(1);
    }

    /*
       The inet socket address data structure "dest" will be used to
       specify the destination of the datagram. We must fill in the
       blanks.
    */

    bzero((char *)&dest, sizeof(dest)); /* They say you must do this */
    if ((hostptr = gethostbyname(argv[1])) == NULL)
    {
        fprintf(stderr, "send_udp: invalid host name, %s\n", argv[1]);
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

    for (;;)
    {
        fsize = sizeof(from);
        cc = recvfrom(socket_fd, &packet, sizeof(msg_pkt), 0, (struct sockaddr *)&from, &fsize);
        if (cc < 0)
            perror("recv_udp:recvfrom");
        printf("From %d: packet type = %d\n", packet.hostid, packet.type);
    }
    return (0);
}
