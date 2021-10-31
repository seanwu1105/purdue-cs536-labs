#if !defined(_PACKET_CODEC_H_)
#define _PACKET_CODEC_H_

#include <stdint.h>
#include <stdlib.h>

#define SEQUENCE_NUMBER_SPACE_TO_WINDOWSIZE_RATIO 2

void encode_packet(const uint8_t num, const uint8_t *const data,
                   const size_t data_size, uint8_t *const packet);
void decode_packet(const uint8_t *const packet, uint8_t *const num,
                   const size_t data_size, uint8_t *const data);

#endif // _PACKET_CODEC_H_