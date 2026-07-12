#pragma once

#include "task.h"

void scheduler_init(void);

void scheduler_add(task_t *task);

void scheduler_remove(task_t *task);

task_t *scheduler_current(void);

void scheduler_next(void);

void scheduler_schedule(void);
