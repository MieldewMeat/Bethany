#include "scheduler.h"

#include "../print/print.h"
#include "../drivers/pit/pit.h"

static task_t *first = NULL;
static task_t *current = NULL;
static task_t *task_to_destroy = NULL;

void scheduler_init(void){
    current = task_create_current();
    first = current;
}

void scheduler_add(task_t *task){

    if(task == NULL) return;

    if(first == NULL){
        first = task;
        current = task;
        return;
    }
    task_t *it = first;

    while(it->next != NULL) it = it->next;

    it->next = task;
}

task_t *scheduler_current(void){
    return current;
}

void scheduler_next(void){
    if(current == NULL) return;

    task_t *start = current;

    do{
        current = current->next;

        if(current == NULL)
            current = first;
        if(current->state == TASK_READY) return;
    }while(current != start);
}

void scheduler_remove(task_t *task){
    if(task == NULL || first == NULL) return;

    if(task == first){

        first = first->next;

        if(current == task) current = first;

        task_destroy(task);
        return;
    }

    task_t *it = first;

    while(it->next && it->next != task) it = it->next;

    if(it->next == NULL) return;

    it->next = task->next;

    if(current == task) current = it->next ? it->next : first;

    task_destroy(task);
}

uint64_t scheduler_schedule_irq(interrupt_frame_t *frame){
    if(task_to_destroy){
        scheduler_remove(task_to_destroy);
        task_to_destroy = NULL;
    }

    task_t *it = first;

    while(it){
        if(it->state == TASK_SLEEPING && it->wake_tick <= pit_ticks()){
            it->state = TASK_READY;
        }

        it = it->next;
    }

    task_t *old = current;

    old->rsp = (uint64_t)frame;

    scheduler_next();

    if(old == current) return old->rsp;

    if(old->state == TASK_RUNNING) old->state = TASK_READY;
    
    current->state = TASK_RUNNING;

    if(old->state == TASK_DEAD) task_to_destroy = old;

    return current->rsp;
}