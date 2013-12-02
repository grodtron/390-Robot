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

   movement_reason_t reason;

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
   int8_t i;
   for(i = 0; i < QUEUELEN; ++i){
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

   // NULL-ify the current movement in the queue
   // move_queue[current_move].reason = NO_REASON;

   // Go to the next index
   //
   // NB - MUST set up moves properly, no range check is done
   move_queue[current_move].reason = NO_REASON;
   current_move = (current_move + 1) % QUEUELEN;

   if(move_queue[current_move].reason != NO_REASON){
      // If there is another move to be executed, then we execute it
      EXECUTE_CURRENT_MOVE();
   }
   // TODO - empty queue event?
}


// Return the offset from the current position where `length` moves with priority
// `reason` can be scheduled. Returns -1 if they cannot be scheduled.
int8_t next_available_index( uint8_t length, movement_reason_t reason, bool overwrite){
   int8_t i;
   for(i = 0; i < QUEUELEN; ++i){
      movement_reason_t why = move_queue[(current_move + i) % QUEUELEN].reason;
      if( why < reason || (why == reason && overwrite) ){
         break;
      }
   }

   if(QUEUELEN - i >= length){
      return i;
   }else{
      return -1;
   }

}


bool movman_schedule_move(movement_t move, movement_reason_t reason, movement_time_t when){

   #define DEFINE_MOVE(NAME, n_moves, DO_SCHEDULE_MOVES) \
         case NAME: \
         i = next_available_index(n_moves, reason, when == IMMEDIATELY_WITH_OVERWRITE); \
         if(i == -1 || (when != NEXT_AVAILABLE_TIME && i != 0)){ \
            return false; \
         } \
         if(when != NEXT_AVAILABLE_TIME){ \
            movman_init();\
         } \
         i = (current_move + i) % QUEUELEN; \
         DO_SCHEDULE_MOVES \
         break;

   #define PAUSE(TIMEOUT) \
         move_queue[i].move_funct = &motors_set_speed; \
         move_queue[i].speed      = 0; \
         move_queue[i].timeout    = TIMEOUT; \
         move_queue[i].reason     = reason; \
         i = (i + 1) % QUEUELEN;

   #define MOVE_STRAIGHT(DIRECTION, TIMEOUT) \
         move_queue[i].move_funct = &motors_set_speed; \
         move_queue[i].speed      = 255;        \
         move_queue[i].dir        = DIRECTION;        \
         move_queue[i].timeout    = TIMEOUT; \
         move_queue[i].reason     = reason; \
         i = (i + 1) % QUEUELEN;

   #define ROTATE(DIRECTION, TIMEOUT) \
         move_queue[i].move_funct = &motors_rotate; \
         move_queue[i].speed      = 255; \
         move_queue[i].turn_dir   = DIRECTION; \
         move_queue[i].timeout    = TIMEOUT; \
         move_queue[i].reason     = reason; \
         i = (i + 1) % QUEUELEN;

   #define TURN(DIRECTION, TURN_DIRECTION, TIMEOUT) \
         move_queue[i].move_funct = &motors_turn_in_arc; \
         move_queue[i].speed      = 255; \
         move_queue[i].dir        = DIRECTION; \
         move_queue[i].turn_dir   = TURN_DIRECTION; \
         move_queue[i].param      = 15; /*radius*/  \
         move_queue[i].timeout    = TIMEOUT; \
         move_queue[i].reason     = reason; \
         i = (i + 1) % QUEUELEN;

   int8_t i;

   switch(move){


      DEFINE_MOVE(
      WAIT_5_SECONDS_THEN_FULL_FORWARD_FOR_A_LONG_TIME,
         2,
         PAUSE(4875)
         MOVE_STRAIGHT(FWD, UINT16_MAX)
      )

      DEFINE_MOVE(
      MOVE_FORWARD,
         1,
         MOVE_STRAIGHT(FWD, 1000)
      )


      DEFINE_MOVE(
      SEARCH_PATTERN,
         3,
         MOVE_STRAIGHT(FWD, 1100)
         ROTATE(LEFT, 700)
         ROTATE(RIGHT, 500)
      )

      DEFINE_MOVE(
      BACKUP_THEN_TURN_90_CW,
         2,
         MOVE_STRAIGHT(REV, 1000)
         ROTATE(RIGHT, 900)
      )

      DEFINE_MOVE(
      BACKUP_THEN_TURN_30_CCW,
         2,
         MOVE_STRAIGHT(REV, 1000)
         ROTATE(LEFT, 700)
      )

      DEFINE_MOVE(
      SMALL_MOVE_FORWARD,
         1,
         MOVE_STRAIGHT(FWD, 400)
      )

      DEFINE_MOVE(
      LONG_ROTATE_LEFT,
         1,
         ROTATE(LEFT, 1750)
      )

      DEFINE_MOVE(
      ROTATE_LEFT,
         1,
         ROTATE(LEFT, 750)
      )

      DEFINE_MOVE(
      ROTATE_RIGHT,
         1,
         ROTATE(RIGHT, 750)
      )

      DEFINE_MOVE(
      SMALL_TURN_LEFT,
         1,
         TURN(FWD, LEFT, 400)
      )

      DEFINE_MOVE(
      SMALL_TURN_RIGHT,
         1,
         TURN(FWD, RIGHT, 400)
      )

      default:
         return false;
   }

   EXECUTE_CURRENT_MOVE();

   return true;
   #undef DEFINE_MOVE
   #undef PAUSE
   #undef MOVE_STRAIGHT
   #undef ROTATE
   #undef TURN
}

#ifndef __AVR__

#define PRINT_MOVE(i) if(move_queue[i].reason == NO_REASON) \
      printf("%s [               ]\n",           \
     i == current_move ? "--> " : "    "); \
     else \
      printf("%s %s(%d, %s, %s, %d)\n",           \
     i == current_move ? "--> " : "    ", \
     move_queue[i].move_funct == &motors_set_speed ? "motors_set_speed" : \
     (move_queue[i].move_funct == &motors_turn_in_arc ? "motors_turn_in_arc" : "motors_rotate"), \
     move_queue[i].speed,    \
     move_queue[i].dir == FWD ? "FWD" : "REV", \
     move_queue[i].turn_dir == LEFT ? "LEFT" : "RIGHT", \
     move_queue[i].timeout)

#define PRINT_QUEUE() do{                  \
   for(uint8_t i = 0; i < QUEUELEN; ++i){ \
      PRINT_MOVE(i); \
   } \
}while(0);

#define STEP() do{                  \
   movman_current_move_completed(); \
   movman_schedule_move(SEARCH_PATTERN, TO_SEARCH, NEXT_AVAILABLE_TIME); \
}while(0)


int main(int argc, const char *argv[])
{

   movman_init();
   PRINT_QUEUE();
   movman_schedule_move(
      WAIT_5_SECONDS_THEN_FULL_FORWARD_FOR_A_LONG_TIME,
      TO_MEET_STARTUP_REQUIREMENT,
      IMMEDIATELY);

   PRINT_QUEUE();
   for(int i = 0; i < 16; ++i){
      STEP();
      PRINT_QUEUE();
   }


   return 0;
}

#endif
