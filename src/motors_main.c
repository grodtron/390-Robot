#include <avr/interrupt.h>

#define F_CPU 1000000UL
#include <util/delay.h>

#include "../include/motors.h"
#include "../include/movement_manager.h"
#include "../include/event_queue.h"
#include "../include/iodefs.h"

void motors_main()
{

   DDRA |= (1<<PA6)|(1<<PA7);
   DDRD |= (1<<PD6)|(1<<PD7);

   PORTA |= (1<<PA6)|(1<<PA7);
   PORTD |= (1<<PD6)|(1<<PD7);

   PORTA ^= (1<<PA7);
   _delay_ms(100);
   PORTA ^= (1<<PA7);
   _delay_ms(100);
   PORTA ^= (1<<PA7);
   _delay_ms(100);
   PORTA ^= (1<<PA7);
   _delay_ms(100);
   PORTA ^= (1<<PA7);
   _delay_ms(100);

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

               PORTA ^= (1<<PA7);
               switch(i){
                  case 0:
                     PORTD &= ~(1<<PD6);
                     PORTA |=  (1<<PA6);
                     PORTD |=  (1<<PD7);
                     movman_schedule_move(MOVE_FORWARD, TO_SEEK, IMMEDIATELY);
                     break;
                  case 1:
                     PORTA &= ~(1<<PA6);
                     PORTD |=  (1<<PD6);
                     PORTD |=  (1<<PD7);
                     movman_schedule_move(ROTATE_LEFT, TO_SEEK, IMMEDIATELY);
                     break;
                  case 2:
                     PORTD &= ~(1<<PD7);
                     PORTD |=  (1<<PD6);
                     PORTA |=  (1<<PA6);
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
