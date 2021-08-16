#include "command.h"

#include <stdio.h>

char buildElevatorCode(elevator_code_t code);
bool initialize(command_t* command, char* command_string);

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
  }

  return false;
}

char buildElevatorCode(elevator_code_t code) {
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

bool initialize(command_t* command, char* command_string) {
  int success = sprintf(command_string, "%cr\r",
                        buildElevatorCode(command->elevator_code));

  return success >= 0;
}