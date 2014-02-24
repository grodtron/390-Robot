#include <avr/interrupt.h>
#include <avr/io.h>
#define F_CPU 1000000UL
#include <util/delay.h>

#include "../include/event_queue.h"
#include "../include/line_sensors.h"
#include "../include/iodefs.h"
#include "../include/leds.h"

void handle_line(){
   line_position_t pos = line_sensors_get_position();

   led_set_fl(pos & LINE_FRONT_LEFT);
   led_set_fr(pos & LINE_FRONT_RIGHT);
   led_set_bl(pos & LINE_BACK_LEFT);
   led_set_br(pos & LINE_BACK_RIGHT);
}

void line_sensors_main()
{

   iodefs_init();

   leds_init();
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
