#include "signal.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "utils/uartstdio.h"

elevator_code_t parseElevatorCode(char code);
signal_code_t parseSignalCode(char *input);
int8_t parseFloor(char *input, signal_code_t signal_code);
elevator_direction_t parseDirection(char *input, signal_code_t signal_code);

bool parseSignal(signal_t *signal, char *input) {
  if (strcmp(input, "initialized") == 0) {
    signal->elevator_code = elevator_unknown;
    signal->code = signal_system_initialized;
    signal->floor = -1;

    return true;
  }
  int signal_length = strlen(input);
  if (signal_length < 2) {
#ifdef DEBUG
    printf("Error: signal string too short. Length: %d\n", signal_length);
#endif
    return false;
  }
  char elevator_code_c = input[0];
  char signal_code_c = input[1];

  elevator_code_t elevator_code = parseElevatorCode(elevator_code_c);
  if (elevator_code == elevator_unknown) {
#ifdef DEBUG
    printf("Error: unknown elevator code '%c' (%d)\n", elevator_code_c,
           elevator_code_c);
#endif
    return false;
  }

  signal_code_t signal_code = parseSignalCode(input);
  if (signal_code == signal_unknown) {
#ifdef DEBUG
    printf("Error: unknown signal code '%c' (%d)\n", signal_code_c,
           signal_code_c);
#endif
    return false;
  }

  int8_t floor = parseFloor(input, signal_code);

  elevator_direction_t direction = parseDirection(input, signal_code);

  signal->elevator_code = elevator_code;
  signal->code = signal_code;
  signal->floor = floor;
  signal->direction = direction;

  return true;
}

elevator_code_t parseElevatorCode(char code) {
  switch (code) {
    case 'e':
      return elevator_left;
    case 'c':
      return elevator_center;
    case 'd':
      return elevator_right;
    default:
      return elevator_unknown;
  }
}

signal_code_t parseSignalCode(char *input) {
  char code = input[1];
  switch (code) {
    case 'A':
      return signal_doors_open;
    case 'F':
      return signal_doors_closed;
    case 'I':
      return signal_internal_button_pressed;
    case 'E':
      return signal_external_button_pressed;
  }
  if (code >= '0' && code <= '9') {
    return signal_reached_floor;
  }
  return signal_unknown;
}

int8_t parseFloor(char *input, signal_code_t signal_code) { return -1; }

elevator_direction_t parseDirection(char *input, signal_code_t signal_code) {
  if (signal_code == signal_external_button_pressed) {
    // <0:elevator>E<2:dozens><3:units><4:direction>
    char direction = input[4];
    switch (direction) {
      case 's':
        return elevator_direction_up;
      case 'd':
        return elevator_direction_down;
    }
  }
  return elevator_direction_unknown;
}

void printSignal(signal_t *signal) {
#ifdef DEBUG
  // Compact form because printing a lot of data is slow
  printf("S: c: %d, el: %d, fl: %d, dir: %d\n", signal->elevator_code,
         signal->code, signal->floor, signal->direction);
#endif
}