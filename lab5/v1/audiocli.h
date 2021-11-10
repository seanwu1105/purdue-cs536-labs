#if !defined(_AUDIOCLI_H_)
#define _AUDIOCLI_H_

#include <netdb.h>
#include <stdint.h>
#include <sys/socket.h>

#define REQUIRED_ARGC 10
#define FILE_REQUEST_TIMEOUT_MS 500
#define AUDIO_REQUEST_INTERVAL_MS 313
#define CONGESTION_CONTROL_METHOD_C 0
#define CONGESTION_CONTROL_METHOD_D 1
#define CONGESTION_CONTROL_PARAMETERS_FILENAME "audiocliparam.dat"
#define FREAD_BUFFER_SIZE 4096

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
    long double epsilon;
    long double beta;
} Config;

static void sigint_handler(int _);
static void sigalrm_handler(int _);
static int get_config(int argc, char **argv, Config *config);
static int read_parameters_file(Config *const config);
static int run(const int sockfd, Config *config);
static int request_file(const int sockfd, const Config *const config);
static int stream_file_and_cancel_request_timeout(const int sockfd,
                                                  const Config *const config);
static int send_feedback(const int sockfd,
                         const struct sockaddr *const server_addr,
                         const socklen_t server_addr_len,
                         const Config *const config);
static int update_influx_rate_methed_c();

static int update_influx_rate_methed_d();

#endif // _AUDIOCLI_H_