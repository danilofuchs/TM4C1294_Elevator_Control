#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"  // CMSIS-RTOS

// Utils
#include "utils/uartstdio.h"

// Internal deps
#include "command.h"
#include "kernel_info.h"
#include "signal.h"
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

void initialize() {
  char input[16];

  while (1) {
    UARTgets(input, sizeof(input));
    if (!strcmp(input, "initialized")) {
      continue;
    }

    UARTprintf("er\r");
    UARTprintf("cr\r");
    UARTprintf("dr\r");
    return;
  }
}

void mainThread(void* arg) {
  char input[16];

  // initialize();

  signal_t signal;

  while (1) {
    UARTgets(input, sizeof(input));
    if (parseSignal(&signal, input) == -1) {
      UARTprintf("Error: Invalid signal %s\r", input);
    }
    printSignal(&signal);
  }
}

void main(void) {
  UARTInit();
  printKernelInfo();
  printKernelState();

  if (osKernelGetState() == osKernelInactive) osKernelInitialize();

  printKernelState();

  uart_id = osMutexNew(NULL);
  main_thread_id = osThreadNew(mainThread, NULL, &main_thread_attr);

  printThreadInfo();

  if (osKernelGetState() == osKernelReady) osKernelStart();

  while (1)
    ;
}
