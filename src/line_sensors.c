#include <avr/interrupt.h>
#include <avr/io.h>

#include "../include/contacts.h"
#include "../include/event_queue.h"
#include "../include/leds.h"

// TODO - just use one interrupt for each?
void contacts_init(){

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

   // CTC mode, no PWM (Atmega644 datasheet, page 99)
   TCCR0A &= ~(1 << WGM00);
   TCCR0A |=  (1 << WGM01);
   TCCR0B &= ~(1 << WGM02);

   OCR0A = 127;

   // The clock will only be started when we need it, and will be stopped
   // after the debounce is done.
   TIMSK0 |= (1 << OCIE0A); // enable interrupt (page 102, atmega644 datasheet)

   // Turn on clock, 256 us period
   TCCR0B |= (1 << CS00);

   DDRA |= (1 << PA0);
}

ISR(TIMER0_COMPA_vect){

   static uint8_t mode = 0;

   if(mode){

      OCR0A = 150;

      // Put the pin into a high-z ground, so that the cap
      // slowly discharged through the photo transistor

      // Set as input
      DDRA &= ~(1 << PA1);
      // Turn off internal pull up.
      PORTA &= ~(1 << PA1);

   }else{

      OCR0A = 64;

      // read
      uint8_t val = (PINA >> PA1) & 1;

      // Set as output
      DDRA |= (1 << PA1);

      // write high
      PORTA |= (1 << PA1);

      // indicate on LED
      if(val){
         PORTA &= ~(1 << PA0);  
      }else{
         PORTA |=  (1 << PA0);  
      }

   }

   mode = !mode;

}

