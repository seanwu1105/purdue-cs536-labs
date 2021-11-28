#include "socket_utils.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int build_addrinfo(struct addrinfo **info, const char *const ip,
                   const char *const port, const int socktype)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
    hints.ai_socktype = socktype;

    int status;
    if ((status = getaddrinfo(ip, port, &hints, info)) != 0)
        fprintf(stderr, "getaddrinfo server: %s\n", gai_strerror(status));

    return status;
}

int create_socket_with_first_usable_addr(const struct addrinfo *const info)
{
    int fd = -1;
    const struct addrinfo *p;

    for (p = info; p != NULL; p = p->ai_next)
    {
        if ((fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("socket (attempt)");
            continue;
        }

        break; // break when find first usable result
    }

    if (p == NULL)
    {
        fprintf(stderr, "failed to create socket\n");
        return -1;
    }

    return fd;
}

int bind_socket_with_first_usable_addr(const struct addrinfo *const info,
                                       const int sockfd)
{
    const struct addrinfo *p;
    for (p = info; p != NULL; p = p->ai_next)
    {

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("bind (attempt)");
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "failed to bind socket\n");
        return -1;
    }
    return 0;
}

uint16_t get_port_number(const int fd)
{
    struct addrinfo *res;
    if (build_addrinfo(&res, NULL, "0", SOCK_DGRAM) != 0) return -1;

    if (getsockname(fd, res->ai_addr, &res->ai_addrlen) < 0)
    {
        perror("getsockname");
        return -1;
    }

    return ntohs(((struct sockaddr_in *)res->ai_addr)->sin_port);
}