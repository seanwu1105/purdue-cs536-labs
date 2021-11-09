#if !defined(_AUDIOSRV_H_)
#define _AUDIOSRV_H_

#include <netdb.h>
#include <sys/socket.h>

#define REQUIRED_ARGC 5
#define EOF_TRIES 8
#define EOF_PACKET NULL
#define EOF_PACKET_SIZE 0

typedef struct
{
    struct addrinfo *server_info;
    unsigned long long packets_per_second;
    char *log_filename;
} Config;

static void sigint_handler(int _);
static int parse_args(int argc, char **argv, Config *config);
static int run(const int request_sockfd, Config *config);
static int read_request(const int request_sockfd, char *const filename,
                        uint16_t *const blocksize,
                        struct sockaddr *const client_addr,
                        socklen_t *const client_addr_len);
static int send_file(const char *const filename, const uint16_t blocksize,
                     Config *const config, struct sockaddr *const client_addr,
                     const socklen_t client_addr_len);
static int send_eof(const int packet_sockfd, struct sockaddr *const client_addr,
                    const socklen_t client_addr_len);

#endif // _AUDIOSRV_H_