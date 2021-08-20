#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"  // CMSIS-RTOS

// Utils
#include "utils/uartstdio.h"

// Internal deps
#include "controller_thread.h"
#include "main_thread.h"
#include "signal.h"
#include "signal_handler_thread.h"
#include "uart.h"

typedef struct {
  elevator_t left_elevator, center_elevator, right_elevator;
  osMessageQueueId_t signal_queue_id;
  osMutexId_t uart_read_mutex_id, uart_write_mutex_id;

  main_thread_t main_thread;
  signal_handler_thread_t signal_handler_thread;
  controller_thread_t controller_thread;
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

  app.signal_queue_id = osMessageQueueNew(
      8, sizeof(signal_t), &(osMessageQueueAttr_t){.name = "Signal Queue"});

  app.uart_read_mutex_id =
      osMutexNew(&(osMutexAttr_t){.name = "UART Read Mutex"});
  app.uart_write_mutex_id =
      osMutexNew(&(osMutexAttr_t){.name = "UART Write Mutex"});

  app.main_thread = (main_thread_t){
      .attr = {.name = "Main Thread"},
      .args = {.uart_write_mutex_id = app.uart_write_mutex_id,
               .left_elevator = &app.left_elevator,
               .center_elevator = &app.center_elevator,
               .right_elevator = &app.right_elevator},
  };
  app.main_thread.thread_id =
      osThreadNew(mainThread, &app.main_thread, &app.main_thread.attr),

  app.signal_handler_thread = (signal_handler_thread_t){
      .attr = {.name = "Signal Handler Thread"},
      .args = {.queue_id = app.signal_queue_id,
               .uart_read_mutex_id = app.uart_read_mutex_id},
  };
  app.signal_handler_thread.thread_id =
      osThreadNew(signalHandlerThread, &app.signal_handler_thread,
                  &app.signal_handler_thread.attr);

  app.controller_thread = (controller_thread_t){
      .attr = {.name = "Controller Thread"},
      .args = {.left_elevator = &app.left_elevator,
               .center_elevator = &app.center_elevator,
               .right_elevator = &app.right_elevator,
               .queue_id = app.signal_queue_id,
               .uart_write_mutex_id = app.uart_write_mutex_id},
  };
  app.controller_thread.thread_id = osThreadNew(
      controllerThread, &app.controller_thread, &app.controller_thread.attr);

  if (osKernelGetState() == osKernelReady) osKernelStart();

  while (1)
    ;
}
