#include <avr/interrupt.h>
#include <avr/io.h>
#define F_CPU 1000000UL
#include <util/delay.h>

#include "../include/event_queue.h"
#include "../include/line_sensors.h"
#include "../include/iodefs.h"
#include "../include/movement_manager.h"
#include "../include/motors.h"

#include "../include/switch_direction.h"

#include "../include/leds.h"

#include "../include/range_sensors.h"

#include "../include/handlers/line_sensors.h"
#include "../include/handlers/range_sensors.h"


static void startup_loop(){

   // Startup wait movement (TODO)
   movman_schedule_move(WAIT_3_SECONDS, TO_MEET_STARTUP_REQUIREMENT, IMMEDIATELY);

   while(1){
      event_t e = event_q_get_next_event();

      switch(e){

         case MOVEMENT_COMPLETE:
            handle_range_sensors_react_accumulate();
            return;

         case NEW_PROXIMITY_READINGS:
            handle_range_sensors_accumulate();
            break;
         default:
            break;
      }
   }
}

static void main_loop(){

   while(1){

      event_t e = event_q_get_next_event();

      switch(e){

         case LINE_DETECTED:
            handle_line_sensors(true);
            break;
         case MOVEMENT_COMPLETE:
            if(movman_current_move_completed(false)){
               // TODO - search pattern
               // statefullness?
               movman_schedule_move(MOVE_FORWARD, TO_SEEK, IMMEDIATELY);
            }
         case NEW_PROXIMITY_READINGS:
            handle_range_sensors(false);
            break;
         default:
            break;

      }
   }
}



void tracking_ring_robot_main()
{
   iodefs_init();
   leds_init();
   motors_init();
   movman_init();
   range_sensors_init();
   event_q_init();

   sei();

   range_sensors_start();

   // In this loop, we wait for precisely 3 seconds, accumulating
   // sensor readings on the two sides the whole time. Once the 3 seconds
   // ends, we determine which side the opponent is on, and attack
   startup_loop();

   //
   line_sensors_init();
   main_loop();

}
