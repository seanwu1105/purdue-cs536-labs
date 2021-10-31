#include <netdb.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "arg_checkers.h"
#include "request_codec.h"
#include "socket_utils.h"

#define REQUIRED_ARGC 7

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
    unsigned long long timeout_microseconds;
} Config;

int parse_args(int argc, char *argv[], struct addrinfo **server_info,
               Config *const config)
{
    if (argc < REQUIRED_ARGC)
    {
        fprintf(stderr,
                "Usage: %s <server_ip> <server_port> <secret_key> <blocksize> "
                "<windowsize> <timeout>\n",
                argv[0]);
        return -1;
    }

    int status;

    if ((status = build_addrinfo(server_info, argv[1], argv[2], SOCK_DGRAM)) !=
        0)
        return status;

    long long secret_key = strtoull(argv[3], NULL, 0);
    if ((status = check_secret_key(secret_key)) != 0) return status;
    config->secret_key = (uint16_t)secret_key;

    long long blocksize = strtoull(argv[4], NULL, 0);
    if ((status = check_blocksize(blocksize)) != 0) return status;
    config->blocksize = (uint16_t)blocksize;

    long long windowsize = strtoull(argv[5], NULL, 0);
    if ((status = check_windowsize(windowsize)) != 0) return status;
    config->windowsize = (uint8_t)windowsize;

    long long timeout_microseconds = strtoull(argv[6], NULL, 0);
    if (timeout_microseconds < 0)
    {
        fprintf(stderr, "Timeout must be non-negative\n");
        return -1;
    }
    config->timeout_microseconds = timeout_microseconds;

    return 0;
}

int read_request(int sockfd, char *const filename, uint16_t *const secret_key,
                 struct sockaddr *const client_addr,
                 socklen_t *const client_addr_len)
{
    uint8_t request[REQUEST_SIZE];
    const ssize_t bytes_read = recvfrom(sockfd, request, REQUEST_SIZE, 0,
                                        client_addr, client_addr_len);
    if (bytes_read == -1)
    {
        perror("read");
        return -1;
    }

    decode_request(request, filename, secret_key);
    return 0;
}

int run(const struct addrinfo *const server_info, const Config *const config)
{
    while (1)
    {
        // Accept file request
        uint16_t secret_key;
        char filename[MAX_FILENAME_LEN];
        struct sockaddr client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        read_request(sockfd, filename, &secret_key, &client_addr,
                     &client_addr_len);

        printf("Received request for file %s\n", filename);
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

    if ((bind_socket_with_first_usable_addr(server_info, sockfd)) == -1)
    {
        close(sockfd);
        return -1;
    }

    return run(server_info, &config);
}