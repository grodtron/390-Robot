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
   movman_schedule_move(GO_FORWARD_BRIEFLY, TO_SEARCH, NEXT_AVAILABLE_TIME);


}

void handle_line_detected(){
   switch(adc_where_is_line()){
      case LINE_LEFT:
      case LINE_RIGHT:
      case LINE_BOTH:
         if(movman_schedule_move(BACKUP_THEN_TURN_90_CCW, TO_AVOID_EDGE, IMMEDIATELY_ELSE_IGNORE)){
         }else{
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

void handle_new_prox_readings(){

   uint8_t left;
   uint8_t right;

   adc_get_prox_readings(&left, &right);

   if((left >> 5) == (right >> 5)){
      // It's too close to call...
      led_set_rgy(1,1,0);
   }else{
      if(left > right){
         led_set_rgy(1,0,0);
         movman_schedule_move(SMALL_TURN_LEFT, TO_SEEK, IMMEDIATELY_ELSE_IGNORE);
      }else{
         led_set_rgy(0,1,0);
         movman_schedule_move(SMALL_TURN_RIGHT, TO_SEEK, IMMEDIATELY_ELSE_IGNORE);
      }
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

   movman_schedule_move(GO_FORWARD_BRIEFLY, TO_SEARCH, NEXT_AVAILABLE_TIME);

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
         case NEW_PROXIMITY_READINGS:
            handle_new_prox_readings();
            break;
         default:
            break;


      }

   }

   return 0;
}
