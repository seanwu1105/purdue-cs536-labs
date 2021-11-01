#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "arg_checkers.h"
#include "packet_codec.h"
#include "request_codec.h"
#include "socket_utils.h"

#define REQUIRED_ARGC 7
#define FILE_REQUEST_TIMEOUT_MS 500
#define NUM_EOF_ACKS 8

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
    const char *filename;
    uint16_t secret_key;
    uint16_t blocksize; // <= 1471
    uint8_t windowsize; // <= 63
} Config;

int parse_args(int argc, char *argv[], struct addrinfo **server_info,
               Config *const config)
{
    if (argc < REQUIRED_ARGC)
    {
        fprintf(stderr,
                "Usage: %s <server_ip> <server_port> <filename> "
                "<secret_key> <blocksize> <windowsize>\n",
                argv[0]);
        return -1;
    }

    int status;

    if ((status = build_addrinfo(server_info, argv[1], argv[2], SOCK_DGRAM)) !=
        0)
        return status;

    config->filename = argv[3];
    if ((status = check_filename(config->filename)) != 0) return status;

    long long secret_key = strtoull(argv[4], NULL, 0);
    if ((status = check_secret_key(secret_key)) != 0) return status;
    config->secret_key = (uint16_t)secret_key;

    long long blocksize = strtoull(argv[5], NULL, 0);
    if ((status = check_blocksize(blocksize)) != 0) return status;
    config->blocksize = (uint16_t)blocksize;

    long long windowsize = strtoull(argv[6], NULL, 0);
    if ((status = check_windowsize(windowsize)) != 0) return status;
    config->windowsize = (uint8_t)windowsize;

    return 0;
}

int request_file(const struct addrinfo *const server_info,
                 const Config *const config)
{
    uint8_t request[REQUEST_SIZE];
    encode_request(config->filename, config->secret_key, request);
    if (sendto(sockfd, request, sizeof(request), 0, server_info->ai_addr,
               server_info->ai_addrlen) < 0)
    {
        perror("sendto");
        return -1;
    }

    return 0;
}

int send_ack(const uint8_t sequence_number,
             const struct sockaddr *const server_addr,
             const socklen_t server_addr_len)
{
    if (sendto(sockfd, &sequence_number, sizeof(sequence_number), 0,
               server_addr, server_addr_len) < 0)
    {
        perror("sendto");
        return -1;
    }

    printf("ACK sent: %d\n", sequence_number);
    return 0;
}

int append_window_to_file(uint8_t *window_data, const uint8_t windowsize,
                          const size_t last_blocksize,
                          const Config *const config)
{
    FILE *file = fopen(config->filename, "a");
    if (file == NULL)
    {
        perror("fopen");
        return -1;
    }

    for (size_t i = 0; i < windowsize; i++)
    {
        fwrite(window_data + i * config->blocksize, sizeof(uint8_t),
               i == windowsize - 1 ? last_blocksize : config->blocksize, file);

        if (ferror(file))
        {
            perror("fwrite");
            fclose(file);
            return -1;
        }
    }

    fclose(file);
    return 0;
}

int receive_window_and_cancel_timeout(const Config *const config,
                                      const uint8_t initial_sequence_number,
                                      uint8_t *const is_eof)
{
    *is_eof = 0;
    uint8_t last_num = initial_sequence_number + config->windowsize - 1;
    uint8_t received_sequence_numbers[config->windowsize];
    memset(received_sequence_numbers, 0, sizeof(received_sequence_numbers));
    uint8_t window_data[config->windowsize][config->blocksize];

    uint8_t buffer[config->blocksize + 2];
    ssize_t bytes_read;
    struct sockaddr server_addr;
    socklen_t server_addr_len = sizeof(server_addr);
    while ((bytes_read = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                                  &server_addr, &server_addr_len)) > 0)
    {
        setitimer(ITIMER_REAL, 0, NULL); // Cancel request timout timer

        uint8_t num;
        size_t blocksize = bytes_read > config->blocksize + 1
                               ? config->blocksize
                               : bytes_read - 1;
        uint8_t block[blocksize];
        decode_packet(buffer, &num, sizeof(block), block);

        if (num < initial_sequence_number)
        {
            uint8_t previous_ack = initial_sequence_number - 1;
            if (send_ack(previous_ack, &server_addr, server_addr_len) < 0)
                return -1;
            continue;
        }
        if (num >= initial_sequence_number + config->windowsize)
        {
            uint8_t previous_ack =
                initial_sequence_number +
                config->windowsize * SEQUENCE_NUMBER_SPACE_TO_WINDOWSIZE_RATIO -
                1;
            if (send_ack(previous_ack, &server_addr, server_addr_len) < 0)
                return -1;
            continue;
        }

        received_sequence_numbers[num - initial_sequence_number] = 1;
        memcpy(window_data[num - initial_sequence_number], block, blocksize);

        if (bytes_read != config->blocksize + 1)
        {
            last_num = num;
            *is_eof = 1;
        }

        uint8_t completed = 1;
        for (size_t i = initial_sequence_number; i <= last_num; i++)
            if (received_sequence_numbers[i - initial_sequence_number] == 0)
            {
                completed = 0;
                break;
            }

        if (completed)
        {
            if (append_window_to_file((uint8_t *)window_data,
                                      last_num - initial_sequence_number + 1,
                                      blocksize, config) < 0)
                return -1;
            break;
        }
    }

    if (bytes_read < 0)
    {
        if (errno != EINTR) perror("recvfrom");
        return -1;
    }

    // If is EOF, send 8 duplicate ACKs. Otherwise, send one ACK.
    for (size_t i = 0; i < (*is_eof ? NUM_EOF_ACKS : 1); i++)
        if (send_ack(last_num, &server_addr, server_addr_len) < 0) return -1;

    return 0;
}

int receive_file_and_cancel_timeout(const Config *const config)
{
    size_t initial_sequence_number = 0;
    uint8_t is_eof = 0;

    do
    {
        if (receive_window_and_cancel_timeout(config, initial_sequence_number,
                                              &is_eof) < 0)
            return -1;
        initial_sequence_number =
            (initial_sequence_number + config->windowsize) %
            (SEQUENCE_NUMBER_SPACE_TO_WINDOWSIZE_RATIO * config->windowsize);
    } while (is_eof == 0);

    return 0;
}

int run(const struct addrinfo *const server_info, const Config *const config)
{
    // Check if file exists
    if (access(config->filename, F_OK) == 0)
    {
        fprintf(stdout, "File already exists: %s\n", config->filename);
        return -1;
    }

    // Request file
    while (1)
    {
        request_file(server_info, config);
        // Start request timeout timer
        setitimer(ITIMER_REAL,
                  &(struct itimerval){{0, FILE_REQUEST_TIMEOUT_MS * 1000},
                                      {0, FILE_REQUEST_TIMEOUT_MS * 1000}},
                  NULL);
        if (receive_file_and_cancel_timeout(config) == 0)
            break;
        else if (errno != EINTR)
            return -1;
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

    int status = run(server_info, &config);

    tear_down();

    return status;
}