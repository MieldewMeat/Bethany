#include "scheduler.h"
#include "context.h"
#include "../print_and_stuff/print.h"

static task_t *first = NULL;
static task_t *current = NULL;

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

    current = current->next;

    if(current == NULL) current = first;
}

void scheduler_remove(task_t *task){
    (void)task;
}

void scheduler_schedule(void){

    task_t *old = current;

    scheduler_next();

    if(old == current) return;

    context_switch(&old->rsp, current->rsp);
}