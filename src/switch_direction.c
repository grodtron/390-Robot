#include <avr/interrupt.h>

#include "../include/motors.h"
#include "../include/iodefs.h"
#include "../include/range_sensors.h"

void switch_direction(){

   cli();

   motors_switch_direction();
   iodefs_switch_direction();
   range_sensors_switch_direction();

   sei();

}
