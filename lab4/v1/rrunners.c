#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "../lib/arg_checkers.h"
#include "../lib/packet_codec.h"
#include "../lib/request_codec.h"
#include "../lib/roadrunner_server.h"
#include "../lib/socket_utils.h"

#define REQUIRED_ARGC 7

int request_sockfd = -1;
int packet_sockfd = -1;

void tear_down()
{
    close(request_sockfd);
    close(packet_sockfd);
    request_sockfd = -1;
    packet_sockfd = -1;
}

static void sigint_handler(int _)
{
    tear_down();
    exit(EXIT_SUCCESS);
}

static void sigalrm_handler(int _)
{
    fprintf(stdout, "Timeout\n");
    return;
}

int parse_args(int argc, char *argv[], struct addrinfo **server_info,
               Config *const config, uint16_t *const secret_key)
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

    long long s_key = strtoull(argv[3], NULL, 0);
    if ((status = check_secret_key(s_key)) != 0) return status;
    *secret_key = (uint16_t)s_key;

    long long blocksize = strtoull(argv[4], NULL, 0);
    if ((status = check_blocksize(blocksize)) != 0) return status;
    config->blocksize = (uint16_t)blocksize;

    long long windowsize = strtoull(argv[5], NULL, 0);
    if ((status = check_windowsize(windowsize)) != 0) return status;
    config->windowsize = (uint8_t)windowsize;

    long long timeout_usec = strtoull(argv[6], NULL, 0);
    if (timeout_usec < 0)
    {
        fprintf(stderr, "Timeout must be non-negative\n");
        return -1;
    }
    config->timeout_usec = timeout_usec;

    return 0;
}

int read_request(char *const filename, uint16_t *const secret_key,
                 struct sockaddr *const client_addr,
                 socklen_t *const client_addr_len)
{
    uint8_t request[REQUEST_SIZE_WITH_SECRET_KEY];
    const ssize_t bytes_read =
        recvfrom(request_sockfd, request, REQUEST_SIZE_WITH_SECRET_KEY, 0,
                 client_addr, client_addr_len);
    if (bytes_read == -1)
    {
        perror("read");
        return -1;
    }

    decode_request_with_secret_key(request, filename, secret_key);
    return 0;
}

int run(const Config *const config, const uint16_t secret_key)
{
    while (1)
    {
        // Accept file request
        uint16_t received_secret_key;
        char filename[MAX_FILENAME_LEN + 1];
        struct sockaddr client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        if (read_request(filename, &received_secret_key, &client_addr,
                         &client_addr_len) < 0)
            continue;

        if (secret_key != received_secret_key)
        {
            fprintf(stderr, "Secret key mismatch: %hu != %hu\n", secret_key,
                    received_secret_key);
            continue;
        }

        if (check_filename(filename) != 0) continue;

        // Check if file exists
        if (access(filename, F_OK) == -1)
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
            if (send_file(&packet_sockfd, filename, config, &client_addr,
                          client_addr_len, NULL) < 0)
                exit(EXIT_FAILURE);
            exit(EXIT_SUCCESS);
        }
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

    if ((request_sockfd = create_socket_with_first_usable_addr(server_info)) ==
        -1)
        return -1;

    if (bind_socket_with_first_usable_addr(server_info, request_sockfd) == -1)
    {
        close(request_sockfd);
        return -1;
    }

    return run(&config, secret_key);
}