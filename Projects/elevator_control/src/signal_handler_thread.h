#ifndef __SIGNAL_HANDLER_THREAD__
#define __SIGNAL_HANDLER_THREAD__

#include "cmsis_os2.h"
#include "elevator.h"

typedef struct {
  osMessageQueueId_t queue;
} signal_handler_thread_args_t;

typedef struct {
  osThreadAttr_t attr;
  osThreadId_t thread_id;
  signal_handler_thread_args_t args;
} signal_handler_thread_t;

void signalHandlerThread(void* arg);

#endif