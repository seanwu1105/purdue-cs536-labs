#if !defined(_CONGESTION_CONTROLS_H_)
#define _CONGESTION_CONTROLS_H_

#include "audio_client.h"
#include "queue.h"

long double update_packet_rate_methed_c(const long double packets_per_second,
                                        const Config *const config,
                                        const Queue *const queue);

long double update_packet_rate_methed_d(const long double packets_per_second,
                                        const Config *const config,
                                        const Queue *const queue);

#endif // _CONGESTION_CONTROLS_H_