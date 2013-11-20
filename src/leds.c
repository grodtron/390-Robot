/*
 * leds.c
 *
 * Created: 19/09/2013 12:07:28 PM
 *  Author: Gordon
 */

#ifdef __AVR__
#include <avr/io.h>
#endif

#include "../include/leds.h"

//TODO - determine which is which

// Macros, woah
#define _PASTE2(a, b) a ## b
#define _PASTE3(a, b, c) a ## b ## c
#define PASTE2(a,  b) _PASTE2(a, b)
#define PASTE3(a,  b, c) _PASTE3(a, b, c)

#define RED_REG C
#define GRN_REG C
#define YLW_REG D

#define RED_PIN 2
#define GRN_PIN 5
#define YLW_PIN 3

#define RED_NAME red
#define GRN_NAME green
#define YLW_NAME yellow

#define DDR(color)  PASTE2(DDR, PASTE2(color, _REG))
#define DDXN(color) PASTE3(DD, PASTE2(color, _REG), PASTE2(color, _PIN))

#define PORT(color) PASTE2(PORT, PASTE2(color, _REG))
#define PXN(color)  PASTE3(P, PASTE2(color, _REG), PASTE2(color, _PIN))

#define DEFINE_TOGGLE(color) \
void PASTE2(led_toggle_, PASTE2(color, _NAME)) () { \
   PORT(color) ^= (1 << PXN(color)); \
}

#define DEFINE_SET(color) \
void PASTE2(led_set_, PASTE2(color, _NAME)) (uint8_t val) { \
   if(val){ \
      PORT(color) |=  (1 << PXN(color)); \
   }else{ \
      PORT(color) &= ~(1 << PXN(color)); \
   } \
}


void leds_init(){
   DDR(RED) |= (1 << DDXN(RED));
   DDR(GRN) |= (1 << DDXN(GRN));
   DDR(YLW) |= (1 << DDXN(YLW));
}

DEFINE_TOGGLE(RED)
DEFINE_TOGGLE(YLW)
DEFINE_TOGGLE(GRN)

DEFINE_SET(RED)
DEFINE_SET(YLW)
DEFINE_SET(GRN)

void led_set_rgy(uint8_t r, uint8_t g, uint8_t y){
   led_set_red(r);
   led_set_green(g);
   led_set_yellow(y);
}
