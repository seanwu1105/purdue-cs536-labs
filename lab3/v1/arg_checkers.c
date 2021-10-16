#include "arg_checkers.h"
#include "request_codec.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int check_filename(const char *const filename)
{
    if (strlen(filename) > MAX_FILENAME_LEN)
    {
        fprintf(stderr, "Filename too long\n");
        return -1;
    }

    char c;
    const char *s = filename;
    while ((c = *s) && isalpha(c))
        s++;
    if (*s != '\0')
    {
        fprintf(stderr, "Filename contains invalid characters\n");
        return -2;
    }
    return 0;
}

int check_secret_key(const long long secret_key)
{
    if (secret_key < MIN_SECRET_KEY || secret_key > MAX_SECRET_KEY)
    {
        fprintf(stderr, "Secret key must be between %d and %d\n",
                MIN_SECRET_KEY, MAX_SECRET_KEY);
        return -1;
    }
    return 0;
}