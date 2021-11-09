#if !defined(_AUDIOSRV_H_)
#define _AUDIOSRV_H_

#include <netdb.h>
#include <sys/socket.h>

#define REQUIRED_ARGC 5

typedef struct
{
    struct addrinfo *server_info;
    unsigned long long packets_per_second;
    char *log_filename;
} Config;

static void sigint_handler(int _);
static void tear_down();
static int parse_args(int argc, char **argv, Config *config);
static int run(Config *config);
static int read_request(char *const filename, uint16_t *const blocksize,
                        struct sockaddr *const client_addr,
                        socklen_t *const client_addr_len);

#endif // _AUDIOSRV_H_