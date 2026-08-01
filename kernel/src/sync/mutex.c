#include "mutex.h"
#include "../scheduler/task.h"
#include "../scheduler/scheduler.h"

void mutex_init(mutex_t *mutex){
    mutex->locked = false;
    mutex->waiters = NULL;
}

bool mutex_try_lock(mutex_t *mutex){
    if(mutex->locked) return false;

    mutex->locked = true;

    return true;
}

void mutex_lock(mutex_t *mutex){
    if(!mutex->locked){
        mutex->locked = true;
        return;
    }

    task_t *current = scheduler_current();

    current->state = TASK_BLOCKED;

    current->wait_next = mutex->waiters;
    mutex->waiters = current;

    task_yield();
}

void mutex_unlock(mutex_t *mutex){
    if(mutex->waiters == NULL){
        mutex->locked = false;
        return;
    }

    task_t *task = mutex->waiters;

    mutex->waiters = task->wait_next;

    task->wait_next = NULL;

    task->state = TASK_READY;
}