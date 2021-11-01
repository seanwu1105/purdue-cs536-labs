#if !defined(_BBCODEC_H_)
#define _BBCODEC_H_

#include <stdint.h>

uint32_t bbdecode(uint32_t x, uint32_t prikey);
uint32_t bbencode(uint32_t y, uint32_t pubkey);

#endif // _BBCODEC_H_