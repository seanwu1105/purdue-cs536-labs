#if !defined(_ROADRUNNER_SERVER_H_)
#define _ROADRUNNER_SERVER_H_

#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>

#define DUMMY_END_OF_PACKET 255

typedef struct
{
    uint16_t blocksize; // <= 1471
    uint8_t windowsize; // <= 63
    unsigned long long timeout_usec;
} Config;

int receive_ack_and_cancel_timeout(const int sockfd, const uint8_t expected);
int send_window(int *const sockfd, const uint8_t *const data,
                const size_t data_size,
                const struct sockaddr *const client_addr,
                const socklen_t client_addr_len, const int is_eof,
                const Config *const config,
                uint8_t *const initial_sequence_number);
int send_file(int *const sockfd, const char *const filename,
              const Config *const config,
              const struct sockaddr *const client_addr,
              const socklen_t client_addr_len);

#endif // _ROADRUNNER_SERVER_H_