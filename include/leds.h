#ifndef LEDS_H_
#define LEDS_H_

#include <stdint.h>
#include "../include/iodefs.h"

// Macros, woah
#define _PASTE2(a, b) a ## b
#define _PASTE3(a, b, c) a ## b ## c
#define PASTE2(a,  b) _PASTE2(a, b)
#define PASTE3(a,  b, c) _PASTE3(a, b, c)

#define MASK(code) PASTE3(io.led_, code, _mask)
#define DDR(code) PASTE3(io.led_, code, _ddr)
#define PORT(code) PASTE3(io.led_, code, _port)

#define DEFINE_TOGGLE(code) \
inline void PASTE2(led_toggle_, code) () { \
   *PORT(code) ^= MASK(code); \
}

#define DEFINE_SET(code) \
inline void PASTE2(led_set_, code) (uint8_t val) { \
   if(!(val)){ \
      *PORT(code) |=  MASK(code); \
   }else{ \
      *PORT(code) &= ~MASK(code); \
   } \
}

DEFINE_TOGGLE(fr)
DEFINE_TOGGLE(fl)
DEFINE_TOGGLE(l)
DEFINE_TOGGLE(br)
DEFINE_TOGGLE(bl)
DEFINE_TOGGLE(r)
DEFINE_TOGGLE(white)
DEFINE_TOGGLE(orange)

DEFINE_SET(fr)
DEFINE_SET(fl)
DEFINE_SET(l)
DEFINE_SET(br)
DEFINE_SET(bl)
DEFINE_SET(r)
DEFINE_SET(white)
DEFINE_SET(orange)

inline void leds_init(){
   *DDR(fr) |= MASK(fr);
   *PORT(fr) |= MASK(fr);

   *DDR(fl) |= MASK(fl);
   *PORT(fl) |= MASK(fl);

   *DDR(l)  |= MASK(l);
   *PORT(l)  |= MASK(l);

   *DDR(br) |= MASK(br);
   *PORT(br) |= MASK(br);

   *DDR(bl) |= MASK(bl);
   *PORT(bl) |= MASK(bl);

   *DDR(r)  |= MASK(r);
   *PORT(r)  |= MASK(r);

   *DDR(orange) |= MASK(orange);
   *PORT(orange) |= MASK(orange);

   *DDR(white)  |= MASK(white);
   *PORT(white)  |= MASK(white);

}

#undef _PASTE2
#undef _PASTE3
#undef PASTE2
#undef PASTE3
#undef DDR
#undef PORT
#undef MASK
#undef DEFINE_TOGGLE
#undef DEFINE_SET

#endif
