#include <stdio.h>

#include "command.h"

static char buildElevatorCode(elevator_code_t code) {
  switch (code) {
    case elevator_code_left:
      return 'e';
    case elevator_code_center:
      return 'c';
    case elevator_code_right:
      return 'd';
  }
  return NULL;
}

static bool initialize(command_t* command, char* command_string) {
  int success = sprintf(command_string, "%cr\r",
                        buildElevatorCode(command->elevator_code));

  return success >= 0;
}

static bool openDoors(command_t* command, char* command_string) {
  int success = sprintf(command_string, "%ca\r",
                        buildElevatorCode(command->elevator_code));

  return success >= 0;
}

static bool closeDoors(command_t* command, char* command_string) {
  int success = sprintf(command_string, "%cf\r",
                        buildElevatorCode(command->elevator_code));

  return success >= 0;
}

static bool goUp(command_t* command, char* command_string) {
  int success = sprintf(command_string, "%cs\r",
                        buildElevatorCode(command->elevator_code));

  return success >= 0;
}

static bool goDown(command_t* command, char* command_string) {
  int success = sprintf(command_string, "%cd\r",
                        buildElevatorCode(command->elevator_code));

  return success >= 0;
}

static bool stop(command_t* command, char* command_string) {
  int success = sprintf(command_string, "%cp\r",
                        buildElevatorCode(command->elevator_code));

  return success >= 0;
}

static bool turnButtonOn(command_t* command, char* command_string,
                         floor_t floor) {
  if (floor < 0 || floor > 15) return false;
  char button_code = 'a' + floor;
  int success = sprintf(command_string, "%cL%c\r",
                        buildElevatorCode(command->elevator_code), button_code);

  return success >= 0;
}

static bool turnButtonOff(command_t* command, char* command_string,
                          floor_t floor) {
  if (floor < 0 || floor > 15) return false;
  char button_code = 'a' + floor;
  int success = sprintf(command_string, "%cD%c\r",
                        buildElevatorCode(command->elevator_code), button_code);

  return success >= 0;
}

// Builds a string with the command
// Returns true if it was built successfully
bool commandBuild(command_t* command, char* command_string) {
  if (command->elevator_code == elevator_code_unknown) {
#ifdef DEBUG
    printf("Error: Unknown elevator.\n");
#endif
    return false;
  }

  switch (command->code) {
    case command_initialize:
      return initialize(command, command_string);
    case command_open_doors:
      return openDoors(command, command_string);
    case command_close_doors:
      return closeDoors(command, command_string);
    case command_go_up:
      return goUp(command, command_string);
    case command_go_down:
      return goDown(command, command_string);
    case command_stop:
      return stop(command, command_string);
    case command_turn_button_on:
      return turnButtonOn(command, command_string, command->floor);
    case command_turn_button_off:
      return turnButtonOff(command, command_string, command->floor);
  }

  return false;
}
