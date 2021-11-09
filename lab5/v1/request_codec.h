#if !defined(_REQUEST_CODEC_H_)
#define _REQUEST_CODEC_H_

#include <stdint.h>

#define REQUEST_SIZE 10

void encode_request(const char *const filename, const uint16_t blocksize,
                    uint8_t *const message);
void decode_request(const uint8_t *const message, char *const filename,
                    uint16_t *const blocksize);

#endif // _REQUEST_CODEC_H_