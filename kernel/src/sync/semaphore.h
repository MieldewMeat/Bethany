#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "../scheduler/task.h"

typedef struct semaphore{
    
    uint64_t count;

    task_t *waiters;
} semaphore_t;

void semaphore_init(semaphore_t *sem, uint64_t initial);

void semaphore_wait(semaphore_t *sem);

void semaphore_signal(semaphore_t *sem);

bool semaphore_try_wait(semaphore_t *sem);