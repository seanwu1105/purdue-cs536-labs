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
        {
            printf("cannot open fifo file for write.\n");
            return -1;
        }
        printf("Opened fifo file fd for write.\n");
        char arr1[100];
        fgets(arr1, 100, stdin);
        write(fd, arr1, strlen(arr1) + 1);
        close(fd);
    }

    return 0;
}
