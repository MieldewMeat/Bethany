#pragma once

#include <stdbool.h>

#include "../scheduler/task.h"

typedef struct mutex{
    
    bool locked;

    task_t *waiters;
} mutex_t;

void mutex_init(mutex_t *mutex);

void mutex_lock(mutex_t *mutex);

bool mutex_try_lock(mutex_t *mutex);

void mutex_unlock(mutex_t *mutex);