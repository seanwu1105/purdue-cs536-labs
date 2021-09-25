#include <stdint.h>

#if !defined(_READ_CONFIG_H_)
#define _READ_CONFIG_H_

typedef struct
{
    unsigned short num_packages; // 1 ~ 7
    unsigned short time_out;     // 1 ~ 5
    uint8_t server_delay;        // 1 byte, 0 ~ 5 | 99
    int32_t first_sequence_num;  // 4 bytes
} Config;

int read_config(const char *const filename, Config *const config);

#endif // _READ_CONFIG_H_