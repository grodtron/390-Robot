#ifndef IODEFS_H_
#define IODEFS_H_

#include <stdint.h>

#define IO_LINESENS_DDR   DDRC
#define IO_LINESENS_PORT  PORTC
#define IO_LINESENS_PIN   PINC
#define IO_LINESENS_MASK  ((1<<PC4)|(1<<PC5)|(1<<PC6)|(1<<PC7))

typedef struct {
   volatile uint8_t * lf_motor_port;
   volatile uint8_t * lf_motor_ddr;
            uint8_t   lf_motor_mask;

   volatile uint8_t * lr_motor_port;
   volatile uint8_t * lr_motor_ddr;
            uint8_t   lr_motor_mask;

   volatile uint8_t * rf_motor_port;
   volatile uint8_t * rf_motor_ddr;
            uint8_t   rf_motor_mask;

   volatile uint8_t * rr_motor_port;
   volatile uint8_t * rr_motor_ddr;
            uint8_t   rr_motor_mask;

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
            uint8_t   linesens_rr;
            uint8_t   linesens_rl;



} iodef_t;

extern iodef_t io;

void iodefs_switch_direction();

void iodefs_init();

#endif
