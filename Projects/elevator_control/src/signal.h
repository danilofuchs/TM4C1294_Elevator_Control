#ifndef __SIGNAL_H_
#define __SIGNAL_H_

#include <stdint.h>

#include "elevator.h"

typedef enum {
  signal_reached_floor,
  signal_doors_open,
  signal_doors_closed,
  signal_internal_button_pressed,
  signal_external_button_pressed,
} signal_code_t;

typedef struct {
  elevator_code_t elevator_code;
  signal_code_t code;

  // Optional number, available when signal is signal_reached_floor,
  // signal_internal_button_pressed or signal_external_button_pressed
  uint8_t floor;
} signal_t;

#endif