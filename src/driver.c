#include <avr/interrupt.h>
// 1MHz clock
#define F_CPU 1000000UL
#include <util/delay.h>

#include "../include/motors.h"

int main()
{

   sei();
   init_motors();
   
   while(1){
      set_speed(255, FWD);

      _delay_ms(5 * 1000);

      set_speed(128, REV);

      _delay_ms(5 * 1000);

      for(int i = 0; i < 5; ++i){
         // 457mm == 1.5 feet
         turn_in_arc(255, FWD, LEFT, 457);
         _delay_ms(3 * 1000);

         turn_in_arc(255, FWD, RIGHT, 457);
         _delay_ms(3 * 1000);
      }

      rotate(255, LEFT);
      _delay_ms(5 * 1000);
      rotate(128, RIGHT);
      _delay_ms(5 * 1000);

      set_speed(255, REV);
      _delay_ms(5 * 1000);

      hard_stop();

      _delay_ms(2 * 1000);

   }

   return 0;
}
