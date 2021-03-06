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
#include "../lib/bbcodec.h"
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
    _exit(EXIT_SUCCESS);
}

static void sigalrm_handler(int _)
{
    fprintf(stdout, "Timeout\n"); // Unsafe to use stdout actually.
    return;
}

int parse_args(int argc, char *argv[], struct addrinfo **server_info,
               Config *const config, uint32_t *const private_key)
{
    if (argc < REQUIRED_ARGC)
    {
        fprintf(stderr,
                "Usage: %s <server_ip> <server_port> <filename> "
                "<private_key> <blocksize> <windowsize>\n",
                argv[0]);
        return -1;
    }

    int status;

    if ((status = build_addrinfo(server_info, argv[1], argv[2], SOCK_DGRAM)) !=
        0)
        return status;

    config->filename = argv[3];
    if ((status = check_filename(config->filename)) != 0) return status;

    unsigned long long prikey = strtoull(argv[4], NULL, 0);
    if (prikey > UINT32_MAX)
    {
        fprintf(stderr, "Private key is too large\n");
        return -1;
    }
    *private_key = (uint32_t)prikey;

    unsigned long long blocksize = strtoull(argv[5], NULL, 0);
    if ((status = check_blocksize(blocksize)) != 0) return status;
    config->blocksize = (uint16_t)blocksize;

    unsigned long long windowsize = strtoull(argv[6], NULL, 0);
    if ((status = check_windowsize(windowsize)) != 0) return status;
    config->windowsize = (uint8_t)windowsize;

    return 0;
}

int request_file(const struct addrinfo *const server_info,
                 const Config *const config, const uint32_t private_key)
{
    uint32_t ip;
    get_udp_host_ip(server_info, &ip);
    uint32_t certificate = bbdecode(ip, private_key);

    uint8_t request[REQUEST_SIZE_WITH_CERTIFICATE];
    encode_request_with_certificate(config->filename, certificate, request);
    if (sendto(sockfd, request, sizeof(request), 0, server_info->ai_addr,
               server_info->ai_addrlen) < 0)
    {
        perror("sendto");
        return -1;
    }

    return 0;
}

int run(const struct addrinfo *const server_info, const Config *const config,
        const uint32_t private_key)
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
        request_file(server_info, config, private_key);
        // Start request timeout timer
        setitimer(ITIMER_REAL,
                  &(struct itimerval){{0, FILE_REQUEST_TIMEOUT_MS * 1000},
                                      {0, FILE_REQUEST_TIMEOUT_MS * 1000}},
                  NULL);
        if (receive_file_and_cancel_timeout(sockfd, config, &private_key) == 0)
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
    uint32_t private_key;
    if (parse_args(argc, argv, &server_info, &config, &private_key) != 0)
        return -1;

    if ((sockfd = create_socket_with_first_usable_addr(server_info)) == -1)
        return -1;

    int status = run(server_info, &config, private_key);

    tear_down();

    return status;
}