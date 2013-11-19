#include <avr/interrupt.h>
// 1MHz clock
#define F_CPU 1000000UL
#include <util/delay.h>

#include "../include/motors.h"
#include "../include/event_queue.h"

uint8_t do_next_move(uint8_t next){
   switch(next){

      case 0:
         set_speed(255, FWD, 1000);
         break;
      case 1:
         rotate(255, LEFT, 500);
         break;
      case 2:
         set_speed(255, REV, 1000);
         break;
      case 3:
         rotate(255, RIGHT, 500);
         break;
      case 4:
      case 6:
         turn_in_arc(255, FWD, RIGHT, 150, 750);
         break;
      case 5:
      case 7:
         turn_in_arc(255, FWD, LEFT, 150, 750);
         break;
      case 8:
      case 10:
         turn_in_arc(255, REV, RIGHT, 150, 750);
         break;
      case 9:
      case 11:
         turn_in_arc(255, REV, LEFT, 150, 750);
         break;
      case 12:
         set_speed(128, FWD, 500);
         break;
      case 13:
         rotate(255, RIGHT, 500);
         break;
      case 14:
         rotate(255, LEFT, 500);
         break;
      case 15:
      default:
         hard_stop();
         set_speed(0, FWD, 1500);
         break;
   }

   return (++next)%16;
}

int main()
{

   sei();
   init_motors();
   
   uint8_t next_movement = 0;

   next_movement = do_next_move(next_movement);

   while(1){

      event_t e = get_next_event();

      switch(e){

         case MOVEMENT_COMPLETE:
            if(! movement_in_progress() ){
               next_movement = do_next_move(next_movement);
            }
            break;
         default:
            break;


      }

   }

   return 0;
}
