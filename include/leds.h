#ifndef LEDS_H_
#define LEDS_H_

void leds_init();

void led_toggle_red ();
void led_toggle_yellow ();
void led_toggle_green ();

void led_set_red (uint8_t val);
void led_set_yellow (uint8_t val);
void led_set_green (uint8_t val);

void led_set_rgy(uint8_t r, uint8_t g, uint8_t y);

#endif
