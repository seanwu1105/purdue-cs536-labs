#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "read_overlay.h"

#define BUFFER_SIZE 4096

static int parse_entry_control(char *buffer, char *ip, char *port)
{
    if (strcmp(buffer, "\n") == 0) return -1;

    ip = strtok(buffer, " \t\n\0");
    if (ip == NULL) return -1;
    port = strtok(NULL, " \t\n\0");
    if (port == NULL) return -1;
    return 0;
}

static int parse_entry_data_forwarding(char *buffer, char *recv_port,
                                       char *send_port, char *send_ip)
{
    if (strcmp(buffer, "\n") == 0) return -1;

    recv_port = strtok(buffer, " \t\n\0");
    if (recv_port == NULL) return -1;
    send_port = strtok(NULL, " \t\n\0");
    if (send_port == NULL) return -1;
    send_ip = strtok(NULL, " \t\n\0");
    if (send_ip == NULL) return -1;
    return 0;
}

static int read_entry(FILE *file, OverlayEntry *const entry)
{
    char buffer[BUFFER_SIZE];
    if (fgets(buffer, BUFFER_SIZE, file) == NULL)
    {
        fprintf(stderr, "Unexpected EOF.\n");
        return -1;
    }
    if (parse_entry_control(buffer, entry->ip, entry->port) < 0)
    {
        fprintf(stderr, "Invalid entry control.\n");
        return -1;
    }

    size_t j = 0;
    while (j < MAX_FORWARDING_PAIRS &&
           fgets(buffer, BUFFER_SIZE, file) != NULL &&
           strcmp(buffer, "\n") != 0)
    {
        if (parse_entry_data_forwarding(buffer,
                                        entry->forwarding_pairs[j].receive_port,
                                        entry->forwarding_pairs[j].send_port,
                                        entry->forwarding_pairs[j].send_ip) < 0)
        {
            fprintf(stderr, "Invalid entry data forwarding.\n");
            return -1;
        }
        j++;
    }
    entry->num_forwarding_pairs = j;
    return 0;
}

int read_overlay(OverlayEntry entries[], size_t *const num_fwd_pairs)
{
    *num_fwd_pairs = 0;

    FILE *file = fopen(OVERLAY_FILENAME, "r");
    if (file == NULL)
    {
        perror("fopen");
        return -1;
    }

    char buffer[BUFFER_SIZE];
    if (fgets(buffer, BUFFER_SIZE, file) == NULL) return 0;
    *num_fwd_pairs = strtoul(buffer, NULL, 0);
    if (*num_fwd_pairs == 0) return 0;

    if (fgets(buffer, BUFFER_SIZE, file) == NULL) return -1;
    if (strcmp(buffer, "\n") != 0)
    {
        fprintf(stderr, "Expected an empty line before entries.\n");
        return -1;
    }

    for (size_t i = 0; i < *num_fwd_pairs; i++)
        if (read_entry(file, &entries[i]) < 0) return -1;

    return 0;
}
