// Simple shell example using fork() and execlp().

#include "../lib/fifo_info.h"
#include "../lib/parse_command.h"
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

int fd = -1;

void tear_down()
{
    close(fd);
    unlink(SERVER_FIFO_NAME);
}

static void sigint_handler(int _)
{
    tear_down();
    _exit(EXIT_SUCCESS);
}

int start_server()
{
    char buf[PIPE_BUF];
    int status;

    while (1)
    {
        // read command from FIFO
        const ssize_t command_len = read(fd, buf, PIPE_BUF);
        if (command_len == -1)
            return -1;
        else if (command_len == 0) // EOF
            continue;

        // split commands from buf with '\n' as delimiter
        char command[PIPE_BUF];
        size_t buf_idx = 0, command_idx = 0;
        while (buf_idx < command_len)
        {
            command_idx = 0;
            while (buf_idx < command_len && buf[buf_idx] != '\n')
                if (buf[buf_idx] == '\0') // ignore '\0' as we use '\n' instead
                    buf_idx++;
                else
                    command[command_idx++] = buf[buf_idx++];

            buf_idx++; // increase buf_idx to ignore '\n'

            if (command_idx == 0) // empty command
                continue;

            // close command string as we ignore it before
            command[command_idx] = '\0';

            // print prompt
            fprintf(stdout, "[%d]$ ", getpid());

            char *arguments[PIPE_BUF];
            parse_command(command, arguments);

            fflush(stdout); // flush stdout before forking
            const pid_t k = fork();
            if (k == 0)
            {
                // child code
                int result = execvp(arguments[0], arguments);

                if (result == -1) // if execution failed, terminate child
                {
                    fprintf(stderr, "command not found: %s\n", command);
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                // parent code
                waitpid(k, &status, 0);
            }
        }
    }

    return 0;
}

int main()
{
    const struct sigaction sigint_action = {.sa_handler = sigint_handler};
    sigaction(SIGINT, &sigint_action, NULL);

    if (mkfifo(SERVER_FIFO_NAME, S_IRUSR | S_IWUSR | S_IWGRP | S_IWOTH) == -1)
        return -1;
    fd = open(SERVER_FIFO_NAME, O_RDONLY);
    if (fd == -1)
    {
        unlink(SERVER_FIFO_NAME);
        return -1;
    }

    int status = start_server();

    tear_down();

    return status;
}
