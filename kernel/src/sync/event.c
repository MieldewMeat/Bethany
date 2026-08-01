#include "event.h"

#include "../scheduler/scheduler.h"

void event_init(event_t *event){
    event->signaled = false;
    event->waiters = NULL;
}

void event_wait(event_t *event){
    if(event->signaled) return;

    task_t *current = scheduler_current();

    current->state = TASK_BLOCKED;

    current->wait_next = event->waiters;
    event->waiters = current;

    task_yield();
}

void event_set(event_t *event){
    event->signaled = true;

    task_t *task = event->waiters;

    while(task){
        task->state = TASK_READY;

        task_t *next = task->wait_next;

        task->wait_next = NULL;

        task = next;
    }

    event->waiters = NULL;
}

void event_reset(event_t *event){
    event->signaled = false;
}