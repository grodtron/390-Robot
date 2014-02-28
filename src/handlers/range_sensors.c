#include <inttypes.h>
#include <stdbool.h>

#include "../../include/movement_manager.h"

#include "../../include/range_sensors.h"

#include "../../include/unused_param.h"

static uint16_t readings[6] = {0,0,0,0,0,0};
static uint8_t max = 0;
static sensor_t max_sensor = R_SENSOR;

static const int NOISE_THRESHOLD = 88; // 1 volt out of 2.9

static void update_readings_running_avg(){

   sensor_t sensor = 0;

   for(sensor = 0; sensor < N_SENSORS; ++sensor){

      uint8_t val = readings[sensor];
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

   UNUSED_PARAM(indicate);

   update_readings_running_avg();

   if(max > NOISE_THRESHOLD){
      if(max_sensor == BR_SENSOR || max_sensor == BL_SENSOR){
         // If it's at the back, switch directions and go towards it!
         movman_schedule_move(SWITCH_DIRECTION_THEN_MOVE_FORWARD, TO_SEEK, IMMEDIATELY_WITH_OVERWRITE);

      }else if(max_sensor ==  R_SENSOR || max_sensor ==  L_SENSOR){
         // If it's on either side, spin. Either the front or back will pick it up
         movman_schedule_move(LONG_ROTATE_LEFT, TO_SEEK, IMMEDIATELY_WITH_OVERWRITE);

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


