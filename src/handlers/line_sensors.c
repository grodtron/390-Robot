#include <stddef.h>

#include "../../include/leds.h"
#include "../../include/movement_manager.h"
#include "../../include/line_sensors.h"

#include "../../include/switch_direction.h"


static void indicate_line(line_position_t pos){
   led_set_fl(pos & LINE_FRONT_LEFT);
   led_set_fr(pos & LINE_FRONT_RIGHT);
   led_set_bl(pos & LINE_BACK_LEFT);
   led_set_br(pos & LINE_BACK_RIGHT);
}

static void avoid_line(line_position_t pos){

   // We want to bias ourselves to turning so that our better ground clearance side is at the front
   // We keep track of whether that is currently the case and turn in a different direction depending
   if(pos == LINE_RIGHT){
      if(current_front_is_good_front){
         movman_schedule_move(ROTATE_90_LEFT_THEN_MOVE_FORWARD,  TO_AVOID_EDGE, IMMEDIATELY_WITH_OVERWRITE);
      }else{
         movman_schedule_move(ROTATE_90_RIGHT_THEN_MOVE_FORWARD, TO_AVOID_EDGE, IMMEDIATELY_WITH_OVERWRITE);
      }

   }else if(pos == LINE_LEFT){
      if(current_front_is_good_front){
         movman_schedule_move(ROTATE_90_RIGHT_THEN_MOVE_FORWARD, TO_AVOID_EDGE, IMMEDIATELY_WITH_OVERWRITE);
      }else{
         movman_schedule_move(ROTATE_90_LEFT_THEN_MOVE_FORWARD,  TO_AVOID_EDGE, IMMEDIATELY_WITH_OVERWRITE);
      }

   }else if(pos & LINE_FRONT){
      movman_schedule_move(SWITCH_DIRECTION_THEN_MOVE_FORWARD, TO_AVOID_EDGE, IMMEDIATELY_WITH_OVERWRITE);
   }
}

void handle_line_sensors(bool indicate){

   line_position_t pos = line_sensors_get_position();

   avoid_line(pos);

   if(indicate){
      indicate_line(pos);
   }
}

