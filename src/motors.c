#ifdef __AVR__
   #include <avr/io.h>
   #include <avr/interrupt.h>
#endif

#include <stdbool.h>

#include "../include/motors.h"
#include "../include/event_queue.h"
#include "../include/static_assert.h"
#include "../include/unused_param.h"

typedef struct {
   uint8_t curr_speed;
   uint8_t targ_speed;

   motor_dir_t curr_dir;
   motor_dir_t targ_dir;

   bool complete;

} motor_tween_t;

bool _update_tween(motor_tween_t * tw);
void _tween_motor_speed(motor_tween_t * tw, uint8_t speed, motor_dir_t dir);


static motor_tween_t l_motor_tween;
static motor_tween_t r_motor_tween;

static uint16_t current_timeout;

// TODO - these should be always atomic operations. If non-interrupt code
// uses them, it should surround with cli(); and sei();
// (page 79);
//
// Also NB that high reg is written before low reg in both cases.
#define SET_L_MOTOR_SPEED(val) do{ OCR1AH=0; OCR1AL=(val); }while(0)
#define SET_R_MOTOR_SPEED(val) do{ OCR1BH=0; OCR1BL=(val); }while(0)

#define SET_L_MOTOR_DIR(dir)   do{\
                                    if( (dir) == FWD ){ \
                                       /* NB turn off and turn on are simultaneous */ \
                                       PORTD = (PORTD | (1<<PD5)) & ~(1<<PD6); \
                                    }else{ \
                                       PORTD = (PORTD | (1<<PD6)) & ~(1<<PD5); \
                                    } \
                               }while(0)
#define SET_R_MOTOR_DIR(dir)   do{\
                                    if( (dir) == FWD ){ \
                                       /* NB turn off before turn on */ \
                                       PORTB &= ~(1 << PB0); \
                                       PORTD |=  (1 << PD7); \
                                    }else{ \
                                       PORTD &= ~(1 << PD7); \
                                       PORTB |=  (1 << PB0); \
                                    } \
                               }while(0)

#define INVERTED_OUTPUT 0

void motors_init(){

   static_assert((INVERTED_OUTPUT == 1) || (INVERTED_OUTPUT == 0))


#ifdef __AVR__
   DDRB  |= (1 << DDB1) | (1 << DDB2)   // Set OC1A and OC1B as outputs
         |  (1 << DDB0);                // Set Motor-Right Backwards as output

   DDRD  |= (1 << DDD5) | (1 << DDD6) | (1 << DDD7); // Set other motor-dir lines as ouput

   TCCR1A = (1 << COM1A1) | (INVERTED_OUTPUT << COM1A0) // Set OC1A as PWM output (page 98)
          | (1 << COM1B1) | (INVERTED_OUTPUT << COM1B0) // Set OC1B as PWM output (page 98)
          | (0 << FOC1A)  | (0 << FOC1B)  // Set these 0 (unused, see page 99)
          | (0 << WGM11)  | (1 << WGM10)  // First half of Fast PWM, 8-bit (page 99)
          ;

   TCCR1B = (0 << WGM13)  | (1 << WGM12)  // Second half of Fast PWM, 8-bit (page 99)
          | (0 << ICNC1)  | (0 << ICES1)  // Stuff we don't care about (page 100)
          | (0 << CS12) // Clock select, we use no prescaler, since we are in 8 bit mode
           |(0 << CS11) // this gives a PWM frequency of 1MHz/256 ~= 4kHZ which is perfect
           |(1 << CS10) // for our uses. (page 100)
          ;

   // turns motors off and resets tweens
   motors_hard_stop();

   // As per table 39 on page 99, we can safely also use this timer for our other timing purposes
   // using the overflow interrupt, which is set once per PWM cycle ~= 4 times per ms.
   //
   TIMSK  |= (1 << TOIE1);
#endif
}

// To avoid harsh stops and starts, we tween (animation term, look it up)
// our motor speeds from zero to max. It is set up currently so that we go
// from stopped to max speed in one second, in 1 bit increments. The acceleration
// is always the same, at a rate of 0.256 "bits of speed"/ms = 256 bits/second
//
// All tweens will take abs(target_speed - current_speed)/256 seconds
#ifdef __AVR__
ISR(TIMER1_OVF_vect){
   static uint8_t n_ticks = 0;

   ++n_ticks;

   // This lets the timeout approximately represent milliseconds
   if(0 == n_ticks % 4 && current_timeout){
      --current_timeout;
      if( ! current_timeout){
         event_q_add_event(MOVEMENT_COMPLETE);
      }
   }


   // Update whatever tweens need updating
   if(! l_motor_tween.complete){
      _update_tween(&l_motor_tween);
      SET_L_MOTOR_SPEED(l_motor_tween.curr_speed);
      SET_L_MOTOR_DIR  (l_motor_tween.curr_dir  );
   }
   if(! r_motor_tween.complete){
      _update_tween(&r_motor_tween);
      SET_R_MOTOR_SPEED(r_motor_tween.curr_speed);
      SET_R_MOTOR_DIR  (r_motor_tween.curr_dir  );
   }

}
#endif

// Update `cur_speed` and `cur_dir` so that they point to the
// next values for the tween. Returns true if the tween is complete
// after this update, and false otherwise. Also updates `tw->complete`
//
// Assumes that the tween is valid and is not complete
bool _update_tween(motor_tween_t * tw){

   // If we are not moving currently, then we can change directions
   if(tw->curr_speed == 0){
      tw->curr_dir = tw->targ_dir;
   }

   // If we are moving in the right direction, and our target
   // speed is greater than our current speed, we increase our
   // speed, otherwise we decrease our speed.
   if(tw->curr_dir == tw->targ_dir && tw->curr_speed < tw->targ_speed){
      ++tw->curr_speed;
   }else{
      --tw->curr_speed;
   }

   return tw->complete = (tw->curr_speed == tw->targ_speed && tw->curr_dir == tw->targ_dir);

}

// Tween a motor's speed, overwriting any previously existing tween.
void _tween_motor_speed(motor_tween_t * tw, uint8_t speed, motor_dir_t dir){

   // disable the current tween, if it exists
   tw->complete = true;

   // Only start a tween if our current values are wrong
   if(tw->curr_speed != speed || tw->curr_dir != dir){
      tw->targ_speed = speed;
      tw->targ_dir   = dir;
      tw->complete   = false;
   }

}

void motors_set_speed(uint8_t speed, motor_dir_t dir, motor_turn_dir_t dummy1, uint16_t dummy2, uint16_t timeout){

   UNUSED_PARAM(dummy1);
   UNUSED_PARAM(dummy2);

   _tween_motor_speed(&l_motor_tween, speed, dir);
   _tween_motor_speed(&r_motor_tween, speed, dir);
   current_timeout = timeout;
}

void motors_turn_in_arc(uint8_t speed, motor_dir_t dir, motor_turn_dir_t turn_dir, uint16_t radius, uint16_t timeout){
   // wheel base is 146mm (from center of one wheel to senter of other wheel).
   //
   // This means that when we turn. our wheels trace out two arcs, one with radius
   // `radius`, the other with radius `radius - 146`. Because arc length is directly
   // proportional to radius, and the two wheels must travel their respective arcs in
   // the same amount of time, the ratio of their speeds is equal to the ratio of their
   // radius.
   //
   // We will set the outer wheel to go at `speed`, and the inner wheel to go at
   //  `((speed * (radius-146)) / radius)`
   //
   //  NB, `radius >= 146` is assumed

   uint8_t slow_speed = (uint8_t)( (((uint16_t)speed)*(radius - 146)) / ((uint16_t)radius) );


   if(turn_dir == LEFT){
      _tween_motor_speed(&l_motor_tween, slow_speed, dir);
      _tween_motor_speed(&r_motor_tween,      speed, dir);
   }else{
      _tween_motor_speed(&l_motor_tween,      speed, dir);
      _tween_motor_speed(&r_motor_tween, slow_speed, dir);
   }

   current_timeout = timeout;
}

void motors_rotate(uint8_t speed, motor_dir_t dummy1, motor_turn_dir_t dir, uint16_t dummy2, uint16_t timeout){

   UNUSED_PARAM(dummy1);
   UNUSED_PARAM(dummy2);

   if(dir == LEFT){
      _tween_motor_speed(&l_motor_tween, speed, REV);
      _tween_motor_speed(&r_motor_tween, speed, FWD);
   }else{
      _tween_motor_speed(&l_motor_tween, speed, FWD);
      _tween_motor_speed(&r_motor_tween, speed, REV);
   }

   current_timeout = timeout;
}

bool motors_movement_in_progress(){
   return current_timeout != 0;
}

void motors_hard_stop(){
#ifdef __AVR__
   // Turn all motors off, and set them non-tweening
   l_motor_tween.curr_speed = 0;
   l_motor_tween.curr_dir   = 0;
   l_motor_tween.targ_speed = 0;
   l_motor_tween.targ_dir   = 0;
   l_motor_tween.complete   = true;
   SET_L_MOTOR_SPEED(0);
   SET_L_MOTOR_DIR(FWD);

   r_motor_tween.curr_speed = 0;
   r_motor_tween.curr_dir   = 0;
   r_motor_tween.targ_speed = 0;
   r_motor_tween.targ_dir   = 0;
   r_motor_tween.complete   = true;
   SET_R_MOTOR_SPEED(0);
   SET_R_MOTOR_DIR(FWD);
#endif
}


#ifndef __AVR__

#include <stdio.h>
#include <assert.h>

int main(int argc, const char *argv[])
{

   printf("Testing Motor Tween!\n");


   motor_tween_t test;

   test.curr_speed = 0;
   test.curr_dir   = FWD;
   test.targ_speed = 0;
   test.targ_dir   = FWD;
   test.complete   = true;

   _tween_motor_speed(&test, 16, FWD);
   while(!test.complete){
      _update_tween(&test);
      // printf("  %d %s\n", test.curr_speed, test.curr_dir == FWD ? "FWD" : "REV");
   }
   assert(test.curr_speed == 16);
   assert(test.curr_dir   == FWD);

   _tween_motor_speed(&test, 16, REV);
   while(!test.complete){
      _update_tween(&test);
      //printf("  %d %s\n", test.curr_speed, test.curr_dir == FWD ? "FWD" : "REV");
   }
   assert(test.curr_speed == 16);
   assert(test.curr_dir   == REV);

   _tween_motor_speed(&test, 8, FWD);
   while(!test.complete){
      _update_tween(&test);
      // printf("  %d %s\n", test.curr_speed, test.curr_dir == FWD ? "FWD" : "REV");
   }
   assert(test.curr_speed == 8);
   assert(test.curr_dir   == FWD);

   printf("All tests passed!\n");

   printf("Testing turn in arc math\n");

   uint16_t radius = 457;
   uint8_t speed   = 255;

   uint8_t slow_speed = (uint8_t)( (((uint16_t)speed)*(radius - 146)) / ((uint16_t)radius) );

   assert(slow_speed > 0);

   printf("All tests passed! slow_speed = %d\n", slow_speed);

   return 0;
}


#endif
