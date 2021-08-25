#include "elevator_thread.h"

#include "command.h"
#include "signal.h"

#define WAIT_FOR_PASSENGERS_TIMEOUT_MS 5000

static void sendCommand(command_t* command, osMutexId_t mutex) {
  char string[16];
  commandBuild(command, string);

  osMutexAcquire(mutex, osWaitForever);
  UARTprintf(string);
  UARTFlushTx(false);
  osMutexRelease(mutex);
}

static void cmdInitialize(elevator_t* elevator, osMutexId_t mutex) {
  command_t command = {
      .code = command_initialize,
      .elevator_code = elevator->code,
  };

  sendCommand(&command, mutex);
}

static void cmdTurnButtonOn(elevator_t* elevator, floor_t floor,
                            osMutexId_t mutex) {
  command_t command = {
      .code = command_turn_button_on,
      .elevator_code = elevator->code,
      .floor = floor,
  };

  sendCommand(&command, mutex);
}

static void cmdTurnButtonOff(elevator_t* elevator, floor_t floor,
                             osMutexId_t mutex) {
  command_t command = {
      .code = command_turn_button_off,
      .elevator_code = elevator->code,
      .floor = floor,
  };

  sendCommand(&command, mutex);
}

static void cmdStop(elevator_t* elevator, osMutexId_t mutex) {
  command_t command = {
      .code = command_stop,
      .elevator_code = elevator->code,
  };

  sendCommand(&command, mutex);
}

static void cmdOpenDoors(elevator_t* elevator, osMutexId_t mutex) {
  command_t command = {
      .code = command_open_doors,
      .elevator_code = elevator->code,
  };

  sendCommand(&command, mutex);
}
static void cmdCloseDoors(elevator_t* elevator, osMutexId_t mutex) {
  command_t command = {
      .code = command_close_doors,
      .elevator_code = elevator->code,
  };

  sendCommand(&command, mutex);
}

static void cmdGoUp(elevator_t* elevator, osMutexId_t mutex) {
  command_t command = {
      .code = command_go_up,
      .elevator_code = elevator->code,
  };

  sendCommand(&command, mutex);
}

static void cmdGoDown(elevator_t* elevator, osMutexId_t mutex) {
  command_t command = {
      .code = command_go_down,
      .elevator_code = elevator->code,
  };

  sendCommand(&command, mutex);
}

static void clearRequestsToFloorAndDirection(elevator_t* elevator) {
  elevator->internal_requests[elevator->floor] = false;
  if (elevator->direction == elevator_direction_up) {
    elevator->external_requests_up[elevator->floor] = false;
  } else if (elevator->direction == elevator_direction_down) {
    elevator->external_requests_down[elevator->floor] = false;
  }
}

static void stopMovingIfNecessary(elevator_t* elevator, signal_t signal,
                                  osMutexId_t mutex) {
  bool should_stop = elevatorShouldStopAtFloor(elevator, signal.floor);
  if (!should_stop) return;

  clearRequestsToFloorAndDirection(elevator);

  elevator->state = elevator_state_opening_doors;

  cmdStop(elevator, mutex);
  cmdOpenDoors(elevator, mutex);
}

static void initializeElevator(elevator_t* elevator, osMutexId_t mutex) {
  elevator->state = elevator_state_idle;

  cmdInitialize(elevator, mutex);
}

static void turnButtonOn(elevator_t* elevator, floor_t floor,
                         osMutexId_t mutex) {
  cmdTurnButtonOn(elevator, floor, mutex);
}

static void setElevatorFloor(elevator_t* elevator, floor_t floor) {
  if (floor < 0) return;
  elevator->floor = floor;
}

static void setDoorsOpen(elevator_t* elevator) {
  elevator->door_state = elevator_door_state_open;
}
static void setDoorsClosed(elevator_t* elevator) {
  elevator->door_state = elevator_door_state_closed;
}

static void addRequestToQueue(elevator_t* elevator, signal_t signal) {
  bool is_internal_request = signal.code == signal_internal_button_pressed;
  bool is_external_request = signal.code == signal_external_button_pressed;
  bool is_up = signal.direction == elevator_direction_up;
  bool is_down = signal.direction == elevator_direction_down;

  if (is_internal_request) {
    elevator->internal_requests[signal.floor] = true;
  } else if (is_external_request && is_up) {
    elevator->external_requests_up[signal.floor] = true;
  } else if (is_external_request && is_down) {
    elevator->external_requests_down[signal.floor] = true;
  }
}

static void startMovingIfNecessary(elevator_t* elevator, signal_t signal,
                                   osMutexId_t mutex) {
  if (elevator->state != elevator_state_idle &&
      elevator->state != elevator_state_closing_doors)
    return;

  floor_t next_request = elevatorGetNextRequest(elevator);

  bool has_request = next_request != floor_none;
  bool are_doors_closed = elevator->door_state == elevator_door_state_closed;
  bool are_doors_open = !are_doors_closed;

  bool should_stay_same_floor = has_request && next_request == elevator->floor;
  bool should_move = !should_stay_same_floor;
  bool should_go_up = has_request && next_request > elevator->floor;
  bool should_go_down = has_request && next_request < elevator->floor;

  bool should_close_doors = has_request && should_move && are_doors_open;
  bool should_open_doors =
      has_request && should_stay_same_floor && are_doors_closed;

  if (should_open_doors) {
    elevator->state = elevator_state_opening_doors;

    cmdOpenDoors(elevator, mutex);
  } else if (should_close_doors) {
    // Close doors. We will be called again once they are closed.
    elevator->state = elevator_state_closing_doors;

    cmdCloseDoors(elevator, mutex);
  } else if (should_go_up) {
    elevator->state = elevator_state_moving;
    elevator->direction = elevator_direction_up;

    cmdGoUp(elevator, mutex);
  } else if (should_go_down) {
    elevator->state = elevator_state_moving;
    elevator->direction = elevator_direction_down;

    cmdGoDown(elevator, mutex);
  } else {
    elevator->state = elevator_state_idle;
  }
}

static void awaitForPassengers(elevator_t* elevator, osTimerId_t timer,
                               osMutexId_t mutex) {
  elevator->state = elevator_state_awaiting_passengers;
  cmdTurnButtonOff(elevator, elevator->floor, mutex);
  osStatus_t status = osTimerStart(timer, WAIT_FOR_PASSENGERS_TIMEOUT_MS);
}

static void closeDoors(elevator_t* elevator, osMutexId_t mutex) {
  elevator->state = elevator_state_closing_doors;
  cmdCloseDoors(elevator, mutex);
}

typedef struct {
  osMessageQueueId_t* queue;
} door_timer_callback_args_t;
static void doorTimerCallback(void* arg) {
  door_timer_callback_args_t* args = (door_timer_callback_args_t*)arg;

  signal_t signal = {
      .code = signal__internal__should_close_doors,
  };

  osMessageQueuePut(*args->queue, &signal, NULL, osWaitForever);
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

  this->door_timer =
      osTimerNew(doorTimerCallback, osTimerOnce,
                 &(door_timer_callback_args_t){.queue = &this->args.queue},
                 &(osTimerAttr_t){.name = "Door Timer"});

  signal_t signal;
  while (1) {
    osStatus_t status =
        osMessageQueueGet(this->args.queue, &signal, NULL, osWaitForever);

    if (status != osOK) continue;

    if (signal.code == signal_internal_button_pressed) {
      turnButtonOn(&elevator, signal.floor, mutex);
      addRequestToQueue(&elevator, signal);
    }
    if (signal.code == signal_external_button_pressed) {
      addRequestToQueue(&elevator, signal);
    }
    if (signal.code == signal_doors_open) {
      setDoorsOpen(&elevator);
    }
    if (signal.code == signal_doors_closed) {
      setDoorsClosed(&elevator);
    }
    if (signal.code == signal_reached_floor) {
      setElevatorFloor(&elevator, signal.floor);
    }

    switch (elevator.state) {
      case elevator_state_uninitialized:
        if (signal.code == signal_system_initialized) {
          initializeElevator(&elevator, mutex);
        }
        break;
      case elevator_state_idle:
        // Listen to all signals
        startMovingIfNecessary(&elevator, signal, mutex);
        break;
      case elevator_state_closing_doors:
        if (signal.code == signal_doors_closed) {
          startMovingIfNecessary(&elevator, signal, mutex);
        }
        break;
      case elevator_state_moving:
        if (signal.code == signal_reached_floor) {
          stopMovingIfNecessary(&elevator, signal, mutex);
        }
        break;
      case elevator_state_opening_doors:
        if (signal.code == signal_doors_open) {
          awaitForPassengers(&elevator, this->door_timer, mutex);
        }
      case elevator_state_awaiting_passengers:
        if (signal.code == signal__internal__should_close_doors) {
          closeDoors(&elevator, mutex);
        }
    }
  }
}