#include "../lib/socket_utils.h"
#include "parse_addrinfo_arg.h"
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define REQUIRED_ARGC 3
#define BUFFER_SIZE 1024
#define TIMEOUT_SEC 2
#define MAX_ATTEMPTS 3
#define ERR_GIVE_UP -2

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

int attempt_write_and_read(const char *command)
{
    for (size_t attempt = 0; attempt < MAX_ATTEMPTS; ++attempt)
    {
        // write command to server
        if (write(sockfd, command, (strlen(command) + 1) * sizeof(char)) == -1)
        {
            perror("write");
            return -1;
        }

        // set timeout alarm
        alarm(TIMEOUT_SEC);

        // read response from server
        char buf[BUFFER_SIZE];
        ssize_t result_len;
        while (result_len = read(sockfd, buf, sizeof(buf) - 1), result_len > 0)
        {
            alarm(0); // cancel timeout alarm
            buf[result_len] = '\0';
            fprintf(stdout, "%s", buf);
        }

        if (result_len == 0) return 0;

        if (result_len == -1)
        {
            if (errno == EINTR)
            {
                // printf("timeout\n");
                continue;
            }
            return -1;
        }
    }

    return ERR_GIVE_UP;
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
            return -1;

        // connect to server
        if (connect(sockfd, server_info->ai_addr, server_info->ai_addrlen) ==
            -1)
        {
            perror("connect");
            return -1;
        }

        int status = attempt_write_and_read(command);

        // close socket
        if (close(sockfd) == -1)
        {
            perror("close");
            return -1;
        }
        sockfd = -1;

        if (status == -1) return -1;
        if (status == ERR_GIVE_UP)
            fprintf(stderr, "after %d attempts failed: giving up\n",
                    MAX_ATTEMPTS);
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
    if (parse_addrinfo_arg(argc, argv, &server_info) != 0) return -1;

    int status = run(server_info);

    return status;
}
