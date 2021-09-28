#include "parse_addrinfo_arg.h"
#include "../lib/socket_utils.h"
#include <netdb.h>
#include <stdio.h>

int parse_addrinfo_arg(int argc, char *argv[], struct addrinfo **info)
{
    if (argc < REQUIRED_ARGC)
    {
        fprintf(stderr, "insufficient arguments: expect %d\n", REQUIRED_ARGC);
        return -1;
    }
    const char *const server_ip = argv[1];
    const char *const server_port = argv[2];

    int status;
    if ((status = build_addrinfo(info, server_ip, server_port)) != 0)
        return status;
    return 0;
}