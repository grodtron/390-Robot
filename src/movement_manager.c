#include <inttypes.h>
#include <limits.h>

#include <stddef.h>

#include "../include/motors.h"
#include "../include/movement_manager.h"
#include "../include/static_assert.h"

// If we're testing algorithmic stuff...
#ifndef __AVR__

// We need stdio for printf
#include <stdio.h>

// Define thse motor functions as no-ops so that we can test
void motors_init(){}
void motors_set_speed(uint8_t a, motor_dir_t b, motor_turn_dir_t c, uint16_t d, uint16_t e){}
void motors_turn_in_arc(uint8_t a, motor_dir_t b, motor_turn_dir_t c, uint16_t d, uint16_t e){}
void motors_rotate(uint8_t a, motor_dir_t b, motor_turn_dir_t c, uint16_t d, uint16_t e){}
void motors_hard_stop(){}
bool motors_movement_in_progress(){ return false; }
#endif


// this struct is used to hold all the info necessary to make a motor movement
// command
typedef struct {

   void (*move_funct)(uint8_t, motor_dir_t, motor_turn_dir_t, uint16_t, uint16_t);
   uint8_t          speed;
   motor_dir_t      dir;
   motor_turn_dir_t turn_dir;
   uint16_t         param;
   uint16_t         timeout;

} pending_movement_t;


#define QUEUELEN 8
static pending_movement_t move_queue[QUEUELEN];
static uint8_t            current_move;
movement_reason_t         current_move_reason;

#ifdef __AVR__
// This macro actually executes the movement
#define EXECUTE_CURRENT_MOVE()        \
move_queue[current_move].move_funct(  \
   move_queue[current_move].speed,    \
   move_queue[current_move].dir,      \
   move_queue[current_move].turn_dir, \
   move_queue[current_move].param,    \
   move_queue[current_move].timeout   \
)
#else
// This macro is just for testing, and prints a description of the movement
#define EXECUTE_CURRENT_MOVE() printf("Executing move %s(%d, %s, %s, %d)\n",           \
     move_queue[current_move].move_funct == &motors_set_speed ? "motors_set_speed" : "motors_rotate", \
     move_queue[current_move].speed,    \
     move_queue[current_move].dir == FWD ? "FWD" : "REV", \
     move_queue[current_move].turn_dir == LEFT ? "LEFT" : "RIGHT", \
     move_queue[current_move].timeout)
#endif

// Clears the queue
void movman_init(){
   // TODO - do we really need to reset all these things all the time just
   // to show one condition
   current_move = 0;
   current_move_reason = NO_REASON;
   move_queue[0].move_funct = NULL;
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

   // NULL-ify the current movement in the queue
   // move_queue[current_move].reason = NO_REASON;

   // Go to the next index
   //
   // NB - MUST set up moves properly, no range check is done
   ++current_move;

   if(move_queue[current_move].move_funct){
      // If there is another move to be executed, then we execute it
      EXECUTE_CURRENT_MOVE();
   }else{
      // If we reached the last move in the queue we reset the queue
      movman_init();
   }
   // TODO - empty queue event?
}


bool movman_schedule_move(movement_t move, movement_reason_t reason){

   if(current_move_reason >= reason){
      return false;  // we're doing something more important
   }

   current_move_reason = reason;
   current_move        = 0;

   switch(move){
      case WAIT_5_SECONDS_THEN_FULL_FORWARD_FOR_A_LONG_TIME:

         move_queue[0].move_funct = &motors_set_speed;
         move_queue[0].speed      = 0;
         move_queue[0].timeout    = 4875; // This is a true 5 seconds, since our milliseconds are not really milliseconds

         move_queue[1].move_funct = &motors_set_speed;
         move_queue[1].speed      = 255;
         move_queue[1].dir        = FWD;
         move_queue[1].timeout    = UINT16_MAX; // Go for as long as we are capable of keepint track

         move_queue[2].move_funct = NULL;

         break;

      case MOVE_FORWARD:

         move_queue[0].move_funct = &motors_set_speed;
         move_queue[0].speed      = 255;
         move_queue[0].dir        = FWD;
         move_queue[0].timeout    = 1000;

         move_queue[1].move_funct = NULL;

         break;

      case SEARCH_PATTERN:
         move_queue[0].move_funct = &motors_rotate;
         move_queue[0].speed      = 255;
         move_queue[0].turn_dir   = LEFT;
         move_queue[0].timeout    = 1400;

         move_queue[1].move_funct = &motors_rotate;
         move_queue[1].speed      = 255;
         move_queue[1].turn_dir   = RIGHT;
         move_queue[1].timeout    = 700;

         move_queue[2].move_funct = &motors_set_speed;
         move_queue[2].speed      = 255;
         move_queue[2].dir        = FWD;
         move_queue[2].timeout    = 3000;

         move_queue[3].move_funct = NULL;

         break;

      case BACKUP_THEN_TURN_90_CCW:
         move_queue[0].move_funct = &motors_set_speed;
         move_queue[0].speed      = 255;
         move_queue[0].dir        = REV;
         move_queue[0].timeout    = 1000;

         move_queue[1].move_funct = &motors_rotate;
         move_queue[1].speed      = 255;
         move_queue[1].turn_dir   = LEFT;
         move_queue[1].timeout    = 1400;

         move_queue[2].move_funct = &motors_set_speed;
         move_queue[2].speed      = 255;
         move_queue[2].dir        = FWD;
         move_queue[2].timeout    = 1500;

         move_queue[3].move_funct = NULL;

         break;

      case SMALL_TURN_LEFT:
         move_queue[0].move_funct = &motors_turn_in_arc;
         move_queue[0].speed      = 255;
         move_queue[0].dir        = FWD;
         move_queue[0].turn_dir   = LEFT;
         move_queue[0].param      = 150; // radius of the turn
         move_queue[0].timeout    = 400;

         move_queue[1].move_funct = NULL;

         break;

      case SMALL_TURN_RIGHT:
         move_queue[0].move_funct = &motors_turn_in_arc;
         move_queue[0].speed      = 255;
         move_queue[0].dir        = FWD;
         move_queue[0].turn_dir   = RIGHT;
         move_queue[0].param      = 150; // radius of the turn
         move_queue[0].timeout    = 400;

         move_queue[1].move_funct = NULL;

         break;

      default:
         move_queue[0].move_funct = NULL;
         current_move_reason      = NO_REASON;
         return false;
   }

   EXECUTE_CURRENT_MOVE();

   return true;
}

#ifndef __AVR__


#define PRINT_MOVE(i) printf("%s %s(%d, %s, %s, %d)\n",           \
     i == current_move ? "--> " : "    ", \
     move_queue[i].move_funct == &motors_set_speed ? "motors_set_speed" : "motors_rotate", \
     move_queue[i].speed,    \
     move_queue[i].dir == FWD ? "FWD" : "REV", \
     move_queue[i].turn_dir == LEFT ? "LEFT" : "RIGHT", \
     move_queue[i].timeout) \

#define STEP() do{                  \
   for(uint8_t i = 0; i < QUEUELEN; ++i){ \
      PRINT_MOVE(i); \
   } \
   movman_current_move_completed(); \
   movman_schedule_move(SEARCH_PATTERN, TO_SEARCH); \
}while(0);



int main(int argc, const char *argv[])
{

   movman_schedule_move(SEARCH_PATTERN, TO_SEARCH);

   STEP();
   STEP();

   movman_schedule_move(BACKUP_THEN_TURN_90_CCW, TO_AVOID_EDGE);

   for(uint8_t i = 0; i < 16; ++i){
      STEP();
   }


   return 0;
}

#endif
