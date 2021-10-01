#include "../lib/socket_utils.h"
#include "message_codec.h"
#include "read_config.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define REQUIRED_ARGC 4

int sockfd = -1;

void tear_down()
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

static void sigalrm_handler(int _) { return; }

int send_pinging(const struct sockaddr *target_addr, const int32_t id,
                 const uint8_t delay)
{
    uint8_t message[MESSAGE_LEN];
    encode_message(id, delay, message);
    if (sendto(sockfd, message, sizeof(message), 0, target_addr,
               sizeof(*target_addr)) == -1)
    {
        perror("sendto");
        return -1;
    }
    return 0;
}

int receive_feedback(int32_t *const id)
{
    uint8_t message[MESSAGE_LEN];

    const ssize_t message_len =
        recvfrom(sockfd, message, sizeof(message), 0, NULL, NULL);

    if (message_len == -1)
    {
        if (errno != EINTR) perror("recvfrom");
        return -1;
    }

    if (message_len == 0) return -2;

    uint8_t delay;
    decode_message(message, id, &delay);
    return 0;
}

int ping(const struct sockaddr *target_addr, const int32_t id,
         const Config config)
{
    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    fprintf(stdout, "send MID: %d\t", id);
    fflush(stdout);
    if (send_pinging(target_addr, id, config.server_delay) == -1) return -1;

    int32_t feedback_id;
    alarm(config.timeout); // start timeout timer
    while (1)
    {
        if (receive_feedback(&feedback_id) == -1)
        {
            if (errno == EINTR) // timeout
            {
                fprintf(stdout, "timeout\n");
                break;
            }
            return -1;
        }
        if (feedback_id == id)
        {
            alarm(0); // cancel timeout timer
            struct timeval end_time;
            gettimeofday(&end_time, NULL);
            fprintf(stdout, "RTT: %ld\n",
                    (end_time.tv_sec - start_time.tv_sec) * 1000 +
                        (end_time.tv_usec - start_time.tv_usec) / 1000);
            break;
        }
    }

    return 0;
}

int run(const struct sockaddr *target_addr, const Config config)
{
    for (size_t i = 0; i < config.num_packages; i++)
        if (ping(target_addr, config.first_sequence_num + (int32_t)i, config) ==
            -1)
            return -1;

    return 0;
}

int parse_arg(int argc, char *argv[], struct addrinfo **client_info,
              struct addrinfo **server_info)
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

int main(int argc, char *argv[])
{
    struct sigaction sigint_action = {.sa_handler = sigint_handler};
    sigaction(SIGINT, &sigint_action, NULL);
    struct sigaction sigalrm_action = {.sa_handler = sigalrm_handler};
    sigaction(SIGALRM, &sigalrm_action, NULL);

    Config config = {};
    struct addrinfo *client_info;
    struct addrinfo *server_info;
    if (parse_arg(argc, argv, &client_info, &server_info) != 0 ||
        read_config(&config) == -1)
        return -1;

    if ((sockfd = create_socket_with_first_usable_addr(server_info)) == -1)
        return -1;

    if (bind_socket_with_first_usable_addr(client_info, sockfd) == -1)
    {
        if (close(sockfd) == -1) perror("close");
        return -1;
    }

    run(server_info->ai_addr, config);

    freeaddrinfo(client_info);
    freeaddrinfo(server_info);
    close(sockfd);
    return 0;
}