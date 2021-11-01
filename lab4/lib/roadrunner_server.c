#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "packet_codec.h"
#include "roadrunner_server.h"

int receive_ack_and_cancel_timeout(const int sockfd, const uint8_t expected)
{
    while (1)
    {
        uint8_t ack;
        if (recvfrom(sockfd, &ack, sizeof(ack), 0, NULL, NULL) < 0)
        {
            if (errno != EINTR) perror("recvfrom");
            return -1;
        }

        if (ack == expected)
        {
            setitimer(ITIMER_REAL, 0, NULL); // Cancel ack timout timer
            break;
        }
    }
    return 0;
}

int send_window(int *const sockfd, const uint8_t *const data,
                const size_t data_size,
                const struct sockaddr *const client_addr,
                const socklen_t client_addr_len, const int is_eof,
                const Config *const config,
                uint8_t *const initial_sequence_number)
{
    while (1)
    {
        uint8_t sequence_number = *initial_sequence_number;
        size_t block_index = 0;
        if ((*sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
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

            if (sendto(*sockfd, packet, sizeof(packet), 0, client_addr,
                       client_addr_len) == -1)
            {
                close(*sockfd);
                perror("sendto");
                return -1;
            }

            sequence_number++;
            block_index += config->blocksize;
        }

        // Start ACK timeout timer
        setitimer(ITIMER_REAL,
                  &(struct itimerval){{config->timeout_usec / 1000000,
                                       config->timeout_usec % 1000000},
                                      {config->timeout_usec / 1000000,
                                       config->timeout_usec % 1000000}},
                  NULL);

        int ack_status =
            receive_ack_and_cancel_timeout(*sockfd, sequence_number - 1);
        close(*sockfd);
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

int send_file(int *const sockfd, const char *const filename,
              const Config *const config,
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
    if (ferror(file))
    {
        perror("fread");
        fclose(file);
        return -1;
    }

    uint8_t initial_sequence_number = 0;
    while (1)
    {
        curr_bytes_read =
            fread(curr_buf, sizeof(uint8_t), sizeof(curr_buf), file);
        if (ferror(file))
        {
            perror("fread");
            fclose(file);
            return -1;
        }
        if (feof(file))
        {
            if (curr_bytes_read == 0)
                if (prev_bytes_read < config->blocksize * config->windowsize)
                // File size is zero or smaller than blocksize * windowsize
                {
                    if (send_window(sockfd, prev_buf, prev_bytes_read,
                                    client_addr, client_addr_len, 1, config,
                                    &initial_sequence_number) < 0)
                    {
                        fclose(file);
                        return -1;
                    }
                }
                else // File size is a multiple of blocksize * windowsize
                {
                    if (send_window(sockfd, prev_buf, prev_bytes_read,
                                    client_addr, client_addr_len, 1, config,
                                    &initial_sequence_number) < 0)
                    {
                        fclose(file);
                        return -1;
                    }
                }

            // File size is larger than blocksize and not a multiple of
            // blocksize * windowsize
            else
            {
                if (send_window(sockfd, prev_buf, prev_bytes_read, client_addr,
                                client_addr_len, 0, config,
                                &initial_sequence_number) < 0)
                {
                    fclose(file);
                    return -1;
                }

                if (send_window(sockfd, curr_buf, curr_bytes_read, client_addr,
                                client_addr_len, 1, config,
                                &initial_sequence_number) < 0)
                {
                    fclose(file);
                    return -1;
                }
            }
            break;
        }

        send_window(sockfd, prev_buf, prev_bytes_read, client_addr,
                    client_addr_len, 0, config, &initial_sequence_number);

        prev_bytes_read = curr_bytes_read;
        memcpy(prev_buf, curr_buf, curr_bytes_read * sizeof(uint8_t));
    }

    fclose(file);

    fprintf(stdout, "Transmission completed\n");

    return 0;
}
