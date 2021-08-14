#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"  // CMSIS-RTOS

// Utils
#include "utils/uartstdio.h"

// Internal deps
#include "kernel_info.h"
#include "uart.h"

osThreadId_t main_thread_id;
osMutexId_t uart_id;

const osThreadAttr_t main_thread_attr = {.name = "Main Thread"};

__NO_RETURN void osRtxIdleThread(void* argument) {
  (void)argument;

  while (1) {
    asm("wfi");
  }
}

void main_thread(void* arg) {
  char input[16];
  UARTgets(input, sizeof(input));
  if (strcmp(input, "initialized")) {
    UARTprintf("er\r");
    UARTprintf("cr\r");
    UARTprintf("dr\r");
  }
}

void main(void) {
  UARTInit();
  printKernelInfo();
  printKernelState();

  if (osKernelGetState() == osKernelInactive) osKernelInitialize();

  printKernelState();

  uart_id = osMutexNew(NULL);
  main_thread_id = osThreadNew(main_thread, NULL, &main_thread_attr);

  printThreadInfo();

  if (osKernelGetState() == osKernelReady) osKernelStart();

  while (1)
    ;
}
