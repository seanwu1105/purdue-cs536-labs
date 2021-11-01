#if !defined(_SOCKET_UTILS_H_)
#define _SOCKET_UTILS_H_

#include <netdb.h>

int build_addrinfo(struct addrinfo **info, const char *const ip,
                   const char *const port, const int socktype);
int create_socket_with_first_usable_addr(const struct addrinfo *const info);
int bind_socket_with_first_usable_addr(const struct addrinfo *const info,
                                       const int sockfd);
int get_udp_host_ip(const int sockfd, const struct addrinfo *const server_info,
                    uint32_t *ip);
#endif // _SOCKET_UTILS_H_