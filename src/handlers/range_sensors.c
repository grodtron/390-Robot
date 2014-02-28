#include <inttypes.h>
#include <stdbool.h>

#include "../../include/movement_manager.h"

#include "../../include/range_sensors.h"

#include "../../include/unused_param.h"

static uint16_t readings[6] = {0,0,0,0,0,0};


void handle_range_sensors(bool indicate){

   UNUSED_PARAM(indicate);

   sensor_t sensor = 0;
   uint8_t max = 0;
   sensor_t max_sensor = R_SENSOR;

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

         uint8_t left  = range_sensors_sensor_readings[FL_SENSOR];
         uint8_t right = range_sensors_sensor_readings[FR_SENSOR];

         // We'll track towards its direction
         if(left  + 50 < right){
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

