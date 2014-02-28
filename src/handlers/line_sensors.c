#include <stddef.h>

#include "../../include/leds.h"
#include "../../include/movement_manager.h"
#include "../../include/line_sensors.h"


static void indicate_line(line_position_t pos){
   led_set_fl(pos & LINE_FRONT_LEFT);
   led_set_fr(pos & LINE_FRONT_RIGHT);
   led_set_bl(pos & LINE_BACK_LEFT);
   led_set_br(pos & LINE_BACK_RIGHT);
}

static void avoid_line(line_position_t pos){

   if(pos == LINE_LEFT){
      movman_schedule_move(ROTATE_90_RIGHT_THEN_MOVE_FORWARD, TO_AVOID_EDGE, IMMEDIATELY_WITH_OVERWRITE);

   }else if(pos == LINE_RIGHT){
      movman_schedule_move(ROTATE_90_LEFT_THEN_MOVE_FORWARD, TO_AVOID_EDGE, IMMEDIATELY_WITH_OVERWRITE);

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

