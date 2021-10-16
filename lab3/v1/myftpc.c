#include "arg_checkers.h"
#include "socket_utils.h"
#include <netdb.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define REQUIRED_ARGC 6

int sockfd = -1;

void tear_down()
{
    if (sockfd != -1 && close(sockfd) == -1)
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

typedef struct
{
    char *filename;
    uint16_t secret_key;
    unsigned long blocksize_byte;
} Config;

int parse_args(int argc, char *argv[], struct addrinfo **server_info,
               Config *const config)
{
    if (argc < REQUIRED_ARGC)
    {
        fprintf(stderr,
                "Usage: %s <server-ip> <server-port> <filename> <secret-key> "
                "<blocksize-byte>\n",
                argv[0]);
        return -1;
    }

    int status;

    if ((status = build_addrinfo(server_info, argv[1], argv[2], SOCK_STREAM)) !=
        0)
        return status;

    config->filename = argv[3];
    if ((status = check_filename(config->filename)) != 0) return status;

    long long secret_key = strtoull(argv[4], NULL, 0);
    if ((status = check_secret_key(secret_key)) != 0) return status;
    config->secret_key = (uint16_t)secret_key;

    config->blocksize_byte = strtoul(argv[5], NULL, 0);

    return 0;
}

int run(const struct addrinfo *const server_info, const Config *const config)
{
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

    int status = run(server_info, &config);
    return status;
}