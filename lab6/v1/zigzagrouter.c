#include "socket_utils.h"
#include <netdb.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

static int control_fd = -1;
static int data_fd_1 = -1;
static int data_fd_2 = -1;

static void sigint_handler(int _)
{
    close(control_fd);
    close(data_fd_1);
    close(data_fd_2);
    control_fd = -1;
    data_fd_1 = -1;
    data_fd_2 = -1;
    _exit(EXIT_SUCCESS);
}

static int run()
{
    while (1)
    {
        uint8_t buffer[BUFFER_SIZE];
        const ssize_t bytes_recv =
            recvfrom(data_fd_1, buffer, sizeof(buffer), 0, NULL, NULL);
        if (bytes_recv == -1)
        {
            perror("recvfrom");
            return -1;
        }

        printf("size: %ld\n", bytes_recv);
    }

    return 0;
}

static int create_and_bind_udp_socket()
{
    struct addrinfo *info;
    if (build_addrinfo(&info, NULL, "0", SOCK_DGRAM) != 0) return -1;

    const int fd = create_socket_with_first_usable_addr(info);
    if (fd == -1) return -1;

    if (bind_socket_with_first_usable_addr(info, fd) < 0)
    {
        close(fd);
        return -1;
    }

    freeaddrinfo(info);
    return fd;
}

int main()
{
    const struct sigaction sigint_action = {.sa_handler = sigint_handler};
    sigaction(SIGINT, &sigint_action, NULL);

    control_fd = create_and_bind_udp_socket();
    if (control_fd < 0)
    {
        close(control_fd);
        return -1;
    }
    const uint16_t control_port = get_port_number(control_fd);
    printf("control socket port: %u\n", control_port);

    data_fd_1 = create_and_bind_udp_socket();
    if (data_fd_1 < 0)
    {
        close(data_fd_1);
        return -1;
    }
    const uint16_t data_port_1 = get_port_number(data_fd_1);
    printf("data socket port 1: %u\n", data_port_1);

    data_fd_2 = create_and_bind_udp_socket();
    if (data_fd_2 < 0)
    {
        close(data_fd_2);
        return -1;
    }
    const uint16_t data_port_2 = get_port_number(data_fd_2);
    printf("data socket port 2: %u\n", data_port_2);

    return run();
}