#include "elevator.h"

bool elevatorIsStoppedAtFloor(elevator_t *elevator, uint8_t floor) {
  if (floor != elevator->floor) return false;

  return (elevator->state == elevator_state_idle ||
          elevator->state == elevator_state_awaiting_passengers ||
          elevator->state == elevator_state_opening_doors);
}