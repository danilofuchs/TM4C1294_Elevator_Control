#ifndef __COMMAND_H__
#define __COMMAND_H__

#include "elevator.h"

typedef enum {
  command_initialize,
  command_open_door,
  command_close_door,
  command_go_up,
  command_go_down,
  command_stop,
  command_query_height,
  command_turn_button_on,
  command_turn_button_off,
} command_code_t;

typedef struct {
  elevator_code_t elevator_code;
  command_code_t code;
  char argument;  // Optional argument
} command_t;

// Builds a string with the command
void buildCommand(command_t command, char *command_string);

#endif