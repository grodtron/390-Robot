#include <avr/interrupt.h>
// 1MHz clock
#define F_CPU 1000000UL
#include <util/delay.h>

#include "../include/adc.h"
#include "../include/motors.h"
#include "../include/leds.h"
#include "../include/event_queue.h"
#include "../include/contacts.h"

void handle_movement_complete(){

   static uint8_t dir = 1;
   dir = !dir;

   if(dir){
         motors_set_speed(255, FWD, 1000);
   }else{
         motors_set_speed(0,   FWD, 500);
   }
}

void handle_line_detected(){
   switch(adc_where_is_line()){
      case LINE_LEFT:
         // Arc backwards to the left with a 1 foot radius for half a second
         motors_turn_in_arc(255, REV, RIGHT, 300, 1000);
         break;

      // For now for testing treat these two cases the same
      case LINE_RIGHT:
      case LINE_BOTH:
         // Arc backwards to the right with a 1 foot radius for half a second
         motors_turn_in_arc(255, REV, LEFT, 300, 1000);
         break;

      // False alarm I guess, do nothing (TODO - is this the correct behaviour?)
      case LINE_NONE:
      default:
         break;
   }
}

void handle_front_contact(){
   switch(contacts_get_position()){
      case CONTACT_FRONT_LEFT:
         motors_turn_in_arc(255, FWD, LEFT, 170, 750);
         break;
      case CONTACT_FRONT_RIGHT:
         motors_turn_in_arc(255, FWD, RIGHT, 170, 750);
         break;
      case CONTACT_FRONT_LEFT | CONTACT_FRONT_RIGHT:
         motors_set_speed(255, FWD, 750);
         break;
      default:
         break;
   }
}

int main()
{

   motors_init();
   contacts_init();
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

         case LINE_DETECTED:
            handle_line_detected();
            break;
         case CONTACT_DETECTED_BOTH:
         case CONTACT_DETECTED_FRONT:
            handle_front_contact();
            break;
         case CONTACT_DETECTED_REAR:
            break;
         case MOVEMENT_COMPLETE:
            handle_movement_complete();
            break;
         default:
            break;


      }

   }

   return 0;
}
