#include "../lib/socket_utils.h"
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define REQUIRED_ARGC 3
#define BUFFER_SIZE 1024
// #define TIMEOUT_SEC 2
// #define MAX_ATTEMPTS 3

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

int run(const struct addrinfo *const server_info)
{
    while (1)
    {
        // send command to server
        char command[BUFFER_SIZE];
        fprintf(stdout, "> ");
        if (!fgets(command, sizeof(command), stdin)) break;

        // create socket
        if ((sockfd = create_socket_with_first_usable_addr(server_info)) == -1)
        {
            perror("socket");
            return -1;
        }

        // connect to server
        if (connect(sockfd, server_info->ai_addr, server_info->ai_addrlen) ==
            -1)
        {
            perror("connect");
            return -1;
        }

        // TODO: attempt to send command -> a nested loop

        // TODO: set timeout alarm

        // write command to server
        if (write(sockfd, command, (strlen(command) + 1) * sizeof(char)) == -1)
        {
            perror("write");
            return -1;
        }

        // read response from server
        char buf[BUFFER_SIZE];
        ssize_t result_len;
        while (result_len = read(sockfd, buf, sizeof(buf) - 1), result_len > 0)
        {
            buf[result_len] = '\0';
            fprintf(stdout, "%s", buf);
        }

        // close socket
        if (close(sockfd) == -1)
        {
            perror("close");
            return -1;
        }
        if (result_len == -1) return -1;
    }
    return 0;
}

int parse_arg(int argc, char *argv[], struct addrinfo **info)
{
    if (argc < REQUIRED_ARGC)
    {
        fprintf(stderr, "insufficient arguments: expect %d\n", REQUIRED_ARGC);
        return -1;
    }
    const char *const server_ip = argv[1];
    const char *const server_port = argv[2];

    int status;
    if ((status = build_addrinfo(info, server_ip, server_port)) != 0)
        return status;
    return 0;
}

int main(int argc, char *argv[])
{
    struct sigaction sigint_action = {.sa_handler = sigint_handler};
    sigaction(SIGINT, &sigint_action, NULL);

    struct addrinfo *server_info;
    if (parse_arg(argc, argv, &server_info) != 0) return -1;

    int status = run(server_info);

    return status;
}
