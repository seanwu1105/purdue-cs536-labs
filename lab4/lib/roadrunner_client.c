#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "packet_codec.h"
#include "roadrunner_client.h"

int send_ack(const int sockfd, const uint8_t sequence_number,
             const struct sockaddr *const server_addr,
             const socklen_t server_addr_len)
{
    if (sendto(sockfd, &sequence_number, sizeof(sequence_number), 0,
               server_addr, server_addr_len) < 0)
    {
        perror("sendto");
        return -1;
    }

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

int receive_window_and_cancel_timeout(const int sockfd,
                                      const Config *const config,
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
            if (send_ack(sockfd, previous_ack, &server_addr, server_addr_len) <
                0)
                return -1;
            continue;
        }
        if (num >= initial_sequence_number + config->windowsize)
        {
            uint8_t previous_ack =
                initial_sequence_number +
                config->windowsize * SEQUENCE_NUMBER_SPACE_TO_WINDOWSIZE_RATIO -
                1;
            if (send_ack(sockfd, previous_ack, &server_addr, server_addr_len) <
                0)
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
        if (send_ack(sockfd, last_num, &server_addr, server_addr_len) < 0)
            return -1;

    return 0;
}

int receive_file_and_cancel_timeout(const int sockfd,
                                    const Config *const config)
{
    size_t initial_sequence_number = 0;
    uint8_t is_eof = 0;

    do
    {
        if (receive_window_and_cancel_timeout(
                sockfd, config, initial_sequence_number, &is_eof) < 0)
            return -1;
        initial_sequence_number =
            (initial_sequence_number + config->windowsize) %
            (SEQUENCE_NUMBER_SPACE_TO_WINDOWSIZE_RATIO * config->windowsize);
    } while (is_eof == 0);

    return 0;
}