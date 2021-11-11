#if !defined(_PSPACING_H_)
#define _PSPACING_H_

#include <stdint.h>

#define MAX_PSPACING_MS 2000

uint16_t to_pspacing_ms(const long double packets_per_second);

#endif // _PSPACING_H_