#include "read_config.h"
#include <stdio.h>
#include <stdlib.h>

int read_config(const char *const filename, Config *const config)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        fprintf(stderr, "cannot open config.\n");
        return -1;
    }

    char key;
    char val[128];
    while (fscanf(file, "%c=%127[^\n]%*c", &key, val) == 2)
        switch (key)
        {
        case 'N':
            config->num_packages = (unsigned short)strtoul(val, NULL, 0);
            break;

        case 'T':
            config->time_out = (unsigned short)strtoul(val, NULL, 0);
            break;

        case 'D':
            config->server_delay = (uint8_t)strtoul(val, NULL, 0);
            break;

        case 'S':
            config->first_sequence_num = (int32_t)strtol(val, NULL, 0);
            break;

        default:
            fprintf(stderr, "unknown configuration key: %c\n", key);
            return -1;
        }

    return 0;
}