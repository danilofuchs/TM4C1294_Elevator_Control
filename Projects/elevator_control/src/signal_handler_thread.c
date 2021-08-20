#include "signal_handler_thread.h"

#include <stdio.h>

#include "signal.h"
#include "utils/uartstdio.h"

void signalHandlerThread(void* arg) {
  signal_handler_thread_t* this = (signal_handler_thread_t*)arg;

  char input[16];
  while (1) {
    osMutexAcquire(this->args.uart_read_mutex_id, osWaitForever);
    if (UARTPeek('\r') < 0) {
      osMutexRelease(this->args.uart_read_mutex_id);
      osThreadYield();
      continue;
    }

    UARTgets(input, sizeof(input));
    osMutexRelease(this->args.uart_read_mutex_id);

    signal_t signal;
    if (!signalParse(&signal, input)) {
#ifdef DEBUG
      printf("Error: Invalid signal %s\n", input);
#endif
    }

    osMessageQueuePut(this->args.queue_id, &signal, 0, osWaitForever);
    osThreadYield();
  }
}