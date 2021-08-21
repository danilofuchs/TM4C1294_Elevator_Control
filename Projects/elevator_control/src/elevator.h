#ifndef __ELEVATOR_H__
#define __ELEVATOR_H__

typedef enum {
  elevator_code_unknown,
  elevator_code_left,
  elevator_code_center,
  elevator_code_right,
} elevator_code_t;

typedef enum {
  elevator_direction_unknown,
  elevator_direction_up,
  elevator_direction_down,
} elevator_direction_t;

typedef enum {
  elevator_state_uninitialized,
  elevator_state_idle,
  elevator_state_moving,
  elevator_state_closing_doors,
  elevator_state_opening_doors,
  elevator_state_awaiting_passengers,
} elevator_state_t;

typedef enum {
  elevator_door_state_closed,
  elevator_door_state_open,
} elevator_door_state_t;

typedef struct {
  elevator_code_t code;
  elevator_state_t state;

  int8_t floor;
  elevator_direction_t direction;
  elevator_door_state_t door_state;
} elevator_t;

#endif