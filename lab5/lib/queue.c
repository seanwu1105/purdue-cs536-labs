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
    return written;
}

size_t get_queue_load(const Queue *const queue)
{
    if (queue->head >= queue->tail)
        return queue->head - queue->tail;
    else
        return queue->length + queue->head - queue->tail;
}