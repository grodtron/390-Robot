#include <avr/interrupt.h>
#include <avr/io.h>
#define F_CPU 1000000UL
#include <util/delay.h>

#include "../include/event_queue.h"
#include "../include/line_sensors.h"
#include "../include/iodefs.h"
#include "../include/movement_manager.h"
#include "../include/motors.h"


static void indicate_line(line_position_t pos){

   if(pos & LINE_FRONT_LEFT){
      PORTD &= ~(1<<PD6);
   }else{
      PORTD |=  (1<<PD6);
   }
   if(pos & LINE_FRONT_RIGHT){
      PORTA &= ~(1<<PA6);
   }else{
      PORTA |=  (1<<PA6);
   }
   if(pos & LINE_REAR_LEFT){
      PORTD &= ~(1<<PD7);
   }else{
      PORTD |=  (1<<PD7);
   }
   if(pos & LINE_REAR_RIGHT){
      PORTA &= ~(1<<PA7);
   }else{
      PORTA |=  (1<<PA7);
   }
}

static void avoid_line(line_position_t pos){
   if(pos & LINE_FRONT){
      movman_schedule_move(BACKUP_THEN_TURN_90_CW, TO_AVOID_EDGE, IMMEDIATELY_WITH_OVERWRITE);
   }else if(pos & LINE_REAR){
      movman_schedule_move(GO_FORWARD_BRIEFLY, TO_AVOID_EDGE, IMMEDIATELY_WITH_OVERWRITE);
      movman_schedule_move(SMALL_TURN_LEFT, TO_AVOID_EDGE, NEXT_AVAILABLE_TIME);
   }
}

static void handle_line(){
   line_position_t pos = line_sensors_get_position();
   avoid_line(pos);
   indicate_line(pos);
}

void ring_robot_main()
{

   DDRA |= (1<<PA6)|(1<<PA7);
   DDRD |= (1<<PD6)|(1<<PD7);

   PORTA |= (1<<PA6)|(1<<PA7);
   PORTD |= (1<<PD6)|(1<<PD7);

   {
      int i;
      for(i = 0; i < (2*10); ++i){
         PORTA ^= (1<<PA7);
         _delay_ms(100);
      }
   }

   iodefs_init();
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
            handle_line();
            break;
         case MOVEMENT_COMPLETE:
            if(movman_current_move_completed()){
               movman_schedule_move(MOVE_FORWARD, TO_SEEK, IMMEDIATELY);
            }
         default:
            break;

      }

   }
}
