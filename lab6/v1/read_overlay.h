#if !defined(_READ_OVERLAY_H_)
#define _READ_OVERLAY_H_

#include <arpa/inet.h>
#include <stdlib.h>

#include "zzconfig_codec.h"

#define OVERLAY_FILENAME "zzoverlay.dat"

typedef struct
{
    char ip[INET_ADDRSTRLEN];
    char port[PORT_STRLEN];
    ForwardingPath forward_path;
    ForwardingPath return_path;
} OverlayEntry;

ssize_t read_overlay(OverlayEntry entries[]);

#endif // _READ_OVERLAY_H_