#include <stdio.h>
#include <sys/time.h>

int log_stream(FILE *stream, const long long int data,
               struct timeval *const init_time, unsigned short *const is_first,
               const char *const data_title)
{
    struct timeval cur_time;

    if (*is_first)
    {
        if (gettimeofday(init_time, NULL) < 0)
        {
            perror("gettimeofday");
            return -1;
        }
        fprintf(stream, "time (us), %s\n", data_title);
        fprintf(stream, "0, %Ld\n", data);
        *is_first = 0;
        return 0;
    }

    if (gettimeofday(&cur_time, NULL) < 0)
    {
        perror("gettimeofday");
        return -1;
    }
    fprintf(stream, "%ld, %Ld\n",
            (cur_time.tv_sec * 1000000 + cur_time.tv_usec -
             (init_time->tv_sec * 1000000 + init_time->tv_usec)),
            data);
    return 0;
}

int dump_logging(const char *const filename, const char *const logging)
{
    FILE *const file = fopen(filename, "w");
    if (file == NULL)
    {
        perror("fopen");
        return -1;
    }

    int status = fputs(logging, file);
    fclose(file);

    if (status < 0) perror("fputs");

    return status;
}