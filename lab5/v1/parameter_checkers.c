#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parameter_checkers.h"

int check_filename(const char *const filename)
{
    if (strnlen(filename, MAX_FILENAME_LEN + 1) > MAX_FILENAME_LEN)
    {
        fprintf(stderr, "Filename too long\n");
        return -1;
    }

    char c;
    const char *s = filename;
    while ((c = *s) && isalpha(c))
        s++;
    if (strcmp(s, ".au") != 0)
    {
        fprintf(stderr, "Filename contains invalid characters or not ended with"
                        " '.au'\n");
        return -2;
    }
    return 0;
}

int check_blocksize(const unsigned long long blocksize)
{
    if (blocksize < MIN_BLOCKSIZE || blocksize > MAX_BLOCKSIZE)
    {
        fprintf(stderr, "Blocksize must be between %d and %d\n", MIN_BLOCKSIZE,
                MAX_BLOCKSIZE);
        return -1;
    }
    return 0;
}

int check_packets_per_second(const unsigned long long packets_per_second)
{
    if (packets_per_second < MIN_PACKETS_PER_SECOND)
    {
        fprintf(stderr, "Packets per second must be larger or equal to %d\n",
                MIN_PACKETS_PER_SECOND);
        return -1;
    }
    return 0;
}