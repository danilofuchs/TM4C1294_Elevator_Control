#ifndef __SIGNAL_H__
#define __SIGNAL_H__

#include <stdbool.h>
#include <stdint.h>

#include "elevator.h"

typedef enum {
  signal_unknown,
  signal_system_initialized,
  signal_reached_floor,
  signal_doors_open,
  signal_doors_closed,
  signal_internal_button_pressed,
  signal_external_button_pressed,
} signal_code_t;

typedef struct {
  elevator_code_t elevator_code;
  signal_code_t code;

  // Available when signal is signal_reached_floor,
  // signal_internal_button_pressed or signal_external_button_pressed
  // -1 otherwise.
  int8_t floor;

  // Available for signal_external_button_pressed,
  // elevator_direction_unknown otherwise.
  elevator_direction_t direction;
} signal_t;

// Tries to parse the signal from string.
// Returns true if successful and populated the signal pointer, false otherwise.
bool parseSignal(signal_t *signal, char *signal_string);

void printSignal(signal_t *signal);

#endif