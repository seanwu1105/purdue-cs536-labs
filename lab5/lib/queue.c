#include "queue.h"

static int is_queue_empty(const Queue *const queue)
{
    return queue->head == queue->tail;
}

static int is_queue_full(const Queue *const queue)
{
    return (queue->head + 1) % queue->length == queue->tail;
}

static int dequeue(Queue *const queue, uint8_t *const value)
{
    if (is_queue_empty(queue)) return -1;
    *value = queue->data[queue->tail];
    queue->tail = (queue->tail + 1) % queue->length;
    return 0;
}

static int enqueue(Queue *const queue, const uint8_t value)
{
    if (is_queue_full(queue)) return -1;
    queue->data[queue->head] = value;
    queue->head = (queue->head + 1) % queue->length;
    return 0;
}

ssize_t read_queue(Queue *const queue, uint8_t *const buffer,
                   const size_t length)
{
    size_t read = 0;
    while (read < length)
    {
        if (dequeue(queue, buffer + read) < 0) return -1;
        read++;
    }
    if (logger(queue) < 0) return -1;
    return read;
}

ssize_t write_queue(Queue *const queue, const uint8_t *const data,
                    const size_t length)
{
    size_t written = 0;
    while (written < length)
    {
        if (enqueue(queue, data[written]) < 0) return -1;
        written++;
    }
    if (logger(queue) < 0) return -1;
    return written;
}

size_t get_queue_load(const Queue *const queue)
{
    if (queue->head >= queue->tail)
        return queue->head - queue->tail;
    else
        return queue->length + queue->head - queue->tail;
}

size_t logger(Queue *const queue)
{
    if (queue->first_timestemp)
    {
        queue->log_buffer = NULL;
        size_t log_buffer_size = 0;
        queue->log_stream =
            open_memstream(&(queue->log_buffer), &log_buffer_size);

        if (!queue->log_stream)
        {
            perror("open_memstream");
            return -1;
        }
        if (gettimeofday(&(queue->init_timestemp), NULL) < 0)
        {
            perror("gettimeofday");
            return -1;
        }
        fprintf(queue->log_stream, "time (us), Q(t) (bytes)\n");
        fprintf(queue->log_stream, "0, %ld\n", get_queue_load(queue));
        queue->first_timestemp = 0;
    }
    else
    {
        struct timeval cur_timestemp;
        if (gettimeofday(&cur_timestemp, NULL) < 0)
        {
            perror("gettimeofday");
            return -1;
        }
        fprintf(queue->log_stream, "%ld, %ld\n",
                (cur_timestemp.tv_sec * 1000000 + cur_timestemp.tv_usec -
                 queue->init_timestemp.tv_sec * 1000000 -
                 queue->init_timestemp.tv_usec),
                get_queue_load(queue));
    }
    return 0;
}

size_t write_log(Queue *const queue, char *file_name)
{
    fflush(queue->log_stream);
    FILE *log_file = fopen(file_name, "w");
    if (log_file == NULL)
    {
        perror("fopen");
        return -1;
    }
    fputs(queue->log_buffer, log_file);
    fclose(queue->log_stream);
    fclose(log_file);

    return 0;
}