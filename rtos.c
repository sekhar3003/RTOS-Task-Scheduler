#define _POSIX_C_SOURCE 200809L
#include "rtos.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
/*
 * rtos.c  —  RTOS Task Scheduler (Simulated, C99)
 *
 * Scheduling policy: Priority-based Round Robin
 *   1. Highest priority ready task runs first.
 *   2. Equal-priority tasks share CPU in round-robin order.
 *   3. Tasks can voluntarily yield or sleep (cooperative hook).
 *   4. Timer ISR simulation ticks delay counters every quantum.
 */

#include "rtos.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>   /* usleep() for simulation timing */

/* ── Scheduler State ──────────────────────────────────────────── */
static TCB_t    task_table[MAX_TASKS];
static uint8_t  task_count    = 0;
static uint8_t  current_task  = 0;
static uint32_t system_ticks  = 0;
static uint8_t  rtos_running  = 0;

/* ── Internal Helpers ─────────────────────────────────────────── */

/* Find the highest-priority READY task, round-robin within same priority */
static int scheduler_select_next(void)
{
    int      best       = -1;
    uint8_t  best_prio  = 255;
    uint8_t  n          = task_count;

    /* Start search from task AFTER current for fairness */
    for (uint8_t i = 0; i < n; i++) {
        uint8_t idx = (current_task + 1 + i) % n;
        TCB_t  *t   = &task_table[idx];

        if (t->state == TASK_READY || t->state == TASK_RUNNING) {
            if (t->priority < best_prio) {
                best_prio = t->priority;
                best      = idx;
            }
        }
    }
    return best;
}

/* Age sleeping tasks — decrement delay, wake if expired */
static void update_sleep_timers(void)
{
    for (uint8_t i = 0; i < task_count; i++) {
        TCB_t *t = &task_table[i];
        if (t->state == TASK_SLEEPING && t->delay_ticks > 0) {
            t->delay_ticks--;
            if (t->delay_ticks == 0) {
                t->state = TASK_READY;
                printf("[RTOS] tick=%u  Task '%s' woke up\n",
                       system_ticks, t->name);
            }
        }
    }
}

/* ── Public API ───────────────────────────────────────────────── */

void rtos_init(void)
{
    memset(task_table, 0, sizeof(task_table));
    task_count   = 0;
    current_task = 0;
    system_ticks = 0;
    rtos_running = 0;
    printf("[RTOS] Kernel initialised. MAX_TASKS=%d  QUANTUM=%dms\n\n",
           MAX_TASKS, TIME_QUANTUM_MS);
}

int rtos_task_create(const char *name, TaskFunc_t func,
                     void *arg, uint8_t priority)
{
    if (task_count >= MAX_TASKS) {
        fprintf(stderr, "[RTOS] ERROR: task table full\n");
        return -1;
    }
    if (priority >= MAX_PRIORITY) {
        fprintf(stderr, "[RTOS] ERROR: invalid priority %d\n", priority);
        return -1;
    }

    TCB_t *t = &task_table[task_count];
    t->task_id     = task_count;
    t->func        = func;
    t->arg         = arg;
    t->state       = TASK_READY;
    t->priority    = priority;
    t->delay_ticks = 0;
    t->run_count   = 0;
    t->total_ticks = 0;
    strncpy(t->name, name, sizeof(t->name) - 1);

    printf("[RTOS] Task created  id=%d  name='%s'  priority=%d\n",
           task_count, name, priority);
    return task_count++;
}

void rtos_task_delay(uint32_t ticks)
{
    if (!rtos_running) return;
    TCB_t *t = &task_table[current_task];
    t->state       = TASK_SLEEPING;
    t->delay_ticks = ticks;
    printf("[RTOS] tick=%u  '%s' sleeping for %u ticks\n",
           system_ticks, t->name, ticks);
    /* In a real RTOS this triggers a context switch.
       Here we return and let the main scheduler loop skip this task. */
}

void rtos_task_yield(void)
{
    if (!rtos_running) return;
    /* Mark current as READY so next call picks someone else */
    task_table[current_task].state = TASK_READY;
    printf("[RTOS] tick=%u  '%s' yielded\n",
           system_ticks, task_table[current_task].name);
}

void rtos_task_delete(uint8_t task_id)
{
    if (task_id >= task_count) return;
    task_table[task_id].state = TASK_DELETED;
    printf("[RTOS] tick=%u  Task '%s' deleted\n",
           system_ticks, task_table[task_id].name);
}

/* Timer ISR simulation — called once per quantum */
void rtos_tick(void)
{
    system_ticks++;
    update_sleep_timers();
}

void rtos_stats_print(void)
{
    printf("\n┌─────────────────────────────────────────────────────┐\n");
    printf("│  RTOS Statistics  (total ticks: %u)                  \n", system_ticks);
    printf("├──────┬──────────────┬──────────┬────────┬────────────┤\n");
    printf("│  ID  │  Name        │  State   │  Runs  │  CPU ticks │\n");
    printf("├──────┼──────────────┼──────────┼────────┼────────────┤\n");

    const char *state_names[] = {
        "READY", "RUNNING", "BLOCKED", "SLEEPING", "DELETED"
    };

    for (uint8_t i = 0; i < task_count; i++) {
        TCB_t *t = &task_table[i];
        printf("│  %2d  │ %-12s │ %-8s │ %6u │ %10u │\n",
               t->task_id,
               t->name,
               state_names[t->state],
               t->run_count,
               t->total_ticks);
    }
    printf("└──────┴──────────────┴──────────┴────────┴────────────┘\n\n");
}

/* ── Main Scheduler Loop ──────────────────────────────────────── */
/*
 * In real RTOS: context switching is done in assembly (save/restore
 * CPU registers, stack pointer). Here we simulate it by calling
 * task functions sequentially with usleep() to mimic time slices.
 */
void rtos_start(void)
{
    printf("\n[RTOS] Scheduler started\n");
    printf("─────────────────────────────────────────────────────\n\n");
    rtos_running = 1;

    /* Run for a fixed number of ticks (simulation limit) */
    const uint32_t SIM_TICKS = 20;

    for (uint32_t tick = 0; tick < SIM_TICKS; tick++) {
        rtos_tick();   /* simulate timer ISR */

        int next = scheduler_select_next();
        if (next < 0) {
            printf("[RTOS] tick=%u  (idle — no ready tasks)\n", system_ticks);
            continue;
        }

        /* Context switch */
        if ((int)current_task != next) {
            if (task_table[current_task].state == TASK_RUNNING)
                task_table[current_task].state = TASK_READY;
        }

        current_task = (uint8_t)next;
        TCB_t *t = &task_table[current_task];
        t->state = TASK_RUNNING;
        t->run_count++;
        t->total_ticks++;

        printf("[RTOS] tick=%u  Running '%s' (prio=%d, run#%u)\n",
               system_ticks, t->name, t->priority, t->run_count);

        /* Execute the task function */
        t->func(t->arg);

        /* Simulate quantum elapsed */
        sleep(TIME_QUANTUM_MS * 100);  /* scaled down for demo */
    }

    rtos_running = 0;
    rtos_stats_print();
}
