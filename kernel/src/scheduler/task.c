#include "task.h"

#include "context.h"
#include "scheduler.h"

#include "../memory/heap.h"
#include "../memory/vmm.h"
#include "../print_and_stuff/print.h"

#define TASK_STACK_SIZE (4 * 1024)

static uint64_t next_pid = 1;

static void task_trampoline(void){

    task_t * task = scheduler_current();

    task->entry();

    task->state = TASK_DEAD;

    for(;;) __asm__ volatile("hlt");
}

task_t *task_create(task_entry_t entry){

    task_t * task = kmalloc(sizeof(task_t));

    if(task == NULL) return NULL;

    task->stack = kmalloc(TASK_STACK_SIZE);

    if(task->stack == NULL){
        kfree(task);
        return NULL;
    }

    task->pid = next_pid++;
    task->stack_size = TASK_STACK_SIZE;
    task->entry = entry;


    uint8_t *stack_top = (uint8_t *)task->stack + task->stack_size;

    stack_top = (uint8_t *)((uintptr_t)stack_top & ~0xFULL);

    stack_top -= sizeof(uint64_t);
    *(uint64_t *)stack_top = (uint64_t)task_trampoline;

    stack_top -= sizeof(context_t);

    context_t *ctx = (context_t*)stack_top;

    *ctx = (context_t){0};

    ctx->rdi = (uint64_t)entry;

    task->rsp = (uint64_t)ctx;

    
    task->cr3 = vmm_read_cr3();
    task->state = TASK_READY;
    task->next = NULL;

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

    task->stack = NULL;
    task->stack_size = 0;

    task->entry = NULL;

    task->rsp = 0;

    task->cr3 = vmm_read_cr3();
    
    task->state = TASK_RUNNING;

    task->next = NULL;

    return task;
}