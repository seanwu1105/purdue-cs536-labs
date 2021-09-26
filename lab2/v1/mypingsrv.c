#include "read_config.h"
#include "socket_utils.h"
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define REQUIRED_ARGC 3

int parse_arg(int argc, char *argv[], struct addrinfo **info)
{
    if (argc < REQUIRED_ARGC)
    {
        fprintf(stderr, "insufficient arguments: expect %d\n", REQUIRED_ARGC);
        return -1;
    }
    const char *const ip = argv[1];
    const char *const port = argv[2];

    int status;
    if ((status = build_addrinfo(info, ip, port)) != 0)
        return status;
    return 0;
}

int main(int argc, char *argv[])
{
    Config config = {};
    struct addrinfo *info;
    if (parse_arg(argc, argv, &info) != 0 || read_config(&config) == -1)
        return -1;

    int sockfd = create_socket_with_first_usable_addr(info);
    if (sockfd == -1)
        return -1;

    if (bind_socket_with_first_usable_addr(info, sockfd) == -1)
    {
        close(sockfd);
        return -1;
    }

    return 0;
}