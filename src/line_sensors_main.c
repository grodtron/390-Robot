#include <avr/interrupt.h>
#include <avr/io.h>
#define F_CPU 1000000UL
#include <util/delay.h>

#include "../include/event_queue.h"
#include "../include/line_sensors.h"
#include "../include/iodefs.h"

void handle_line(){
   line_position_t pos = line_sensors_get_position();

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

void line_sensors_main()
{

   DDRA |= (1<<PA6)|(1<<PA7);
   DDRD |= (1<<PD6)|(1<<PD7);

   PORTA |= (1<<PA6)|(1<<PA7);
   PORTD |= (1<<PD6)|(1<<PD7);

   iodefs_init();
   line_sensors_init();
   event_q_init();


   sei();


   while(1){

      // Testing in the lab showed this runs every ~95us

      event_t e = event_q_get_next_event();

      switch(e){

         case LINE_DETECTED:
            handle_line();
            break;
         default:
            break;

      }

   }
}
