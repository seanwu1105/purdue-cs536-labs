#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int main()
{
    char *fifo_filename = "serverfifo.dat";
    int mkfifo_ret = mkfifo(fifo_filename, S_IRUSR | S_IWUSR | S_IWGRP | S_IWOTH);
    if (mkfifo_ret != 0)
    {
        printf("bad things happened: %d\n", mkfifo_ret);
        return -1;
    }
    printf("fifo created.\n");

    int count = 0;
    while (count < 10)
    {
        count++;
        int fd = open(fifo_filename, O_RDONLY);
        if (fd == -1)
        {
            printf("cannot open fifo file for read.\n");
            return -1;
        }
        printf("Opened fifo file fd for read.\n");
        char arr2[100];
        read(fd, arr2, sizeof(arr2));

        printf("read: %s[end]", arr2);

        close(fd);
    }
    unlink(fifo_filename);
    return 0;
}