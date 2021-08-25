#include "signal_handler_thread.h"

#include <stdio.h>

#include "signal.h"
#include "uart.h"

void signalHandlerThread(void* arg) {
  signal_handler_thread_t* this = (signal_handler_thread_t*)arg;

  char input[16];
  osStatus_t status;
  while (1) {
    if (status != osOK) continue;

    if (UARTPeek('\r') < 0) {
      osThreadYield();
      continue;
    }

    UARTgets(input, sizeof(input));

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