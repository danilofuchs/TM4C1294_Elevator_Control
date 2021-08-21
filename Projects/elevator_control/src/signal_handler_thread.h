#ifndef __SIGNAL_HANDLER_THREAD__
#define __SIGNAL_HANDLER_THREAD__

#include "cmsis_os2.h"

typedef struct {
  osMessageQueueId_t queue;
  osMutexId_t uart_read_mutex;
} signal_handler_thread_args_t;

typedef struct {
  osThreadAttr_t attr;
  osThreadId_t thread_id;
  signal_handler_thread_args_t args;
} signal_handler_thread_t;

void signalHandlerThread(void* arg);

#endif