#include "signal.h"

#include <stdbool.h>
#include <string.h>

#include "utils/uartstdio.h"

const uint8_t invalid_elevator_code = 100;
const uint8_t invalid_signal_code = 100;

elevator_code_t parseElevatorCode(char code);
signal_code_t parseSignalCode(char code);

int parseSignal(signal_t *signal, char *input) {
  int signal_length = strlen(input);
  if (signal_length < 2) {
    UARTprintf("Error: signal string too short. Length: %d\r\n", signal_length);
    return -1;
  }
  char elevator_code_c = input[0];
  char signal_code_c = input[1];

  elevator_code_t elevator_code = parseElevatorCode(elevator_code_c);
  if (elevator_code == invalid_elevator_code) {
    UARTprintf("Error: unknown elevator code '%c' (%d)\r\n", elevator_code_c,
               elevator_code_c);
    return -1;
  }

  signal_code_t signal_code = parseSignalCode(signal_code_c);
  if (signal_code == invalid_signal_code) {
    UARTprintf("Error: unknown signal code '%c' (%d)\r\n", signal_code_c,
               signal_code_c);
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
      return invalid_elevator_code;
  }
}

signal_code_t parseSignalCode(char code) { return signal_reached_floor; }

void printSignal(signal_t *signal) {
  UARTprintf("Signal: elevator_code: %d, code: %d, floor: %d",
             signal->elevator_code, signal->code, signal->floor);
}