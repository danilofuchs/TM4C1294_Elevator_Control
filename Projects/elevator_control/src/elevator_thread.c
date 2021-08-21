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

static void initialize(elevator_t* elevator, osMutexId_t mutex) {
  elevator->state = elevator_state_idle;

  command_t command = {
      .code = command_initialize,
      .elevator_code = elevator->code,
      .floor = -1,
  };

  sendCommand(&command, mutex);
}

static void turnButtonOn(elevator_t* elevator, int8_t floor,
                         osMutexId_t mutex) {
  command_t command = {
      .code = command_turn_button_on,
      .elevator_code = elevator->code,
      .floor = floor,
  };

  sendCommand(&command, mutex);
}

static void turnButtonOff(elevator_t* elevator, int8_t floor,
                          osMutexId_t mutex) {
  command_t command = {
      .code = command_turn_button_off,
      .elevator_code = elevator->code,
      .floor = floor,
  };

  sendCommand(&command, mutex);
}

static void reachedFloor(elevator_t* elevator, int8_t floor,
                         osMutexId_t mutex) {
  if (floor < 0) return;
  elevator->floor = floor;
}

static void closeDoors(elevator_t* elevator, osMutexId_t mutex) {
  elevator->state = elevator_state_closing_doors;

  command_t command = {
      .code = command_close_doors,
      .elevator_code = elevator->code,
      .floor = -1,
  };

  sendCommand(&command, mutex);
}

static void goUp(elevator_t* elevator, osMutexId_t mutex) {
  elevator->state = elevator_state_moving;
  elevator->direction = elevator_direction_up;

  command_t command = {
      .code = command_go_up,
      .elevator_code = elevator->code,
      .floor = -1,
  };

  sendCommand(&command, mutex);
}

static void internalButtonWasPressed(elevator_t* elevator, int8_t floor,
                                     osMutexId_t mutex) {
  turnButtonOn(elevator, floor, mutex);

  elevator->internal_requests[floor] = true;

  closeDoors(elevator, mutex);
}
static void externalButtonWasPressed(elevator_t* elevator, int8_t floor,
                                     elevator_direction_t direction,
                                     osMutexId_t mutex) {
  if (direction == elevator_direction_up) {
    elevator->external_requests_up[floor] = true;
  } else if (direction == elevator_direction_down) {
    elevator->external_requests_down[floor] = true;
  }

  closeDoors(elevator, mutex);
}

static void doorsAreClosed(elevator_t* elevator, osMutexId_t mutex) {
  elevator->door_state = elevator_door_state_closed;
  goUp(elevator, mutex);
}

void elevatorThread(void* arg) {
  elevator_thread_t* this = (elevator_thread_t*)arg;
  osMutexId_t mutex = this->args.uart_write_mutex;

  elevator_t elevator = {
      .code = this->args.code,
      .state = elevator_state_uninitialized,
      .floor = 0,
      .direction = elevator_direction_none,
      .door_state = elevator_door_state_closed,
  };

  signal_t signal;
  while (1) {
    osStatus_t status =
        osMessageQueueGet(this->args.queue, &signal, NULL, osWaitForever);

    if (status != osOK) continue;

    switch (signal.code) {
      case signal_system_initialized:
        initialize(&elevator, mutex);
        break;
      case signal_internal_button_pressed:
        internalButtonWasPressed(&elevator, signal.floor, mutex);
        break;
      case signal_external_button_pressed:
        externalButtonWasPressed(&elevator, signal.floor, signal.direction,
                                 mutex);
        break;
      case signal_doors_closed:
        doorsAreClosed(&elevator, mutex);
        break;
      case signal_reached_floor:
        reachedFloor(&elevator, signal.floor, mutex);
        break;
    }
  }
}