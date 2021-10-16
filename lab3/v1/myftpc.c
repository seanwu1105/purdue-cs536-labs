#include "arg_checkers.h"
#include "socket_utils.h"
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define REQUIRED_ARGC 6

typedef struct
{
    char *filename;
    uint16_t secret_key;
    unsigned long blocksize_byte;
} Config;

int parse_args(int argc, char *argv[], struct addrinfo **server_info,
               Config *const config)
{
    if (argc < REQUIRED_ARGC)
    {
        fprintf(stderr,
                "Usage: %s <server-ip> <server-port> <filename> <secret-key> "
                "<blocksize-byte>\n",
                argv[0]);
        return -1;
    }

    int status;

    if (status =
            build_addrinfo(server_info, argv[1], argv[2], SOCK_STREAM) != 0)
        return status;

    config->filename = argv[3];
    if (status = check_filename(config->filename) != 0) return status;

    long long secret_key = strtoull(argv[4], NULL, 0);
    if (status = check_secret_key(secret_key) != 0) return status;
    config->secret_key = (uint16_t)secret_key;

    config->blocksize_byte = strtoul(argv[5], NULL, 0);

    return 0;
}

int main(int argc, char *argv[])
{
    struct addrinfo *server_info;
    Config config;

    if (parse_args(argc, argv, &server_info, &config) != 0) return -1;
    return 0;
}