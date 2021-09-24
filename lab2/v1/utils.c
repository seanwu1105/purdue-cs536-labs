#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "utils.h"

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