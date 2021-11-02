#include <stdint.h>
#include <stdlib.h>

#include "bbcodec.h"

uint32_t bbdecode(const uint32_t x, const uint32_t prikey)
{
    return x ^ prikey;
}

void bbdecode_data(uint8_t *const data, const size_t data_size,
                   const uint32_t prikey)
{
    for (size_t i = 0; i < data_size; i++)
    {
        data[i] = bbdecode(data[i], prikey);
    }
}

uint32_t bbencode(const uint32_t y, const uint32_t pubkey)
{
    return y ^ pubkey;
}

void bbencode_data(uint8_t *const data, const size_t data_size,
                   const uint32_t pubkey)
{
    for (size_t i = 0; i < data_size; i++)
    {
        data[i] = bbencode(data[i], pubkey);
    }
}
