#ifndef __FAN_OUT_THREAD__
#define __FAN_OUT_THREAD__

#include "cmsis_os2.h"

typedef struct {
  osMessageQueueId_t incoming_signal_queue;

  osMessageQueueId_t left_elevator_signal_queue;
  osMessageQueueId_t center_elevator_signal_queue;
  osMessageQueueId_t right_elevator_signal_queue;

  osMessageQueueId_t height_checker_signal_queue;
} fan_out_thread_args_t;

typedef struct {
  osThreadId_t thread_id;
  osThreadAttr_t attr;
  fan_out_thread_args_t args;
} fan_out_thread_t;

void fanOutThread(void* arg);

#endif