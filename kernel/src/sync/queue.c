#include "queue.h"

#include "../memory/heap.h"
bool queue_init(queue_t *queue, size_t capacity) {

    if(queue == NULL || capacity == 0) return false;

    
    queue->buffer = kmalloc(sizeof(void *) * capacity);

    if(queue->buffer == NULL) return false;

    queue->capacity = capacity;

    queue->head = 0;
    queue->tail = 0;

    mutex_init(&queue->lock);

    semaphore_init(&queue->items, 0);
    semaphore_init(&queue->space, capacity);

    return true;
}

void queue_destroy(queue_t *queue) {

    if(queue == NULL) return;

    kfree(queue->buffer);

    queue->buffer = NULL;

    queue->capacity = 0;
    queue->head = 0;
    queue->tail = 0;
}

void queue_push(queue_t *queue, void *item) {

    if(queue == NULL || item == NULL) return;

    semaphore_wait(&queue->space);

    mutex_lock(&queue->lock);

    queue->buffer[queue->tail] = item;
    queue->tail++;
    queue->size++;

    if(queue->tail == queue->capacity) {
        queue->tail = 0;
    }

    mutex_unlock(&queue->lock);

    semaphore_signal(&queue->items);
}

void *queue_pop(queue_t *queue) {

    if(queue == NULL) return NULL;

    semaphore_wait(&queue->items);

    mutex_lock(&queue->lock);

    void *item = queue->buffer[queue->head];
    queue->head++;
    queue->size--;

    if(queue->head == queue->capacity) {
        queue->head = 0;
    }

    mutex_unlock(&queue->lock);

    semaphore_signal(&queue->space);

    return item;
}