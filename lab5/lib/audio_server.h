#if !defined(_AUDIO_SERVER_H_)
#define _AUDIO_SERVER_H_

#include <netdb.h>
#include <sys/socket.h>

#define REQUIRED_ARGC 5
#define EOF_TRIES 8
#define EOF_PACKET NULL
#define EOF_PACKET_SIZE 0

typedef struct
{
    struct addrinfo *server_info;
    long double packets_per_second;
    char *log_filename;
} Config;

int parse_args(int argc, char **argv, Config *config);
int start_server(Config *config);
int read_request(const int request_sockfd, char *const filename,
                 uint16_t *const blocksize, struct sockaddr *const client_addr,
                 socklen_t *const client_addr_len);
int send_file(const char *const filename, const uint16_t blocksize,
              Config *const config, struct sockaddr *const client_addr,
              const socklen_t client_addr_len, const unsigned int client_id);
int receive_feedback(const int packet_sockfd,
                     uint16_t *const packet_interval_ms);
int send_eof(const int packet_sockfd, struct sockaddr *const client_addr,
             const socklen_t client_addr_len);

#endif // _AUDIO_SERVER_H_