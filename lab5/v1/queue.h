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

ssize_t read_queue(Queue *queue, uint8_t *buffer, size_t length);
ssize_t write_queue(Queue *queue, uint8_t *data, size_t length);
size_t get_queue_load(Queue *queue);
size_t logger(Queue *queue);
size_t write_log(Queue *queue, char *file_name);

#endif // _QUEUE_H_