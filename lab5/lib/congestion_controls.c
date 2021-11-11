
#include "congestion_controls.h"
#include "audio_client.h"
#include "queue.h"

long double update_packet_rate_methed_c(const long double packets_per_second,
                                        const Config *const config,
                                        const Queue *const queue)
{
    const long double bytes_per_second = packets_per_second * config->blocksize;
    const long long occupancy_diff =
        (long long)config->target_buffer_occupancy - get_queue_load(queue);
    const long double new_bytes_per_second =
        bytes_per_second + config->epsilon * occupancy_diff;
    if (new_bytes_per_second < 0) return 0;
    return new_bytes_per_second / config->blocksize;
}

long double update_packet_rate_methed_d(const long double packets_per_second,
                                        const Config *const config,
                                        const Queue *const queue)
{
    const long double bytes_per_second = packets_per_second * config->blocksize;
    const long long occupancy_diff =
        (long long)config->target_buffer_occupancy - get_queue_load(queue);

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