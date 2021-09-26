#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include "utils.h"
#include "read_config.h"
#include "message_codec.h"

#define REQUIRED_ARGC 4

int ping(int sockfd, struct sockaddr *target_addr, int32_t id, uint8_t delay)
{
    uint8_t message[5];
    encode_message(id, delay, message);
    if (sendto(sockfd, message, sizeof(message), 0, target_addr, sizeof(*target_addr)) == -1)
    {
        perror("sendto");
        return -1;
    }
    printf("sent\n");
    return 0;
}

int run(int sockfd, struct sockaddr *target_addr, Config config)
{
    for (size_t i = 0; i < config.num_packages; i++)
        if (ping(sockfd, target_addr, config.first_sequence_num + (int32_t)i, config.server_delay) == -1)
            return -1;

    return 0;
}

int parse_arg(int argc, char *argv[], struct addrinfo **client_info, struct addrinfo **server_info)
{
    if (argc < REQUIRED_ARGC)
    {
        fprintf(stderr, "insufficient arguments: expect %d\n", REQUIRED_ARGC);
        return -1;
    }
    const char *const client_ip = argv[1];
    const char *const server_ip = argv[2];
    const char *const server_port = argv[3];

    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;      // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP

    int status;
    if ((status = getaddrinfo(server_ip, server_port, &hints, server_info)) != 0)
    {
        fprintf(stderr, "getaddrinfo server: %s\n", gai_strerror(status));
        return -1;
    }

    if ((status = getaddrinfo(client_ip, "0", &hints, client_info)) != 0) // assign 0 to port number for automatic assignment
    {
        fprintf(stderr, "getaddrinfo client: %s\n", gai_strerror(status));
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    Config config = {};
    struct addrinfo *client_info;
    struct addrinfo *server_info;
    if (parse_arg(argc, argv, &client_info, &server_info) == -1 || read_config("pingparam.dat", &config) == -1)
        return -1;

    int sockfd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (sockfd == -1)
    {
        perror("socket");
        return -1;
    }

    if (bind(sockfd, client_info->ai_addr, client_info->ai_addrlen) == -1)
    {
        perror("bind");
        return -1;
    }

    run(sockfd, server_info->ai_addr, config);

    freeaddrinfo(server_info);
    close(sockfd);
    return 0;
}