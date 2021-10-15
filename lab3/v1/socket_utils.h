#include <netdb.h>

#if !defined(_SOCKET_UTILS_H_)
#define _SOCKET_UTILS_H_

int build_addrinfo(struct addrinfo **info, const char *const ip,
                   const char *const port, const int socktype);

#endif // _SOCKET_UTILS_H_