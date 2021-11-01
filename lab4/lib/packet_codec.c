#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "packet_codec.h"

void encode_packet(const uint8_t num, const uint8_t *const data,
                   const size_t data_size, uint8_t *const packet)
{
    packet[0] = num;
    memcpy(packet + 1, data, data_size * sizeof(uint8_t));
}
void decode_packet(const uint8_t *const packet, uint8_t *const num,
                   const size_t data_size, uint8_t *const data)
{
    *num = packet[0];
    memcpy(data, packet + 1, data_size * sizeof(uint8_t));
}
