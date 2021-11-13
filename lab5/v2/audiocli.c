#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../lib/audio_client.h"
#include "../lib/congestion_controls.h"
#include "../lib/logger.h"
#include "../lib/queue.h"

static Queue queue;
static snd_pcm_t *pcm_handle = NULL;

// For logging
FILE *logging_stream;
struct timeval init_time;
unsigned short is_first = 1;

static void sigint_handler(int _) { _exit(EXIT_SUCCESS); }

static void sigalrm_handler(int _)
{
    if (stream_audio_to_device(&pcm_handle, &queue, logging_stream, &init_time,
                               &is_first) < 0)
        _exit(EXIT_FAILURE);
}

long double update_packet_rate_methed_e(const long double packets_per_second,
                                        const Config *const config,
                                        const Queue *const queue)
{
    // TODO: Be creative(?) to implement this function.
    return packets_per_second;
}

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

    // Set configuration.
    Config config;
    if (get_config(argc, argv, &config,
                   "Usage: %s <server-ip> <server-port> <audio-filename> "
                   "<blocksize> <buffer-size> <target-buffer-occupancy> "
                   "<packets-per-seconds> <method=[0(C)|1(D)|2(E)]> "
                   "<log-filename>\n") != 0)
        return -1;

    // Build circular queue.
    uint8_t buffer[config.buffer_size + 1]; // +1 for circular FIFO capacity.
    queue = (Queue){.head = 0,
                    .tail = 0,
                    .length = sizeof(buffer) / sizeof(uint8_t),
                    .data = buffer};

    // Register congestion control methods.
    CongestionControlMethod congestion_control_methods[] = {
        update_packet_rate_methed_c, update_packet_rate_methed_d,
        update_packet_rate_methed_e};

    char *logging_buffer = NULL;
    size_t logging_buffer_size = 0;
    logging_stream = open_memstream(&logging_buffer, &logging_buffer_size);
    if (logging_stream == NULL)
    {
        perror("open_memstream");
        return -1;
    }

    if (start_client(&pcm_handle, &queue, &config, congestion_control_methods,
                     logging_stream, &init_time, &is_first) < 0)
        return -1;

    fflush(logging_stream);
    fclose(logging_stream);
    if (dump_logging(config.log_filename, logging_buffer) < 0) return -1;

    free(logging_buffer);
    return 0;
}
