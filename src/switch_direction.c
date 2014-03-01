#include <avr/interrupt.h>

#include "../include/motors.h"
#include "../include/iodefs.h"
#include "../include/range_sensors.h"

#include "../include/switch_direction.h"

bool current_front_is_good_front = true;

void switch_direction(){

   cli();

   motors_switch_direction();
   iodefs_switch_direction();
   range_sensors_switch_direction();

   current_front_is_good_front = !current_front_is_good_front;

   sei();

}
