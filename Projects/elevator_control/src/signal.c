#include "signal.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "utils/uartstdio.h"

elevator_code_t parseElevatorCode(char code);
signal_code_t parseSignalCode(char code);

int parseSignal(signal_t *signal, char *input) {
  int signal_length = strlen(input);
  if (signal_length < 2) {
#ifdef DEBUG
    printf("Error: signal string too short. Length: %d\n", signal_length);
#endif
    return -1;
  }
  char elevator_code_c = input[0];
  char signal_code_c = input[1];

  elevator_code_t elevator_code = parseElevatorCode(elevator_code_c);
  if (elevator_code == elevator_unknown) {
#ifdef DEBUG
    printf("Error: unknown elevator code '%c' (%d)\n", elevator_code_c,
           elevator_code_c);
#endif
    return -1;
  }

  signal_code_t signal_code = parseSignalCode(signal_code_c);
  if (signal_code == signal_unknown) {
#ifdef DEBUG
    printf("Error: unknown signal code '%c' (%d)\n", signal_code_c,
           signal_code_c);
#endif
    return -1;
  }

  signal->elevator_code = elevator_code;
  signal->code = signal_code;
  signal->floor = 0;

  return 0;
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

signal_code_t parseSignalCode(char code) { return signal_reached_floor; }

void printSignal(signal_t *signal) {
#ifdef DEBUG
  printf("Signal: elevator_code: %d, code: %d, floor: %d\n",
         signal->elevator_code, signal->code, signal->floor);
#endif
}