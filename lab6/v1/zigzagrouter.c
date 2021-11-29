#include <netdb.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "socket_utils.h"
#include "zzconfig_codec.h"

#define BUFFER_SIZE 4096

static int control_fd = -1;
static int forward_fd = -1;
static int return_fd = -1;

static void tear_down()
{
    close(control_fd);
    control_fd = -1;
    close(forward_fd);
    forward_fd = -1;
    close(return_fd);
    return_fd = -1;
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
    FD_SET(forward_fd, read_fds);
    FD_SET(return_fd, read_fds);

    int max_fd = 0;
    max_fd = forward_fd > return_fd ? forward_fd : return_fd;
    max_fd = control_fd > max_fd ? control_fd : max_fd;

    if (select(max_fd + 1, read_fds, NULL, NULL, NULL) < 0)
    {
        perror("select");
        return -1;
    }
    return 0;
}

static int create_and_bind_udp_socket(const char *const port)
{
    struct addrinfo *info;
    if (build_addrinfo(&info, NULL, port, SOCK_DGRAM) != 0) return -1;

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

static int update_forwardings(ForwardingPath *const forward_path,
                              ForwardingPath *const return_path)
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

    ForwardingPath new_forward_path = {0};
    ForwardingPath new_return_path = {0};
    if (decode_zzconfig(buffer, &new_forward_path, &new_return_path) == -1)
        return -1;

    time_t ltime = time(NULL);
    fprintf(stdout, "\n%s", asctime(localtime(&ltime)));

    if (get_port_number(forward_fd) !=
        (uint16_t)strtoul(new_forward_path.receive_port, NULL, 0))
    {
        fprintf(stdout, "Update forward path receiving port from %u",
                get_port_number(forward_fd));
        close(forward_fd);
        forward_fd = create_and_bind_udp_socket(new_forward_path.receive_port);
        printf(" to %u\n", get_port_number(forward_fd));
        strncpy(forward_path->receive_port, new_forward_path.receive_port,
                PORT_STRLEN);
    }

    if (get_port_number(return_fd) !=
        (uint16_t)strtoul(new_return_path.receive_port, NULL, 0))
    {
        fprintf(stdout, "Update return path receiving port from %u",
                get_port_number(return_fd));
        close(return_fd);
        return_fd = create_and_bind_udp_socket(new_return_path.receive_port);
        printf(" to %u\n", get_port_number(return_fd));
        strncpy(return_path->receive_port, new_return_path.receive_port,
                PORT_STRLEN);
    }

    strncpy(forward_path->send_port, new_forward_path.send_port, PORT_STRLEN);
    strncpy(forward_path->send_ip, new_forward_path.send_ip, INET_ADDRSTRLEN);
    strncpy(return_path->send_port, new_return_path.send_port, PORT_STRLEN);
    strncpy(return_path->send_ip, new_return_path.send_ip, INET_ADDRSTRLEN);

    fprintf(stdout, "Update forward path sending: %s:%s\n",
            forward_path->send_ip, forward_path->send_port);
    fprintf(stdout, "Update return path sending: %s:%s\n", return_path->send_ip,
            return_path->send_port);

    return 0;
}

static int forward_data(const int fd, ForwardingPath *const path)
{
    uint8_t buffer[BUFFER_SIZE];
    const ssize_t bytes_recv =
        recvfrom(fd, buffer, sizeof(buffer), 0, NULL, NULL);
    if (bytes_recv == -1)
    {
        perror("recvfrom");
        return -1;
    }
    printf("data received with size: %ld\n", bytes_recv);

    if (path->send_ip == NULL || path->send_port == NULL ||
        strnlen(path->send_ip, INET_ADDRSTRLEN) == 0 ||
        strnlen(path->send_port, PORT_STRLEN) == 0)
    {
        fprintf(stderr, "Forward path sending address has not initialized.\n");
        return -1;
    }

    struct addrinfo *info;
    if (build_addrinfo(&info, path->send_ip, path->send_port, SOCK_DGRAM) < 0)
        return -1;

    // Always use the return_fd to send data so the receiver can send feedback
    // to router.
    if (sendto(return_fd, buffer, bytes_recv, 0, info->ai_addr,
               info->ai_addrlen) < 0)
    {
        perror("sendto");
        return -1;
    }

    return 0;
}

static int run(ForwardingPath *const forward_path,
               ForwardingPath *const return_path)
{
    while (1)
    {
        fd_set read_fds;
        if (select_fds(&read_fds) < 0) return -1;

        if (FD_ISSET(control_fd, &read_fds))
            update_forwardings(forward_path, return_path);
        if (FD_ISSET(forward_fd, &read_fds))
            forward_data(forward_fd, forward_path);
        if (FD_ISSET(return_fd, &read_fds))
            forward_data(return_fd, return_path);
    }

    return 0;
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
    control_fd = create_and_bind_udp_socket("0");
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
    fprintf(stdout, "Control socket port: %u\n", control_port);

    forward_fd = create_and_bind_udp_socket("0");
    if (forward_fd < 0)
    {
        tear_down();
        return -1;
    }
    fprintf(stdout, "Forward data socket port: %u\n",
            get_port_number(forward_fd));

    return_fd = create_and_bind_udp_socket("0");
    if (return_fd < 0)
    {
        tear_down();
        return -1;
    }
    fprintf(stdout, "Return data socket port: %u\n",
            get_port_number(return_fd));

    ForwardingPath forward_path = {.send_ip = "", .send_port = ""};
    ForwardingPath return_path = {.send_ip = "", .send_port = ""};

    sprintf(forward_path.receive_port, "%u", get_port_number(forward_fd));
    sprintf(return_path.receive_port, "%u", get_port_number(return_fd));

    return run(&forward_path, &return_path);
}