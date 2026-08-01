#pragma once
#include <stdint.h>
#include <stddef.h>

typedef void (*task_entry_t)(void *arg);

typedef enum{
    TASK_READY,
    TASK_RUNNING,
    TASK_SLEEPING,
    TASK_BLOCKED,
    TASK_DEAD
} task_state_t;

typedef struct task{

    uint64_t pid;

    uint64_t rsp;

    uint64_t cr3;

    uint64_t wake_tick;

    void *stack;
    size_t stack_size;

    task_entry_t entry;

    task_state_t state;

    void *arg;

    struct task *next;

    struct task *waiters;
    struct task *wait_next;
} task_t;

task_t *task_create(task_entry_t entry, void *arg);

void task_destroy(task_t *task);

task_t *task_create_current(void);

void task_exit(void);

void task_yield(void);

void task_sleep(uint64_t ticks);

void task_join(task_t *task);