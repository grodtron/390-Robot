#include <inttypes.h>
#include <avr/interrupt.h>

#include "../include/static_assert.h"

#include "../include/adc.h"
#include "../include/leds.h"
#include "../include/event_queue.h"
#include "../include/motors.h"

#include "../include/swap.h"

#include "../include/leds.h"

static uint8_t count = 0;
static sensor_t current_sensor;


static uint8_t sensor_channel[N_SENSORS];

uint8_t adc_sensor_readings[N_SENSORS] = {0,0,0,0,0,0};

static const uint8_t ADMUX_MASK = ~(  (1 << MUX0) | (1 << MUX1) | (1 << MUX2)  );

void adc_init(){

   static_assert(FR_SENSOR < N_SENSORS);
   static_assert(FR_SENSOR >= 0);
   static_assert(FL_SENSOR < N_SENSORS);
   static_assert(FL_SENSOR >= 0);
   static_assert( L_SENSOR < N_SENSORS);
   static_assert( L_SENSOR >= 0);
   static_assert(BL_SENSOR < N_SENSORS);
   static_assert(BL_SENSOR >= 0);
   static_assert(BR_SENSOR < N_SENSORS);
   static_assert(BR_SENSOR >= 0);
   static_assert( R_SENSOR < N_SENSORS);
   static_assert( R_SENSOR >= 0);

   // Assignment based on physical connections
   sensor_channel[FR_SENSOR] = 1;
   sensor_channel[FL_SENSOR] = 0;
   sensor_channel[L_SENSOR ] = 4;
   sensor_channel[BL_SENSOR] = 2;
   sensor_channel[BR_SENSOR] = 3;
   sensor_channel[R_SENSOR ] = 5;

   // ADMUX  = (0 << REFS1)  | (0 << REFS0) // External AREF (page 248)
   ADMUX  = (1 << REFS1)  | (1 << REFS0) // Internal 2.56 reference (page 248)
          | (1 << ADLAR)                 // Left adjust the result, we only need 8 bits really
          ;

   ADCSRA |= (1 << ADEN) // enable ADC   (page 250)
          |  (0 << ADATE) // NOT auto trigger enabled (page 250)
          |  (1 << ADIE) // enable interrupt (page 207)

          // We just need to read quickly enough that we see every sensor at least
          // once every 40ms or so. We have 6 sensors, so 40ms/6 ~= 6.6ms.
          //
          // So our measurement period should be around that.
          //
          // Each reading takes 13 ADC clock cycles. This means we should have
          // 13*6 = 78 clock cycles taking approximately 40ms. If each cycle takes
          // around half a ms, then this works out to around 39 ms per read cycle
          //
          // The highest available prescaler is 128, which gives one cycle per 128us
          // (f_cpu=1MHz)
          //
          // So total time to sweep all 6 sensors will be:
          //
          //   6*13*128e-6 = 0.009984
          //
          // So approximately 10ms to sweep sensors. Best thing should be to average over
          // 4 readings, but take the most recent one if it's significantly different...
          //
          // Should it moving towards have an effect? Like if over the course of the four
          // readings it jumps closer?


          | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) // value (128) (page 250)
          ;

   // Woohoo, ADC init'd

}

// NB - assume init_adc called already
void adc_start(){
   count = 0;
   current_sensor = FR_SENSOR;

   ADMUX  |= (sensor_channel[current_sensor] << MUX0); // ADC mux channel select   (page 206)

   ADCSRA |= (1 << ADSC);

}

void adc_switch_direction(){

   UINT8_SWAP(sensor_channel[FR_SENSOR], sensor_channel[BL_SENSOR]);
   UINT8_SWAP(sensor_channel[FL_SENSOR], sensor_channel[BR_SENSOR]);
   UINT8_SWAP(sensor_channel[ L_SENSOR], sensor_channel[ R_SENSOR]);

   switch(current_sensor){
      case FR_SENSOR:
         current_sensor = BL_SENSOR;
         break;
      case FL_SENSOR:
         current_sensor = BR_SENSOR;
         break;
      case L_SENSOR:
         current_sensor = R_SENSOR;
         break;
      case BL_SENSOR:
         current_sensor = FR_SENSOR;
         break;
      case BR_SENSOR:
         current_sensor = FL_SENSOR;
         break;
      case R_SENSOR:
         current_sensor = L_SENSOR;
         break;
   }

   // Set a high count so that the main loop gets updated sensor reading quickly.
   // By setting it to 2, we ensure atleast a full sweep of all sensors is done
   // (if we set it to 3, then unless the current sensor is the first sensor in
   // the rotation, we wouldn't get a full sweep).
   count = 2;

}

// The basic idea is that we toggle polling between our two line sensors,
// as well as keeping track of the elapsed time so that we can also grab
// our proximity sensors when they refresh.
ISR(ADC_vect){

   // NB that this interrupt was conflicting with the timer interrupt for the
   // line sensors
   sei();

   uint8_t reading = ADCH; // Read only the high bits

   // Store it on the last pass through, this way we always get the latest reading
   // could do averaging or something like that, but it just feels like it's not worth
   // it / necessary. Besides, may as well keep these ISRs small.
   //
   // Even though it seems shitty to throw out a bunch of readings... (TODO)
   if(count == 3){
      adc_sensor_readings[current_sensor]  = reading;
   }

   // Notify that there is a new reading available
   if(current_sensor == R_SENSOR){
      ++count;
      if(count == 4){
         cli();
         event_q_add_event(NEW_PROXIMITY_READINGS);
         sei();
         count = 0;
      }
   }

   // Update the current sensor
   current_sensor = (current_sensor + 1) % N_SENSORS;

   // Change the channel to the new sensor
   ADMUX  &= ADMUX_MASK;
   ADMUX  |= (sensor_channel[current_sensor] << MUX0);

   // Start the next conversion
   ADCSRA |= (1 << ADSC);
}

