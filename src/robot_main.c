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

void handle_new_prox_readings();
void handle_movement_complete(){

   movman_current_move_completed();
   handle_new_prox_readings(); // TODO - wanring, bad hack alert!!
   movman_schedule_move(SEARCH_PATTERN, TO_SEARCH, NEXT_AVAILABLE_TIME);



}

void handle_line_detected(){
   switch(adc_where_is_line()){
      case LINE_LEFT:
      case LINE_RIGHT:
      case LINE_BOTH:
         if(movman_schedule_move(BACKUP_THEN_TURN_30_CCW, TO_AVOID_EDGE, IMMEDIATELY)){
         }else{
         }
      case LINE_NONE:
      default:
         break;
   }
}

void handle_back_contact(){
   movman_schedule_move(LONG_ROTATE_LEFT, TO_DEFEND, IMMEDIATELY_WITH_OVERWRITE);
}

void handle_front_contact(){
   uint8_t left;
   uint8_t right;


   adc_get_prox_readings(&left, &right);

   // Then we're contacting our opponent
   if(left > 70 || right > 70){
      switch(contacts_get_position()){
         case CONTACT_FRONT_LEFT:
            movman_schedule_move(SMALL_TURN_LEFT, TO_ATTACK, IMMEDIATELY);
            break;
         case CONTACT_FRONT_RIGHT:
            movman_schedule_move(SMALL_TURN_RIGHT, TO_ATTACK, IMMEDIATELY);
            break;
         case CONTACT_FRONT_LEFT | CONTACT_FRONT_RIGHT:
            movman_schedule_move(GO_FORWARD_BRIEFLY, TO_ATTACK, IMMEDIATELY);
            break;
         default:
            break;
      }
   }else{
      movman_schedule_move(BACKUP_THEN_TURN_90_CW, TO_AVOID_FIREPIT, IMMEDIATELY);
      // we're contacting the firepit
   }
}

void handle_new_prox_readings(){

   #define SEEK_MODE 1
   #define SEARCH_MODE 0

   #define TURN_ON_MARGIN 12
   #define TURN_OFF_MARGIN 8


   // hysterisis! Lower the noise margin if we're already in seek mode
   #define margin (mode == SEEK_MODE ? TURN_OFF_MARGIN : TURN_ON_MARGIN)
   static uint8_t mode = SEARCH_MODE;


   #define SEEK_NONE 0
   #define SEEK_LEFT 1
   #define SEEK_RIGHT 2
   static uint8_t seek_direction = SEEK_NONE;

   #define SEEK_THRESH 5
   static uint8_t seek_count = 0;

   #define yellow (seek_count > SEEK_THRESH)


   uint8_t left;
   uint8_t right;

   adc_get_prox_readings(&left, &right);

   if(left > 35 || right > 35){
      if(left > right + margin){
            led_set_rgy(1,0,yellow);
            movman_schedule_move(SMALL_TURN_LEFT, TO_SEEK, IMMEDIATELY_WITH_OVERWRITE);
            mode = SEEK_MODE;
            if(seek_direction != SEEK_LEFT){
               seek_direction = SEEK_LEFT;
               seek_count = 0;
            }
            if (seek_count < 255){
               ++seek_count;
            }
      }else if(right > left + margin){
            led_set_rgy(0,1,yellow);
            movman_schedule_move(SMALL_TURN_RIGHT, TO_SEEK, IMMEDIATELY_WITH_OVERWRITE);
            mode = SEEK_MODE;
            if(seek_direction != SEEK_RIGHT){
               seek_direction = SEEK_RIGHT;
               seek_count = 0;
            }
            if (seek_count < 255){
               ++seek_count;
            }
      }else{
            led_set_rgy(1,1,yellow);
            movman_schedule_move(SMALL_MOVE_FORWARD, TO_SEEK, IMMEDIATELY_WITH_OVERWRITE);
            mode = SEEK_MODE;
            seek_direction = SEEK_NONE;
      }
   }else{
         led_set_rgy(0,0,yellow);
         // ~ 73ms * 8 ~= 1 second
         if(seek_count > SEEK_THRESH){
            switch(seek_direction){
               case SEEK_RIGHT:
                  movman_schedule_move(ROTATE_RIGHT, TO_SEEK, IMMEDIATELY_WITH_OVERWRITE);
                  break;
               case SEEK_LEFT:
                  movman_schedule_move(ROTATE_LEFT, TO_SEEK, IMMEDIATELY_WITH_OVERWRITE);
                  break;
               default:
                  break;
            }
         }

         mode = SEARCH_MODE;
         seek_direction = SEEK_NONE;

   }

   #undef SEARCH_MODE
   #undef SEEK_MODE
   #undef SEEK_NONE
   #undef SEEK_LEFT
   #undef SEEK_RIGHT
   #undef margin
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

   movman_schedule_move(
      WAIT_5_SECONDS_THEN_FULL_FORWARD_FOR_A_LONG_TIME,
      TO_MEET_STARTUP_REQUIREMENT,
      IMMEDIATELY);

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
         case CONTACT_DETECTED_BACK:
            handle_back_contact();
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
