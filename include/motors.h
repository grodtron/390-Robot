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

typedef enum {
   LEFT,
   RIGHT
} turn_dir_t;

typedef enum {
   FWD,
   REV
} motor_dir_t;


void init_motors();

void set_speed(uint8_t speed, motor_dir_t dir);

void turn_in_arc(uint8_t speed, motor_dir_t dir, turn_dir_t turn_dir, uint16_t radius);

void rotate(uint8_t speed, turn_dir_t dir);

void hard_stop();

#endif
