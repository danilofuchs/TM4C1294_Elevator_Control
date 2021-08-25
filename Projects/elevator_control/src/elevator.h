#ifndef __ELEVATOR_H__
#define __ELEVATOR_H__

#include <stdbool.h>

#define ELEVATOR_NUM_FLOORS 16
#define ELEVATOR_MAX_NUM_REQUESTS ELEVATOR_NUM_FLOORS * 3

typedef int8_t floor_t;
extern const floor_t floor_none;

typedef enum {
  elevator_code_unknown,
  elevator_code_left,
  elevator_code_center,
  elevator_code_right,
} elevator_code_t;

typedef enum {
  elevator_direction_none,
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

typedef enum {
  elevator_request_type_inactive,
  elevator_request_type_internal,
  elevator_request_type_external,
} elevator_request_type_t;

typedef struct {
  elevator_code_t code;
  elevator_state_t state;

  floor_t floor;
  elevator_direction_t direction;
  elevator_door_state_t door_state;

  bool internal_requests[ELEVATOR_NUM_FLOORS];
  bool external_requests_up[ELEVATOR_NUM_FLOORS];
  bool external_requests_down[ELEVATOR_NUM_FLOORS];
} elevator_t;

bool elevatorIsStoppedAtFloor(elevator_t *elevator, floor_t floor);
bool elevatorShouldStopAtFloor(elevator_t *elevator, floor_t floor);
floor_t elevatorGetNextRequest(elevator_t *elevator);

#endif