#if !defined(_LOGGER_H_)
#define _LOGGER_H_

#include <stdio.h>
#include <sys/time.h>

int log_stream(FILE *stream, const long long int data,
               struct timeval *const init_time, unsigned short *const is_first,
               const char *const data_title);
int dump_logging(const char *const filename, const char *const logging);

#endif // _LOGGER_H_