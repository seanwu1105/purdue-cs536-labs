#include "socket_utils.h"
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int build_addrinfo(struct addrinfo **info, const char *const ip,
                   const char *const port, const int socktype)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;    // IPv4
    hints.ai_socktype = socktype; // UDP

    int status;
    if ((status = getaddrinfo(ip, port, &hints, info)) != 0)
        fprintf(stderr, "getaddrinfo server: %s\n", gai_strerror(status));

    return status;
}