#include "elevator.h"

#include <math.h>
#include <stdlib.h>

bool elevatorIsStoppedAtFloor(elevator_t *elevator, uint8_t floor) {
  if (floor != elevator->floor) return false;

  return elevator->state == elevator_state_idle ||
         elevator->state == elevator_state_awaiting_passengers ||
         elevator->state == elevator_state_opening_doors;
}

bool elevatorShouldStopAtFloor(elevator_t *elevator, uint8_t floor) {
  if (elevator->internal_requests[floor]) return true;

  if (elevator->external_requests_up[floor] &&
      elevator->direction == elevator_direction_up)
    return true;

  if (elevator->external_requests_down[floor] &&
      elevator->direction == elevator_direction_down)
    return true;

  return false;
}

static bool hasRequestForFloor(elevator_t *elevator, uint8_t floor) {
  return elevator->internal_requests[floor] ||
         elevator->external_requests_down[floor] ||
         elevator->external_requests_up[floor];
}

static uint8_t getClosestRequestedFloor(elevator_t *elevator) {
  int8_t closest_floor = 0;
  uint8_t closest_distance = UINT8_MAX;
  for (uint8_t floor = 0; floor < ELEVATOR_NUM_FLOORS; floor++) {
    if (!hasRequestForFloor(elevator, floor)) continue;

    uint8_t distance = abs(elevator->floor - floor);

    // Prioritize the closest floor
    // In case of a tie, as we are going upwards in the array, so we end up
    // prioritizing the floor that is lower
    if (distance < closest_distance) {
      closest_floor = floor;
      closest_distance = distance;
    }
  }
  return closest_floor;
}

elevator_direction_t elevatorGetNextDirection(elevator_t *elevator) {
  if (elevator->direction != elevator_direction_none) {
    // Already moving, should keep direction
    return elevator->direction;
  }
  uint8_t closest_request = getClosestRequestedFloor(elevator);

  if (elevator->floor < closest_request) {
    return elevator_direction_up;
  } else if (elevator->floor > closest_request) {
    return elevator_direction_down;
  }
  return elevator_direction_none;
}

// Finds out if elevator is aligned with floor based on the current height.
// Returns the floor if aligned, otherwise returns -1.
int8_t elevatorGetEstimatedFloorGivenHeight(elevator_t *elevator) {
  float floor_fractions =
      (float)elevator->height / (float)ELEVATOR_FLOOR_HEIGHT;

  float fraction = modf(floor_fractions, NULL);

  float tolerancy =
      (float)ELEVATOR_FLOOR_HEIGHT / (float)ELEVATOR_FLOOR_TOLERANCY;

  if (floor_fractions > tolerancy && floor_fractions < 1.0 - tolerancy) {
    // We are between floors, so we are need to detect the floor that is closest
    // to the current position
    if (fraction < 0.5) {
      return (int8_t)floor(floor_fractions);
    } else {
      return (int8_t)ceil(floor_fractions);
    }
  }

  return -1;
}