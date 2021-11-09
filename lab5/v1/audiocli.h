#if !defined(_AUDIOCLI_H_)
#define _AUDIOCLI_H_

#include <netdb.h>
#include <stdint.h>
#include <sys/socket.h>

#define REQUIRED_ARGC 10
#define FILE_REQUEST_TIMEOUT_MS 500

typedef struct
{
    struct addrinfo *server_info;
    char *audio_filename;
    uint16_t blocksize;
    unsigned long long buffer_size;
    unsigned long long target_buffer_occupancy;
    unsigned long long packets_per_second;
    unsigned short method;
    char *log_filename;
} Config;

static void sigint_handler(int _);
static void sigalrm_handler(int _);
static int parse_args(int argc, char **argv, Config *config);
static int run(const int sockfd, Config *config);
static int request_file(const int sockfd, const Config *const config);
static int stream_file_and_cancel_timeout(const int sockfd,
                                          const Config *const config);

#endif // _AUDIOCLI_H_