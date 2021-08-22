#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <stdbool.h>

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
  command_query_height,
} command_code_t;

typedef struct {
  elevator_code_t elevator_code;
  command_code_t code;

  // Required when command is command_turn_button_on,
  // command_turn_button_off.
  int8_t floor;  // Optional argument
} command_t;

// Builds a string with the command
// Returns true if it was built successfully
bool commandBuild(command_t* command, char* command_string);

#endif