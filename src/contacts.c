#include <avr/interrupt.h>
#include <avr/io.h>

#include "../include/contacts.h"
#include "../include/event_queue.h"
#include "../include/leds.h"

#define PF  PD2
#define PR  PD3

// TODO - just use one interrupt for each?
void contacts_init(){
   // Set INT0 aka PD2 as an input (Page 52, ATmega8 Datasheet)
   DDRD  &= ~((1 << PD2)|(1 << PD3));

   // Enable INT0 (interrupt 0) (Page 61, ATmega644 Datasheet)
   EIMSK  |= (1 << INT0)
          |  (1 << INT1);

   // Any edge of INT0 or INT1 generates an interrupt request (page 61, ATmega644 datasheet)
   EICRA  |= (0 << ISC01) | (1 << ISC00)
          |  (0 << ISC11) | (1 << ISC10);

   // // TODO - reverify with our new switches
   //
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

   // Normal mode, no PWM (Atmega644 datasheet, page 99)
   TCCR0A &= ~((1 << WGM01)|(1 << WGM00));
   TCCR0B &= ~(1 << WGM02);

   // The clock will only be started when we need it, and will be stopped
   // after the debounce is done.
   TIMSK0 |= (1 << TOIE0); // enable interrupt (page 101, atmega644 datasheet)

}

// No clock source == timer off (datasheet page 100)
#define TIMER0_OFF() do{ TCCR0B &= ~((1 << CS02)|(1 << CS01)|(1 << CS00)); }while(0)
// No prescaler (timer on) (datasheet 100)
#define TIMER0_ON()  do{ TCCR0B |= (1 << CS00); } while(0)

static contact_position_t handle_contact(){

   contact_position_t pos = contacts_get_position();

   event_t e = NULL_EVENT;

   switch(pos){
      case CONTACT_NONE:
         e = CONTACT_LOST;
         break;
      case CONTACT_FRONT:
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

ISR(TIMER0_OVF_vect){

   static uint8_t reads_0 = 0xAA; // 10101010
   static uint8_t reads_1 = 0xAA; // 10101010

   reads_0 <<= 1;
   reads_0  |= (PIND >> PD2) & 1;

   reads_1 <<= 1;
   reads_1  |= (PIND >> PD3) & 1;

   // TODO - debugging, remove
   PORTA ^= (1 << PA1);

   if(
      (reads_0 == 0xFF || reads_0 == 0x00)
      &&
      (reads_1 == 0xFF || reads_1 == 0x00)
   ){
      // Here it means that both contacts have been stable for a full
      // debouncing cycle, and we should be able to safely report their
      // values to the main loop.

      // We'll determine the right event type and add it to the queue
      handle_contact();

      // Then reset the debouncing logic
      TIMER0_OFF();

      reads_0 = 0xAA;
      reads_1 = 0xAA;
   }
}

// All the ISR does is start the timer
#define EXTERNAL_INT_ISR(n) \
ISR(INT ## n ## _vect){ \
   TIMER0_ON(); \
}

EXTERNAL_INT_ISR(0)
EXTERNAL_INT_ISR(1)

#undef EXTERNAL_INT_ISR

contact_position_t contacts_get_position(){
   contact_position_t pos = CONTACT_NONE;

   if(PIND & (1 << PF)){
      pos |= CONTACT_FRONT;
   }
   if(PIND & (1 << PR)){
      pos |= CONTACT_REAR;
   }

   return pos;
}
