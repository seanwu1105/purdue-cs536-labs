#if !defined(_AUDIO_CLIENT_CONFIG_H_)
#define _AUDIO_CLIENT_CONFIG_H_

#include <netdb.h>
#include <stdint.h>

typedef struct
{
    struct addrinfo *server_info;
    char *audio_filename;
    uint16_t blocksize;
    unsigned long long buffer_size;
    unsigned long long target_buffer_occupancy;
    long double packets_per_second;
    unsigned short congestion_control_index;
    char *log_filename;
    long double epsilon; // The weight of P term in PID formula
    long double beta;    // The weight of I term in PID formula
    long double alpha;   // The weight of D term in PID formula
} Config;

#endif // _AUDIO_CLIENT_CONFIG_H_