#include <stdint.h>
#include "message_codec.h"

// Encode message, whose length should be exactly 5 bytes.
void encode_message(const int32_t id, const uint8_t delay, uint8_t *const message)
{
    *(int32_t *)message = id;
    message[4] = delay;
}

// Decode message, whose length should be exactly 5 bytes.
void decode_message(const uint8_t *const message, int32_t *const id, uint8_t *const delay)
{
    *id = *(int32_t *)message;
    *delay = message[4];
}
