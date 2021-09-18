#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include "fifo_info.h"
#include "../lib/fifo_info.h"

#define COMMAND_SIZE PIPE_BUF * 2

int client_fifo_fd;
char client_fifo_name[100];

void tear_down()
{
    close(client_fifo_fd);
    unlink(client_fifo_name);
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
        int server_fifo_fd = open(SERVER_FIFO_NAME, O_WRONLY);
        if (server_fifo_fd == -1)
        {
            fprintf(stderr, "Cannot open FIFO: %s\n", SERVER_FIFO_NAME);
            return -1;
        }
        char command[COMMAND_SIZE];
        fprintf(stdout, "> ");
        if (!fgets(command, sizeof(command), stdin))
            break;

        char prefixed_command[COMMAND_SIZE + 100];
        snprintf(prefixed_command, sizeof(prefixed_command), "%d\n%s", getpid(), command);

        size_t len = strlen(prefixed_command) + 1;

        if (len * sizeof(char) <= PIPE_BUF)
            write(server_fifo_fd, command, len);
        else
            fprintf(stderr, "Command length too long.\n");
        close(server_fifo_fd);
    }
}

void signal_handler(int _)
{
    tear_down();
    exit(EXIT_SUCCESS);
}

int main()
{
    get_client_fifo_name(client_fifo_name, sizeof(client_fifo_name));

    if (mkfifo(client_fifo_name, S_IRUSR | S_IWUSR | S_IWGRP | S_IWOTH) == -1)
        return -1;
    client_fifo_fd = open(client_fifo_name, O_RDONLY);
    if (client_fifo_fd == -1)
    {
        close(client_fifo_fd);
        unlink(client_fifo_name);
        return -1;
    }

    signal(SIGINT, signal_handler);

    int status = start_client();

    tear_down();

    return status;
}
