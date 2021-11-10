#include <stdint.h>

#include "pspacing.h"

uint16_t to_pspacing_ms(const long double packets_per_second)
{
    if (packets_per_second < 0) return 0;
    if (packets_per_second == 0) return MAX_PSPACING_MS;
    return (uint16_t)(1000.0 / packets_per_second);
}