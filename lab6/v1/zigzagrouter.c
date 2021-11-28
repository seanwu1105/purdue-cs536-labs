#include <netdb.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include "socket_utils.h"
#include "zzconfig_codec.h"

#define BUFFER_SIZE 4096
#define DATA_FDS_COUNT 2

static int control_fd = -1;
static int data_fds[DATA_FDS_COUNT] = {-1, -1};

static void tear_down()
{
    close(control_fd);
    control_fd = -1;
    for (size_t i = 0; i < DATA_FDS_COUNT; i++)
    {
        close(data_fds[i]);
        data_fds[i] = -1;
    }
}

static void sigint_handler(int _)
{
    tear_down();
    _exit(EXIT_SUCCESS);
}

static int select_fds(fd_set *const read_fds)
{
    FD_ZERO(read_fds);
    FD_SET(control_fd, read_fds);
    for (int i = 0; i < DATA_FDS_COUNT; i++)
        FD_SET(data_fds[i], read_fds);

    int max_fd = 0;
    for (int i = 0; i < DATA_FDS_COUNT; i++)
        max_fd = data_fds[i] > max_fd ? data_fds[i] : max_fd;
    max_fd = control_fd > max_fd ? control_fd : max_fd;

    if (select(max_fd + 1, read_fds, NULL, NULL, NULL) < 0)
    {
        perror("select");
        return -1;
    }
    return 0;
}

static int update_forwardings()
{
    uint8_t buffer[ZZCONFIG_SIZE];
    const ssize_t bytes_recv =
        recvfrom(control_fd, buffer, sizeof(buffer), 0, NULL, NULL);
    if (bytes_recv == -1)
    {
        perror("recvfrom");
        return -1;
    }
    if (bytes_recv != ZZCONFIG_SIZE)
    {
        fprintf(stderr, "Invalid zzconfig message size: %ld\n", bytes_recv);
        return -1;
    }
    ForwardingPair forward_path;
    ForwardingPair return_path;
    if (decode_zzconfig(buffer, &forward_path, &return_path) == -1) return -1;

    printf("fwd recv port: %s\n", forward_path.receive_port);
    printf("fwd send port: %s\n", forward_path.send_port);
    printf("fwd send ip: %s\n", forward_path.send_ip);
    printf("ret recv port: %s\n", return_path.receive_port);
    printf("ret send port: %s\n", return_path.send_port);
    printf("ret send ip: %s\n", return_path.send_ip);

    // TODO: Only close and reopen receive socket if we need to.
    // TODO: Only close and reopen send socket if we need to.
    return 0;
}

static int forward_data(const int fd_idx)
{
    uint8_t buffer[BUFFER_SIZE];
    const ssize_t bytes_recv =
        recvfrom(data_fds[fd_idx], buffer, sizeof(buffer), 0, NULL, NULL);
    if (bytes_recv == -1)
    {
        perror("recvfrom");
        return -1;
    }
    printf("data_fds[%d] received with size: %ld\n", fd_idx, bytes_recv);

    // TODO: Check if we can forward.
    // TODO: Forward.
    return 0;
}

static int run()
{
    while (1)
    {
        fd_set read_fds;
        if (select_fds(&read_fds) < 0) return -1;

        if (FD_ISSET(control_fd, &read_fds)) update_forwardings();

        for (int i = 0; i < DATA_FDS_COUNT; i++)
            if (FD_ISSET(data_fds[i], &read_fds)) forward_data(i);
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

static int create_and_bind_control_socket(int argc, char *argv[])
{
    struct addrinfo *info;
    if (argc > 1)
    {
        if (build_addrinfo(&info, NULL, argv[1], SOCK_DGRAM) != 0) return -1;
        control_fd = create_socket_with_first_usable_addr(info);
        if (control_fd == -1) return -1;
        if (bind_socket_with_first_usable_addr(info, control_fd) < 0)
        {
            close(control_fd);
            return -1;
        }
        return 0;
    }
    control_fd = create_and_bind_udp_socket();
    if (control_fd < 0)
    {
        close(control_fd);
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    const struct sigaction sigint_action = {.sa_handler = sigint_handler};
    sigaction(SIGINT, &sigint_action, NULL);

    if (create_and_bind_control_socket(argc, argv) < 0) return -1;
    const uint16_t control_port = get_port_number(control_fd);
    fprintf(stdout, "control socket port: %u\n", control_port);

    for (size_t i = 0; i < DATA_FDS_COUNT; i++)
    {
        data_fds[i] = create_and_bind_udp_socket();
        if (data_fds[i] < 0)
        {
            tear_down();
            return -1;
        }
        const uint16_t data_port = get_port_number(data_fds[i]);
        fprintf(stdout, "data socket %ld port: %u\n", i, data_port);
    }

    return run();
}