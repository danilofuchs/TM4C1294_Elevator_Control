#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"  // CMSIS-RTOS

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
  osMutexId_t uart_read_mutex, uart_write_mutex;

  main_thread_t main_thread;
  signal_handler_thread_t signal_handler_thread;
  fan_out_thread_t fan_out_thread;
  elevator_thread_t left_elevator_thread, center_elevator_thread,
      right_elevator_thread;
} app_t;

static app_t app;

void __error__(char* pcFilename, unsigned long ulLine) {
  printf("[ERROR driverlib]\nat %s:%d\n", pcFilename, ulLine);
}

__NO_RETURN void osRtxIdleThread(void* argument) {
  (void)argument;

  while (1) {
    asm("wfi");
  }
}

void main(void) {
  UARTInit();
  printKernelInfo();
  printKernelState();

  if (osKernelGetState() == osKernelInactive) osKernelInitialize();

  app.signal_queue = osMessageQueueNew(
      8, sizeof(signal_t), &(osMessageQueueAttr_t){.name = "Signal Queue"});

  app.uart_read_mutex = osMutexNew(&(osMutexAttr_t){.name = "UART Read Mutex"});
  app.uart_write_mutex =
      osMutexNew(&(osMutexAttr_t){.name = "UART Write Mutex"});

  app.main_thread = (main_thread_t){
      .attr = {.name = "Main Thread"},
      .args = {.uart_write_mutex = app.uart_write_mutex},
  };
  app.main_thread.thread_id =
      osThreadNew(mainThread, &app.main_thread, &app.main_thread.attr),

  app.signal_handler_thread = (signal_handler_thread_t){
      .attr = {.name = "Signal Handler Thread"},
      .args = {.queue = app.signal_queue,
               .uart_read_mutex = app.uart_read_mutex},
  };
  app.signal_handler_thread.thread_id =
      osThreadNew(signalHandlerThread, &app.signal_handler_thread,
                  &app.signal_handler_thread.attr);

  app.left_elevator_thread = (elevator_thread_t){
      .attr = {.name = "Left Elevator Thread"},
      .args = {.code = elevator_code_left,
               .queue = osMessageQueueNew(
                   8, sizeof(signal_t),
                   &(osMessageQueueAttr_t){.name = "Left Elevator Queue"}),
               .uart_write_mutex = app.uart_write_mutex},
  };
  app.left_elevator_thread.thread_id =
      osThreadNew(elevatorThread, &app.left_elevator_thread,
                  &app.left_elevator_thread.attr);

  app.fan_out_thread = (fan_out_thread_t){
      .attr = {.name = "Fan-Out Thread"},
      .args = {.incoming_signal_queue = app.signal_queue,
               .left_elevator_signal_queue =
                   app.left_elevator_thread.args.queue,
               .center_elevator_signal_queue =
                   app.center_elevator_thread.args.queue,
               .right_elevator_signal_queue =
                   app.right_elevator_thread.args.queue},
  };
  app.fan_out_thread.thread_id =
      osThreadNew(fanOutThread, &app.fan_out_thread, &app.fan_out_thread.attr);

  if (osKernelGetState() == osKernelReady) osKernelStart();

  while (1)
    ;
}
