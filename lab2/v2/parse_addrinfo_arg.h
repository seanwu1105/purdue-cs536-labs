#include <netdb.h>

#if !defined(_PARSE_ADDRINFO_ARG_H_)
#define _PARSE_ADDRINFO_ARG_H_

#define REQUIRED_ARGC 3

int parse_addrinfo_arg(int argc, char *argv[], struct addrinfo **info);

#endif // _PARSE_ADDRINFO_ARG_H_