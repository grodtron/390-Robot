#include <avr/interrupt.h>

#define F_CPU 1000000UL
#include <util/delay.h>

#include "../include/motors.h"
#include "../include/movement_manager.h"
#include "../include/event_queue.h"
#include "../include/iodefs.h"

#include "../include/switch_direction.h"

void blink_one(){
   int i;
   for(i = 0; i < 2*5; ++i){
      PORTA ^= (1<<PA7);
      _delay_ms(100);
   }
}
void blink_all(){
   int i;
   for(i = 0; i < 2*10; ++i){
      PORTA ^= (1<<PA7)|(1<<PA6);
      PORTD ^= (1<<PD7)|(1<<PD6);
      _delay_ms(100);
   }
}

void motors_main()
{

   DDRA |= (1<<PA6)|(1<<PA7);
   DDRD |= (1<<PD6)|(1<<PD7);

   PORTA |= (1<<PA6)|(1<<PA7);
   PORTD |= (1<<PD6)|(1<<PD7);

   blink_all();

   iodefs_init();

   motors_init();
   movman_init();
   event_q_init();

   sei();

   movman_schedule_move(MOVE_FORWARD, TO_SEEK, IMMEDIATELY);

   uint8_t i = 0;


   while(1){

      // Testing in the lab showed this runs every ~95us

      event_t e = event_q_get_next_event();

      switch(e){

         case MOVEMENT_COMPLETE:

            if(movman_current_move_completed()){

               i = (i+1)%3;

               switch(i){
                  case 0:

                     switch_direction();

                     movman_schedule_move(MOVE_FORWARD, TO_SEEK, IMMEDIATELY);
                     blink_one();
                     break;
                  case 1:
                     movman_schedule_move(ROTATE_LEFT, TO_SEEK, IMMEDIATELY);
                     break;
                  case 2:
                     movman_schedule_move(ROTATE_RIGHT, TO_SEEK, IMMEDIATELY);
                     break;
               }
            }

            break;

         default:
            break;

      }

   }
}
