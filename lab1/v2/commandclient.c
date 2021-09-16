#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "fifo_info.h"

int main()
{
    while (1)
    {
        int fd = open(FIFO_FILENAME, O_WRONLY);
        if (fd == -1)
            return -1;
        char command[100];
        printf("> ");
        fgets(command, 100, stdin);
        write(fd, command, strlen(command) + 1);
        close(fd);
    }

    return 0;
}
