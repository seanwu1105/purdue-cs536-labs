#include "message_codec.h"
#include "socket_utils.h"
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define REQUIRED_ARGC 3

int run(int sockfd)
{
    uint8_t message[MESSAGE_LEN];
    struct sockaddr originating_addr;
    socklen_t originating_addr_len = sizeof(originating_addr);
    ssize_t message_len = recvfrom(sockfd, message, sizeof(message), 0, &originating_addr, &originating_addr_len);

    if (message_len == -1)
    {
        perror("recvfrom");
        return -1;
    }

    if (message_len == 0)
        return 0;

    printf("received\n");
    int32_t id;
    uint8_t delay;
    decode_message(message, &id, &delay);

    printf("%d\t%hu\n", id, delay);

    return 0;
}

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
    struct addrinfo *info;
    if (parse_arg(argc, argv, &info) != 0)
        return -1;

    int sockfd = create_socket_with_first_usable_addr(info);
    if (sockfd == -1)
        return -1;

    if (bind_socket_with_first_usable_addr(info, sockfd) == -1)
    {
        close(sockfd);
        return -1;
    }

    run(sockfd);

    freeaddrinfo(info);
    close(sockfd);
    return 0;
}