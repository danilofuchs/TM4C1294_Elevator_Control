#ifndef __ELEVATOR_H__
#define __ELEVATOR_H__

typedef enum {
  elevator_unknown,
  elevator_left,
  elevator_center,
  elevator_right,
} elevator_code_t;

typedef enum {
  elevator_direction_unknown,
  elevator_direction_up,
  elevator_direction_down,
} elevator_direction_t;

#endif