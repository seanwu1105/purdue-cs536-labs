#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "arg_checkers.h"
#include "packet_codec.h"
#include "request_codec.h"
#include "socket_utils.h"

#define REQUIRED_ARGC 7
#define DUMMY_END_OF_PACKET 255

int request_sockfd = -1;
int packet_sockfd = -1;

void tear_down()
{
    close(request_sockfd);
    close(packet_sockfd);
}

static void sigint_handler(int _)
{
    tear_down();
    exit(EXIT_SUCCESS);
}

static void sigalrm_handler(int _)
{
    printf("timeout\n");
    return;
}

typedef struct
{
    uint16_t secret_key;
    uint16_t blocksize; // <= 1471
    uint8_t windowsize; // <= 63
    unsigned long long timeout_usec;
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

    long long timeout_usec = strtoull(argv[6], NULL, 0);
    if (timeout_usec < 0)
    {
        fprintf(stderr, "Timeout must be non-negative\n");
        return -1;
    }
    config->timeout_usec = timeout_usec;

    return 0;
}

int read_request(char *const filename, uint16_t *const secret_key,
                 struct sockaddr *const client_addr,
                 socklen_t *const client_addr_len)
{
    uint8_t request[REQUEST_SIZE];
    const ssize_t bytes_read = recvfrom(request_sockfd, request, REQUEST_SIZE,
                                        0, client_addr, client_addr_len);
    if (bytes_read == -1)
    {
        perror("read");
        return -1;
    }

    decode_request(request, filename, secret_key);
    return 0;
}

int receive_ack_and_cancel_timeout(const uint8_t expected)
{
    while (1)
    {
        uint8_t ack;
        printf(" Waiting ACK... ");
        if (recvfrom(packet_sockfd, &ack, sizeof(ack), 0, NULL, NULL) < 0)
        {
            if (errno != EINTR) perror("recvfrom");
            return -1;
        }
        printf("ACK received: %u (expect: %u)\t", ack, expected);

        if (ack == expected)
        {
            setitimer(ITIMER_REAL, 0, NULL); // Cancel ack timout timer
            break;
        }
    }
    return 0;
}

int send_window(const uint8_t *const data, const size_t data_size,
                const struct sockaddr *const client_addr,
                const socklen_t client_addr_len, const int is_eof,
                const Config *const config,
                uint8_t *const initial_sequence_number)
{
    while (1)
    {
        uint8_t sequence_number = *initial_sequence_number;
        size_t block_index = 0;
        if ((packet_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        {
            perror("socket");
            return -1;
        }

        while (block_index < data_size)
        {
            size_t blocksize = data_size - block_index < config->blocksize
                                   ? data_size - block_index
                                   : config->blocksize;
            uint8_t block_data[blocksize];
            memcpy(block_data, data + block_index, blocksize * sizeof(uint8_t));

            unsigned short need_to_append_dummy =
                is_eof && (block_index + config->blocksize) == data_size;

            uint8_t packet[1 + blocksize + (need_to_append_dummy ? 1 : 0)];
            encode_packet(sequence_number, block_data, blocksize, packet);
            if (need_to_append_dummy)
                packet[blocksize + 1] = DUMMY_END_OF_PACKET;

            if (sendto(packet_sockfd, packet, sizeof(packet), 0, client_addr,
                       client_addr_len) == -1)
            {
                close(packet_sockfd);
                perror("sendto");
                return -1;
            }

            printf("send num: %u\n", sequence_number);

            sequence_number++;
            block_index += config->blocksize;
        }

        // Start ACK timeout timer
        setitimer(ITIMER_REAL,
                  &(struct itimerval){{0, config->timeout_usec},
                                      {0, config->timeout_usec}},
                  NULL);

        int ack_status = receive_ack_and_cancel_timeout(sequence_number - 1);
        close(packet_sockfd);
        if (ack_status == 0)
            break;
        else if (errno != EINTR)
            return -1;
    }

    *initial_sequence_number =
        (*initial_sequence_number + config->windowsize) %
        (SEQUENCE_NUMBER_SPACE_TO_WINDOWSIZE_RATIO * config->windowsize);
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

    uint8_t prev_buf[config->blocksize * config->windowsize];
    uint8_t curr_buf[config->blocksize * config->windowsize];
    size_t prev_bytes_read;
    size_t curr_bytes_read;
    prev_bytes_read = fread(prev_buf, sizeof(uint8_t), sizeof(prev_buf), file);
    printf("%ld bytes read\n", prev_bytes_read);
    uint8_t initial_sequence_number = 0;
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
                if (prev_bytes_read < config->blocksize * config->windowsize)
                // File size is zero or smaller than blocksize * windowsize
                {
                    printf("send %ld bytes with prev\t", prev_bytes_read);
                    if (send_window(prev_buf, prev_bytes_read, client_addr,
                                    client_addr_len, 1, config,
                                    &initial_sequence_number) < 0)
                        return -1;
                }
                else // File size is a multiple of blocksize * windowsize
                {
                    printf("send %ld bytes with prev\t", prev_bytes_read + 1);
                    if (send_window(prev_buf, prev_bytes_read, client_addr,
                                    client_addr_len, 1, config,
                                    &initial_sequence_number) < 0)
                        return -1;
                }

            // File size is larger than blocksize and not a multiple of
            // blocksize * windowsize
            else
            {
                printf("send %ld bytes with prev\t", prev_bytes_read);
                if (send_window(prev_buf, prev_bytes_read, client_addr,
                                client_addr_len, 0, config,
                                &initial_sequence_number) < 0)
                    return -1;

                printf("send %ld bytes with curr\t", curr_bytes_read);
                if (send_window(curr_buf, curr_bytes_read, client_addr,
                                client_addr_len, 1, config,
                                &initial_sequence_number) < 0)
                    return -1;
            }
            printf("EOF\n");
            break;
        }

        printf("send %ld bytes with prev", prev_bytes_read);
        send_window(prev_buf, prev_bytes_read, client_addr, client_addr_len, 0,
                    config, &initial_sequence_number);

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
            close(request_sockfd);
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

    if ((request_sockfd = create_socket_with_first_usable_addr(server_info)) ==
        -1)
        return -1;

    if ((bind_socket_with_first_usable_addr(server_info, request_sockfd)) == -1)
    {
        close(request_sockfd);
        return -1;
    }

    return run(&config);
}