#include <avr/interrupt.h>
// 1MHz clock
#define F_CPU 1000000UL
#include <util/delay.h>

#include "../include/adc.h"
#include "../include/motors.h"
#include "../include/movement_manager.h"
#include "../include/leds.h"
#include "../include/event_queue.h"
#include "../include/contacts.h"

void handle_movement_complete(){

   movman_current_move_completed();

   movman_schedule_move(FORWARD_THEN_WIDE_TURN_RIGHT, TO_SEARCH, NEXT_AVAILABLE_TIME);

}

void handle_line_detected(){
   switch(adc_where_is_line()){
      case LINE_LEFT:
      case LINE_RIGHT:
      case LINE_BOTH:
         if(movman_schedule_move(BACKUP_THEN_TURN_90_CCW, TO_AVOID_EDGE, IMMEDIATELY_ELSE_IGNORE)){
            led_set_red(0);
            led_set_green(1);
            led_toggle_yellow();
         }else{
            led_set_red(1);
            led_set_green(0);
            led_toggle_yellow();
         }
      case LINE_NONE:
      default:
         break;
   }
}

void handle_front_contact(){
   switch(contacts_get_position()){
      case CONTACT_FRONT_LEFT:
         movman_schedule_motor_instruction(TO_AVOID_FIREPIT,
            &motors_turn_in_arc, 255, FWD, LEFT, 170, 750, IMMEDIATELY_ELSE_IGNORE + 1);
         break;
      case CONTACT_FRONT_RIGHT:
         movman_schedule_motor_instruction(TO_AVOID_FIREPIT,
            &motors_turn_in_arc, 255, FWD, RIGHT, 170, 750, IMMEDIATELY_ELSE_IGNORE + 1);
         break;
      case CONTACT_FRONT_LEFT | CONTACT_FRONT_RIGHT:
         movman_schedule_motor_instruction(TO_AVOID_FIREPIT,
            &motors_set_speed, 255, FWD, 0, 0, 750, IMMEDIATELY_ELSE_IGNORE + 1);
         break;
      default:
         break;
   }
}

int main()
{

   motors_init();
   movman_init();
   contacts_init();
   event_q_init();
   adc_init();
   leds_init();

   sei();

   adc_start();

   movman_schedule_move(SPIRAL_OUTWARDS, TO_SEARCH, NEXT_AVAILABLE_TIME);

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
