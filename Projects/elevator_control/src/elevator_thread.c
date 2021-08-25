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

static void turnButtonOff(elevator_t* elevator, floor_t floor,
                          osMutexId_t mutex) {
  command_t command = {
      .code = command_turn_button_off,
      .elevator_code = elevator->code,
      .floor = floor,
  };

  sendCommand(&command, mutex);
}

static void stop(elevator_t* elevator, osMutexId_t mutex) {
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

// static void clearRequestsToFloorAndDirection(elevator_t* elevator) {
//   elevator->internal_requests[elevator->floor] = false;
//   if (elevator->direction == elevator_direction_up) {
//     elevator->external_requests_up[elevator->floor] = false;
//   } else if (elevator->direction == elevator_direction_down) {
//     elevator->external_requests_down[elevator->floor] = false;
//   }
// }

// static void stopIfNecessary(elevator_t* elevator, floor_t floor,
//                             osMutexId_t mutex) {
//   if (!elevatorShouldStopAtFloor(elevator, floor)) return;

//   stop(elevator, mutex);
//   clearRequestsToFloorAndDirection(elevator);
//   openDoors(elevator, mutex);
// }

// static void moveToAtendRequests(elevator_t* elevator, osMutexId_t mutex) {
//   if (elevator->state == elevator_state_moving) return;

//   if (elevator->door_state != elevator_door_state_closed) {
//     // Must wait for doors to close before moving.
//     // Close doors will call this function again.
//     closeDoors(elevator, mutex);
//     return;
//   }

//   elevator_direction_t next_direction = elevatorGetNextDirection(elevator);
//   if (next_direction == elevator_direction_up) {
//     goUp(elevator, mutex);
//   } else if (next_direction == elevator_direction_down) {
//     goDown(elevator, mutex);
//   } else {
//     elevator->state = elevator_state_idle;
//   }
// }

static void initializeElevator(elevator_t* elevator, osMutexId_t mutex) {
  elevator->state = elevator_state_idle;

  cmdInitialize(elevator, mutex);
}

static void turnButtonOn(elevator_t* elevator, floor_t floor,
                         osMutexId_t mutex) {
  cmdTurnButtonOn(elevator, floor, mutex);
}

// static void evReachedFloor(elevator_t* elevator, floor_t floor,
//                            osMutexId_t mutex) {
//   if (floor < 0) return;
//   elevator->floor = floor;

//   stopIfNecessary(elevator, floor, mutex);
// }

// static void evInternalButtonWasPressed(elevator_t* elevator, floor_t floor,
//                                        osMutexId_t mutex) {
//   if (elevatorIsStoppedAtFloor(elevator, floor)) {
//     openDoors(elevator, mutex);
//     return;
//   };

//   turnButtonOn(elevator, floor, mutex);

//   elevator->internal_requests[floor] = true;

//   // moveToAtendRequests(elevator, mutex);
// }

// static void evExternalButtonWasPressed(elevator_t* elevator, floor_t floor,
//                                        elevator_direction_t direction,
//                                        osMutexId_t mutex) {
//   bool should_open_doors = elevatorIsStoppedAtFloor(elevator, floor) &&
//                            (elevator->direction == direction ||
//                             elevator->state == elevator_state_idle);
//   if (should_open_doors) {
//     openDoors(elevator, mutex);
//     return;
//   };
//   if (direction == elevator_direction_up) {
//     elevator->external_requests_up[floor] = true;
//   } else if (direction == elevator_direction_down) {
//     elevator->external_requests_down[floor] = true;
//   }

//   // moveToAtendRequests(elevator, mutex);
// }

// static void evDoorsAreClosed(elevator_t* elevator, osMutexId_t mutex) {
//   elevator->door_state = elevator_door_state_closed;
//   // moveToAtendRequests(elevator, mutex);
// }

static void setDoorsOpen(elevator_t* elevator) {
  elevator->door_state = elevator_door_state_open;
}
static void setDoorsClosed(elevator_t* elevator) {
  elevator->door_state = elevator_door_state_closed;
}

// static void evDoorsAreOpen(elevator_t* elevator, osMutexId_t mutex) {
//   elevator->door_state = elevator_door_state_open;
//   // turnButtonOff(elevator, elevator->floor, mutex);
//   elevator->state = elevator_state_awaiting_passengers;
//   // TODO: Fire a timer to close the doors after a certain amount of time.
//   // osDelay(5000);
//   // closeDoors(elevator, mutex);
// }

// static void evHeightChanged(elevator_t* elevator, uint32_t height,
//                             osMutexId_t mutex) {
//   elevator->height = height;
//   floor_t estimated_floor = elevatorGetEstimatedFloorGivenHeight(elevator);
//   if (estimated_floor != -1) {
//     stopIfNecessary(elevator, estimated_floor, mutex);
//   }
// }

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
  }
}

void elevatorThread(void* arg) {
  elevator_thread_t* this = (elevator_thread_t*)arg;
  osMutexId_t mutex = this->args.uart_write_mutex;

  elevator_t elevator = {
      .code = this->args.code,
      .state = elevator_state_uninitialized,
      .floor = 0,
      .height = 0,
      .direction = elevator_direction_none,
      .door_state = elevator_door_state_closed,
  };

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

    switch (elevator.state) {
      case elevator_state_uninitialized:
        if (signal.code == signal_system_initialized) {
          initializeElevator(&elevator, mutex);
        }
        break;
      case elevator_state_idle:
        startMovingIfNecessary(&elevator, signal, mutex);
        break;
      case elevator_state_closing_doors:
        if (signal.code == signal_doors_closed) {
          startMovingIfNecessary(&elevator, signal, mutex);
        }
        break;
        // case elevator_state_moving:
        //   if (signal.code == signal_reached_floor) {
        //     evReachedFloor(&elevator, signal.floor, mutex);
        //   }
        //   break;
        // case elevator_state_awaiting_passengers:
        //   if (signal.code == signal_doors_closed) {
        //     evDoorsAreClosed(&elevator, mutex);
        //   }
        //   break;
        // case elevator_state_closing_doors:
        //   if (signal.code == signal_doors_closed) {
        //     evDoorsAreClosed(&elevator, mutex);
        //   }
        //   break;
        // case elevator_state_opening_doors:
        //   if (signal.code == signal_doors_open) {
        //     evDoorsAreOpen(&elevator, mutex);
        //   }
        //   break;
    }

    // switch (signal.code) {
    //   case signal_system_initialized:
    //     evInitialized(&elevator, mutex);
    //     break;
    //   case signal_internal_button_pressed:
    //     evInternalButtonWasPressed(&elevator, signal.floor, mutex);
    //     break;
    //   case signal_external_button_pressed:
    //     evExternalButtonWasPressed(&elevator, signal.floor, signal.direction,
    //                                mutex);
    //     break;
    //   case signal_doors_closed:
    //     evDoorsAreClosed(&elevator, mutex);
    //     break;
    //   case signal_doors_open:
    //     evDoorsAreOpen(&elevator, mutex);
    //     break;
    //   case signal_reached_floor:
    //     evReachedFloor(&elevator, signal.floor, mutex);
    //     break;
    //   case signal_height_changed:
    //     evHeightChanged(&elevator, signal.height, mutex);
    //     break;
    // }
  }
}