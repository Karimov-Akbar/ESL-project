#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "nrf_gpio.h"

#define LED_1_GREEN     NRF_GPIO_PIN_MAP(0, 6)
#define LED_2_RED       NRF_GPIO_PIN_MAP(0, 8)
#define LED_2_GREEN     NRF_GPIO_PIN_MAP(1, 9)
#define LED_2_BLUE      NRF_GPIO_PIN_MAP(0, 12)
#define BUTTON_PIN      NRF_GPIO_PIN_MAP(1, 6)

#define DEVICE_ID       7205
#define DEFAULT_HUE_PERCENT  (DEVICE_ID % 100)
#define PWM_TOP_VALUE   1000

#define DOUBLE_CLICK_TIMEOUT_MS  400
#define DEBOUNCE_MS              50
#define MODE_BLINK_SLOW_MS       1000
#define MODE_BLINK_FAST_MS       200
#define VALUE_CHANGE_INTERVAL_MS 50

typedef enum {
    MODE_NO_INPUT = 0,
    MODE_HUE,
    MODE_SATURATION,
    MODE_BRIGHTNESS
} input_mode_t;

typedef struct {
    uint16_t h;
    uint8_t s;
    uint8_t v;
} hsv_color_t;

#endif