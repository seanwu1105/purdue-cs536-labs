#include <netdb.h>
#include <stdint.h>
#include <stdio.h>

#include "read_overlay.h"
#include "socket_utils.h"
#include "zzconfig_codec.h"

#define MAX_OVERLAY_SIZE 100

int send_overlay_entry(const OverlayEntry *const entry, const int sockfd)
{
    uint8_t config[ZZCONFIG_SIZE];
    if (encode_zzconfig(&(entry->forward_path), &(entry->return_path), config) <
        0)
        return -1;

    struct addrinfo *info;
    if (build_addrinfo(&info, entry->ip, entry->port, SOCK_DGRAM) < 0)
        return -1;
    if (sendto(sockfd, config, sizeof(config), 0, info->ai_addr,
               info->ai_addrlen) < 0)
    {
        perror("sendto");
        return -1;
    }
    freeaddrinfo(info);
    return 0;
}

int send_overlay_configurations(const OverlayEntry entries[],
                                const size_t entries_count)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return -1;

    for (size_t i = 0; i < entries_count; ++i)
        send_overlay_entry(&entries[i], sockfd);

    return 0;
}

int main()
{
    OverlayEntry entries[MAX_OVERLAY_SIZE];
    const ssize_t num_entries = read_overlay(entries);
    if (num_entries < 0) return -1;

    return send_overlay_configurations(entries, num_entries);
}