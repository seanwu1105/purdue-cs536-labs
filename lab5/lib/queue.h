#if !defined(_QUEUE_H_)
#define _QUEUE_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

typedef struct
{
    size_t head;
    size_t tail;
    size_t length; // The capacity is length - 1.
    uint8_t *data;
    char *log_buffer;
    FILE *log_stream;
    uint8_t first_timestemp;
    struct timeval init_timestemp;
} Queue;

ssize_t read_queue(Queue *const queue, uint8_t *const buffer,
                   const size_t length);
ssize_t write_queue(Queue *const queue, const uint8_t *const data,
                    const size_t length);
size_t get_queue_load(const Queue *const queue);
size_t logger(Queue *const queue);
size_t write_log(Queue *const queue, char *file_name);
#endif // _QUEUE_H_