#include <avr/io.h>
#include <avr/interrupt.h>

#include "../include/iodefs.h"
#include "../include/swap.h"

iodef_t io;

void iodefs_switch_direction(){
   REG_SWAP(io.lf_motor_port, io.rr_motor_port)
   REG_SWAP(io.lf_motor_ddr,  io.rr_motor_ddr)
   UINT8_SWAP(io.lf_motor_mask, io.rr_motor_mask)

   REG_SWAP(io.lr_motor_port, io.rf_motor_port)
   REG_SWAP(io.lr_motor_ddr,  io.rf_motor_ddr)
   UINT8_SWAP(io.lr_motor_mask, io.rf_motor_mask)

   REG_SWAP(io.lpwm_motor_ddr, io.rpwm_motor_ddr)
   REG_SWAP(io.lpwm_motor_reg_l, io.rpwm_motor_reg_l)
   REG_SWAP(io.lpwm_motor_reg_h, io.rpwm_motor_reg_h)
   UINT8_SWAP(io.lpwm_motor_mask, io.rpwm_motor_mask)

   UINT8_SWAP(io.linesens_fl, io.linesens_rr)
   UINT8_SWAP(io.linesens_fr, io.linesens_rl)
}

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

   io.linesens_fl = 1 << PC4;
   io.linesens_fr = 1 << PC5;
   io.linesens_rl = 1 << PC6;
   io.linesens_rr = 1 << PC7;
}
