#include <arpa/inet.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "zzconfig_codec.h"

int encode_zzconfig(const ForwardingPair *const forward_path,
                    const ForwardingPair *const return_path,
                    uint8_t *const config)
{
    ((uint16_t *)config)[0] =
        (uint16_t)strtoul(forward_path->receive_port, NULL, 0);
    ((uint16_t *)config)[1] =
        (uint16_t)strtoul(forward_path->send_port, NULL, 0);
    if (inet_pton(AF_INET, forward_path->send_ip, &((uint32_t *)config)[1]) < 0)
    {
        perror("inet_pton");
        return -1;
    }

    ((uint16_t *)config)[4] =
        (uint16_t)strtoul(return_path->receive_port, NULL, 0);
    ((uint16_t *)config)[5] =
        (uint16_t)strtoul(return_path->send_port, NULL, 0);
    if (inet_pton(AF_INET, return_path->send_ip, &((uint32_t *)config)[3]) < 0)
    {
        perror("inet_pton");
        return -1;
    }
    return 0;
}

int decode_zzconfig(const uint8_t *const config,
                    ForwardingPair *const forward_path,
                    ForwardingPair *const return_path)
{
    sprintf(forward_path->receive_port, "%u", ((uint16_t *)config)[0]);
    sprintf(forward_path->send_port, "%u", ((uint16_t *)config)[1]);
    if (inet_ntop(AF_INET, &((uint32_t *)config)[1], forward_path->send_ip,
                  INET_ADDRSTRLEN) < 0)
    {
        perror("inet_ntop");
        return -1;
    }

    sprintf(return_path->receive_port, "%u", ((uint16_t *)config)[4]);
    sprintf(return_path->send_port, "%u", ((uint16_t *)config)[5]);
    if (inet_ntop(AF_INET, &((uint32_t *)config)[3], return_path->send_ip,
                  INET_ADDRSTRLEN) < 0)
    {
        perror("inet_ntop");
        return -1;
    }

    return 0;
}
