#include "read_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 128

int sanitize_parameter(const int param)
{
    if (param < 0 || (param > 5 && param != 99)) return -1;
    return 0;
}

int read_config(Config *const config)
{
    FILE *file = fopen(CONFIG_FILENAME, "r");
    if (file == NULL)
    {
        perror("fopen");
        return -1;
    }

    char content[BUFFER_SIZE];
    const size_t bytes_read =
        fread(content, sizeof(char), sizeof(content), file);
    fclose(file);
    content[bytes_read] = '\0';
    if (bytes_read == 0) return -1;

    const char *val = strtok(content, "  \t\n\0");
    if (val == NULL) return -1;
    config->num_packages = (unsigned short)strtoul(val, NULL, 0);

    val = strtok(NULL, "  \t\n\0");
    if (val == NULL) return -1;
    config->timeout = (unsigned short)strtoul(val, NULL, 0);

    val = strtok(NULL, "  \t\n\0");
    if (val == NULL) return -1;
    config->server_delay = (uint8_t)strtoul(val, NULL, 0);

    val = strtok(NULL, "  \t\n\0");
    if (val == NULL) return -1;
    config->first_sequence_num = (int32_t)strtol(val, NULL, 0);

    if (sanitize_parameter(config->num_packages) == -1 ||
        sanitize_parameter(config->timeout) == -1 ||
        sanitize_parameter(config->first_sequence_num) == -1)
    {
        fprintf(stderr, "invalid parameter: N=%hu, T=%hu, S=%d. %s\n",
                config->num_packages, config->timeout,
                config->first_sequence_num, PARAMTER_RESTRICTION_MSG);
        return -1;
    }

    return 0;
}