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

#include "audio.h"
#include "audiocli.h"
#include "parameter_checkers.h"
#include "pspacing.h"
#include "queue.h"
#include "request_codec.h"
#include "socket_utils.h"

Queue queue;
snd_pcm_t *pcm_handle;

int main(int argc, char *argv[])
{
    const struct sigaction sigint_action = {.sa_handler = sigint_handler};
    if (sigaction(SIGINT, &sigint_action, NULL) < 0)
    {
        perror("sigaction");
        return -1;
    }

    const struct sigaction sigalrm_action = {.sa_handler = sigalrm_handler};
    if (sigaction(SIGALRM, &sigalrm_action, NULL) < 0)
    {
        perror("sigaction");
        return -1;
    }

    Config config;
    if (get_config(argc, argv, &config) != 0) return 1;

    uint8_t buffer[config.buffer_size + 1]; // +1 for circular FIFO capacity
    queue = (Queue){.head = 0,
                    .tail = 0,
                    .length = sizeof(buffer) / sizeof(uint8_t),
                    .data = buffer,
                    .first_timestemp = 1};

    int sockfd = -1;
    if ((sockfd = create_socket_with_first_usable_addr(config.server_info)) ==
        -1)
        return -1;

    /*if (mulawopen(&pcm_handle) < 0)
    {
        close(sockfd);
        return -1;
    }*/

    int status = run(sockfd, &config);

    /*if (mulawclose(pcm_handle) < 0)
    {
        close(sockfd);
        return -1;
    }*/

    close(sockfd);

    return status;
}

static void sigint_handler(int _) { _exit(EXIT_SUCCESS); }

static void sigalrm_handler(int _)
{
    uint8_t buffer[4096];
    ssize_t bytes_read = read_queue(&queue, buffer, 4096);
    if (bytes_read < 0)
    {
        // Though it is unsafe to use in signal handler, we print the error for
        // debugging purposes only.
        fprintf(stderr, "Queue is empty\n");
        fflush(stderr);
    }

    /*if (bytes_read > 0)
        if (mulawwrite(pcm_handle, buffer, bytes_read) < 0)
       _exit(EXIT_FAILURE);*/
}

static int get_config(int argc, char **argv, Config *config)
{
    if (argc < REQUIRED_ARGC)
    {
        fprintf(stderr,
                "Usage: %s <server-ip> <server-port> <audio-filename> "
                "<blocksize> <buffer-size> <target-buffer-occupancy> "
                "<packets-per-seconds> <method=[0(C)|1(D)]> <log-filename>\n",
                argv[0]);
        return -1;
    }

    if (build_addrinfo(&config->server_info, argv[1], argv[2], SOCK_DGRAM) != 0)
        return -1;

    config->audio_filename = argv[3];
    if (check_filename(config->audio_filename) != 0) return -1;

    const unsigned long long blocksize = strtoull(argv[4], NULL, 0);
    if (check_blocksize(blocksize) != 0) return -1;
    config->blocksize = (uint16_t)blocksize;

    config->buffer_size = strtoull(argv[5], NULL, 0) * BUFFER_UNIT_SIZE;
    config->target_buffer_occupancy =
        strtoull(argv[6], NULL, 0) * BUFFER_UNIT_SIZE;

    const long double packets_per_second = strtold(argv[7], NULL);
    if (check_packets_per_second(packets_per_second) != 0) return -1;
    config->packets_per_second = packets_per_second;

    config->method = (unsigned short)strtoul(argv[8], NULL, 0);
    config->log_filename = argv[9];

    if (read_parameters_file(config) < 0) return -1;

    return 0;
}

static int read_parameters_file(Config *const config)
{
    config->epsilon = 0;
    config->beta = 0;

    FILE *const file = fopen(CONGESTION_CONTROL_PARAMETERS_FILENAME, "r");
    if (file == NULL)
    {
        perror("fopen");
        return -1;
    }
    char content[FREAD_BUFFER_SIZE + 1];
    const size_t bytes_read =
        fread(content, sizeof(char), FREAD_BUFFER_SIZE, file);
    fclose(file);

    content[bytes_read] = '\0';
    if (bytes_read == 0) return 0;

    const char *val = strtok(content, "  \t\n\0");
    if (val == NULL) return 0;
    config->epsilon = strtold(val, NULL);

    val = strtok(NULL, "  \t\n\0");
    if (val == NULL) return 0;
    config->beta = strtold(val, NULL);

    return 0;
}

static int run(const int sockfd, Config *config)
{
    while (1)
    {
        request_file(sockfd, config);
        // Start request timeout timer.
        if (setitimer(ITIMER_REAL,
                      &(struct itimerval){{0, FILE_REQUEST_TIMEOUT_MS * 1000},
                                          {0, FILE_REQUEST_TIMEOUT_MS * 1000}},
                      NULL) < 0)
        {
            perror("setitimer");
            return -1;
        }
        if (stream_file_and_cancel_request_timeout(sockfd, config) == 0)
            break;
        else if (errno != EINTR)
            return -1;
    }

    write_log(&queue, config->log_filename);

    return 0;
}

static int request_file(const int sockfd, const Config *const config)
{
    uint8_t request[REQUEST_SIZE];
    encode_request(config->audio_filename, config->blocksize, request);
    if (sendto(sockfd, request, sizeof(request), 0,
               config->server_info->ai_addr,
               config->server_info->ai_addrlen) < 0)
    {
        perror("sendto");
        return -1;
    }
    return 0;
}

static int stream_file_and_cancel_request_timeout(const int sockfd,
                                                  const Config *const config)
{
    unsigned short is_request_successful = 0;

    long double packets_per_second = config->packets_per_second;
    uint8_t buffer[config->blocksize];
    struct sockaddr server_addr;
    socklen_t server_addr_len = sizeof(server_addr);
    while (1)
    {
        ssize_t bytes_read = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                                      &server_addr, &server_addr_len);
        if (bytes_read == 0)
            break;
        else if (bytes_read < 0)
        {
            if (errno != EINTR)
            {
                perror("recvfrom");
                return -1;
            }
            else if (!is_request_successful)
            {
                fprintf(stderr, "Request timed out\n");
                fflush(stderr);
                return -1;
            }
            else
                continue;
        }

        if (!is_request_successful)
        {
            is_request_successful = 1;
            // Start audio playback timer.
            if (setitimer(
                    ITIMER_REAL,
                    &(struct itimerval){{0, AUDIO_REQUEST_INTERVAL_MS * 1000},
                                        {0, AUDIO_REQUEST_INTERVAL_MS * 1000}},
                    NULL) < 0)
            {
                perror("setitimer");
                return -1;
            }
        }

        // We do not need to take care race conditions here as only one writer
        // can touch the write pointer (head) and only one reader can touch the
        // read pointer (tail). There is a possibility that the buffer load is
        // off by one (not up-to-date), but this is an acceptable trade-off.
        if (write_queue(&queue, buffer, bytes_read) < 0)
        {
            fprintf(stderr, "Queue is full\n");
        }
        fprintf(stdout, "enqueued bytes: %lu\t", get_queue_load(&queue));
        fflush(stdout);

        if (send_feedback(sockfd, &server_addr, server_addr_len, config,
                          &packets_per_second) < 0)
            return -1;
    }

    // Cancel timer.
    if (setitimer(ITIMER_REAL, 0, NULL) < 0)
    {
        perror("setitimer");
        return -1;
    }
    return 0;
}

static int send_feedback(const int sockfd,
                         const struct sockaddr *const server_addr,
                         const socklen_t server_addr_len,
                         const Config *const config,
                         long double *const packets_per_second)
{
    if (config->method == CONGESTION_CONTROL_METHOD_C)
        *packets_per_second =
            update_packet_rate_methed_c(*packets_per_second, config);
    else if (config->method == CONGESTION_CONTROL_METHOD_D)
        *packets_per_second =
            update_packet_rate_methed_d(*packets_per_second, config);

    fprintf(stdout, "packets per sec: %Lf\t", *packets_per_second);
    uint16_t packet_interval = to_pspacing_ms(*packets_per_second);
    fprintf(stdout, "packet interval (ms): %hu\n", packet_interval);

    if (sendto(sockfd, &packet_interval, sizeof(uint16_t), 0, server_addr,
               server_addr_len) < 0)
    {
        if (errno != EINTR) // Ignore EINTR from audio playback timer.
        {
            perror("sendto");
            return -1;
        }
    }

    return 0;
}

static long double
update_packet_rate_methed_c(const long double packets_per_second,
                            const Config *const config)
{
    const long double bytes_per_second = packets_per_second * config->blocksize;
    const long long occupancy_diff =
        (long long)config->target_buffer_occupancy - get_queue_load(&queue);
    const long double new_bytes_per_second =
        bytes_per_second + config->epsilon * occupancy_diff;
    if (new_bytes_per_second < 0) return 0;
    return new_bytes_per_second / config->blocksize;
}

static long double
update_packet_rate_methed_d(const long double packets_per_second,
                            const Config *const config)
{
    const long double bytes_per_second = packets_per_second * config->blocksize;
    const long long occupancy_diff =
        (long long)config->target_buffer_occupancy - get_queue_load(&queue);

    const long double audio_request_bytes_per_second =
        (long double)1000.0 / AUDIO_REQUEST_INTERVAL_MS * AUDIO_FRAME_SIZE;
    const long double net_influx_bytes_per_second =
        bytes_per_second - audio_request_bytes_per_second;
    const long double new_bytes_per_second =
        bytes_per_second + config->epsilon * occupancy_diff -
        config->beta * net_influx_bytes_per_second;
    if (new_bytes_per_second < 0) return 0;
    return new_bytes_per_second / config->blocksize;
}