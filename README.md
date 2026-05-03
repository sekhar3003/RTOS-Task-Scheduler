# RTOS Task Scheduler

A priority based preemptive task scheduler written in C,
simulating core Real-Time Operating System (RTOS) behaviour.

## What it does
- Manages up to 8 concurrent tasks with configurable priorities
- Implements priority based round-robin scheduling
- Simulates a hardware Timer ISR driving system ticks
- Supports task sleep, wake, yield and delete operations
- Prints CPU usage statistics per task at runtime

## How to run locally
gcc -std=c99 -o rtos main.c rtos.c && ./rtos

## Live demo
Run it in your browser instantly, no install needed:
(https://onlinegdb.com/F4evLCjot)

## Key concepts demonstrated
- Task Control Block (TCB) data structure
- Context switching simulation
- Priority preemption and round-robin fairness
- Timer interrupt driven scheduling
- Task starvation observable with current priority setup

## Files
- rtos.h — kernel types, TCB struct, API declarations
- rtos.c — scheduler core, timer ISR simulation, task management
- main.c — three demo tasks: sensor, process, logger

## Tech
Language: C (C99)
Build: gcc / Makefile
Platform: Linux / OnlineGDB
