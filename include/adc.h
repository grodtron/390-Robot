#ifndef ADC_H_
#define ADC_H_

typedef enum {
   LEFT_LINE_SENSOR  = 0,
   RIGHT_LINE_SENSOR = 1,
   LEFT_PROX_SENSOR  = 2,
   RIGHT_PROX_SENSOR = 3
} sensor_t;

typedef enum {
   LINE_LEFT  = 1,
   LINE_RIGHT = 2,
   LINE_BOTH  = 1 | 2,
   LINE_NONE  = 0
} line_dir_t;

void adc_init();

// NB - assume init_adc called already
void adc_start();

line_dir_t adc_where_is_line();


#endif
