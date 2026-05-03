#ifndef RTOS_H
#define RTOS_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* ── Configuration ─────────────────────────────────────────────── */
#define MAX_TASKS        8
#define STACK_SIZE       1024   /* bytes per task (simulated) */
#define TIME_QUANTUM_MS  100    /* default round-robin slice   */
#define MAX_PRIORITY     4      /* 0 = highest, 3 = lowest     */

/* ── Task States ───────────────────────────────────────────────── */
typedef enum {
    TASK_READY    = 0,
    TASK_RUNNING  = 1,
    TASK_BLOCKED  = 2,
    TASK_SLEEPING = 3,
    TASK_DELETED  = 4
} TaskState_t;

/* ── Task Function Signature ───────────────────────────────────── */
typedef void (*TaskFunc_t)(void *arg);

/* ── Task Control Block (TCB) ──────────────────────────────────── */
typedef struct {
    uint8_t      task_id;           /* unique task identifier      */
    char         name[16];          /* human-readable name         */
    TaskFunc_t   func;              /* task entry point            */
    void        *arg;               /* argument passed to task     */
    TaskState_t  state;             /* current state               */
    uint8_t      priority;          /* 0 = highest priority        */
    uint32_t     delay_ticks;       /* ticks remaining in sleep    */
    uint32_t     run_count;         /* how many times it has run   */
    uint32_t     total_ticks;       /* total CPU ticks consumed    */
    uint8_t      stack[STACK_SIZE]; /* simulated stack             */
} TCB_t;

/* ── Scheduler API ─────────────────────────────────────────────── */
void  rtos_init(void);
int   rtos_task_create(const char *name, TaskFunc_t func,
                       void *arg, uint8_t priority);
void  rtos_start(void);
void  rtos_task_delay(uint32_t ticks);
void  rtos_task_yield(void);
void  rtos_task_delete(uint8_t task_id);
void  rtos_stats_print(void);

/* ── Simulated system tick (called by timer ISR simulation) ─────── */
void  rtos_tick(void);

#endif /* RTOS_H */
