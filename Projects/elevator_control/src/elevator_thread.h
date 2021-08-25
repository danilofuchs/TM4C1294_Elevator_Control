#ifndef __ELEVATOR_THREAD__
#define __ELEVATOR_THREAD__

#include "cmsis_os2.h"
#include "elevator.h"

typedef struct {
  elevator_code_t code;

  osMutexId_t uart_write_mutex;

  osMessageQueueId_t queue;
} elevator_thread_args_t;

typedef struct {
  osThreadId_t thread_id;
  osThreadAttr_t attr;

  // Created internally. Notifies when the door can be closed
  osTimerId_t door_timer;
  elevator_thread_args_t args;
} elevator_thread_t;

void elevatorThread(void* arg);

#endif