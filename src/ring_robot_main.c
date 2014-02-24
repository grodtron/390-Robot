#include <avr/interrupt.h>
#include <avr/io.h>
#define F_CPU 1000000UL
#include <util/delay.h>

#include "../include/event_queue.h"
#include "../include/line_sensors.h"
#include "../include/iodefs.h"
#include "../include/movement_manager.h"
#include "../include/motors.h"

#include "../include/switch_direction.h"

#include "../include/leds.h"


static void indicate_line(line_position_t pos){
   led_set_fl(pos & LINE_FRONT_LEFT);
   led_set_fr(pos & LINE_FRONT_RIGHT);
   led_set_bl(pos & LINE_BACK_LEFT);
   led_set_br(pos & LINE_BACK_RIGHT);
}

static void avoid_line(line_position_t pos){
   if(pos & LINE_FRONT){
      motors_hard_stop();
      switch_direction();
      movman_schedule_move(MOVE_FORWARD, TO_SEEK, IMMEDIATELY_WITH_OVERWRITE);
   }
}

static void handle_line(){
   line_position_t pos = line_sensors_get_position();
   avoid_line(pos);
   indicate_line(pos);
}

void ring_robot_main()
{
   iodefs_init();
   leds_init();

   {
      int i;
      for(i = 0; i < (2*10); ++i){
         led_toggle_fl();
         _delay_ms(100);
      }
   }

   line_sensors_init();
   motors_init();
   movman_init();
   event_q_init();

   sei();

   movman_schedule_move(MOVE_FORWARD, TO_SEEK, IMMEDIATELY);

   while(1){

      event_t e = event_q_get_next_event();

      switch(e){

         case LINE_DETECTED:
            handle_line();
            break;
         case MOVEMENT_COMPLETE:
            if(movman_current_move_completed(false)){
               movman_schedule_move(MOVE_FORWARD, TO_SEEK, IMMEDIATELY);
            }
         default:
            break;

      }

   }
}
