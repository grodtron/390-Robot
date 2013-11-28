#include <inttypes.h>
#include <limits.h>

#include "../include/movement_manager.h"
#ifdef __AVR__
#include "../include/motors.h"
#else

#include <stdio.h>

void motors_init(){}

void motors_set_speed(uint8_t speed, motor_dir_t dir, motor_turn_dir_t dummy1, uint16_t dummy2, uint16_t timeout){}

void motors_turn_in_arc(uint8_t speed, motor_dir_t dir, motor_turn_dir_t turn_dir, uint16_t radius, uint16_t timeout){}

void motors_rotate(uint8_t speed, motor_dir_t dummy1, motor_turn_dir_t dir, uint16_t dummy2, uint16_t timeout){}

void motors_hard_stop(){}

bool motors_movement_in_progress(){}
#endif
#include "../include/static_assert.h"


// this struct is used to hold all the info necessary to make a motor movement
// command, as well as associating the command with a level of priority
typedef struct {

   movement_reason_t    reason;

   void (*move_funct)(uint8_t, motor_dir_t, motor_turn_dir_t, uint16_t, uint16_t);
   uint8_t          speed;
   motor_dir_t      dir;
   motor_turn_dir_t turn_dir;
   uint16_t         param;
   uint16_t         timeout;

} pending_movement_t;


#define QUEUELEN 8
static pending_movement_t move_queue[QUEUELEN];

static uint8_t current_move;

#ifdef __AVR__
#define EXECUTE_CURRENT_MOVE()        \
move_queue[current_move].move_funct(  \
   move_queue[current_move].speed,    \
   move_queue[current_move].dir,      \
   move_queue[current_move].turn_dir, \
   move_queue[current_move].param,    \
   move_queue[current_move].timeout   \
)
#else
#define EXECUTE_CURRENT_MOVE() printf("Executing move %s(%d, %s, %s, %d) with priority %d\n",           \
     move_queue[current_move].move_funct == &motors_set_speed ? "motors_set_speed" : "motors_rotate", \
     move_queue[current_move].speed,    \
     move_queue[current_move].dir == FWD ? "FWD" : "REV", \
     move_queue[current_move].turn_dir == LEFT ? "LEFT" : "RIGHT", \
     move_queue[current_move].timeout, \
     move_queue[current_move].reason)
#endif

void movman_init(){
   for(uint8_t i = 0; i < QUEUELEN; ++i){
      move_queue[i].reason = NO_REASON;
   }
   current_move = 0;
}

void movman_current_move_completed(){

   #ifdef __AVR__
   if(motors_movement_in_progress()){
      // Bullshit, the move's not complete at all!
      //
      // But seriously, when we overwrite moves with higher priority
      // ones, it's possible we have old move-completed events in
      // the queue, so we double check to avoid spuriously cancelling
      // high-priority moves
      return;
   }
   #endif

   move_queue[current_move].reason = NO_REASON;

   current_move = (current_move + 1) % QUEUELEN;

   if(move_queue[current_move].reason){
      // If there's a reason to execute the next move,
      // then do it, but we don't make moves for no reason :)
      //
      // But seriously, if reason == NO_REASON then it's a null
      // move and the queue is empty

      // This move did have a reason, so we execute it
      EXECUTE_CURRENT_MOVE();
   }else{
      movman_init();
   }
}

// Schedules a move and returns its 1-indexed position in the movement queue.
// 1 means that it is executed immediately, n>1 means that it will be executed
// after n-1 other moves complete, and 0 means that it was not added because its
// priority was too low nad the queue is full
uint8_t movman_schedule_motor_instruction(
   movement_reason_t reason,

   void (*move_funct)(uint8_t, motor_dir_t, motor_turn_dir_t, uint16_t, uint16_t),
   uint8_t          speed,
   motor_dir_t      dir,
   motor_turn_dir_t turn_dir,
   uint16_t         param,
   uint16_t         timeout,

   movement_time_t within_first_n
){

   uint8_t n;
   const uint8_t LIM = QUEUELEN < within_first_n ? QUEUELEN : within_first_n;
   for(n = 0; n < LIM; ++n){
      uint8_t i = (current_move + n) % QUEUELEN;
      if(move_queue[i].reason < reason){
         move_queue[i].reason     = reason;
         move_queue[i].move_funct = move_funct;
         move_queue[i].speed      = speed;
         move_queue[i].dir        = dir;
         move_queue[i].turn_dir   = turn_dir;
         move_queue[i].param      = param;
         move_queue[i].timeout    = timeout;

         break;
      }
   }

   if(n == LIM){
      // If we ended on QUEUELEN, it means we didn't find a viable space
      // in the queue and we should return 0 indicating that the move
      // was not scheduled
      return 0;
   }
   if(n == 0){
      // If we ended on a zero, it means we just added something to the
      // first position in the queue, and it should be executed immediately
      EXECUTE_CURRENT_MOVE();
   }
   // If we ended on anything but LIM, we should return n+1
   return n + 1;

}

#define DUMMY 0

bool movman_schedule_move(movement_t move, movement_reason_t reason, movement_time_t when){

   // We don't want to overflow when incrementing `when`
   static_assert(QUEUELEN < 128);

   when *= QUEUELEN;

   if(move_queue[current_move].reason < reason){
      movman_init();
   }else{
      return false;
   }

   switch(move){
      case WAIT_5_SECONDS_THEN_FULL_FORWARD_FOR_A_LONG_TIME:
         return
            movman_schedule_motor_instruction(reason,
               &motors_set_speed,
               0,
               FWD,
               DUMMY,
               DUMMY,
               4875, // This is a true 5 seconds, since our milliseconds are not really milliseconds
               when + 1)
         &&
            movman_schedule_motor_instruction(reason,
               &motors_set_speed,
               255,
               FWD,
               DUMMY,
               DUMMY,
               UINT16_MAX,
               when + 2);
      case MOVE_FORWARD:
         return
            movman_schedule_motor_instruction(reason,
               &motors_set_speed,
               255,
               FWD,
               DUMMY,
               DUMMY,
               1000,
               when + 1);

      case SEARCH_PATTERN:
         return
            movman_schedule_motor_instruction(reason,
               &motors_rotate,
               255,
               DUMMY,
               LEFT,
               DUMMY,
               1400,
               when + 1)
         &&
            movman_schedule_motor_instruction(reason,
               &motors_rotate,
               255,
               DUMMY,
               RIGHT,
               DUMMY,
               700,
               when + 2)
         &&
            movman_schedule_motor_instruction(reason,
               &motors_set_speed,
               255,
               FWD,
               DUMMY,
               DUMMY,
               3000,
               when + 3);

      case BACKUP_THEN_TURN_90_CCW:
         return
            movman_schedule_motor_instruction(reason,
               &motors_set_speed,
               255,
               REV,
               DUMMY,
               DUMMY,
               1000,
               when + 1)
         &&
            movman_schedule_motor_instruction(reason,
               &motors_rotate,
               255,
               DUMMY,
               LEFT,
               DUMMY,
               1400,
               when + 2)
         &&
            movman_schedule_motor_instruction(reason,
               &motors_set_speed,
               255,
               FWD,
               DUMMY,
               DUMMY,
               750,
               when + 3);
      case SMALL_TURN_LEFT:
         return
            movman_schedule_motor_instruction(reason,
               &motors_turn_in_arc,
               255,
               FWD,
               LEFT,
               150,
               400,
               when + 1);
      case SMALL_TURN_RIGHT:
         return
            movman_schedule_motor_instruction(reason,
               &motors_turn_in_arc,
               255,
               FWD,
               RIGHT,
               150,
               400,
               when + 1);
      case GO_FORWARD_BRIEFLY:
         return
            movman_schedule_motor_instruction(reason,
               &motors_set_speed,
               255,
               FWD,
               DUMMY,
               DUMMY,
               250,
               when + 1);
      default:
         return false;
   }
}

#ifndef __AVR__


#define PRINT_MOVE(i) printf("%s %s(%d, %s, %s, %d) with priority %d\n",           \
     i == current_move ? "--> " : "    ", \
     move_queue[i].move_funct == &motors_set_speed ? "motors_set_speed" : "motors_rotate", \
     move_queue[i].speed,    \
     move_queue[i].dir == FWD ? "FWD" : "REV", \
     move_queue[i].turn_dir == LEFT ? "LEFT" : "RIGHT", \
     move_queue[i].timeout, \
     move_queue[i].reason) \

#define STEP() do{                  \
   for(uint8_t i = 0; i < QUEUELEN; ++i){ \
      PRINT_MOVE(i); \
   } \
   movman_current_move_completed(); \
   movman_schedule_move(SEARCH_PATTERN, TO_SEARCH, NEXT_AVAILABLE_TIME); \
}while(0);



int main(int argc, const char *argv[])
{

   movman_schedule_move(SEARCH_PATTERN, TO_SEARCH, NEXT_AVAILABLE_TIME);

   STEP();
   STEP();

   movman_schedule_move(BACKUP_THEN_TURN_90_CCW, TO_AVOID_EDGE, IMMEDIATELY_ELSE_IGNORE);

   for(uint8_t i = 0; i < 32; ++i){
      STEP();
   }


   return 0;
}

#endif
