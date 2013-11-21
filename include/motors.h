#ifndef MOTORS_H_
#define MOTORS_H_

/* TODO ?
typedef enum {
   ACCELING,
   DECELING,
   TURNING_R,
   TURNING_L,
   FORWARD,
   REVERSE,
   STOPPED
} motor_state_t;
*/

#include <inttypes.h>
#include <stdbool.h>

typedef enum {
   LEFT,
   RIGHT
} motor_turn_dir_t;

typedef enum {
   FWD,
   REV
} motor_dir_t;


void motors_init();

void motors_set_speed(uint8_t speed, motor_dir_t dir, motor_turn_dir_t dummy1, uint16_t dummy2, uint16_t timeout);

void motors_turn_in_arc(uint8_t speed, motor_dir_t dir, motor_turn_dir_t turn_dir, uint16_t radius, uint16_t timeout);

void motors_rotate(uint8_t speed, motor_dir_t dummy1, motor_turn_dir_t dir, uint16_t dummy2, uint16_t timeout);

void motors_hard_stop();

bool motors_movement_in_progress();

#endif
