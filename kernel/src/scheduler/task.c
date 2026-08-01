#include "task.h"

#include "context.h"
#include "scheduler.h"

#include "../timer/pit.h"

#include "../memory/heap.h"
#include "../memory/vmm.h"

#define TASK_STACK_SIZE (4 * 1024)

static uint64_t next_pid = 1;

void task_yield(void){
    scheduler_schedule();
}

static void task_trampoline(task_entry_t entry, void *arg){

    entry(arg);

    task_exit();
}

task_t *task_create(task_entry_t entry, void *arg){

    task_t * task = kmalloc(sizeof(task_t));

    if(task == NULL) return NULL;

    void *stack = kmalloc(TASK_STACK_SIZE);

    if(stack == NULL){
        kfree(task);
        return NULL;
    }

    uint8_t *stack_top = (uint8_t *)stack + TASK_STACK_SIZE;

    stack_top = (uint8_t *)((uintptr_t)stack_top & ~0xFULL);

    stack_top -= sizeof(uint64_t);
    *(uint64_t *)stack_top = (uint64_t)task_trampoline;

    stack_top -= sizeof(context_t);

    context_t *ctx = (context_t*)stack_top;

    *ctx = (context_t){0};

    ctx->rdi = (uint64_t)entry;
    ctx->rsi = (uint64_t)arg;

    task->pid = next_pid++;

    task->rsp = (uint64_t) ctx;
    task->cr3 = vmm_read_cr3();

    task->stack = stack;
    task->stack_size = TASK_STACK_SIZE;

    task->entry = entry;
    task->arg = arg;

    task->state = TASK_READY;
    task->wake_tick = 0;

    task->next = NULL;

    task->waiters = NULL;
    task->wait_next = NULL;

    return task;
}

void task_destroy(task_t *task){

    if(task == NULL) return;

    kfree(task->stack);
    kfree(task);
}

task_t *task_create_current(void){
    task_t *task = kmalloc(sizeof(task_t));

    if(task == NULL) return NULL;

    task->pid = 0;

    task->rsp = 0;
    task->cr3 = vmm_read_cr3();

    task->stack = NULL;
    task->stack_size = 0;

    task->entry = NULL;
    
    task->state = TASK_RUNNING;
    task->wake_tick = 0;

    task->next = NULL;

    task->waiters = NULL;
    task->wait_next = NULL;

    return task;
}

void task_exit(void){
    task_t *task = scheduler_current();

    task_t *waiter = task->waiters;

    while(waiter){
        waiter->state = TASK_READY;

        task_t *next = waiter->wait_next;

        waiter->wait_next = NULL;

        waiter = next;
    }

    task->waiters = NULL;

    task->state = TASK_DEAD;

    task_yield();

    for(;;) __asm__ volatile ("hlt");

    __builtin_unreachable();
}

void task_sleep(uint64_t ticks){
    task_t *task = scheduler_current();
    
    task->wake_tick = pit_ticks() + ticks;
    task->state = TASK_SLEEPING;

    task_yield();
}

void task_join(task_t *task){

    if(task == NULL || task->state == TASK_DEAD || task == scheduler_current()) return;

    task_t *current = scheduler_current();

    current->state = TASK_BLOCKED;

    current->wait_next = task->waiters;
    task->waiters = current;

    task_yield();
}