
#include "command.h"

extern bool commandBuild(command_t* command, char* command_string);

static void sendCommand(command_t* command, osMutexId_t mutex) {
  char string[16];
  commandBuild(command, string);

  osMutexAcquire(mutex, osWaitForever);
  UARTprintf(string);
  UARTFlushTx(false);
  osMutexRelease(mutex);
}

void cmdInitialize(elevator_t* elevator, osMutexId_t mutex) {
  command_t command = {
      .code = command_initialize,
      .elevator_code = elevator->code,
  };

  sendCommand(&command, mutex);
}

void cmdTurnButtonOn(elevator_t* elevator, floor_t floor, osMutexId_t mutex) {
  command_t command = {
      .code = command_turn_button_on,
      .elevator_code = elevator->code,
      .floor = floor,
  };

  sendCommand(&command, mutex);
}

void cmdTurnButtonOff(elevator_t* elevator, floor_t floor, osMutexId_t mutex) {
  command_t command = {
      .code = command_turn_button_off,
      .elevator_code = elevator->code,
      .floor = floor,
  };

  sendCommand(&command, mutex);
}

void cmdStop(elevator_t* elevator, osMutexId_t mutex) {
  command_t command = {
      .code = command_stop,
      .elevator_code = elevator->code,
  };

  sendCommand(&command, mutex);
}

void cmdOpenDoors(elevator_t* elevator, osMutexId_t mutex) {
  command_t command = {
      .code = command_open_doors,
      .elevator_code = elevator->code,
  };

  sendCommand(&command, mutex);
}

void cmdCloseDoors(elevator_t* elevator, osMutexId_t mutex) {
  command_t command = {
      .code = command_close_doors,
      .elevator_code = elevator->code,
  };

  sendCommand(&command, mutex);
}

void cmdGoUp(elevator_t* elevator, osMutexId_t mutex) {
  command_t command = {
      .code = command_go_up,
      .elevator_code = elevator->code,
  };

  sendCommand(&command, mutex);
}

void cmdGoDown(elevator_t* elevator, osMutexId_t mutex) {
  command_t command = {
      .code = command_go_down,
      .elevator_code = elevator->code,
  };

  sendCommand(&command, mutex);
}