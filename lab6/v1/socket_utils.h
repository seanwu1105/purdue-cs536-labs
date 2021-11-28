#include <netdb.h>

#if !defined(_SOCKET_UTILS_H_)
#define _SOCKET_UTILS_H_

int build_addrinfo(struct addrinfo **info, const char *const ip,
                   const char *const port, const int socktype);
int create_socket_with_first_usable_addr(const struct addrinfo *const info);
int bind_socket_with_first_usable_addr(const struct addrinfo *const info,
                                       int sockfd);
uint16_t get_port_number(const int fd);

#endif // _SOCKET_UTILS_H_