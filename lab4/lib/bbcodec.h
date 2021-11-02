#if !defined(_BBCODEC_H_)
#define _BBCODEC_H_

#include <stdint.h>

uint32_t bbdecode(const uint32_t x, const uint32_t prikey);
uint32_t bbencode(const uint32_t y, const uint32_t pubkey);
void bbdecode_data(uint8_t *const data, const size_t data_size,
                   const uint32_t prikey);
void bbencode_data(uint8_t *const data, const size_t data_size,
                   const uint32_t pubkey);

#endif // _BBCODEC_H_