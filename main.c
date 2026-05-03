/*
 * main.c  —  Demo application for RTOS Task Scheduler
 *
 * Demonstrates:
 *  - Three tasks at different priorities
 *  - Task sleeping/delay
 *  - Task yield
 *  - Statistics printout
 */

#include "rtos.h"
#include <stdio.h>

/* ── Task Definitions ─────────────────────────────────────────── */

/*
 * High-priority sensor task (priority 0)
 * Simulates reading a sensor every 3 ticks.
 */
void task_sensor(void *arg)
{
    static uint32_t reading = 0;
    reading += 10;
    printf("         [sensor]  ADC reading = %u mV\n", reading);
    rtos_task_delay(3);   /* sleep 3 ticks between reads */
}

/*
 * Medium-priority processing task (priority 1)
 * Simulates data processing — runs and yields.
 */
void task_process(void *arg)
{
    static uint32_t packets = 0;
    packets++;
    printf("         [process] Processed packet #%u\n", packets);
    rtos_task_yield();    /* hand back CPU voluntarily */
}

/*
 * Low-priority logging task (priority 2)
 * Simulates writing to a log — runs every time it gets CPU.
 */
void task_logger(void *arg)
{
    static uint32_t log_line = 0;
    log_line++;
    printf("         [logger]  Log entry #%u written\n", log_line);
    /* no delay — will be preempted by higher-priority tasks */
}

/* ── Entry Point ──────────────────────────────────────────────── */
int main(void)
{
    printf("╔═══════════════════════════════════════════════════╗\n");
    printf("║        RTOS Task Scheduler Simulation             ║\n");
    printf("╚═══════════════════════════════════════════════════╝\n\n");

    /* 1. Initialise the kernel */
    rtos_init();

    /* 2. Create tasks (name, function, arg, priority) */
    rtos_task_create("sensor",  task_sensor,  NULL, 0); /* high   */
    rtos_task_create("process", task_process, NULL, 1); /* medium */
    rtos_task_create("logger",  task_logger,  NULL, 2); /* low    */

    printf("\n");

    /* 3. Hand control to the scheduler */
    rtos_start();

    return 0;
}
