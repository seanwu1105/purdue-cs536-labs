#if !defined(_REQUEST_CODEC_H_)
#define _REQUEST_CODEC_H_

#include <stdint.h>

#define REQUEST_SIZE_WITH_SECRET_KEY 10

void encode_request_with_secret_key(const char *const filename,
                                    const uint16_t secret_key,
                                    uint8_t *const message);
void decode_request_with_secret_key(const uint8_t *const message,
                                    char *const filename,
                                    uint16_t *const secret_key);

#define REQUEST_SIZE_WITH_CERTIFICATE 12

void encode_request_with_certificate(const char *const filename,
                                     const uint32_t certificate,
                                     uint8_t *const message);
void decode_request_with_certificate(const uint8_t *const message,
                                     char *const filename,
                                     uint32_t *const certificate);

#endif // _REQUEST_CODEC_H_