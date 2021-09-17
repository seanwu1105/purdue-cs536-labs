#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include "fifo_info.h"

int main()
{
    while (1)
    {
        int fd = open(FIFO_FILENAME, O_WRONLY);
        if (fd == -1)
        {
            fprintf(stderr, "Cannot open FIFO: %s\n", FIFO_FILENAME);
            return -1;
        }
        char command[PIPE_BUF];
        fprintf(stdout, "> ");
        fgets(command, PIPE_BUF, stdin);
        size_t len = strlen(command) + 1;
        if (len * sizeof(char) <= PIPE_BUF)
            write(fd, command, len);
        else
            fprintf(stderr, "Command length too long.\n");
        close(fd);
    }

    return 0;
}
