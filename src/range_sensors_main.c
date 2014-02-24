#include <avr/interrupt.h>
#include <avr/io.h>
#define F_CPU 1000000UL
#include <util/delay.h>

#include "../include/event_queue.h"
#include "../include/adc.h"
#include "../include/iodefs.h"
#include "../include/leds.h"

static void handle_new_prox(){

   sensor_t sensor = 0;
   uint8_t max = 0;
   sensor_t max_sensor = R_SENSOR;
   for(sensor = 0; sensor < N_SENSORS; ++sensor){
      if(adc_sensor_readings[sensor] > max){
         max = adc_sensor_readings[sensor];
         max_sensor = sensor;
      }
   }

   led_set_fl(max_sensor == FL_SENSOR || max_sensor == L_SENSOR);
   led_set_fr(max_sensor == FR_SENSOR || max_sensor == R_SENSOR);
   led_set_bl(max_sensor == BL_SENSOR || max_sensor == L_SENSOR);
   led_set_br(max_sensor == BR_SENSOR || max_sensor == R_SENSOR);
}

void range_sensors_main()
{

   iodefs_init();

   leds_init();
   adc_init();
   event_q_init();

   sei();

   adc_start();

   while(1){

      // Testing in the lab showed this runs every ~95us

      event_t e = event_q_get_next_event();

      switch(e){

         case NEW_PROXIMITY_READINGS:
            handle_new_prox();
            break;
         default:
            break;

      }

   }
}
