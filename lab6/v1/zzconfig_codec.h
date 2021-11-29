#if !defined(_ZZCONFIG_CODEC_H_)
#define _ZZCONFIG_CODEC_H_

#include <arpa/inet.h>
#include <stdint.h>

#define PORT_STRLEN 6
#define ZZCONFIG_SIZE 16

typedef struct
{
    char receive_port[PORT_STRLEN];
    char send_port[PORT_STRLEN];
    char send_ip[INET_ADDRSTRLEN];
} ForwardingPath;

int encode_zzconfig(const ForwardingPath *const forward_path,
                    const ForwardingPath *const return_path,
                    uint8_t *const config);

int decode_zzconfig(const uint8_t *const config,
                    ForwardingPath *const forward_path,
                    ForwardingPath *const return_path);

#endif // _ZZCONFIG_CODEC_H_