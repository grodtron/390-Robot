#include <avr/interrupt.h>
// 1MHz clock
#define F_CPU 1000000UL
#include <util/delay.h>

#include "../include/adc.h"
#include "../include/motors.h"
#include "../include/leds.h"
#include "../include/event_queue.h"

void handle_movement_complete(){
   static uint8_t next = 0;

   if( motors_movement_in_progress() ){
      // False alarm, there actually is a move in progress already
      return;
   }

   switch(next){

      case 0:
         motors_set_speed(255, FWD, 1000);
         break;
      case 1:
         motors_rotate(255, LEFT, 500);
         break;
      case 2:
         motors_set_speed(255, REV, 1000);
         break;
      case 3:
         motors_rotate(255, RIGHT, 500);
         break;
      case 4:
      case 6:
         motors_turn_in_arc(255, FWD, RIGHT, 150, 750);
         break;
      case 5:
      case 7:
         motors_turn_in_arc(255, FWD, LEFT, 150, 750);
         break;
      case 8:
      case 10:
         motors_turn_in_arc(255, REV, RIGHT, 150, 750);
         break;
      case 9:
      case 11:
         motors_turn_in_arc(255, REV, LEFT, 150, 750);
         break;
      case 12:
         motors_set_speed(128, FWD, 500);
         break;
      case 13:
         motors_rotate(255, RIGHT, 500);
         break;
      case 14:
         motors_rotate(255, LEFT, 500);
         break;
      case 15:
      default:
         motors_hard_stop();
         motors_set_speed(0, FWD, 1500);
         break;
   }

   next = (next + 1) % 16;
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
