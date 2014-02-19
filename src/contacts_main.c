#include <avr/interrupt.h>
#include <avr/io.h>

#include "../include/event_queue.h"
#include "../include/contacts.h"

int main()
{

   contacts_init();
   event_q_init();

   sei();

   DDRA |= (1 << PA0);

   while(1){

      // Testing in the lab showed this runs every ~95us

      event_t e = event_q_get_next_event();

      switch(e){

         case CONTACT_DETECTED_BOTH:
         case CONTACT_DETECTED_FRONT:
         case CONTACT_DETECTED_REAR:
         case CONTACT_LOST:
            PORTA ^= (1 << PA0);
            break;
         default:
            break;

      }

   }

   return 0;
}
