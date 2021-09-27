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
            write(fd, command, len * sizeof(char));
        else
            fprintf(stderr, "Command length too long.\n");
        close(fd);
    }

    return 0;
}
