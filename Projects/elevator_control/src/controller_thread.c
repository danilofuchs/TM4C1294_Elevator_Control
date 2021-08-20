#include "controller_thread.h"

#include "command.h"
#include "signal.h"
#include "utils/uartstdio.h"

static void sendCommand(command_t* command, osMutexId_t mutex) {
  char string[16];
  commandBuild(command, string);

  osMutexAcquire(mutex, osWaitForever);
  UARTprintf(string);
  osMutexRelease(mutex);
}

static void elevatorInit(elevator_t* elevator, osMutexId_t mutex) {
  command_t command = {
      .code = command_initialize,
      .elevator_code = elevator->code,
  };

  sendCommand(mutex, &command);
}

void controllerThread(void* arg) {
  controller_thread_t* this = (controller_thread_t*)arg;
  osMutexId_t mutex = this->args.uart_write_mutex_id;

  elevator_t* left_elevator = this->args.left_elevator;
  elevator_t* center_elevator = this->args.center_elevator;
  elevator_t* right_elevator = this->args.right_elevator;

  signal_t signal;
  while (1) {
    osMessageQueueGet(this->args.queue_id, &signal, 0, osWaitForever);

    switch (signal.code) {
      case signal_system_initialized:
        elevatorInit(left_elevator, mutex);
        elevatorInit(center_elevator, mutex);
        elevatorInit(right_elevator, mutex);
        break;
    }

    signalDebug(&signal);
  }
}