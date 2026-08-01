#include "semaphore.h"
#include "../scheduler/task.h"
#include "../scheduler/scheduler.h"

void semaphore_init(semaphore_t *sem, uint64_t initial){
    sem->count = initial;
    sem->waiters = NULL;
}

bool semaphore_try_wait(semaphore_t *sem){
    if(sem->count == 0) return false;

    sem->count--;

    return true;
}

void semaphore_wait(semaphore_t *sem){
    if(sem->count > 0){
        sem->count--;
        return;
    }

    task_t *current = scheduler_current();

    current->state = TASK_BLOCKED;

    current->wait_next = sem->waiters;
    sem->waiters = current;

    task_yield();
}

void semaphore_signal(semaphore_t *sem){
    if(sem->waiters){
        
        task_t *task = sem->waiters;

        sem->waiters = task->wait_next;

        task->wait_next = NULL;

        task->state = TASK_READY;
        
        return;
    }
    
    sem->count++;
}