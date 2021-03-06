#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"  // CMSIS-RTOS
#include "stack_protection.h"

// Internal deps
#include "elevator.h"
#include "elevator_thread.h"
#include "fan_out_thread.h"
#include "kernel_info.h"
#include "main_thread.h"
#include "signal.h"
#include "signal_handler_thread.h"
#include "uart.h"

typedef struct {
  osMessageQueueId_t signal_queue;
  osMutexId_t uart_write_mutex;

  main_thread_t main_thread;
  signal_handler_thread_t signal_handler_thread;
  fan_out_thread_t fan_out_thread;
  elevator_thread_t left_elevator_thread, center_elevator_thread,
      right_elevator_thread;
} app_t;

static void createMutex(app_t* app) {
  app->uart_write_mutex =
      osMutexNew(&(osMutexAttr_t){.name = "UART Write Mutex"});
}

static void createMainThread(app_t* app) {
  app->main_thread = (main_thread_t){
      .attr = {.name = "Main Thread"},
      .args =
          {
              .uart_write_mutex = app->uart_write_mutex,
          },
  };
  app->main_thread.thread_id =
      osThreadNew(mainThread, &app->main_thread, &app->main_thread.attr);
}

static void createSignalHandlerThread(app_t* app) {
  app->signal_queue = osMessageQueueNew(
      8, sizeof(signal_t), &(osMessageQueueAttr_t){.name = "Signal Queue"});
  app->signal_handler_thread = (signal_handler_thread_t){
      .attr = {.name = "Signal Handler Thread"},
      .args =
          {
              .queue = app->signal_queue,
          },
  };
  app->signal_handler_thread.thread_id =
      osThreadNew(signalHandlerThread, &app->signal_handler_thread,
                  &app->signal_handler_thread.attr);
}

static void createElevatorThread(app_t* app, elevator_thread_t* thread,
                                 elevator_code_t code, char* thread_name,
                                 char* queue_name) {
  *thread = (elevator_thread_t){
      .attr = {.name = thread_name},
      .args =
          {
              .code = code,
              .queue = osMessageQueueNew(
                  8, sizeof(signal_t),
                  &(osMessageQueueAttr_t){.name = queue_name}),
              .uart_write_mutex = app->uart_write_mutex,
          },
  };
  thread->thread_id = osThreadNew(elevatorThread, thread, &thread->attr);
}

static void createFanOutThread(app_t* app) {
  app->fan_out_thread = (fan_out_thread_t){
      .attr = {.name = "Fan-Out Thread"},
      .args =
          {
              .incoming_signal_queue = app->signal_queue,
              .left_elevator_signal_queue =
                  app->left_elevator_thread.args.queue,
              .center_elevator_signal_queue =
                  app->center_elevator_thread.args.queue,
              .right_elevator_signal_queue =
                  app->right_elevator_thread.args.queue,
          },
  };
  app->fan_out_thread.thread_id = osThreadNew(
      fanOutThread, &app->fan_out_thread, &app->fan_out_thread.attr);
}

void main(void) {
  static app_t app;

  UARTInit();
  printKernelInfo();
  printKernelState();

  if (osKernelGetState() == osKernelInactive) osKernelInitialize();

  createMutex(&app);
  createMainThread(&app);
  createSignalHandlerThread(&app);
  createElevatorThread(&app, &app.left_elevator_thread, elevator_code_left,
                       "Left Elevator Thread", "Left Elevator Queue");
  createElevatorThread(&app, &app.center_elevator_thread, elevator_code_center,
                       "Center Elevator Thread", "Center Elevator Queue");
  createElevatorThread(&app, &app.right_elevator_thread, elevator_code_right,
                       "Right Elevator Thread", "Right Elevator Queue");
  createFanOutThread(&app);

  if (osKernelGetState() == osKernelReady) osKernelStart();

  while (1)
    ;
}
