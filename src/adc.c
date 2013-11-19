#include <inttypes.h>
#include <avr/interrupt.h>

typedef enum {
   LEFT_LINE_SENSOR  = 0,
   RIGHT_LINE_SENSOR = 1,
   LEFT_PROX_SENSOR  = 2,
   RIGHT_PROX_SENSOR = 3
} sensor_t;

static sensor_t current_sensor;

#define CURRENT_SENSOR_IS_LINE_SENSOR() (current_sensor <= RIGHT_LINE_SENSOR)

static uint8_t sensor_channel[4];
   sensor_channel[LEFT_LINE_SENSOR]  = 1;
   sensor_channel[RIGHT_LINE_SENSOR] = 0;
   // TODO - determine which is which
   sensor_channel[LEFT_PROX_SENSOR]  = 4;
   sensor_channel[RIGHT_PROX_SENSOR] = 3;

static uint8_t sensor_readings[4] = {0,0,0,0};

void init_adc(){

   ADMUX  = (0 << REFS1)  | (0 << REFS0) // External AREF (page 206)
          | (1 << ADLAR)                 // Left adjust the result, we only need 8 bits really
          ;

   ADCSRA |= (1 << ADEN) // enable ADC   (page 207)
          |  (0 << ADFR) // NOT free running
          |  (1 << ADIE) // enable interrupt (page 207)
          // For the prescaler selection, our IR range sensors are basically
          // irrelevant, since they have a measurement period of 40ms. We want
          // to poll our line sensors at a fast rate though.
          //
          // Each reading takes 13 ADC clock cycles, and we have 2 line sensors
          // This means that if we neglect the time spent in the ISR we can do
          // complete polls at F_ADC/(13*2)
          //
          // Let F_ADC = F_CPU/ADC_PS, then we poll at 1MHz/(ADC_PS * 13 * 2)
          // (~48kHz)
          // Assume the insane top speed of 2m/s for our robot. From the above
          // we have 1Mpolls/(ADC_PS * 26)s. Combining the two gives:
          //
          // 1Mpoll/(ADC_PS * 26 * 2)m = 1,000,000 polls / (ADC_PS * 52) meters
          //
          // If we want to poll at around 1mm/poll, then:
          //
          // 1e-3   = (ADC_PS * 52) / 1,000,000
          // 1e3/52 = ADC_PS
          // 19.23  = ADC_PS
          //
          // We will therefore undershoot slightly and take a prescaler of 16, which
          // will give us a poll rate of >1 poll/mm at 2m/s, which is more than sufficient
          //
          | (1 << ADPS2) | (0 << ADPS1) | (0 << ADPS0) // value (128) (page 208)
          ;
          // Based on this value, our actual ADC period is:
          //  (1MHz/(13*16))**-1 == 0.208ms (~4.8kHz)
          //
          // The ADC clock frequency is 62.5kHz

   // TODO - business logic initialization
   
   // Woohoo, ADC init'd

}

// NB - assume init_adc called already
void start_adc(){
   current_sensor = LEFT_LINE_SENSOR;

   ADMUX  |= (sensor_channel[current_sensor] << MUX0); // ADC mux channel select   (page 206)

   ADCSRA |= (1 << ADSC);
}


// The basic idea is that we toggle polling between our two line sensors,
// as well as keeping track of the elapsed time so that we can also grab
// our proximity sensors when they refresh.
ISR(ADC_vect){

   // Each call of this ISR represents the passage of 0.208ms
   static uint8_t n_ticks = 0;
   ++n_ticks;

   uint8_t reading = ADCH; // We only read the high 8 bits (see page 208)

   if(CURRENT_SENSOR_IS_LINE_SENSOR()){

      if(reading < threshold && sensor_readings[current_sensor] >= threshold){
         // Then we hit the edge!! Panic!! Ahhhhjhh!!
         motors_hard_stop();
         event_q_add_event(EDGE_DETECTED);
      }

      if(n_ticks < 193){
         // We just toggle between our two line sensors
         if(current_sensor == LEFT_LINE_SENSOR){
            current_sensor = RIGHT_LINE_SENSOR;
         }else{
            current_sensor = RIGHT_LINE_SENSOR;
         }
      }else{
         // 193*0.208==40.144ms have passed, let's check out our IR sensors
         n_ticks = 0;
         current_sensor = LEFT_PROX_SENSOR;
      }
   }else{
      // We always read left ot right
      if(current_sensor == LEFT_PROX_SENSOR){
         current_sensor =  RIGHT_PROX_SENSOR;
      }else{
         // We don't actually know which line sensor we should
         // be on, but we also don't really care, it doesn't make
         // any difference really.
         current_sensor = LEFT_LINE_SENSOR;
         // Signal to the main loop that a new reading is available
         event_q_add_event(NEW_PROXIMITY_READINGS);
      }
   }

   // Store the reading for retrieval later by the main loop and possibly by us
   sensor_readings[current_sensor] = reading;

   // Change the channel to the new sensor
   ADMUX  |= (sensor_channel[current_sensor] << MUX0);

   // Start the next conversion
   ADCSRA |= (1 << ADSC);

}

