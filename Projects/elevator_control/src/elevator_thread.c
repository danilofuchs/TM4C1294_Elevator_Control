#include "elevator_thread.h"

#include "command.h"
#include "signal.h"

static void sendCommand(command_t* command, osMutexId_t mutex) {
  char string[16];
  commandBuild(command, string);

  osMutexAcquire(mutex, osWaitForever);
  UARTprintf(string);
  UARTFlushTx(false);
  osMutexRelease(mutex);
}

static void elevatorInit(elevator_t* elevator, osMutexId_t mutex) {
  command_t command = {
      .code = command_initialize,
      .elevator_code = elevator->code,
      .floor = -1,
  };

  sendCommand(&command, mutex);
}

void elevatorThread(void* arg) {
  elevator_thread_t* this = (elevator_thread_t*)arg;
  elevator_t elevator = {
      .code = this->args.code,
      .direction = elevator_direction_unknown,
  };

  signal_t signal;
  while (1) {
    osStatus_t status =
        osMessageQueueGet(this->args.queue, &signal, NULL, osWaitForever);

    if (status != osOK) continue;

    switch (signal.code) {
      case signal_system_initialized:
        elevatorInit(&elevator, this->args.uart_write_mutex);
        break;
    }
  }
}