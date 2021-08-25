#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <stdbool.h>

#include "cmsis_os2.h"
#include "elevator.h"

typedef enum {
  command_initialize,
  command_open_doors,
  command_close_doors,
  command_go_up,
  command_go_down,
  command_stop,
  command_turn_button_on,
  command_turn_button_off,
} command_code_t;

typedef struct {
  elevator_code_t elevator_code;
  command_code_t code;

  // Required when command is command_turn_button_on,
  // command_turn_button_off.
  floor_t floor;  // Optional argument
} command_t;

void cmdInitialize(elevator_t* elevator, osMutexId_t mutex);
void cmdTurnButtonOn(elevator_t* elevator, floor_t floor, osMutexId_t mutex);
void cmdTurnButtonOff(elevator_t* elevator, floor_t floor, osMutexId_t mutex);
void cmdStop(elevator_t* elevator, osMutexId_t mutex);
void cmdOpenDoors(elevator_t* elevator, osMutexId_t mutex);
void cmdCloseDoors(elevator_t* elevator, osMutexId_t mutex);
void cmdGoUp(elevator_t* elevator, osMutexId_t mutex);
void cmdGoDown(elevator_t* elevator, osMutexId_t mutex);

#endif