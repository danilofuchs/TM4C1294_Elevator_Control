#ifndef __SIGNAL_HANDLER_THREAD__
#define __SIGNAL_HANDLER_THREAD__

#include "cmsis_os2.h"
#include "elevator.h"

typedef elevator_code_t (*getElevatorCodeFromThreadIdFunction)(
    osThreadId_t threadId);

typedef struct {
  osMessageQueueId_t queue;
  osMutexId_t uart_read_mutex;

  osMutexId_t height_query_mutex;
  getElevatorCodeFromThreadIdFunction getElevatorCodeFromThreadId;
} signal_handler_thread_args_t;

typedef struct {
  osThreadAttr_t attr;
  osThreadId_t thread_id;
  signal_handler_thread_args_t args;
} signal_handler_thread_t;

void signalHandlerThread(void* arg);

#endif