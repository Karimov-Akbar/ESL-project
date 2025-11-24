#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include "nrfx_gpiote.h"

typedef void (*button_handler_t)(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

void button_init(button_handler_t handler);

#endif