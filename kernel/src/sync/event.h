#pragma once

#include <stdbool.h>

#include "../scheduler/task.h"

typedef struct event{
    
    bool signaled;

    task_t *waiters;
} event_t;

void event_init(event_t *event);

void event_wait(event_t *event);

void event_set(event_t *event);

void event_reset(event_t *event);