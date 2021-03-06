// Simple shell example using fork() and execlp().

#include "../lib/socket_utils.h"
#include "parse_addrinfo_arg.h"
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define MAX_LISTEN_NUM 5

int sockfd_half = -1;
int sockfd_full = -1;

void tear_down()
{
    close(sockfd_half);
    close(sockfd_full);
}

int sanitize_client_addr(const struct sockaddr *const addr)
{
    char ipstr[INET_ADDRSTRLEN];
    if (!inet_ntop(addr->sa_family, &((struct sockaddr_in *)addr)->sin_addr,
                   ipstr, sizeof(ipstr)))
    {
        perror("inet_ntop");
        return -1;
    }

    // only allow 128.10.25.* or 128.10.112.*
    const char *const allowed_ips[] = {"128.10.25.", "128.10.112."};
    for (size_t i = 0; i < sizeof(allowed_ips) / sizeof(allowed_ips[0]); i++)
        if (strncmp(ipstr, allowed_ips[i], strlen(allowed_ips[i])) == 0)
            return 0;

    fprintf(stderr, "client address denied: %s\n", ipstr);
    return -1;
}

int sanitize_command(const char *const command)
{
    // only allow `date` and `/bin/date`
    const char *const allowed_commands[] = {"date", "/bin/date"};

    for (size_t i = 0; i < sizeof(allowed_commands) / sizeof(char *); i++)
        if (strncmp(command, allowed_commands[i],
                    strlen(allowed_commands[i])) == 0)
            return 0;
    fprintf(stderr, "command not allowed: %s", command);
    return -1;
}

void remove_newline(char *const command)
{
    const int len = strlen(command);
    if (command[len - 1] == '\n') command[len - 1] = '\0';
}

ssize_t read_command(const int sockfd_full, char *command)
{
    const ssize_t command_len =
        read(sockfd_full, command, BUFFER_SIZE * sizeof(char));
    if (command_len == -1)
    {
        perror("read");
        return -1;
    }

    command[command_len] = '\0';

    return command_len;
}

int run()
{
    while (1)
    {
        // accept connection
        struct sockaddr client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        if ((sockfd_full =
                 accept(sockfd_half, &client_addr, &client_addr_len)) == -1)
        {
            perror("accept");
            continue;
        }

        if (sanitize_client_addr(&client_addr) == -1) continue;

        char command[BUFFER_SIZE];
        ssize_t command_len;
        while ((command_len = read_command(sockfd_full, command)) > 0)
        {
            int toss_coin = rand() % 2;
            if (toss_coin == 0) // ignore command
                continue;
            else
                break;
        }

        if (command_len == -1) return -1;
        if (command_len == 0) continue;

        if (sanitize_command(command) == -1) continue;

        remove_newline(command);

        fflush(stdout); // flush stdout before forking
        const pid_t k = fork();
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
            char *const argv[] = {strdup(command), NULL};
            if (execvp(command, argv) == -1)
            {
                fprintf(stderr, "command not found: %s", command);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            // parent code
            int status;
            if (waitpid(k, &status, 0) == -1) perror("waitpid");

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
    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    const struct sigaction sigint_action = {.sa_handler = sigint_handler};
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
        perror("listen");
        if (close(sockfd_half) == -1) perror("close");
        return -1;
    }

    int status = run();

    tear_down();

    return status;
}
