#include "read_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 128

int read_config(Config *const config)
{
    FILE *file = fopen(CONFIG_FILENAME, "r");
    if (file == NULL)
    {
        fprintf(stderr, "cannot open config.\n");
        return -1;
    }

    char content[BUFFER_SIZE];
    const ssize_t read_bytes = fread(content, 1, BUFFER_SIZE, file);
    fclose(file);
    if (read_bytes <= 0) return -1;

    const char *key = strtok(content, "  \t\n");
    while (key)
    {
        if (strlen(key) != 1) return -1;

        const char *val = strtok(NULL, "  \t\n");

        switch (key[0])
        {
        case 'N':
            config->num_packages = (unsigned short)strtoul(val, NULL, 0);
            break;

        case 'T':
            config->timeout = (unsigned short)strtoul(val, NULL, 0);
            break;

        case 'D':
            config->server_delay = (uint8_t)strtoul(val, NULL, 0);
            break;

        case 'S':
            config->first_sequence_num = (int32_t)strtol(val, NULL, 0);
            break;

        default:
            fprintf(stderr, "unknown configuration key: %s\n", key);
            return -1;
        }

        key = strtok(NULL, "  \t\n");
    }

    return 0;
}