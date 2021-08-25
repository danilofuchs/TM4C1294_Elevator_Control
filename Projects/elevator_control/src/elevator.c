#include "elevator.h"

#include <math.h>
#include <stdlib.h>

const floor_t floor_none = -1;

bool elevatorIsStoppedAtFloor(elevator_t *elevator, floor_t floor) {
  if (floor != elevator->floor) return false;

  return elevator->state == elevator_state_idle ||
         elevator->state == elevator_state_awaiting_passengers ||
         elevator->state == elevator_state_opening_doors;
}

bool elevatorShouldStopAtFloor(elevator_t *elevator, floor_t floor) {
  bool has_internal_request = elevator->internal_requests[floor];
  bool has_external_request_up = elevator->external_requests_up[floor];
  bool has_external_request_down = elevator->external_requests_down[floor];

  bool is_already_going_up = elevator->direction == elevator_direction_up;
  bool is_already_going_down = elevator->direction == elevator_direction_down;

  return has_internal_request ||
         (has_external_request_up && is_already_going_up) ||
         (has_external_request_down && is_already_going_down);
}

static bool hasRequestForFloor(elevator_t *elevator, floor_t floor) {
  return elevator->internal_requests[floor] ||
         elevator->external_requests_down[floor] ||
         elevator->external_requests_up[floor];
}

static floor_t getClosestRequestAbove(elevator_t *elevator) {
  for (floor_t floor = elevator->floor + 1; floor <= ELEVATOR_NUM_FLOORS;
       floor++) {
    if (hasRequestForFloor(elevator, floor)) return floor;
  }
  return floor_none;
}

static floor_t getClosestRequestBelow(elevator_t *elevator) {
  for (floor_t floor = elevator->floor - 1; floor >= 0; floor--) {
    if (hasRequestForFloor(elevator, floor)) return floor;
  }
  return floor_none;
}

static floor_t getClosestRequest(elevator_t *elevator) {
  floor_t closest_floor = -1;
  uint8_t closest_distance = UINT8_MAX;
  for (floor_t floor = 0; floor < ELEVATOR_NUM_FLOORS; floor++) {
    if (!hasRequestForFloor(elevator, floor)) continue;

    uint8_t distance = abs(elevator->floor - floor);

    // Prioritize the closest floor
    // In case of a tie, as we are going upwards in the array, so we end up
    // prioritizing the floor that is lower
    if (distance < closest_distance) {
      closest_floor = floor;
      closest_distance = distance;
    } else {
      break;
    }
  }
  return closest_floor;
}

floor_t elevatorGetNextRequest(elevator_t *elevator) {
  if (elevator->direction == elevator_direction_up) {
    floor_t closest = getClosestRequestAbove(elevator);
    if (closest != floor_none) return closest;
  }
  if (elevator->direction == elevator_direction_down) {
    floor_t closest = getClosestRequestBelow(elevator);
    if (closest != floor_none) return closest;
  }

  floor_t closest = getClosestRequest(elevator);
  if (closest != floor_none) return closest;

  return floor_none;
}
