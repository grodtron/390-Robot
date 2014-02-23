#include <avr/interrupt.h>
#include <avr/io.h>

#include "../include/line_sensors.h"
#include "../include/event_queue.h"
#include "../include/leds.h"
#include "../include/iodefs.h"

static uint8_t val;

void line_sensors_init(){

   // CTC mode, no PWM (Atmega644 datasheet, page 99)
   TCCR0A &= ~(1 << WGM00);
   TCCR0A |=  (1 << WGM01);
   TCCR0B &= ~(1 << WGM02);

   TIMSK0 |= (1 << OCIE0A); // enable interrupt (page 102, atmega644 datasheet)

   // Turn on clock, 1MHz fcpu, with /8 prescaler, counting up to 63 gives a 512 us period
   TCCR0B |= (1 << CS01);
   OCR0A = 49;
}

// Because we want max speed in this interrupt, and because we are only checking
// for the presence of the line, not the location, we don't use io_defs here
ISR(TIMER0_COMPA_vect){

   static uint8_t mode = 0;

   if(mode){

      // Put the pin into a high-z ground, so that the cap
      // slowly discharged through the photo transistor

      // Set as input
      IO_LINESENS_DDR  &= ~IO_LINESENS_MASK;
      // Turn off internal pull up.
      IO_LINESENS_PORT &= ~IO_LINESENS_MASK;

   }else{

      // read
      uint8_t new_val = (~IO_LINESENS_PIN) & IO_LINESENS_MASK;

      // Set as output
      IO_LINESENS_DDR  |= IO_LINESENS_MASK;

      // write high
      IO_LINESENS_PORT |= IO_LINESENS_MASK;

      // If the status has changed, keep track of that.
      if(val != new_val){
         val = new_val;
         // TODO - Is it useful to notify for every change in line status?
         // should we only notify when there is a change and there is a line
         // detected?
         event_q_add_event(LINE_DETECTED);
      }
   }

   mode = !mode;
}

line_position_t line_sensors_get_position(){

   line_position_t ret = LINE_NONE;

   if(val & io.linesens_fl){
      ret |= LINE_FRONT_LEFT;
   }
   if(val & io.linesens_fr){
      ret |= LINE_FRONT_RIGHT;
   }
   if(val & io.linesens_rl){
      ret |= LINE_REAR_LEFT;
   }
   if(val & io.linesens_rr){
      ret |= LINE_REAR_RIGHT;
   }

   return ret;
}

