#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include "utils.h"

int parse_arg(int argc, char *argv[], struct addrinfo **server_info)
{
    if (argc < 4)
    {
        fprintf(stderr, "Insufficient arguments.");
        return -1;
    }
    // const char *const client_ip = argv[1];
    const char *const server_ip = argv[2];
    const char *const server_port = argv[3];

    printf("%s\n", server_ip);

    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;      // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP

    int status;
    if ((status = getaddrinfo(server_ip, server_port, &hints, server_info)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return -1;
    }
    return 0;
}

typedef struct
{
    unsigned short num_packages; // 1 ~ 7
    unsigned short time_out;     // 1 ~ 5
    unsigned short server_delay; // 1 byte, 0 ~ 5 | 99
    long first_sequence_num;     // 4 bytes
} Config;

int read_config(const char *const filename, Config *const config)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Cannot open config.\n");
        return -1;
    }

    char key;
    char val[128];
    while (fscanf(file, "%c=%127[^\n]%*c", &key, val) == 2)
        switch (key)
        {
        case 'N':
            config->num_packages = (unsigned short)strtoul(val, NULL, 0);
            break;

        case 'T':
            config->time_out = (unsigned short)strtoul(val, NULL, 0);
            break;

        case 'D':
            config->server_delay = (unsigned short)strtoul(val, NULL, 0);
            break;

        case 'S':
            config->first_sequence_num = strtol(val, NULL, 0);
            break;

        default:
            fprintf(stderr, "Unknown configuration key: %c\n", key);
            return -1;
        }

    return 0;
}

int main(int argc, char *argv[])
{
    Config config = {};
    struct addrinfo *server_info;
    if (parse_arg(argc, argv, &server_info) == -1 || read_config("pingparam.dat", &config) == -1)
        return -1;

    print_addrinfo(server_info);
    return 0;
}