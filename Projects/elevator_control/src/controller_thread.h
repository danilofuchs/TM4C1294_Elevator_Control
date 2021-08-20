#ifndef __CONTROLLER_THREAD_H__
#define __CONTROLLER_THREAD_H__

#include "cmsis_os2.h"
#include "elevator.h"

typedef struct {
  osMessageQueueId_t queue_id;
  osMutexId_t uart_write_mutex_id;
  elevator_t* left_elevator;
  elevator_t* center_elevator;
  elevator_t* right_elevator;
} controller_thread_args_t;

typedef struct {
  osThreadAttr_t attr;
  osThreadId_t thread_id;
  controller_thread_args_t args;
} controller_thread_t;

void controllerThread(void* arg);

#endif