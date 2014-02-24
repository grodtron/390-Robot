#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "../include/iodefs.h"
#include "../include/swap.h"

iodef_t io;

void iodefs_switch_direction(){
   REG_SWAP(io.fl_motor_port, io.br_motor_port)
   REG_SWAP(io.fl_motor_ddr,  io.br_motor_ddr)
   UINT8_SWAP(io.fl_motor_mask, io.br_motor_mask)

   REG_SWAP(io.bl_motor_port, io.fr_motor_port)
   REG_SWAP(io.bl_motor_ddr,  io.fr_motor_ddr)
   UINT8_SWAP(io.bl_motor_mask, io.fr_motor_mask)

   REG_SWAP(io.lpwm_motor_ddr, io.rpwm_motor_ddr)
   REG_SWAP(io.lpwm_motor_reg_l, io.rpwm_motor_reg_l)
   REG_SWAP(io.lpwm_motor_reg_h, io.rpwm_motor_reg_h)
   UINT8_SWAP(io.lpwm_motor_mask, io.rpwm_motor_mask)

   UINT8_SWAP(io.linesens_fl, io.linesens_br)
   UINT8_SWAP(io.linesens_fr, io.linesens_bl)

   REG_SWAP(io.led_fl_ddr, io.led_br_ddr)
   REG_SWAP(io.led_fr_ddr, io.led_bl_ddr)
   REG_SWAP(io.led_l_ddr, io.led_r_ddr)

   REG_SWAP(io.led_fl_port, io.led_br_port)
   REG_SWAP(io.led_fr_port, io.led_bl_port)
   REG_SWAP(io.led_l_port, io.led_r_port)

   UINT8_SWAP(io.led_fl_mask, io.led_br_mask)
   UINT8_SWAP(io.led_fr_mask, io.led_bl_mask)
   UINT8_SWAP(io.led_r_mask, io.led_l_mask)
}

void iodefs_init(){
   io.fl_motor_port = &PORTC;
   io.fl_motor_ddr  = &DDRC;
   io.fl_motor_mask = 1 << PC3;

   io.bl_motor_port = &PORTC;
   io.bl_motor_ddr  = &DDRC;
   io.bl_motor_mask = 1 << PC2;

   io.fr_motor_port = &PORTC;
   io.fr_motor_ddr  = &DDRC;
   io.fr_motor_mask = 1 << PC1;

   io.br_motor_port = &PORTC;
   io.br_motor_ddr  = &DDRC;
   io.br_motor_mask = 1 << PC0;

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
   io.linesens_bl = 1 << PC6;
   io.linesens_br = 1 << PC7;


   io.led_fl_ddr = &DDRD;
   io.led_fr_ddr = &DDRA;
   io.led_l_ddr  = NULL;
   io.led_br_ddr = &DDRA;
   io.led_bl_ddr = &DDRD;
   io.led_r_ddr  = NULL;

   io.led_fl_port = &PORTD;
   io.led_fr_port = &PORTA;
   io.led_l_port  = NULL;
   io.led_br_port = &PORTA;
   io.led_bl_port = &PORTD;
   io.led_r_port  = NULL;

   io.led_fl_mask = 1 << PD6;
   io.led_fr_mask = 1 << PA6;
   io.led_l_mask  = 0;
   io.led_br_mask = 1 << PA7;
   io.led_bl_mask = 1 << PD7;
   io.led_r_mask  = 0;

}
