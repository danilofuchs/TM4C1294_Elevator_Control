#include "signal_handler_thread.h"

#include <stdio.h>

#include "signal.h"
#include "uart.h"

// Height change signal does not contain information about which elevator
// has queried it, so we need to detect it by checking the current owner of the
// height change mutex.
static elevator_code_t findOutWhichElevatorQueriedHeight(
    signal_handler_thread_args_t* args) {
  osThreadId_t thread = osMutexGetOwner(args->height_query_mutex);

  if (thread == NULL) return elevator_code_unknown;

  return args->getElevatorCodeFromThreadId(thread);
}

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

    if (signal.code == signal_height_changed) {
      signal.elevator_code = findOutWhichElevatorQueriedHeight(&this->args);
    }

    do {
      status = osMessageQueuePut(this->args.queue, &signal, 0, osWaitForever);
    } while (status != osOK);
    osThreadYield();
  }
}