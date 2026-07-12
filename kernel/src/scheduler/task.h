#pragma once
#include <stdint.h>
#include <stddef.h>

typedef void (*task_entry_t)(void);


typedef enum{
    TASK_READY,
    TASK_RUNNING,
    TASK_SLEEPING,
    TASK_DEAD
} task_state_t;

typedef struct task{

    uint64_t pid;

    uint64_t rsp;

    uint64_t cr3;

    void *stack;
    size_t stack_size;

    task_entry_t entry;

    task_state_t state;

    struct task *next;
} task_t;

task_t *task_create(task_entry_t entry);

void task_destroy(task_t *task);

task_t *task_create_current(void);