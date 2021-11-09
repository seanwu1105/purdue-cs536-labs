#include <netdb.h>

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