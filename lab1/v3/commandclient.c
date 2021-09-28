#include "../lib/fifo_info.h"
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define COMMAND_SIZE PIPE_BUF * 2

char client_fifo_name[100];

void tear_down() { unlink(client_fifo_name); }

static void sigint_handler(int _)
{
    tear_down();
    exit(EXIT_SUCCESS);
}

void get_client_fifo_name(char *name, size_t size)
{
    pid_t pid = getpid();
    snprintf(name, size, "%s%d", CLIENT_FIFO_NAME_PREFIX, pid);
}

int start_client()
{
    while (1)
    {
        // send command to server
        char command[COMMAND_SIZE];
        fprintf(stdout, "> ");
        if (!fgets(command, sizeof(command), stdin)) break;

        char prefixed_command[COMMAND_SIZE + 100];
        snprintf(prefixed_command, sizeof(prefixed_command), "%d\n%s", getpid(),
                 command);

        size_t len = strlen(prefixed_command) + 1;

        int server_fifo_fd = open(SERVER_FIFO_NAME, O_WRONLY);
        if (server_fifo_fd == -1)
        {
            fprintf(stderr, "Cannot open FIFO: %s\n", SERVER_FIFO_NAME);
            return -1;
        }

        if (len * sizeof(char) <= PIPE_BUF)
            if (write(server_fifo_fd, prefixed_command, len * sizeof(char)) ==
                -1)
            {
                perror("write");
                return -1;
            }
            else
                fprintf(stderr, "Command length too long.\n");

        if (close(server_fifo_fd) == -1)
        {
            perror("close");
            return -1;
        }

        // read result from server
        int client_fifo_fd = open(client_fifo_name, O_RDONLY);
        if (client_fifo_fd == -1) return -1;

        char buf[PIPE_BUF];
        ssize_t result_len;
        while (result_len = read(client_fifo_fd, buf, sizeof(buf) - 1),
               result_len > 0)
        {
            buf[result_len] = '\0';
            fprintf(stdout, "%s", buf);
        }

        if (close(client_fifo_fd) == -1)
        {
            perror("close");
            return -1;
        }

        if (result_len == -1) return -1;
    }
    return 0;
}

int main()
{
    struct sigaction sigint_action = {.sa_handler = sigint_handler};
    sigaction(SIGINT, &sigint_action, NULL);

    get_client_fifo_name(client_fifo_name, sizeof(client_fifo_name));

    if (mkfifo(client_fifo_name, S_IRUSR | S_IWUSR | S_IWGRP | S_IWOTH) == -1)
        return -1;

    int status = start_client();

    tear_down();

    return status;
}
