#include <avr/interrupt.h>
#include <avr/io.h>

#include "../include/contacts.h"
#include "../include/event_queue.h"

#define PFL PD0
#define PFR PD1
#define PR  PD3

void contacts_init(){
   // Set INT0 aka PD2 as an input (Page 52, ATmega8 Datasheet)
   DDRD  &= ~( (1 << PD0)|(1 << PD1)|(1 << PD2)|(1 << PD4) );

   // Enable INT0 (interrupt 0) (Page 67, ATmega8 Datasheet)
   GICR  |= (1 << INT0);

   // "The rising edge of INT0 generates an interrupt request" (page 67, ATmega8 datasheet)
   MCUCR |=  (1 << ISC01) | (1 << ISC00);

}

ISR(INT0_vect){
   contact_position_t pos = contacts_get_position();

   event_t e = NULL_EVENT;

   switch(pos){
      case CONTACT_NONE:
         break;
      case CONTACT_FRONT_LEFT:
      case CONTACT_FRONT_RIGHT:
      case CONTACT_FRONT_LEFT | CONTACT_FRONT_RIGHT:
         e = CONTACT_DETECTED_FRONT;
         break;
      case CONTACT_REAR:
         e = CONTACT_DETECTED_REAR;
         break;
      default:
         e = CONTACT_DETECTED_BOTH;
         break;
   }

   event_q_add_event(e);

}

contact_position_t contacts_get_position(){
   contact_position_t pos = CONTACT_NONE;

   if(PORTD & (1 << PFL)){
      pos |= CONTACT_FRONT_LEFT;
   }
   if(PORTD & (1 << PFR)){
      pos |= CONTACT_FRONT_RIGHT;
   }
   if(PORTD & (1 << PR)){
      pos |= CONTACT_REAR;
   }

   return pos;
}
