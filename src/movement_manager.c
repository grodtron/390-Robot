#include <inttypes.h>

#include "../include/movement_manager.h"
#include "../include/motors.h"
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

#define EXECUTE_CURRENT_MOVE()        \
move_queue[current_move].move_funct(  \
   move_queue[current_move].speed,    \
   move_queue[current_move].dir,      \
   move_queue[current_move].turn_dir, \
   move_queue[current_move].param,    \
   move_queue[current_move].timeout   \
)

void movman_init(){
   for(uint8_t i = 0; i < QUEUELEN; ++i){
      move_queue[i].reason = NO_REASON;
   }
   current_move = 0;
}

void movman_current_move_completed(){

   if(motors_movement_in_progress()){
      // Bullshit, the move's not complete at all!
      //
      // But seriously, when we overwrite moves with higher priority
      // ones, it's possible we have old move-completed events in
      // the queue, so we double check to avoid spuriously cancelling
      // high-priority moves
      return;
   }

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

   switch(move){
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
               750,
               when + 2);  //  TODO - tweak timeout to get true 90
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
