#include <netdb.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "arg_checkers.h"
#include "packet_codec.h"
#include "request_codec.h"
#include "socket_utils.h"

#define REQUIRED_ARGC 7
#define DUMMY_END_OF_PACKET 255

int sockfd = -1;

void tear_down() { close(sockfd); }

static void sigint_handler(int _)
{
    tear_down();
    exit(EXIT_SUCCESS);
}

static void sigalrm_handler(int _) { return; }

typedef struct
{
    uint16_t secret_key;
    uint16_t blocksize; // <= 1471
    uint8_t windowsize; // <= 63
    unsigned long long timeout_microseconds;
} Config;

int parse_args(int argc, char *argv[], struct addrinfo **server_info,
               Config *const config)
{
    if (argc < REQUIRED_ARGC)
    {
        fprintf(stderr,
                "Usage: %s <server_ip> <server_port> <secret_key> <blocksize> "
                "<windowsize> <timeout>\n",
                argv[0]);
        return -1;
    }

    int status;

    if ((status = build_addrinfo(server_info, argv[1], argv[2], SOCK_DGRAM)) !=
        0)
        return status;

    long long secret_key = strtoull(argv[3], NULL, 0);
    if ((status = check_secret_key(secret_key)) != 0) return status;
    config->secret_key = (uint16_t)secret_key;

    long long blocksize = strtoull(argv[4], NULL, 0);
    if ((status = check_blocksize(blocksize)) != 0) return status;
    config->blocksize = (uint16_t)blocksize;

    long long windowsize = strtoull(argv[5], NULL, 0);
    if ((status = check_windowsize(windowsize)) != 0) return status;
    config->windowsize = (uint8_t)windowsize;

    long long timeout_microseconds = strtoull(argv[6], NULL, 0);
    if (timeout_microseconds < 0)
    {
        fprintf(stderr, "Timeout must be non-negative\n");
        return -1;
    }
    config->timeout_microseconds = timeout_microseconds;

    return 0;
}

int read_request(char *const filename, uint16_t *const secret_key,
                 struct sockaddr *const client_addr,
                 socklen_t *const client_addr_len)
{
    uint8_t request[REQUEST_SIZE];
    const ssize_t bytes_read = recvfrom(sockfd, request, REQUEST_SIZE, 0,
                                        client_addr, client_addr_len);
    if (bytes_read == -1)
    {
        perror("read");
        return -1;
    }

    decode_request(request, filename, secret_key);
    return 0;
}

int send_packet(const uint8_t *const data, const size_t data_size,
                const struct sockaddr *const client_addr,
                const socklen_t client_addr_len, const int append_dummy_end)
{
    uint8_t packet[append_dummy_end ? 1 + data_size + 1 : 1 + data_size];
    encode_packet(99, data, data_size, packet);
    if (append_dummy_end) packet[data_size + 1] = DUMMY_END_OF_PACKET;
    if (sendto(sockfd, packet, sizeof(packet), 0, client_addr,
               client_addr_len) < 0)
    {
        perror("sendto");
        return -1;
    }
    return 0;
}

int send_file(const char *const filename, const Config *const config,
              const struct sockaddr *const client_addr,
              const socklen_t client_addr_len)
{
    FILE *const file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("fopen");
        return -1;
    }

    uint8_t prev_buf[config->blocksize];
    uint8_t curr_buf[config->blocksize];
    size_t prev_bytes_read;
    size_t curr_bytes_read;
    prev_bytes_read = fread(prev_buf, sizeof(uint8_t), sizeof(prev_buf), file);
    printf("%ld bytes read\t", prev_bytes_read);
    printf("\n");
    while (1)
    {
        curr_bytes_read =
            fread(curr_buf, sizeof(uint8_t), sizeof(curr_buf), file);
        printf("%ld bytes read\t", curr_bytes_read);
        if (ferror(file))
        {
            perror("fread");
            return -1;
        }
        if (feof(file))
        {
            if (curr_bytes_read == 0)
                // File size is zero or smaller than blocksize
                if (prev_bytes_read < config->blocksize)
                {
                    printf("send %ld prev\t", prev_bytes_read);
                    if (send_packet(prev_buf, prev_bytes_read, client_addr,
                                    client_addr_len, 0) < 0)
                        return -1;
                }
                else // File size is a multiple of blocksize
                {
                    printf("send %ld prev\t", prev_bytes_read + 1);
                    if (send_packet(prev_buf, prev_bytes_read, client_addr,
                                    client_addr_len, 1) < 0)
                        return -1;
                }

            // File size is larger than blocksize and not a multiple of
            // blocksize
            else
            {
                printf("send %ld prev\t", prev_bytes_read);
                if (send_packet(prev_buf, prev_bytes_read, client_addr,
                                client_addr_len, 0) < 0)
                    return -1;

                printf("send %ld curr\t", curr_bytes_read);
                if (send_packet(curr_buf, curr_bytes_read, client_addr,
                                client_addr_len, 0) < 0)
                    return -1;
            }
            printf("EOF\n");
            break;
        }

        printf("send %ld prev", prev_bytes_read);
        send_packet(prev_buf, prev_bytes_read, client_addr, client_addr_len, 0);

        printf("\n");
    }

    fclose(file);

    return 0;
}

int run(const Config *const config)
{
    while (1)
    {
        // Accept file request
        uint16_t secret_key;
        char filename[MAX_FILENAME_LEN];
        struct sockaddr client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        if (read_request(filename, &secret_key, &client_addr,
                         &client_addr_len) < 0)
            continue;

        if (config->secret_key != secret_key)
        {
            fprintf(stderr, "Secret key mismatch: %hu != %hu\n",
                    config->secret_key, secret_key);
            continue;
        }

        if (check_filename(filename) != 0) continue;

        // Check if file exists
        if (access(filename, F_OK) == -1)
        {
            perror("access");
            continue;
        }

        printf("Received request for file %s\n", filename);

        fflush(stdout);
        const pid_t pid = fork();
        if (pid == 0)
        {
            // Child process
            if (send_file(filename, config, &client_addr, client_addr_len) < 0)
                exit(EXIT_FAILURE);
            exit(EXIT_SUCCESS);
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    const struct sigaction sigint_action = {.sa_handler = sigint_handler};
    sigaction(SIGINT, &sigint_action, NULL);
    const struct sigaction sigalrm_action = {.sa_handler = sigalrm_handler};
    sigaction(SIGALRM, &sigalrm_action, NULL);

    struct addrinfo *server_info;
    Config config;
    if (parse_args(argc, argv, &server_info, &config) != 0) return -1;

    if ((sockfd = create_socket_with_first_usable_addr(server_info)) == -1)
        return -1;

    if ((bind_socket_with_first_usable_addr(server_info, sockfd)) == -1)
    {
        close(sockfd);
        return -1;
    }

    return run(&config);
}