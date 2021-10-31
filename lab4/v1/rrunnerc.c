#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "arg_checkers.h"
#include "request_codec.h"
#include "socket_utils.h"

#define REQUIRED_ARGC 7
#define FILE_REQUEST_TIMEOUT_MS 500

int sockfd = -1;

void tear_down() { close(sockfd); }

static void sigint_handler(int _)
{
    tear_down();
    exit(EXIT_SUCCESS);
}

static void sigalrm_handler(int _) { return; }
typedef struct
{
    const char *filename;
    uint16_t secret_key;
    uint16_t blocksize; // <= 1471
    uint8_t windowsize; // <= 63
} Config;

int parse_args(int argc, char *argv[], struct addrinfo **server_info,
               Config *const config)
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

    long long secret_key = strtoull(argv[4], NULL, 0);
    if ((status = check_secret_key(secret_key)) != 0) return status;
    config->secret_key = (uint16_t)secret_key;

    long long blocksize = strtoull(argv[5], NULL, 0);
    if ((status = check_blocksize(blocksize)) != 0) return status;
    config->blocksize = (uint16_t)blocksize;

    long long windowsize = strtoull(argv[6], NULL, 0);
    if ((status = check_windowsize(windowsize)) != 0) return status;
    config->windowsize = (uint8_t)windowsize;

    return 0;
}

int request_file_with_timeout(const struct addrinfo *const server_info,
                              const Config *const config,
                              unsigned long timeout_ms)
{
    uint8_t request[REQUEST_SIZE];
    encode_request(config->filename, config->secret_key, request);
    if (sendto(sockfd, request, sizeof(request), 0, server_info->ai_addr,
               server_info->ai_addrlen) < 0)
    {
        perror("sendto");
        return -1;
    }

    setitimer(
        ITIMER_REAL,
        &(struct itimerval){{0, timeout_ms * 1000}, {0, timeout_ms * 1000}},
        NULL);
    return 0;
}

int receive_file_and_cancel_timeout(const Config *const config)
{
    uint8_t buffer[config->blocksize];
    ssize_t bytes_read;
    while ((bytes_read =
                recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL)) >= 0)
    {
        setitimer(ITIMER_REAL, 0, NULL);
        printf("%ld\n", bytes_read);

        if (bytes_read != config->blocksize)
        {
            break;
        }
    }

    if (bytes_read < 0)
    {
        if (errno != EINTR) perror("recvfrom");
        return -1;
    }

    return 0;
}

int run(const struct addrinfo *const server_info, const Config *const config)
{
    // TODO: Check if file exists
    // if (access(config->filename, F_OK) == 0)
    // {
    //     fprintf(stdout, "File already exists: %s\n", config->filename);
    //     return -1;
    // }

    // Request file
    while (1)
    {
        request_file_with_timeout(server_info, config, FILE_REQUEST_TIMEOUT_MS);
        printf("Requested file %s\n", config->filename);
        receive_file_and_cancel_timeout(config);
        printf("Received file %s\n", config->filename);
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
    if (parse_args(argc, argv, &server_info, &config) != 0) return -1;

    if ((sockfd = create_socket_with_first_usable_addr(server_info)) == -1)
        return -1;

    int status = run(server_info, &config);

    tear_down();

    return status;
}