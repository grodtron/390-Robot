#include <avr/interrupt.h>
// 1MHz clock
#define F_CPU 1000000UL
#include <util/delay.h>

#include "../include/adc.h"
#include "../include/motors.h"
#include "../include/leds.h"
#include "../include/event_queue.h"

void handle_movement_complete(){

   static uint8_t dir = 1;
   dir = !dir;

   if(dir){
         motors_set_speed(255, FWD, 1000);
   }else{
         motors_set_speed(255, REV, 1000);
   }
}

void handle_line_detected(){
   switch(adc_where_is_line()){
      case LINE_LEFT:
         // Arc backwards to the left with a 1 foot radius for half a second
         motors_turn_in_arc(255, REV, LEFT, 300, 500);
         break;

      // For now for testing treat these two cases the same
      case LINE_RIGHT:
      case LINE_BOTH:
         // Arc backwards to the right with a 1 foot radius for half a second
         motors_turn_in_arc(255, REV, RIGHT, 300, 500);
         break;

      // False alarm I guess, do nothing (TODO - is this the correct behaviour?)
      case LINE_NONE:
      default:
         break;
   }
}

int main()
{

   motors_init();
   event_q_init();
   adc_init();
   leds_init();

   sei();

   adc_start();

   motors_set_speed(255, FWD, 1000);

   while(1){

      // Testing in the lab showed this runs every ~95us

      event_t e = event_q_get_next_event();

      switch(e){

         case MOVEMENT_COMPLETE:
            led_set_red(0);
            led_toggle_green();
            handle_movement_complete();
            break;
         case LINE_DETECTED:
            led_set_rgy(1, 0, 0);
            handle_line_detected();
            break;
         case NEW_PROXIMITY_READINGS:
            // do nothing
            break;
         default:
            break;


      }

   }

   return 0;
}
