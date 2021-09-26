#include <netdb.h>

#if !defined(_SOCKET_UTILS_H_)
#define _SOCKET_UTILS_H_

int print_addrinfo(const struct addrinfo *const info);
int build_addrinfo(struct addrinfo **info, const char *const ip, const char *const port);
int connect_and_bind_first_usable_addr(const struct addrinfo *const target, const struct addrinfo *const self);

#endif // _SOCKET_UTILS_H_