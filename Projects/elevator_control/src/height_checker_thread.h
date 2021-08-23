#ifndef __HEIGHT_CHECKER_THREAD_H__
#define __HEIGHT_CHECKER_THREAD_H__

#include "cmsis_os2.h"

typedef struct {
  osMutexId_t uart_write_mutex;

  osMessageQueueId_t queue;

  osMessageQueueId_t left_elevator_queue;
} height_checker_thread_args_t;

typedef struct {
  osThreadAttr_t attr;
  osThreadId_t thread_id;

  // Created internally
  osTimerId_t timer;

  height_checker_thread_args_t args;
} height_checker_thread_t;

void heightCheckerThread(void* arg);

#endif