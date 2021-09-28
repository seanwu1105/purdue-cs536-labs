#include "../lib/fifo_info.h"
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main()
{
    while (1)
    {
        int fd = open(SERVER_FIFO_NAME, O_WRONLY);
        if (fd == -1)
        {
            fprintf(stderr, "Cannot open FIFO: %s\n", SERVER_FIFO_NAME);
            return -1;
        }
        char command[PIPE_BUF * 2];
        fprintf(stdout, "> ");
        if (!fgets(command, sizeof(command), stdin)) break;
        size_t len = strlen(command) + 1;
        if (len * sizeof(char) <= PIPE_BUF)
        {
            if (write(fd, command, len * sizeof(char)) == -1)
            {
                perror("write");
                return -1;
            }
            else
                fprintf(stderr, "Command length too long.\n");
        }

        if (close(fd) == -1)
        {
            perror("close");
            return -1;
        }
    }

    return 0;
}
