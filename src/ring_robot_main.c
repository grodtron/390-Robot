#include <avr/interrupt.h>
#include <avr/io.h>
#define F_CPU 1000000UL
#include <util/delay.h>

#include "../include/event_queue.h"
#include "../include/line_sensors.h"
#include "../include/iodefs.h"

#include "../include/leds.h"

#include "../include/movement_manager.h"

#include "../include/handlers/line_sensors.h"



void ring_robot_main()
{
   iodefs_init();
   leds_init();

   {
      int i;
      for(i = 0; i < (2*10); ++i){
         led_toggle_fl();
         _delay_ms(100);
      }
   }

   line_sensors_init();
   motors_init();
   movman_init();
   event_q_init();

   sei();

   movman_schedule_move(MOVE_FORWARD, TO_SEEK, IMMEDIATELY);

   while(1){

      event_t e = event_q_get_next_event();

      switch(e){

         case LINE_DETECTED:
            handle_line_sensors(true);
            break;
         case MOVEMENT_COMPLETE:
            if(movman_current_move_completed(false)){
               movman_schedule_move(MOVE_FORWARD, TO_SEEK, IMMEDIATELY);
            }
         default:
            break;

      }

   }
}
