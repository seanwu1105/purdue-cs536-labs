#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "../lib/parameter_checkers.h"
#include "../lib/pspacing.h"
#include "../lib/request_codec.h"
#include "../lib/socket_utils.h"
#include "audiosrv.h"

int main(int argc, char *argv[])
{
    const struct sigaction sigint_action = {.sa_handler = sigint_handler};
    if (sigaction(SIGINT, &sigint_action, NULL) < 0)
    {
        perror("sigaction");
        return -1;
    }

    Config config;
    if (parse_args(argc, argv, &config) != 0) return -1;

    int request_sockfd = -1;
    if ((request_sockfd =
             create_socket_with_first_usable_addr(config.server_info)) == -1)
        return -1;

    if (bind_socket_with_first_usable_addr(config.server_info,
                                           request_sockfd) != 0)
    {
        close(request_sockfd);
        return -1;
    }

    return run(request_sockfd, &config);
}

static void sigint_handler(int _) { _exit(EXIT_SUCCESS); }

static int parse_args(int argc, char **argv, Config *config)
{
    if (argc < REQUIRED_ARGC)
    {
        fprintf(stderr,
                "Usage: %s <server-ip> <server-port>"
                " <packets-per-second> <log-filename>\n",
                argv[0]);
        return -1;
    }

    if (build_addrinfo(&(config->server_info), argv[1], argv[2], SOCK_DGRAM) !=
        0)
        return -1;

    const long double packets_per_second = strtold(argv[3], NULL);
    if (check_packets_per_second(packets_per_second) != 0) return -1;
    config->packets_per_second = packets_per_second;

    config->log_filename = argv[4];

    return 0;
}

static int run(int const request_sockfd, Config *config)
{
    uint16_t blocksize;
    char filename[MAX_FILENAME_LEN + 1];
    struct sockaddr client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    while (1)
    {
        if (read_request(request_sockfd, filename, &blocksize, &client_addr,
                         &client_addr_len) < 0)
            continue;

        if (check_filename(filename) != 0) continue;

        if (access(filename, F_OK) != 0)
        {
            perror("access");
            continue;
        }

        fflush(stdout);
        const pid_t pid = fork();
        if (pid == 0)
        {
            // Child process
            close(request_sockfd);
            send_file(filename, blocksize, config, &client_addr,
                      client_addr_len);
            exit(EXIT_SUCCESS);
        }
    }

    return 0;
}

static int read_request(const int request_sockfd, char *const filename,
                        uint16_t *const blocksize,
                        struct sockaddr *const client_addr,
                        socklen_t *const client_addr_len)
{
    uint8_t request[REQUEST_SIZE];
    // Initialized request as it may be smaller than REQUEST_SIZE.
    memset(request, 0, REQUEST_SIZE);
    const ssize_t bytes_read = recvfrom(request_sockfd, request, REQUEST_SIZE,
                                        0, client_addr, client_addr_len);
    if (bytes_read == -1)
    {
        perror("read");
        return -1;
    }

    decode_request(request, filename, blocksize);
    return 0;
}

static int send_file(const char *const filename, const uint16_t blocksize,
                     Config *const config, struct sockaddr *const client_addr,
                     const socklen_t client_addr_len)
{
    FILE *const file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("fopen");
        return -1;
    }

    const int packet_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (packet_sockfd == -1)
    {
        perror("socket");
        fclose(file);
        return -1;
    }

    uint16_t packet_interval_ms = to_pspacing_ms(config->packets_per_second);
    uint8_t buffer[blocksize];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, blocksize, file)) > 0)
    {
        if (sendto(packet_sockfd, buffer, bytes_read, 0, client_addr,
                   client_addr_len) < 0)
        {
            perror("sendto");
            close(packet_sockfd);
            fclose(file);
            return -1;
        }

        struct timespec sleep_time = {
            .tv_sec = packet_interval_ms / 1000,
            .tv_nsec = (packet_interval_ms % 1000) * 1000000,
        };
        if (nanosleep(&sleep_time, NULL) < 0)
        {
            perror("nanosleep");
            close(packet_sockfd);
            fclose(file);
            return -1;
        }

        fprintf(stdout, "sent (B): %lu\t", bytes_read);
        if (receive_feedback(packet_sockfd, &packet_interval_ms) < 0)
        {
            close(packet_sockfd);
            fclose(file);
            return -1;
        }
    }
    fprintf(stdout, "\n");

    if (ferror(file) != 0)
    {
        perror("fread");
        close(packet_sockfd);
        fclose(file);
        return -1;
    }

    fclose(file);

    if (send_eof(packet_sockfd, client_addr, client_addr_len) < 0)
    {
        close(packet_sockfd);
        return -1;
    }

    close(packet_sockfd);
    fprintf(stdout, "Transmission completed: %s\n", filename);
    return 0;
}

static int receive_feedback(const int packet_sockfd,
                            uint16_t *const packet_interval_ms)
{
    uint16_t feedback;
    if (recvfrom(packet_sockfd, &feedback, sizeof(feedback), MSG_DONTWAIT, NULL,
                 NULL) < 0)
    {
        if (errno != EAGAIN)
        {
            perror("recvfrom");
            return -1;
        }
    }
    else
    {
        fprintf(stdout, "received packet interval (ms): %hu\n", feedback);
        *packet_interval_ms = feedback;
    }
    return 0;
}

static int send_eof(const int packet_sockfd, struct sockaddr *const client_addr,
                    const socklen_t client_addr_len)
{
    for (size_t i = 0; i < EOF_TRIES; i++)
    {
        if (sendto(packet_sockfd, EOF_PACKET, EOF_PACKET_SIZE, 0, client_addr,
                   client_addr_len) < 0)
        {
            perror("sendto");
            return -1;
        }
    }
    return 0;
}