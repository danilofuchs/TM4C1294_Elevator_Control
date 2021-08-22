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

static void stop(elevator_t* elevator, osMutexId_t mutex) {
  command_t command = {
      .code = command_stop,
      .elevator_code = elevator->code,
  };

  sendCommand(&command, mutex);
}

static void openDoors(elevator_t* elevator, osMutexId_t mutex) {
  elevator->state = elevator_state_opening_doors;

  command_t command = {
      .code = command_open_doors,
      .elevator_code = elevator->code,
  };

  sendCommand(&command, mutex);
}
static void closeDoors(elevator_t* elevator, osMutexId_t mutex) {
  elevator->state = elevator_state_closing_doors;

  command_t command = {
      .code = command_close_doors,
      .elevator_code = elevator->code,
  };

  sendCommand(&command, mutex);
}

static void goUp(elevator_t* elevator, osMutexId_t mutex) {
  elevator->state = elevator_state_moving;
  elevator->direction = elevator_direction_up;

  command_t command = {
      .code = command_go_up,
      .elevator_code = elevator->code,
  };

  sendCommand(&command, mutex);
}

static void goDown(elevator_t* elevator, osMutexId_t mutex) {
  elevator->state = elevator_state_moving;
  elevator->direction = elevator_direction_down;

  command_t command = {
      .code = command_go_down,
      .elevator_code = elevator->code,
  };

  sendCommand(&command, mutex);
}

static void queryHeight(elevator_t* elevator, osMutexId_t height_query_mutex,
                        osMutexId_t mutex) {
  command_t command = {
      .code = command_query_height,
      .elevator_code = elevator->code,
  };

  osMutexAcquire(height_query_mutex, osWaitForever);
  sendCommand(&command, mutex);
}

static void reachedFloor(elevator_t* elevator, int8_t floor,
                         osMutexId_t mutex) {
  if (floor < 0) return;
  elevator->floor = floor;
}

static void internalButtonWasPressed(elevator_t* elevator, int8_t floor,
                                     osMutexId_t mutex) {
  if (elevatorIsStoppedAtFloor(elevator, floor)) return;

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
  elevator_direction_t next_direction = elevatorGetNextDirection(elevator);
  if (next_direction == elevator_direction_up) {
    goUp(elevator, mutex);
  } else if (next_direction == elevator_direction_down) {
    goDown(elevator, mutex);
  }
}

static void stopIfNecessary(elevator_t* elevator, uint8_t floor,
                            osMutexId_t mutex) {
  if (!elevatorShouldStopAtFloor(elevator, floor)) return;

  stop(elevator, mutex);
  openDoors(elevator, mutex);
}

static void heightChanged(elevator_t* elevator, uint32_t height,
                          osMutexId_t height_query_mutex) {
  osMutexRelease(height_query_mutex);
  elevator->height = height;
}

typedef struct {
  elevator_t* elevator;
  osMessageQueueId_t queue;
} height_querier_callback_args_t;

void heightQuerier(void* arg) {
  height_querier_callback_args_t* args = (height_querier_callback_args_t*)arg;

  if (args->elevator->state != elevator_state_moving) return;

  signal_t signal = {
      .code = signal__internal__should_query_height,
      .elevator_code = args->elevator->code,
      .direction = elevator_direction_none,
      .floor = -1,
      .height = 0,
  };
  osMessageQueuePut(args->queue, &signal, 0, 0);
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

  this->height_querier_timer =
      osTimerNew(heightQuerier, osTimerPeriodic,
                 &(height_querier_callback_args_t){
                     .elevator = &elevator,
                     .queue = this->args.queue,
                 },
                 &(osTimerAttr_t){.name = "Height Querier Timer"});
  osTimerStart(this->height_querier_timer,
               ELEVATOR_THREAD_HEIGHT_QUERIER_PERIOD);

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
        stopIfNecessary(&elevator, signal.floor, mutex);
        break;
      case signal__internal__should_query_height:
        queryHeight(&elevator, this->args.height_query_mutex, mutex);
        break;
      case signal_height_changed:
        heightChanged(&elevator, signal.height, this->args.height_query_mutex);
        break;
    }
  }
}