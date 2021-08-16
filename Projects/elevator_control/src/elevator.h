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

typedef struct {
  elevator_code_t code;
  elevator_direction_t direction;
} elevator_t;

#endif