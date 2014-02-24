#ifndef ADC_H_
#define ADC_H_

#define N_SENSORS 6

typedef enum {
   FR_SENSOR = 0,
   FL_SENSOR = 1,
   L_SENSOR  = 2,
   BL_SENSOR = 3,
   BR_SENSOR = 4,
   R_SENSOR  = 5
} sensor_t;


void adc_init();

// NB - assume init_adc called already
void adc_start();

void adc_switch_direction();

extern uint8_t adc_sensor_readings[N_SENSORS];

#endif
