#pragma once

#include "task.h"
#include "../interruptions/isr.h"

void scheduler_init(void);

void scheduler_add(task_t *task);

void scheduler_remove(task_t *task);

task_t *scheduler_current(void);

void scheduler_next(void);

uint64_t scheduler_schedule_irq(interrupt_frame_t *frame);