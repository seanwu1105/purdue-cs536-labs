#if !defined(_ROADRUNNER_CLIENT_H_)
#define _ROADRUNNER_CLIENT_H_

#include <stdint.h>
#include <sys/socket.h>

#define FILE_REQUEST_TIMEOUT_MS 500
#define NUM_EOF_ACKS 8

typedef struct
{
    const char *filename;
    uint16_t blocksize; // <= 1471
    uint8_t windowsize; // <= 63
} Config;

int send_ack(const int sockfd, const uint8_t sequence_number,
             const struct sockaddr *const server_addr,
             const socklen_t server_addr_len);

int append_window_to_file(uint8_t *window_data, const uint8_t windowsize,
                          const size_t last_blocksize,
                          const Config *const config);

int receive_window_and_cancel_timeout(const int sockfd,
                                      const Config *const config,
                                      const uint8_t initial_sequence_number,
                                      const uint32_t *const prikey,
                                      uint8_t *const is_eof);

int receive_file_and_cancel_timeout(const int sockfd,
                                    const Config *const config,
                                    const uint32_t *const prikey);

#endif // _ROADRUNNER_CLIENT_H_