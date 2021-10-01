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

    const char *val = strtok(content, "  \t\n");
    config->num_packages = (unsigned short)strtoul(val, NULL, 0);

    val = strtok(NULL, "  \t\n");
    config->timeout = (unsigned short)strtoul(val, NULL, 0);

    val = strtok(NULL, "  \t\n");
    config->server_delay = (uint8_t)strtoul(val, NULL, 0);

    val = strtok(NULL, "  \t\n");
    config->first_sequence_num = (int32_t)strtol(val, NULL, 0);

    return 0;
}