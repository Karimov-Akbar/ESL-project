#include "nrf_delay.h"
#include <stdbool.h>
#include "nrf_gpio.h"

#define LED_PINS_COUNT      4 

#define LED_RED_PIN         NRF_GPIO_PIN_MAP(0, 6) 
#define LED_GREEN_PIN       NRF_GPIO_PIN_MAP(0, 8) 
#define LED_BLUE_PIN        NRF_GPIO_PIN_MAP(1, 9) 
#define LED_SECONDARY_RED   NRF_GPIO_PIN_MAP(0, 12) 

#define BUTTON_1_PIN        NRF_GPIO_PIN_MAP(1, 6) 

#define LONG_PRESS_THRESHOLD_MS 500 
#define DEBOUNCE_TIME_MS        50 
#define TICK_INTERVAL_MS        5 

static bool is_green_on = false;
static bool button_is_down = false;
static uint32_t press_duration_ms = 0;
static uint32_t blue_red_blink_state = 0;

void my_led_on(uint32_t pin)
{
    nrf_gpio_pin_clear(pin);
}

void my_led_off(uint32_t pin)
{
    nrf_gpio_pin_set(pin);
}

void gpio_init()
{
    uint32_t led_pins[] = {LED_RED_PIN, LED_GREEN_PIN, LED_BLUE_PIN, LED_SECONDARY_RED};
    for (int i = 0; i < LED_PINS_COUNT; i++)
    {
        nrf_gpio_cfg_output(led_pins[i]); 
        nrf_gpio_pin_set(led_pins[i]);
    }

    nrf_gpio_cfg_input(BUTTON_1_PIN, NRF_GPIO_PIN_PULLUP);
}

void handle_short_click()
{
    blue_red_blink_state = 0; 
    my_led_off(LED_BLUE_PIN);
    my_led_off(LED_SECONDARY_RED);

    if (is_green_on)
    {
        my_led_off(LED_GREEN_PIN);
        is_green_on = false;
    }
    else
    {
        my_led_on(LED_GREEN_PIN);
        is_green_on = true;
    }
}

void handle_long_press()
{
    if (is_green_on)
    {
        my_led_off(LED_GREEN_PIN);
        is_green_on = false;
    }
    
    if (blue_red_blink_state == 0)
    {
        my_led_on(LED_BLUE_PIN);
        my_led_off(LED_SECONDARY_RED);
        blue_red_blink_state = 1;
    }
    else
    {
        my_led_off(LED_BLUE_PIN);
        my_led_on(LED_SECONDARY_RED);
        blue_red_blink_state = 0;
    }
}

int main(void)
{
    gpio_init();

    while (true)
    {
        if (!nrf_gpio_pin_read(BUTTON_1_PIN))
        {
            if (!button_is_down)
            {
                press_duration_ms = 0;
                button_is_down = true;
            }
            else
            {
                press_duration_ms += TICK_INTERVAL_MS;
                
                if (press_duration_ms >= LONG_PRESS_THRESHOLD_MS)
                {
                    handle_long_press();
                    
                    press_duration_ms = 0; 
                }
            }
        }
        else
        {
            if (button_is_down)
            {
                uint32_t press_duration = press_duration_ms;

                if (press_duration < LONG_PRESS_THRESHOLD_MS && press_duration >= DEBOUNCE_TIME_MS)
                {
                    handle_short_click();
                }
                
                press_duration_ms = 0;
                button_is_down = false;
            }
            
            if (blue_red_blink_state != 0)
            {
                blue_red_blink_state = 0;
                my_led_off(LED_BLUE_PIN);
                my_led_off(LED_SECONDARY_RED);
            }
        }
        
        nrf_delay_ms(TICK_INTERVAL_MS);
    }
}