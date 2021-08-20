#ifndef __MAIN_THREAD_H__
#define __MAIN_THREAD_H__

#include "cmsis_os2.h"
#include "elevator.h"

typedef struct {
  osMutexId_t uart_write_mutex_id;

  elevator_t* left_elevator;
  elevator_t* center_elevator;
  elevator_t* right_elevator;
} main_thread_args_t;

typedef struct {
  osThreadAttr_t attr;
  osThreadId_t thread_id;
  main_thread_args_t args;
} main_thread_t;

void mainThread(void* arg);

#endif