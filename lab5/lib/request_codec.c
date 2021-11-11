#include <stdint.h>
#include <string.h>

#include "parameter_checkers.h"
#include "request_codec.h"

void encode_request(const char *const filename, const uint16_t blocksize,
                    uint8_t *const message)
{
    *((uint16_t *)message) = blocksize;
    strncpy((char *)(message + (REQUEST_SIZE - MAX_FILENAME_LEN)), filename,
            MAX_FILENAME_LEN);
}

void decode_request(const uint8_t *const message, char *const filename,
                    uint16_t *const blocksize)
{
    memset(filename, 0, MAX_FILENAME_LEN + 1);
    *blocksize = *((uint16_t *)message);
    strncpy(filename,
            (const char *)(message + (REQUEST_SIZE - MAX_FILENAME_LEN)),
            MAX_FILENAME_LEN);
}
