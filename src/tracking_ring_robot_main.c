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

#include "../include/adc.h"


static void indicate_line(line_position_t pos){
   led_set_fl(pos & LINE_FRONT_LEFT);
   led_set_fr(pos & LINE_FRONT_RIGHT);
   led_set_bl(pos & LINE_BACK_LEFT);
   led_set_br(pos & LINE_BACK_RIGHT);
}

static void avoid_line(line_position_t pos){
   if(pos & LINE_FRONT){
      motors_hard_stop();
      //switch_direction();
      movman_schedule_move(SWITCH_DIRECTION_THEN_MOVE_FORWARD, TO_AVOID_EDGE, IMMEDIATELY);
   }
}

static void handle_line(){
   line_position_t pos = line_sensors_get_position();
   avoid_line(pos);
   indicate_line(pos);
}

static void handle_new_prox(){

   sensor_t sensor = 0;
   uint8_t max = 0;
   sensor_t max_sensor = R_SENSOR;
   for(sensor = 0; sensor < N_SENSORS; ++sensor){
      if(adc_sensor_readings[sensor] > max){
         max = adc_sensor_readings[sensor];
         max_sensor = sensor;
      }
   }

   // Approximately .5 volts
   if(max > 50){
      // If it's at the back, switch directions and go towards it!
      if(max_sensor == BR_SENSOR || max_sensor == BL_SENSOR){
         movman_schedule_move(SWITCH_DIRECTION_THEN_MOVE_FORWARD, TO_SEEK, IMMEDIATELY_WITH_OVERWRITE);
      }else
      // If it's on either side, spin. Either the front or back will pick it up
      if(max_sensor ==  R_SENSOR || max_sensor ==  L_SENSOR){
         movman_schedule_move(LONG_ROTATE_LEFT, TO_SEEK, IMMEDIATELY_WITH_OVERWRITE);
      }else{
         // If it's at the front, move towards it!

         uint8_t left  = adc_sensor_readings[FL_SENSOR];
         uint8_t right = adc_sensor_readings[FR_SENSOR];

         // We'll track towards its direction
         if(left +  50 < right){
            movman_schedule_move(SMALL_TURN_LEFT, TO_SEEK, IMMEDIATELY_WITH_OVERWRITE);
         }else
         if(right + 50 < left){
            movman_schedule_move(SMALL_TURN_RIGHT, TO_SEEK, IMMEDIATELY_WITH_OVERWRITE);
         }else{
            movman_schedule_move(SMALL_MOVE_FORWARD, TO_SEEK, IMMEDIATELY_WITH_OVERWRITE);
         }
      }
   }

}

void tracking_ring_robot_main()
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
   adc_init();
   event_q_init();

   sei();

   adc_start();

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
         case NEW_PROXIMITY_READINGS:
            handle_new_prox();
            break;
         default:
            break;

      }

   }
}
