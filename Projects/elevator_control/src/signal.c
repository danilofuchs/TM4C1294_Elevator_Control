#include "signal.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "uart.h"

static bool isNumeric(char ch) { return ch >= '0' && ch <= '9'; }
static uint8_t parseIntFromChar(char ch) { return ch - '0'; }

static floor_t parseFloor(char *input, signal_code_t signal_code) {
  if (signal_code == signal_reached_floor) {
    // <0:elevator><1:dozens or units><2:units>
    char dozensOrUnits = input[1];
    char units = input[2];

    if (isNumeric(units) && isNumeric(dozensOrUnits)) {
      return 10 * parseIntFromChar(dozensOrUnits) + parseIntFromChar(units);
    } else if (isNumeric(dozensOrUnits)) {
      return parseIntFromChar(dozensOrUnits);
    }
    return -1;
  } else if (signal_code == signal_external_button_pressed) {
    // <0:elevator><1:code><2:dozens><3:units><4:direction>
    char dozens = input[2];
    char units = input[3];
    if (isNumeric(dozens) && isNumeric(units)) {
      return 10 * parseIntFromChar(dozens) + parseIntFromChar(units);
    }
    return -1;
  } else if (signal_code == signal_internal_button_pressed) {
    // <0:elevator><1:code><2:button_code>
    // button_code is from 'a' to 'p'
    char button_code = input[2];
    if (button_code >= 'a' && button_code <= 'p') {
      return button_code - 'a';
    }
    return -1;
  }
  return -1;
}

elevator_code_t parseElevatorCode(char *input) {
  char code = input[0];
  switch (code) {
    case 'e':
      return elevator_code_left;
    case 'c':
      return elevator_code_center;
    case 'd':
      return elevator_code_right;
    default:
      return elevator_code_unknown;
  }
}

signal_code_t parseSignalCode(char *input) {
  if (isNumeric(input[1])) {
    return signal_reached_floor;
  }
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

  return signal_unknown;
}

static elevator_direction_t parseDirection(char *input,
                                           signal_code_t signal_code) {
  if (signal_code == signal_external_button_pressed) {
    // <0:elevator><1:code><2:dozens><3:units><4:direction>
    char direction = input[4];
    switch (direction) {
      case 's':
        return elevator_direction_up;
      case 'd':
        return elevator_direction_down;
    }
  }
  return elevator_direction_none;
}

bool signalParse(signal_t *signal, char *input) {
  if (strcmp(input, "initialized") == 0) {
    signal->elevator_code = elevator_code_unknown;
    signal->code = signal_system_initialized;
    signal->floor = -1;

    return true;
  }

  signal_code_t signal_code = parseSignalCode(input);
  if (signal_code == signal_unknown) {
#ifdef DEBUG
    printf("Error: unknown signal code\n");
#endif
    return false;
  }

  elevator_code_t elevator_code = parseElevatorCode(input);

  floor_t floor = parseFloor(input, signal_code);

  elevator_direction_t direction = parseDirection(input, signal_code);

  signal->elevator_code = elevator_code;
  signal->code = signal_code;
  signal->floor = floor;
  signal->direction = direction;

  return true;
}

void signalDebug(signal_t *signal) {
#ifdef DEBUG
  // Compact form because printing a lot of data is slow
  printf("S: c:%d;el:%d,fl:%d,dir:%d\n", signal->code, signal->elevator_code,
         signal->floor, signal->direction);
#endif
}