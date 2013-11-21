#ifndef MOVEMENT_MANAGER_H_
#define MOVEMENT_MANAGER_H_

#include <inttypes.h>

#include "../include/motors.h"

typedef enum {
   TO_AVOID_EDGE     = 255,
   TO_AVOID_FIREPIT  = 200,
   TO_ATTACK         = 150,
   TO_DEFEND         = 100,
   TO_SEARCH         = 50,
   NO_REASON         = 0
} movement_reason_t;

typedef enum {
   BACKUP_THEN_TURN_90_CCW,
   SPIRAL_OUTWARDS,
   FORWARD_THEN_WIDE_TURN_RIGHT
} movement_t;

typedef enum {
   IMMEDIATELY_ELSE_IGNORE = 0,
   NEXT_AVAILABLE_TIME     = 1
} movement_time_t;

void movman_init();

void movman_current_move_completed();

// Schedules a move and returns its 1-indexed position in the movement queue.
// 1 means that it is executed immediately, n>1 means that it will be executed
// after n-1 other moves complete, and 0 means that it was not added because its
// priority was too low nad the queue is full
uint8_t movman_schedule_motor_instruction(
   movement_reason_t reason,

   void (*move_funct)(uint8_t, motor_dir_t, motor_turn_dir_t, uint16_t, uint16_t),
   uint8_t          speed,
   motor_dir_t      dir,
   motor_turn_dir_t turn_dir,
   uint16_t         param,
   uint16_t         timeout,

   movement_time_t when
);


bool movman_schedule_move(movement_t move, movement_reason_t reason, movement_time_t when);

#endif
