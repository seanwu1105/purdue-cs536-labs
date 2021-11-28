#if !defined(_READ_OVERLAY_H_)
#define _READ_OVERLAY_H_

#define OVERLAY_FILENAME "zzoverlay.dat"
#define MAX_FORWARDING_PAIRS 10

#include "zzconfig_codec.h"

typedef struct
{
    char *ip;
    char *port;
    ForwardingPair forwarding_pairs[MAX_FORWARDING_PAIRS];
    int num_forwarding_pairs;
} OverlayEntry;

int read_overlay(OverlayEntry entries[], size_t *const num_fwd_pairs);

#endif // _READ_OVERLAY_H_