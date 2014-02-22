#include <stdint.h>

#include <avr/io.h>

#define F_CPU 1000000UL
#include <util/delay.h>

// no point in making headers for these
void robot_main();
void line_sensors_main();
void motors_main();
void ring_robot_main();

int main()
{
   // Disable jtag! must write this bit twice within four cycles...
   // page 267, Atmega644 datasheet
   uint8_t fuck_jtag = MCUCR | (1<<JTD);
   MCUCR = fuck_jtag;
   MCUCR = fuck_jtag;

   // Read the operating mode off the dip switches
   uint8_t mode = (~PINB) & 0x0F;

   // Execute the corresponding program
   switch(mode){

      case 0:
         // TODO robot_main();
      case 1:
         line_sensors_main();
      case 2:
         motors_main();
      case 3:
         ring_robot_main();
      default:
         break;
   }


   return 0;
}
