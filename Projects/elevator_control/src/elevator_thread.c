#include "elevator_thread.h"

#include "command.h"
#include "signal.h"

#define WAIT_FOR_PASSENGERS_TIMEOUT_MS 5000
#define WAIT_FOR_DOORS_TO_OPEN_TIMEOUT_MS 1000

static void initializeElevator(elevator_t* elevator, osMutexId_t mutex) {
  elevator->state = elevator_state_idle;

  cmdInitialize(elevator, mutex);
}

static void turnButtonOn(elevator_t* elevator, floor_t floor,
                         osMutexId_t mutex) {
  cmdTurnButtonOn(elevator, floor, mutex);
}

static void turnButtonOff(elevator_t* elevator, floor_t floor,
                          osMutexId_t mutex) {
  cmdTurnButtonOff(elevator, floor, mutex);
}

static void clearRequestsToFloorAndDirection(elevator_t* elevator) {
  elevator->internal_requests[elevator->floor] = false;
  if (elevator->direction == elevator_direction_up) {
    elevator->external_requests_up[elevator->floor] = false;
  } else if (elevator->direction == elevator_direction_down) {
    elevator->external_requests_down[elevator->floor] = false;
  }
}

static void openDoorsWithTimeout(elevator_t* elevator, osTimerId_t timer,
                                 osMutexId_t mutex) {
  cmdOpenDoors(elevator, mutex);
  osTimerStart(timer, WAIT_FOR_DOORS_TO_OPEN_TIMEOUT_MS);
}

static void stopMovingOrKeepGoing(elevator_t* elevator, signal_t signal,
                                  osTimerId_t timer, osMutexId_t mutex) {
  bool should_stop = elevatorShouldStopAtFloor(elevator, signal.floor);
  if (!should_stop) return;

  elevator->state = elevator_state_opening_doors;

  cmdStop(elevator, mutex);
  openDoorsWithTimeout(elevator, timer, mutex);
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

static void addRequestToMap(elevator_t* elevator, signal_t signal) {
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

static void awaitForPassengers(elevator_t* elevator, osTimerId_t timer,
                               osMutexId_t mutex) {
  elevator->state = elevator_state_awaiting_passengers;

  // Stop open-door timeout as the door was open successfully
  osTimerStop(timer);

  clearRequestsToFloorAndDirection(elevator);
  turnButtonOff(elevator, elevator->floor, mutex);
  osTimerStart(timer, WAIT_FOR_PASSENGERS_TIMEOUT_MS);
}

static void closeDoors(elevator_t* elevator, osMutexId_t mutex) {
  elevator->state = elevator_state_closing_doors;
  cmdCloseDoors(elevator, mutex);
}

static void startMovingOrReturnToIdle(elevator_t* elevator, signal_t signal,
                                      osMutexId_t mutex) {
  if (elevator->state != elevator_state_idle &&
      elevator->state != elevator_state_closing_doors)
    return;

  floor_t next_request = elevatorGetNextRequest(elevator);

  bool has_request = next_request != floor_none;
  bool doors_are_open = elevator->door_state == elevator_door_state_open;

  bool should_stay_same_floor = has_request && next_request == elevator->floor;
  bool should_move = !should_stay_same_floor;
  bool should_go_up = has_request && next_request > elevator->floor;
  bool should_go_down = has_request && next_request < elevator->floor;

  bool should_close_doors = has_request && should_move && doors_are_open;

  if (should_close_doors) {
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

// A request may come in to the same floor we are at.
// If we are at idle, we should make sure the door is open
// and we are waiting for passengers.
//
// If we are already waiting for passengers, we should
// wait a little more, restarting the timer.
//
// If we are moving
static void attendNewRequestToCurrentFloorIfStopped(elevator_t* elevator,
                                                    signal_t signal,
                                                    osTimerId_t timer,
                                                    osMutexId_t mutex) {
  if (signal.code != signal_internal_button_pressed &&
      signal.code != signal_external_button_pressed)
    return;

  bool is_stopped_at_this_floor =
      elevatorIsStoppedAtFloor(elevator, signal.floor);
  if (!is_stopped_at_this_floor) return;

  bool doors_are_closed = elevator->door_state == elevator_door_state_closed;
  bool doors_are_open = !doors_are_closed;

  bool is_idle = elevator->state == elevator_state_idle;
  bool is_awaiting_passengers =
      elevator->state == elevator_state_awaiting_passengers;

  if (is_idle && doors_are_open) {
    awaitForPassengers(elevator, timer, mutex);
  } else if (is_idle && doors_are_closed) {
    elevator->state = elevator_state_opening_doors;
    openDoorsWithTimeout(elevator, timer, mutex);
  } else if (is_awaiting_passengers) {
    awaitForPassengers(elevator, timer, mutex);
  }
}

static void goInOppositeDirectionUntilFindsAnotherFloor(elevator_t* elevator,
                                                        osMutexId_t mutex) {
  elevator->state = elevator_state_finding_alignment;
  if (elevator->direction == elevator_direction_up) {
    elevator->direction = elevator_direction_down;
    cmdGoDown(elevator, mutex);
  } else if (elevator->direction == elevator_direction_down) {
    elevator->direction = elevator_direction_up;
    cmdGoUp(elevator, mutex);
  }
}

static void stopAndMoveInPreviousDirection(elevator_t* elevator,
                                           osMutexId_t mutex) {
  cmdStop(elevator, mutex);
  elevator->state = elevator_state_moving;
  if (elevator->direction == elevator_direction_up) {
    elevator->direction = elevator_direction_down;
    cmdGoDown(elevator, mutex);
  } else if (elevator->direction == elevator_direction_down) {
    elevator->direction = elevator_direction_up;
    cmdGoUp(elevator, mutex);
  }
}

typedef struct {
  osMessageQueueId_t* queue;
} door_timer_callback_args_t;
static void doorTimerCallback(void* arg) {
  door_timer_callback_args_t* args = (door_timer_callback_args_t*)arg;

  signal_t signal = {
      .code = signal__internal__doors_timeout,
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

  osTimerId_t timer = this->door_timer;

  signal_t signal;
  while (1) {
    osStatus_t status =
        osMessageQueueGet(this->args.queue, &signal, NULL, osWaitForever);

    if (status != osOK) continue;

    if (signal.code == signal_internal_button_pressed) {
      turnButtonOn(&elevator, signal.floor, mutex);
      addRequestToMap(&elevator, signal);
    }
    if (signal.code == signal_external_button_pressed) {
      addRequestToMap(&elevator, signal);
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
    attendNewRequestToCurrentFloorIfStopped(&elevator, signal, timer, mutex);

    switch (elevator.state) {
      case elevator_state_uninitialized:
        if (signal.code == signal_system_initialized) {
          initializeElevator(&elevator, mutex);
        }
        break;
      case elevator_state_idle:
        // Listen to all signals
        startMovingOrReturnToIdle(&elevator, signal, mutex);
        break;
      case elevator_state_closing_doors:
        if (signal.code == signal_doors_closed) {
          startMovingOrReturnToIdle(&elevator, signal, mutex);
        }
        break;
      case elevator_state_moving:
        if (signal.code == signal_reached_floor) {
          stopMovingOrKeepGoing(&elevator, signal, timer, mutex);
        }
        break;
      case elevator_state_opening_doors:
        // Tried opening doors but timed out. Probably is stuck a little bit
        // over or under the floor alignment. As we don't know the floor height,
        // there is no way to know exactly where we are. We must get to the
        // opposite floor and then retry moving up.
        if (signal.code == signal__internal__doors_timeout) {
          goInOppositeDirectionUntilFindsAnotherFloor(&elevator, mutex);
        } else if (signal.code == signal_doors_open) {
          awaitForPassengers(&elevator, timer, mutex);
        }
        break;
      case elevator_state_awaiting_passengers:
        if (signal.code == signal__internal__doors_timeout) {
          closeDoors(&elevator, mutex);
        }
        break;
      case elevator_state_finding_alignment:
        if (signal.code == signal_reached_floor) {
          stopAndMoveInPreviousDirection(&elevator, mutex);
        }
        break;
    }
  }
}