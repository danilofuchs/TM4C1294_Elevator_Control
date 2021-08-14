#include <stdbool.h>
#include <stdio.h>

#include "cmsis_os2.h"  // CMSIS-RTOS

// Utils
#include "utils/uartstdio.h"

// Internal deps
#include "kernel_info.h"
#include "uart.h"

osThreadId_t thread1_id, thread2_id;
osMutexId_t uart_id;

const osThreadAttr_t thread1_attr = {.name = "Thread 1"};

const osThreadAttr_t thread2_attr = {.name = "Thread 2"};

__NO_RETURN void osRtxIdleThread(void *argument) {
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

  printKernelState();

  uart_id = osMutexNew(NULL);

  if (osKernelGetState() == osKernelReady) osKernelStart();

  while (1)
    ;
}
