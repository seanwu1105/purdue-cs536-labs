#include "request_codec.h"
#include <stdint.h>
#include <string.h>

void encode_request(const char *const filename, const uint16_t secret_key,
                    uint8_t *const message)
{
    *((uint16_t *)message) = secret_key;
    strncpy((char *)(message + (REQUEST_SIZE - MAX_FILENAME_LEN)), filename,
            MAX_FILENAME_LEN);
}

void decode_request(const uint8_t *const message, char *const filename,
                    uint16_t *const secret_key)
{
    *secret_key = *((uint16_t *)message);
    strncpy(filename,
            (const char *)(message + (REQUEST_SIZE - MAX_FILENAME_LEN)),
            MAX_FILENAME_LEN);
    filename[MAX_FILENAME_LEN] = '\0';
}
