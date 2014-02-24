#ifndef IODEFS_H_
#define IODEFS_H_

#include <stdint.h>

#define IO_LINESENS_DDR   DDRC
#define IO_LINESENS_PORT  PORTC
#define IO_LINESENS_PIN   PINC
#define IO_LINESENS_MASK  ((1<<PC4)|(1<<PC5)|(1<<PC6)|(1<<PC7))

typedef struct {
   volatile uint8_t * fl_motor_port;
   volatile uint8_t * fl_motor_ddr;
            uint8_t   fl_motor_mask;

   volatile uint8_t * bl_motor_port;
   volatile uint8_t * bl_motor_ddr;
            uint8_t   bl_motor_mask;

   volatile uint8_t * fr_motor_port;
   volatile uint8_t * fr_motor_ddr;
            uint8_t   fr_motor_mask;

   volatile uint8_t * br_motor_port;
   volatile uint8_t * br_motor_ddr;
            uint8_t   br_motor_mask;

   volatile uint8_t * lpwm_motor_ddr;
   volatile uint8_t * lpwm_motor_reg_l;
   volatile uint8_t * lpwm_motor_reg_h;
            uint8_t   lpwm_motor_mask;

   volatile uint8_t * rpwm_motor_ddr;
   volatile uint8_t * rpwm_motor_reg_l;
   volatile uint8_t * rpwm_motor_reg_h;
            uint8_t   rpwm_motor_mask;


            uint8_t   linesens_fr;
            uint8_t   linesens_fl;
            uint8_t   linesens_br;
            uint8_t   linesens_bl;


   volatile uint8_t * led_fl_ddr;
   volatile uint8_t * led_fr_ddr;
   volatile uint8_t * led_l_ddr;
   volatile uint8_t * led_br_ddr;
   volatile uint8_t * led_bl_ddr;
   volatile uint8_t * led_r_ddr;

   volatile uint8_t * led_fl_port;
   volatile uint8_t * led_fr_port;
   volatile uint8_t * led_l_port;
   volatile uint8_t * led_br_port;
   volatile uint8_t * led_bl_port;
   volatile uint8_t * led_r_port;

            uint8_t   led_fl_mask;
            uint8_t   led_fr_mask;
            uint8_t   led_l_mask;
            uint8_t   led_bl_mask;
            uint8_t   led_br_mask;
            uint8_t   led_r_mask;


} iodef_t;

extern iodef_t io;

void iodefs_switch_direction();

void iodefs_init();

#endif
