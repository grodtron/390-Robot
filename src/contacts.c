#include <avr/interrupt.h>
#include <avr/io.h>

#include <stdbool.h>

#include "../include/contacts.h"
#include "../include/event_queue.h"
#include "../include/leds.h"

#define PFR PD4
#define PR  PD0
#define PFL PD1

void contacts_init(){
   // Set INT0 aka PD2 as an input (Page 52, ATmega8 Datasheet)
   DDRD  &= ~( (1 << PD0)|(1 << PD1)|(1 << PD2)|(1 << PD4) );

   // Enable INT0 (interrupt 0) (Page 67, ATmega8 Datasheet)
   GICR  |= (1 << INT0);

   // "The rising edge of INT0 generates an interrupt request" (page 67, ATmega8 datasheet)
   MCUCR |=  (1 << ISC01) | (1 << ISC00);

   // Now we set up timer 0, which we will use for debouncing purposes
   // Based on observations in the lab, the switch tends to bounce for
   // about 2ms before becoming stable. We also want to react as fast as
   // possible.
   //
   // Since the timer will generate an interrupt every time it overflows,
   // and it overflows once every 256 clocks, it already acts as a frequency
   // divider. The period will be 0.256ms ~= 0.25 ms
   //
   // With this, if we wait until 8 stable readings in a row, we will only
   // fire an event after a little over 2ms of stability, which should be perfect

   // The clock will only be started when we need it, and will be stopped
   // after the debounce is done.
   TIMSK |= (1 << TOIE0);
}

// No clock source == timer off (datasheet page 72)
#define TIMER0_OFF() do{ TCCR0 &= ~((1 << CS02)|(1 << CS01)|(1 << CS00)); }while(0)
// No prescaler (timer on) (datasheet 72)
#define TIMER0_ON()  do{ TIMER0_OFF(); TCCR0 |= (1 << CS00); } while(0)

static contact_position_t handle_contact(){

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

   return pos;
}

static bool debouncing;

ISR(TIMER0_OVF_vect){

   if(debouncing){
      // we're debouncing an interrupt request

      static uint8_t reads = 0xAA; // 10101010

      reads <<= 1;
      reads  |= (PIND >> PD2) & 1;

      if(reads == 0xFF){
         // Here it means we've read eight 1s in a row.
         // With our period of 0.25ms, this represents
         // 2ms of a stable logic high, which is good enough for me!

         // We'll determine the right event type and add it to the queue
         handle_contact();

         // Then reset the debouncing logic
         reads = 0xAA;
         debouncing = false;

      }else if(reads == 0x00){
         // Likewise this represents a stable logic low,
         // which we don't care about, so we just ignore it.

         // Reset the debouncing logic
         reads = 0xAA;
         // Shut off the timer, there's nothing to report about now
         TIMER0_OFF();
      }
   }else{

      static uint8_t count = 0;
      ++count;

      // this works out to an update about one every 65ms, which is perfectly fine
      // for our needs
      if(count == 255){
         contact_position_t pos = handle_contact();

         if(pos == CONTACT_NONE){
            // We can stop reporting in this case
            TIMER0_OFF();
         }
      }
   }
}

ISR(INT0_vect){
   // All we do here is start a debouncing timer
   debouncing = true;
   TIMER0_ON();

}

contact_position_t contacts_get_position(){
   contact_position_t pos = CONTACT_NONE;

   if(PIND & (1 << PFL)){
      pos |= CONTACT_FRONT_LEFT;
   }
   if(PIND & (1 << PFR)){
      pos |= CONTACT_FRONT_RIGHT;
   }
   if(PIND & (1 << PR)){
      pos |= CONTACT_REAR;
   }

   return pos;
}
