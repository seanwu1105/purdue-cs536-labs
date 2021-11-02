#if !defined(_ACCESS_CONTROL_H_)
#define _ACCESS_CONTROL_H_

#include <stdint.h>
#include <sys/socket.h>

#define MAX_PUBLIC_KEYS_SIZE 100

typedef struct
{
    char ip[INET_ADDRSTRLEN];
    uint32_t pubkey;
} PublicKey;

ssize_t load_public_keys(PublicKey public_keys[]);
int check_access(const struct sockaddr *const addr, const uint32_t certificate,
                 const PublicKey public_keys[], const size_t public_keys_size,
                 uint32_t *pubkey);

#endif // _ACCESS_CONTROL_H_