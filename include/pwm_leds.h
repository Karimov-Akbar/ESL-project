#ifndef PWM_LEDS_H
#define PWM_LEDS_H

#include <stdint.h>
#include "app_config.h"

void pwm_rgb_init(void);
void pwm_indicator_init(void);
void pwm_set_rgb_values(uint16_t r, uint16_t g, uint16_t b);
void pwm_set_indicator_value(uint16_t value);

#endif