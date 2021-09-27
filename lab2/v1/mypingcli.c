#include "message_codec.h"
#include "read_config.h"
#include "socket_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define REQUIRED_ARGC 4

int sockfd;

void tear_down()
{
    close(sockfd);
}

int send_pinging(const struct sockaddr *target_addr, const int32_t id, const uint8_t delay)
{
    uint8_t message[MESSAGE_LEN];
    printf("sending: id=%d, delay=%hu", id, delay);
    encode_message(id, delay, message);
    if (sendto(sockfd, message, sizeof(message), 0, target_addr, sizeof(*target_addr)) == -1)
    {
        perror("sendto");
        return -1;
    }
    printf("sent\n");
    return 0;
}

int receive_feedback()
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

    int32_t id;
    uint8_t delay;
    decode_message(message, &id, &delay);

    printf("received: id=%d, delay=%hu\n", id, delay);
    return 0;
}

int ping(const struct sockaddr *target_addr, int32_t id, uint8_t delay)
{
    if (send_pinging(target_addr, id, delay) == -1)
        return -1;
    if (receive_feedback() == -1)
        return -1;
    return 0;
}

int run(const struct sockaddr *target_addr, Config config)
{
    for (size_t i = 0; i < config.num_packages; i++)
        if (ping(target_addr, config.first_sequence_num + (int32_t)i, config.server_delay) == -1)
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

    int status;
    if ((status = build_addrinfo(server_info, server_ip, server_port)) != 0)
        return status;

    if ((status = build_addrinfo(client_info, client_ip, "0")) != 0)
        return status;
    return 0;
}

void signal_handler(int _)
{
    tear_down();
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    Config config = {};
    struct addrinfo *client_info;
    struct addrinfo *server_info;
    if (parse_arg(argc, argv, &client_info, &server_info) != 0 || read_config(&config) == -1)
        return -1;

    sockfd = create_socket_with_first_usable_addr(server_info);
    if (sockfd == -1)
        return -1;

    if (bind_socket_with_first_usable_addr(client_info, sockfd) == -1)
    {
        close(sockfd);
        return -1;
    }

    run(server_info->ai_addr, config);

    freeaddrinfo(client_info);
    freeaddrinfo(server_info);
    close(sockfd);
    return 0;
}