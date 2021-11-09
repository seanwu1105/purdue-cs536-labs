#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "audiosrv.h"
#include "socket_utils.h"

static int sockfd = -1;

int main(int argc, char *argv[])
{
    const struct sigaction sigint_action = {.sa_handler = sigint_handler};
    sigaction(SIGINT, &sigint_action, NULL);

    Config config;
    if (parse_args(argc, argv, &config) != 0) return -1;
    return 0;
}

static void sigint_handler(int _)
{
    tear_down();
    exit(EXIT_SUCCESS);
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