#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "audiocli.h"
#include "parameter_checkers.h"
#include "request_codec.h"
#include "socket_utils.h"

static int sockfd = -1;

int main(int argc, char *argv[])
{
    const struct sigaction sigint_action = {.sa_handler = sigint_handler};
    sigaction(SIGINT, &sigint_action, NULL);

    Config config;
    if (parse_args(argc, argv, &config) != 0) return 1;

    if ((sockfd = create_socket_with_first_usable_addr(config.server_info)) ==
        -1)
        return -1;

    int status = run(&config);

    tear_down();

    return status;
}

static void sigint_handler(int _)
{
    tear_down();
    _exit(EXIT_SUCCESS);
}

static void tear_down()
{
    close(sockfd);
    sockfd = -1;
}

static int parse_args(int argc, char **argv, Config *config)
{
    if (argc < REQUIRED_ARGC)
    {
        fprintf(stderr,
                "Usage: %s <server-ip> <server-port> <audio-filename> "
                "<blocksize> <buffer-size> <target-buffer-occupancy> "
                "<packets-per-seconds> <method=[0(C)|1(D)]> <log-filename>\n",
                argv[0]);
        return -1;
    }

    if (build_addrinfo(&config->server_info, argv[1], argv[2], SOCK_DGRAM) != 0)
        return -1;

    config->audio_filename = argv[3];
    if (check_filename(config->audio_filename) != 0) return -1;

    unsigned long long blocksize = strtoull(argv[4], NULL, 0);
    if (check_blocksize(blocksize) != 0) return -1;
    config->blocksize = (uint16_t)blocksize;

    config->buffer_size = strtoull(argv[5], NULL, 0);
    config->target_buffer_occupancy = strtoull(argv[6], NULL, 0);
    config->packets_per_second = strtoull(argv[7], NULL, 0);
    config->method = (unsigned short)strtoul(argv[8], NULL, 0);
    config->log_filename = argv[9];

    return 0;
}

static int run(Config *config)
{
    while (1)
    {
        request_file(config);
        // Start request timeout timer
        setitimer(ITIMER_REAL,
                  &(struct itimerval){{0, FILE_REQUEST_TIMEOUT_MS * 1000},
                                      {0, FILE_REQUEST_TIMEOUT_MS * 1000}},
                  NULL);
        if (stream_file_and_cancel_timeout(sockfd, config) == 0)
            break;
        else if (errno != EINTR)
            return -1;
    }
    return 0;
}

static int request_file(const Config *const config)
{
    uint8_t request[REQUEST_SIZE];
    encode_request(config->audio_filename, config->blocksize, request);
    if (sendto(sockfd, request, sizeof(request), 0,
               config->server_info->ai_addr,
               config->server_info->ai_addrlen) < 0)
    {
        perror("sendto");
        return -1;
    }
    return 0;
}

static int stream_file_and_cancel_timeout(int sockfd,
                                          const Config *const config)
{

    return 0;
}