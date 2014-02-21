#include <avr/io.h>

#include "../include/iodefs.h"

iodef_t io;

void iodefs_init(){
   io.lf_motor_port = &PORTC;
   io.lf_motor_ddr  = &DDRC;
   io.lf_motor_mask = 1 << PC3;

   io.lr_motor_port = &PORTC;
   io.lr_motor_ddr  = &DDRC;
   io.lr_motor_mask = 1 << PC2;

   io.rf_motor_port = &PORTC;
   io.rf_motor_ddr  = &DDRC;
   io.rf_motor_mask = 1 << PC1;

   io.rr_motor_port = &PORTC;
   io.rr_motor_ddr  = &DDRC;
   io.rr_motor_mask = 1 << PC0;

   io.lpwm_motor_ddr    = &DDRD;
   io.lpwm_motor_reg_l  = &OCR1AL;
   io.lpwm_motor_reg_h  = &OCR1AH;
   io.lpwm_motor_mask   = 1 << PD5;

   io.rpwm_motor_ddr    = &DDRD;
   io.rpwm_motor_reg_l  = &OCR1BL;
   io.rpwm_motor_reg_h  = &OCR1BH;
   io.rpwm_motor_mask   = 1 << PD4;
}
