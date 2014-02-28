#ifndef MOVEMENT_MANAGER_H_
#define MOVEMENT_MANAGER_H_

#include <inttypes.h>

#include "../include/motors.h"

typedef enum {
   IMMEDIATELY,
   IMMEDIATELY_WITH_OVERWRITE,
   NEXT_AVAILABLE_TIME
} movement_time_t;

typedef enum {
   TO_AVOID_EDGE     = 255,
   TO_MEET_STARTUP_REQUIREMENT = 220,
   TO_ATTACK         = 150,
   TO_DEFEND         = 100,
   TO_SEEK           = 75,
   TO_SEARCH         = 50,
   NO_REASON         = 0
} movement_reason_t;

typedef enum {
   ROTATE_LEFT,
   LONG_ROTATE_LEFT,
   ROTATE_RIGHT,
   ROTATE_90_RIGHT_THEN_MOVE_FORWARD,
   ROTATE_90_LEFT_THEN_MOVE_FORWARD,
   SMALL_TURN_LEFT,
   SMALL_TURN_RIGHT,
   FORWARD_THEN_WIDE_TURN_RIGHT,
   SEARCH_PATTERN,
   SMALL_MOVE_FORWARD,
   MOVE_FORWARD,
   SWITCH_DIRECTION_THEN_MOVE_FORWARD
} movement_t;

void movman_init();

bool movman_current_move_completed(bool);

bool movman_schedule_move(movement_t move, movement_reason_t reason, movement_time_t);

#endif
