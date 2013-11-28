#ifndef MOVEMENT_MANAGER_H_
#define MOVEMENT_MANAGER_H_

#include <inttypes.h>

#include "../include/motors.h"

typedef enum {
   TO_AVOID_EDGE     = 255,
   TO_MEET_STARTUP_REQUIREMENT = 220,
   TO_AVOID_FIREPIT  = 200,
   TO_ATTACK         = 150,
   TO_DEFEND         = 100,
   TO_SEEK           = 75,
   TO_SEARCH         = 50,
   NO_REASON         = 0
} movement_reason_t;

typedef enum {
   BACKUP_THEN_TURN_90_CCW,
   WAIT_5_SECONDS_THEN_FULL_FORWARD_FOR_A_LONG_TIME,
   GO_FORWARD_BRIEFLY,
   SMALL_TURN_LEFT,
   SMALL_TURN_RIGHT,
   FORWARD_THEN_WIDE_TURN_RIGHT,
   SEARCH_PATTERN,
   MOVE_FORWARD
} movement_t;

void movman_init();

void movman_current_move_completed();

bool movman_schedule_move(movement_t move, movement_reason_t reason);

#endif
