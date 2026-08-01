#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "mutex.h"
#include "semaphore.h"

typedef struct queue{
    void **buffer;

    size_t capacity;

    size_t head;
    size_t tail;

    size_t size;

    semaphore_t items;
    semaphore_t space;

    mutex_t lock;
} queue_t;

bool queue_init(queue_t *queue, size_t capacity);

void queue_destroy(queue_t *queue);

void queue_push(queue_t *queue, void *item);

void *queue_pop(queue_t *queue);