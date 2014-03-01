#include <inttypes.h>
#include <stdbool.h>

#include "../../include/movement_manager.h"

#include "../../include/range_sensors.h"

#include "../../include/unused_param.h"

#include "../../include/leds.h"

#include "../../include/switch_direction.h"

static uint16_t readings[6] = {0,0,0,0,0,0};
static uint8_t max = 0;
static sensor_t max_sensor = R_SENSOR;

static const int NOISE_THRESHOLD = 173; // 1 volt out of 2.9
// 1/2.9 * 255 = 88
//
// accounting for exponential running average:
// 88 + 44 + 22 + 11 + 5 + 2 + 1 = 173

static void update_readings_running_avg(){

   sensor_t sensor = 0;

   max = 0;
   for(sensor = 0; sensor < N_SENSORS; ++sensor){

      uint16_t val = readings[sensor];
      val /= 2;
      val += range_sensors_sensor_readings[sensor];

      readings[sensor] = val;

      if(val > max){
         max = val;
         max_sensor = sensor;
      }
   }
}


void handle_range_sensors(bool indicate){

   update_readings_running_avg();

   if(max > NOISE_THRESHOLD){
      if(max_sensor == BR_SENSOR || max_sensor == BL_SENSOR){
         // If it's at the back, switch directions and go towards it!
         movman_schedule_move(SWITCH_DIRECTION_THEN_MOVE_FORWARD, TO_SEEK, IMMEDIATELY_WITH_OVERWRITE);

         if(indicate){ led_toggle_orange(); }

      }else if(max_sensor == L_SENSOR){

         if(current_front_is_good_front){
            movman_schedule_move(LONG_ROTATE_LEFT, TO_SEEK, IMMEDIATELY_WITH_OVERWRITE);
         }else{
            movman_schedule_move(LONG_ROTATE_RIGHT, TO_SEEK, IMMEDIATELY_WITH_OVERWRITE);
         }

         if(indicate){ led_toggle_white(); }

      }else if(max_sensor == R_SENSOR){

         if(current_front_is_good_front){
            movman_schedule_move(LONG_ROTATE_RIGHT, TO_SEEK, IMMEDIATELY_WITH_OVERWRITE);
         }else{
            movman_schedule_move(LONG_ROTATE_LEFT, TO_SEEK, IMMEDIATELY_WITH_OVERWRITE);
         }

         if(indicate){ led_toggle_white(); }

      }else{
         // If it's at the front, move towards it!
         //
         // We're not going to try to do any curving moves to align ourselves, we're just gonna go
         // all out. If we attack and miss, we still catch them on the side...

         movman_schedule_move(SMALL_MOVE_FORWARD, TO_SEEK, IMMEDIATELY_WITH_OVERWRITE);
      }
   }
}

static uint16_t left_accumulate  = 0;
static uint16_t right_accumulate = 0;
void handle_range_sensors_accumulate(){

   update_readings_running_avg();

   left_accumulate  += range_sensors_sensor_readings[L_SENSOR];
   right_accumulate += range_sensors_sensor_readings[R_SENSOR];

}


void handle_range_sensors_react_accumulate(){

   if(left_accumulate > right_accumulate){
      movman_schedule_move(ROTATE_90_LEFT_THEN_MOVE_FORWARD, TO_SEEK, IMMEDIATELY_WITH_OVERWRITE);
   }else{
      movman_schedule_move(ROTATE_90_RIGHT_THEN_MOVE_FORWARD, TO_SEEK, IMMEDIATELY_WITH_OVERWRITE);
   }

}


