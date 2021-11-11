#include <stdint.h>

#include "pspacing.h"

uint16_t to_pspacing_ms(const long double packets_per_second)
{
    if (packets_per_second == 0) return MAX_PSPACING_MS;
    uint16_t packet_interval_ms = (uint16_t)(1000.0 / packets_per_second);
    return packet_interval_ms > MAX_PSPACING_MS ? MAX_PSPACING_MS
                                                : packet_interval_ms;
}