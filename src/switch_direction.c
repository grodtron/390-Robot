#include <avr/interrupt.h>

#include "../include/motors.h"
#include "../include/iodefs.h"
#include "../include/movement_manager.h"

void switch_direction(){

   cli();

   motors_switch_direction();
   iodefs_switch_direction();
   movman_switch_direction();

   sei();

}
