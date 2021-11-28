#if !defined(_READ_CONFIG_H_)
#define _READ_CONFIG_H_

#include <stdint.h>

#define CONFIG_FILENAME "pingparam.dat"

#define PARAMTER_RESTRICTION_MSG                                               \
    "All numbers other than 99 and 0-5 will be considered invalid."

typedef struct
{
    unsigned short num_packages; // 1 ~ 7
    unsigned short timeout;      // 1 ~ 5
    uint8_t server_delay;        // 1 byte, 0 ~ 5 | 99
    int32_t first_sequence_num;  // 4 bytes
} Config;

int sanitize_parameter(const int param);

int read_config(Config *const config);

#endif // _READ_CONFIG_H_