#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "print_payload.h"

void print_payload(const uint8_t *const payload, const size_t payload_len)
{
    printf("Payload: [");
    for (int i = 0; i < payload_len; i++)
    {
        printf(" %02x", payload[i]);
    }
    printf(" ]\n");
}