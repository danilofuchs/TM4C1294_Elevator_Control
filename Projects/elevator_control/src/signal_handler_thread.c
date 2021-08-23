#include "signal_handler_thread.h"

#include <stdio.h>

#include "signal.h"
#include "uart.h"

void signalHandlerThread(void* arg) {
  signal_handler_thread_t* this = (signal_handler_thread_t*)arg;

  char input[16];
  while (1) {
    osStatus_t status =
        osMutexAcquire(this->args.uart_read_mutex, osWaitForever);
    if (status != osOK) continue;

    if (UARTPeek('\r') < 0) {
      osMutexRelease(this->args.uart_read_mutex);
      osThreadYield();
      continue;
    }

    UARTgets(input, sizeof(input));
    osMutexRelease(this->args.uart_read_mutex);

    signal_t signal;
    if (!signalParse(&signal, input)) {
#ifdef DEBUG
      printf("Error: Invalid signal %s\n", input);
#endif
      continue;
    }

    do {
      status = osMessageQueuePut(this->args.queue, &signal, 0, osWaitForever);
    } while (status != osOK);
    osThreadYield();
  }
}