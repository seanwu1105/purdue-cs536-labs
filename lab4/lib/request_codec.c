#include <stdint.h>
#include <string.h>

#include "arg_checkers.h"
#include "request_codec.h"

void encode_request_with_secret_key(const char *const filename,
                                    const uint16_t secret_key,
                                    uint8_t *const message)
{
    *((uint16_t *)message) = secret_key;
    strncpy(
        (char *)(message + (REQUEST_SIZE_WITH_SECRET_KEY - MAX_FILENAME_LEN)),
        filename, MAX_FILENAME_LEN);
}

void decode_request_with_secret_key(const uint8_t *const message,
                                    char *const filename,
                                    uint16_t *const secret_key)
{
    *secret_key = *((uint16_t *)message);
    strncpy(filename,
            (const char *)(message +
                           (REQUEST_SIZE_WITH_SECRET_KEY - MAX_FILENAME_LEN)),
            MAX_FILENAME_LEN);
    filename[MAX_FILENAME_LEN] = '\0';
}

void encode_request_with_certificate(const char *const filename,
                                     const uint32_t certificate,
                                     uint8_t *const message)
{
    *((uint32_t *)message) = certificate;
    strncpy(
        (char *)(message + (REQUEST_SIZE_WITH_CERTIFICATE - MAX_FILENAME_LEN)),
        filename, MAX_FILENAME_LEN);
}

void decode_request_with_certificate(const uint8_t *const message,
                                     char *const filename,
                                     uint32_t *const certificate)
{
    *certificate = *((uint32_t *)message);
    strncpy(filename,
            (const char *)(message +
                           (REQUEST_SIZE_WITH_CERTIFICATE - MAX_FILENAME_LEN)),
            MAX_FILENAME_LEN);
    filename[MAX_FILENAME_LEN] = '\0';
}
