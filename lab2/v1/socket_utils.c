#include "socket_utils.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int print_addrinfo(const struct addrinfo *const info)
{
    char ipstr[INET_ADDRSTRLEN];

    for (const struct addrinfo *p = info; p != NULL; p = p->ai_next)
    {
        inet_ntop(p->ai_family, &((struct sockaddr_in *)p->ai_addr)->sin_addr, ipstr, sizeof ipstr);
        printf("IP address: %s\n", ipstr);
    }
    return 0;
}

int build_addrinfo(struct addrinfo **info, const char *const ip, const char *const port)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;      // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP

    int status;
    if ((status = getaddrinfo(ip, port, &hints, info)) != 0)
        fprintf(stderr, "getaddrinfo server: %s\n", gai_strerror(status));

    return status;
}

int connect_and_bind_first_usable_addr(const struct addrinfo *const target, const struct addrinfo *const self)
{
    int fd;
    const struct addrinfo *p;

    for (p = target; p != NULL; p = p->ai_next)
    {
        if ((fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("[attempt] socket");
            continue;
        }

        break; // break when find first usable result
    }

    if (p == NULL)
    {
        fprintf(stderr, "failed to create socket.\n");
        return -1;
    }

    for (p = self; p != NULL; p = p->ai_next)
    {

        if (bind(fd, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("[attempt] bind");
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        close(fd);
        fprintf(stderr, "failed to bind socket.\n");
        return -1;
    }

    return fd;
}