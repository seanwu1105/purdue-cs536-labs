#include "../lib/socket_utils.h"
#include "message_codec.h"
#include "read_config.h"
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define REQUIRED_ARGC 3

int sockfd = -1;

static void tear_down()
{
    if (close(sockfd) == -1)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
}

static void sigint_handler(int _)
{
    tear_down();
    exit(EXIT_SUCCESS);
}

int feedback(const struct sockaddr target_addr, const int32_t id,
             const uint8_t delay)
{
    sleep(delay);

    uint8_t message[MESSAGE_LEN];
    encode_message(id, delay, message);
    if (sendto(sockfd, message, MESSAGE_LEN, 0, &target_addr,
               sizeof(target_addr)) < 0)
    {
        perror("sendto");
        return -1;
    }
    return 0;
}

int run()
{
    uint8_t message[MESSAGE_LEN];
    struct sockaddr originating_addr;
    socklen_t originating_addr_len = sizeof(originating_addr);

    while (1)
    {
        const ssize_t message_len =
            recvfrom(sockfd, message, sizeof(message), 0, &originating_addr,
                     &originating_addr_len);

        if (message_len == -1)
        {
            perror("recvfrom");
            return -1;
        }

        if (message_len == 0) continue;

        int32_t id;
        uint8_t delay;
        decode_message(message, &id, &delay);

        if (sanitize_paramter(id) == -1 || sanitize_paramter(delay) == -1)
        {
            fprintf(stderr, "invalid message received: id=%d, delay=%hu. %s\n",
                    id, delay, PARAMTER_RESTRICTION_MSG);
            continue;
        }

        if (delay == 99) return 0;

        fflush(stdout);
        const pid_t pid = fork();
        if (pid == 0) // child process
            feedback(originating_addr, id, delay);
    }

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
    if ((status = build_addrinfo(info, ip, port, SOCK_DGRAM)) != 0)
        return status;
    return 0;
}

int main(int argc, char *argv[])
{
    const struct sigaction sigint_action = {.sa_handler = sigint_handler};
    sigaction(SIGINT, &sigint_action, NULL);

    struct addrinfo *info;
    if (parse_arg(argc, argv, &info) != 0) return -1;

    if ((sockfd = create_socket_with_first_usable_addr(info)) == -1) return -1;

    if (bind_socket_with_first_usable_addr(info, sockfd) == -1)
    {
        if (close(sockfd) == -1) perror("close");
        return -1;
    }

    int status = run();

    freeaddrinfo(info);
    if (close(sockfd) == -1)
    {
        perror("close");
        return -1;
    }
    return status;
}