#include <avr/interrupt.h>

#include "../include/motors.h"
#include "../include/movement_manager.h"
#include "../include/event_queue.h"
#include "../include/iodefs.h"

void motors_main()
{

   iodefs_init();

   motors_init();
   movman_init();
   event_q_init();

   sei();

   movman_schedule_move(MOVE_FORWARD, TO_SEEK, IMMEDIATELY);

   while(1){

      // Testing in the lab showed this runs every ~95us

      event_t e = event_q_get_next_event();

      switch(e){

         case LINE_DETECTED:
            break;
         case CONTACT_DETECTED_BOTH:
         case CONTACT_DETECTED_FRONT:
            break;
         case CONTACT_DETECTED_REAR:
            break;
         case MOVEMENT_COMPLETE:
            break;
         case NEW_PROXIMITY_READINGS:
            break;
         default:
            break;


      }

   }
}
