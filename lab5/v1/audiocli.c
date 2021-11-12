#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../lib/audio_client.h"
#include "../lib/congestion_controls.h"
#include "../lib/queue.h"

static Queue queue;
static snd_pcm_t *pcm_handle = NULL;

static void sigint_handler(int _) { _exit(EXIT_SUCCESS); }

static void sigalrm_handler(int _)
{
    if (stream_audio_to_device(&pcm_handle, &queue) < 0) _exit(EXIT_FAILURE);
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
    if (get_config(
            argc, argv, &config,
            "Usage: %s <server-ip> <server-port> <audio-filename> "
            "<blocksize> <buffer-size> <target-buffer-occupancy> "
            "<packets-per-seconds> <method=[0(C)|1(D)]> <log-filename>\n") != 0)
        return 1;

    // Build circular queue.
    uint8_t buffer[config.buffer_size + 1]; // +1 for circular FIFO capacity.
    queue = (Queue){.head = 0,
                    .tail = 0,
                    .length = sizeof(buffer) / sizeof(uint8_t),
                    .data = buffer};

    // Register congestion control methods.
    CongestionControlMethod congestion_control_methods[] = {
        update_packet_rate_methed_c, update_packet_rate_methed_d};

    return start_client(&pcm_handle, &queue, &config,
                        congestion_control_methods);
}
