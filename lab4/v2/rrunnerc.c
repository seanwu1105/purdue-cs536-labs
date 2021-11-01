#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "../lib/arg_checkers.h"
#include "../lib/packet_codec.h"
#include "../lib/request_codec.h"
#include "../lib/roadrunner_client.h"
#include "../lib/socket_utils.h"

#define REQUIRED_ARGC 7

int sockfd = -1;

void tear_down()
{
    close(sockfd);
    sockfd = -1;
}

static void sigint_handler(int _)
{
    tear_down();
    exit(EXIT_SUCCESS);
}

static void sigalrm_handler(int _) { return; }

int parse_args(int argc, char *argv[], struct addrinfo **server_info,
               Config *const config, uint16_t *const secret_key)
{
    if (argc < REQUIRED_ARGC)
    {
        fprintf(stderr,
                "Usage: %s <server_ip> <server_port> <filename> "
                "<secret_key> <blocksize> <windowsize>\n",
                argv[0]);
        return -1;
    }

    int status;

    if ((status = build_addrinfo(server_info, argv[1], argv[2], SOCK_DGRAM)) !=
        0)
        return status;

    config->filename = argv[3];
    if ((status = check_filename(config->filename)) != 0) return status;

    long long s_key = strtoull(argv[4], NULL, 0);
    if ((status = check_secret_key(s_key)) != 0) return status;
    *secret_key = (uint16_t)s_key;

    long long blocksize = strtoull(argv[5], NULL, 0);
    if ((status = check_blocksize(blocksize)) != 0) return status;
    config->blocksize = (uint16_t)blocksize;

    long long windowsize = strtoull(argv[6], NULL, 0);
    if ((status = check_windowsize(windowsize)) != 0) return status;
    config->windowsize = (uint8_t)windowsize;

    return 0;
}

int request_file(const struct addrinfo *const server_info,
                 const Config *const config, const uint16_t secret_key)
{
    uint8_t request[REQUEST_SIZE_WITH_CERTIFICATION];
    encode_request_with_certification(config->filename, secret_key, request);
    if (sendto(sockfd, request, sizeof(request), 0, server_info->ai_addr,
               server_info->ai_addrlen) < 0)
    {
        perror("sendto");
        return -1;
    }

    return 0;
}

int run(const struct addrinfo *const server_info, const Config *const config,
        const uint16_t secret_key)
{
    // Check if file exists
    if (access(config->filename, F_OK) == 0)
    {
        fprintf(stdout, "File already exists: %s\n", config->filename);
        return -1;
    }

    // Request file
    while (1)
    {
        request_file(server_info, config, secret_key);
        // Start request timeout timer
        setitimer(ITIMER_REAL,
                  &(struct itimerval){{0, FILE_REQUEST_TIMEOUT_MS * 1000},
                                      {0, FILE_REQUEST_TIMEOUT_MS * 1000}},
                  NULL);
        if (receive_file_and_cancel_timeout(sockfd, config) == 0)
            break;
        else if (errno != EINTR)
            return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    const struct sigaction sigint_action = {.sa_handler = sigint_handler};
    sigaction(SIGINT, &sigint_action, NULL);
    const struct sigaction sigalrm_action = {.sa_handler = sigalrm_handler};
    sigaction(SIGALRM, &sigalrm_action, NULL);

    struct addrinfo *server_info;
    Config config;
    uint16_t secret_key;
    if (parse_args(argc, argv, &server_info, &config, &secret_key) != 0)
        return -1;

    if ((sockfd = create_socket_with_first_usable_addr(server_info)) == -1)
        return -1;

    int status = run(server_info, &config, secret_key);

    tear_down();

    return status;
}