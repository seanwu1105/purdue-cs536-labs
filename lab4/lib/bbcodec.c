#include <stdint.h>

#include "bbcodec.h"

uint32_t bbdecode(uint32_t x, uint32_t prikey) { return x ^ prikey; }

uint32_t bbencode(uint32_t y, uint32_t pubkey) { return y ^ pubkey; }