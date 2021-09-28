// Simple shell example using fork() and execlp().

#include "../lib/socket_utils.h"
#include "parse_addrinfo_arg.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define MAX_LISTEN_NUM 5

int sockfd_half = -1;

void tear_down()
{
    if (close(sockfd_half) == -1)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
}

int run()
{
    int status;

    while (1)
    {
        // accept connection
        int sockfd_full = -1;
        struct sockaddr client_addr;
        socklen_t client_addr_len;
        if ((sockfd_full =
                 accept(sockfd_half, &client_addr, &client_addr_len)) == -1)
        {
            perror("accept");
            return -1;
        }

        // TODO: Toss coin to ignore or not ignore the command

        // TODO: check source address

        // read command
        char command[BUFFER_SIZE];
        ssize_t command_len =
            read(sockfd_full, command, BUFFER_SIZE * sizeof(char));
        if (command_len == -1)
        {
            perror("read");
            return -1;
        }
        else if (command_len == 0) // EOF
            continue;

        command[command_len] = '\0';

        // only allow `date` and `/bin/date`
        if (strncmp(command, "date", 4) != 0 &&
            strncmp(command, "/bin/date", 9) != 0)
        {
            fprintf(stderr, "command not allowed: %s\n", command);
            continue;
        }

        fflush(stdout); // flush stdout before forking
        pid_t k = fork();
        if (k == 0)
        {
            // child code

            // redirect stdout and stderr to socket (full)
            if (dup2(sockfd_full, STDOUT_FILENO) == -1 ||
                dup2(sockfd_full, STDERR_FILENO) == -1)
            {
                perror("dup2");
                exit(EXIT_FAILURE);
            }

            // execute command
            char *argv[] = {strdup(command), NULL};
            if (execvp(command, argv) == -1)
            {
                fprintf(stderr, "command not found: %s", command);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            // parent code
            waitpid(k, &status, 0);

            // close socket (full)
            if (close(sockfd_full) == -1)
            {
                perror("close");
                return -1;
            }
        }
    }

    return 0;
}

static void sigint_handler(int _)
{
    tear_down();
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    struct sigaction sigint_action = {.sa_handler = sigint_handler};
    sigaction(SIGINT, &sigint_action, NULL);

    struct addrinfo *info;
    if (parse_addrinfo_arg(argc, argv, &info) != 0) return -1;

    // create half-associate socket
    if ((sockfd_half = create_socket_with_first_usable_addr(info)) == -1)
        return -1;

    // bind to the socket
    if ((bind_socket_with_first_usable_addr(info, sockfd_half)) == -1)
    {
        if (close(sockfd_half) == -1) perror("close");
        return -1;
    }

    // listen to the socket
    if ((listen(sockfd_half, MAX_LISTEN_NUM)) == -1)
    {
        if (close(sockfd_half) == -1) perror("close");
        return -1;
    }

    int status = run();

    tear_down();

    return status;
}
