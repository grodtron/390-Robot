#ifndef IODEFS_H_
#define IODEFS_H_

#include <stdint.h>

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
   

} iodef_t;

extern iodef_t io;

void iodefs_init();

#endif
