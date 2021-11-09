#include <netdb.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "audiosrv.h"
#include "parameter_checkers.h"
#include "request_codec.h"
#include "socket_utils.h"

static int request_sockfd = -1;
static int packet_sockfd = -1;

int main(int argc, char *argv[])
{
    const struct sigaction sigint_action = {.sa_handler = sigint_handler};
    sigaction(SIGINT, &sigint_action, NULL);

    Config config;
    if (parse_args(argc, argv, &config) != 0) return -1;

    if ((request_sockfd =
             create_socket_with_first_usable_addr(config.server_info)) == -1)
        return -1;

    if (bind_socket_with_first_usable_addr(config.server_info,
                                           request_sockfd) != 0)
    {
        tear_down();
        return -1;
    }

    return run(&config);
}

static void sigint_handler(int _)
{
    tear_down();
    exit(EXIT_SUCCESS);
}

static void tear_down()
{
    close(request_sockfd);
    request_sockfd = -1;
    close(packet_sockfd);
    packet_sockfd = -1;
}

static int parse_args(int argc, char **argv, Config *config)
{
    if (argc < REQUIRED_ARGC)
    {
        fprintf(stderr,
                "Usage: %s <server-ip> <server-port>"
                " <packets-per-second> <log-filename>\n",
                argv[0]);
        return -1;
    }

    if (build_addrinfo(&(config->server_info), argv[1], argv[2], SOCK_DGRAM) !=
        0)
        return -1;

    config->packets_per_second = strtoull(argv[3], NULL, 0);
    config->log_filename = argv[4];

    return 0;
}

static int run(Config *config)
{
    uint16_t blocksize;
    char filename[MAX_FILENAME_LEN + 1];
    struct sockaddr client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    while (1)
    {
        if (read_request(filename, &blocksize, &client_addr, &client_addr_len) <
            0)
            continue;

        if (check_filename(filename) != 0) continue;

        if (access(filename, F_OK) != 0)
        {
            perror("access");
            continue;
        }

        fflush(stdout);
        const pid_t pid = fork();
        if (pid == 0)
        {
            // Child process
            close(request_sockfd);
            printf("child exit!\n");
            exit(EXIT_SUCCESS);
        }
    }

    return 0;
}

static int read_request(char *const filename, uint16_t *const blocksize,
                        struct sockaddr *const client_addr,
                        socklen_t *const client_addr_len)
{
    uint8_t request[REQUEST_SIZE];
    const ssize_t bytes_read = recvfrom(request_sockfd, request, REQUEST_SIZE,
                                        0, client_addr, client_addr_len);
    if (bytes_read == -1)
    {
        perror("read");
        return -1;
    }

    decode_request(request, filename, blocksize);
    return 0;
}