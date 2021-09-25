#include <stdint.h>

#if !defined(_MESSAGE_CODEC_H_)
#define _MESSAGE_CODEC_H_

void encode_message(const int32_t id, const uint8_t control, uint8_t *const message);
void decode_message(const uint8_t *const message, int32_t *const id, uint8_t *const control);

#endif // _MESSAGE_CODEC_H_