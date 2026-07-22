#include "task.h"

#include "scheduler.h"

#include "../drivers/pit/pit.h"

#include "../memory/heap.h"
#include "../memory/vmm.h"
#include "../print/print.h"
#include "../interruptions/idt.h"

#define TASK_STACK_SIZE (4 * 1024)

static uint64_t next_pid = 1;

void task_yield(void){
    __asm__ volatile("int $0x80");
}

static void task_trampoline(void){

    print_string("TRAMPOLINE\n");

    scheduler_current()->entry();

    task_exit();
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

    stack_top -= sizeof(interrupt_frame_t);

    interrupt_frame_t *frame = (interrupt_frame_t *)stack_top;

    *frame = (interrupt_frame_t){0};

    frame->rip = (uint64_t)task_trampoline;

    print_string("task_trampoline = ");
print_hex((uint64_t)task_trampoline);
print_char('\n');

print_string("frame->rip = ");
print_hex(frame->rip);
print_char('\n');

print_string("frame addr = ");
print_hex((uint64_t)frame);
print_char('\n');

    frame->cs = KERNEL_CODE_SELECTOR;
    frame->rflags = 0x202;

    frame->interrupt = 0;
    frame->error = 0;
    
    task->rsp = (uint64_t)frame;

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

void task_exit(void){
    task_t *task = scheduler_current();

    task->state = TASK_DEAD;

    task_yield();

    for(;;) __asm__ volatile ("hlt");
}

void task_sleep(uint64_t ticks){
    task_t *task = scheduler_current();
    
    task->wake_tick = pit_ticks() + ticks;
    task->state = TASK_SLEEPING;

    task_yield();
}