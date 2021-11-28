#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "read_overlay.h"

#define BUFFER_SIZE 4096

static int parse_entry_control(char *buffer, char *ip, char *port)
{
    if (strcmp(buffer, "\n") == 0) return -1;

    char *token = strtok(buffer, " \t\n\0");
    if (token == NULL) return -1;
    strncpy(ip, token, INET_ADDRSTRLEN);
    token = strtok(NULL, " \t\n\0");
    if (token == NULL) return -1;
    strncpy(port, token, PORT_STRLEN);
    return 0;
}

static int parse_entry_data_forwarding(char *buffer, ForwardingPair *const pair)
{
    if (strcmp(buffer, "\n") == 0) return -1;

    char *token = strtok(buffer, " \t\n\0");
    if (token == NULL) return -1;
    strncpy(pair->receive_port, token, PORT_STRLEN);
    token = strtok(NULL, " \t\n\0");
    if (token == NULL) return -1;
    strncpy(pair->send_port, token, PORT_STRLEN);
    token = strtok(NULL, " \t\n\0");
    if (token == NULL) return -1;
    strncpy(pair->send_ip, token, INET_ADDRSTRLEN);

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
        fprintf(stderr, "Invalid entry control: %s\n", buffer);
        return -1;
    }

    if (fgets(buffer, BUFFER_SIZE, file) == NULL)
    {
        fprintf(stderr, "Unexpected EOF.\n");
        return -1;
    }
    if (parse_entry_data_forwarding(buffer, &(entry->forward_path)) < 0)
    {
        fprintf(stderr, "Invalid forward path: %s\n", buffer);
        return -1;
    }

    if (fgets(buffer, BUFFER_SIZE, file) == NULL)
    {
        fprintf(stderr, "Unexpected EOF.\n");
        return -1;
    }
    if (parse_entry_data_forwarding(buffer, &(entry->return_path)) < 0)
    {
        fprintf(stderr, "Invalid return path: %s\n", buffer);
        return -1;
    }

    if (fgets(buffer, BUFFER_SIZE, file) != NULL && strcmp(buffer, "\n") != 0)
    {
        fprintf(stderr,
                "Unexpected data (an empty line should separate entries): %s\n",
                buffer);
        return -1;
    }
    return 0;
}

ssize_t read_overlay(OverlayEntry entries[])
{
    FILE *file = fopen(OVERLAY_FILENAME, "r");
    if (file == NULL)
    {
        perror("fopen");
        return -1;
    }

    char buffer[BUFFER_SIZE];
    if (fgets(buffer, BUFFER_SIZE, file) == NULL) return 0;
    const size_t num_fwd_pairs = strtoul(buffer, NULL, 0);
    if (num_fwd_pairs == 0) return 0;

    if (fgets(buffer, BUFFER_SIZE, file) == NULL) return -1;
    if (strcmp(buffer, "\n") != 0)
    {
        fprintf(stderr, "Expected an empty line before entries.\n");
        return -1;
    }

    for (size_t i = 0; i < num_fwd_pairs; i++)
        if (read_entry(file, &entries[i]) < 0) return -1;

    return num_fwd_pairs;
}
