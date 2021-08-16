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
osMutexId_t uart_read_id, uart_write_id;

const osThreadAttr_t main_thread_attr = {.name = "Main Thread"};

void __error__(char* pcFilename, unsigned long ulLine) {
  printf("[ERROR driverlib]\nat %s:%d\n", pcFilename, ulLine);
}

__NO_RETURN void osRtxIdleThread(void* argument) {
  (void)argument;

  while (1) {
    asm("wfi");
  }
}

void init() {
  osMutexAcquire(uart_write_id, osWaitForever);
  printKernelState();
  printThreadInfo();
  osMutexRelease(uart_write_id);
}

void sendCommand(command_t* command) {
  char string[16];
  commandBuild(command, string);

  osMutexAcquire(uart_write_id, osWaitForever);
  UARTprintf(string);
  osMutexRelease(uart_write_id);
}

void readSignal(signal_t* signal) {
  char input[16];
  osMutexAcquire(uart_read_id, osWaitForever);
  UARTgets(input, sizeof(input));
  osMutexRelease(uart_read_id);

  if (!signalParse(signal, input)) {
#ifdef DEBUG
    printf("Error: Invalid signal %s\n", input);
#endif
  }
}

void mainThread(void* arg) {
  signal_t signal;

  while (1) {
    readSignal(&signal);

    switch (signal.code) {
      case signal_system_initialized:
        sendCommand(&(command_t){.code = command_initialize,
                                 .elevator_code = elevator_left});
        sendCommand(&(command_t){.code = command_initialize,
                                 .elevator_code = elevator_center});
        sendCommand(&(command_t){.code = command_initialize,
                                 .elevator_code = elevator_right});
        break;
    }

    signalDebug(&signal);
  }
}

void main(void) {
  UARTInit();
  printKernelInfo();
  printKernelState();

  if (osKernelGetState() == osKernelInactive) osKernelInitialize();

  uart_read_id = osMutexNew(NULL);
  uart_write_id = osMutexNew(NULL);
  main_thread_id = osThreadNew(mainThread, NULL, &main_thread_attr);

  if (osKernelGetState() == osKernelReady) osKernelStart();

  while (1)
    ;
}
