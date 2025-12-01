#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "nrfx_gpiote.h"
#include "app_config.h"
#include "hsv.h"
#include "pwm_leds.h"
#include "button.h"
#include "storage.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrfx_clock.h"

static volatile input_mode_t current_mode = MODE_NO_INPUT;
static hsv_color_t current_hsv;
static volatile uint32_t last_button_time = 0;
static volatile uint32_t first_click_time = 0;
static volatile bool waiting_for_second_click = false;
static volatile uint32_t system_ticks = 0;
static uint32_t last_mode_blink_time = 0;
static uint32_t last_value_change_time = 0;
static bool mode_led_state = false;
static volatile bool m_switch_mode_pending = false;

static uint32_t millis(void)
{
    return system_ticks;
}

static void update_rgb_led(void)
{
    uint16_t r, g, b;
    hsv_to_rgb_simple(current_hsv.h, current_hsv.s, current_hsv.v, &r, &g, &b);
    pwm_set_rgb_values(r, g, b);
}

static void update_mode_indicator(void)
{
    uint32_t current_time = millis();
    uint32_t blink_interval;
    
    switch (current_mode) {
        case MODE_NO_INPUT:
            pwm_set_indicator_value(0);
            break;
            
        case MODE_HUE:
            blink_interval = MODE_BLINK_SLOW_MS;
            if (current_time - last_mode_blink_time >= blink_interval) {
                mode_led_state = !mode_led_state;
                pwm_set_indicator_value(mode_led_state ? PWM_TOP_VALUE : 0);
                last_mode_blink_time = current_time;
            }
            break;
            
        case MODE_SATURATION:
            blink_interval = MODE_BLINK_FAST_MS;
            if (current_time - last_mode_blink_time >= blink_interval) {
                mode_led_state = !mode_led_state;
                pwm_set_indicator_value(mode_led_state ? PWM_TOP_VALUE : 0);
                last_mode_blink_time = current_time;
            }
            break;
            
        case MODE_BRIGHTNESS:
            pwm_set_indicator_value(PWM_TOP_VALUE);
            break;
    }
}

static void switch_to_next_mode(void)
{
    current_mode = (input_mode_t)((current_mode + 1) % 4);
    
    NRF_LOG_INFO("Switched to Mode: %d", current_mode);

    if (current_mode == MODE_NO_INPUT) {
        NRF_LOG_INFO("Saving to Flash (addr: 0x%X)...", FLASH_STORAGE_ADDR);
        
        NRF_LOG_FLUSH();
        nrf_delay_ms(10); 
        
        storage_save_hsv(&current_hsv);
        
        NRF_LOG_INFO("Saved successfully!");
    }
    
    last_mode_blink_time = millis();
    mode_led_state = false;
    update_mode_indicator();
}

static void handle_value_change(void)
{
    bool is_button_pressed = (nrf_gpio_pin_read(BUTTON_PIN) == 0);
    
    if (!is_button_pressed || current_mode == MODE_NO_INPUT) {
        return;
    }
    
    uint32_t current_time = millis();
    if (current_time - last_value_change_time < VALUE_CHANGE_INTERVAL_MS) {
        return;
    }
    last_value_change_time = current_time;
    
    switch (current_mode) {
        case MODE_HUE:
            current_hsv.h = (current_hsv.h + 1) % 360;
            break;
        case MODE_SATURATION:
            current_hsv.s = (current_hsv.s + 1) % 101;
            break;
        case MODE_BRIGHTNESS:
            current_hsv.v = (current_hsv.v + 1) % 101;
            break;
        default:
            break;
    }
    
    update_rgb_led();
    
    if (current_hsv.h % 10 == 0) { 
        NRF_LOG_INFO("HSV: %d, %d, %d", current_hsv.h, current_hsv.s, current_hsv.v);
    }
}

void button_event_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if (pin != BUTTON_PIN) return;
    
    uint32_t current_time = millis();
    
    if (current_time - last_button_time < DEBOUNCE_MS) {
        return;
    }
    last_button_time = current_time;
    
    if (waiting_for_second_click) {
        if (current_time - first_click_time < DOUBLE_CLICK_TIMEOUT_MS) {
            waiting_for_second_click = false;
            m_switch_mode_pending = true;
        } else {
            first_click_time = current_time;
            waiting_for_second_click = true;
        }
    } else {
        first_click_time = current_time;
        waiting_for_second_click = true;
    }
}

static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

int main(void)
{
    ret_code_t clock_ret = nrfx_clock_init(NULL);
    if (clock_ret != NRFX_SUCCESS && clock_ret != NRFX_ERROR_INVALID_STATE)
    {
        APP_ERROR_CHECK(clock_ret);
    }

    nrfx_clock_lfclk_start();

    log_init();
    NRF_LOG_INFO("System Started!");

    pwm_rgb_init();
    pwm_indicator_init();
    button_init(button_event_handler);
    
    if (storage_read_hsv(&current_hsv)) {
        NRF_LOG_INFO("Restored from Flash.");
    } else {
        NRF_LOG_INFO("First run. Using default.");
        current_hsv.h = (DEFAULT_HUE_PERCENT * 360) / 100;
        current_hsv.s = 100;
        current_hsv.v = 100;
    }
    
    update_rgb_led();
    
    pwm_set_rgb_values(PWM_TOP_VALUE, PWM_TOP_VALUE, PWM_TOP_VALUE);
    pwm_set_indicator_value(PWM_TOP_VALUE);
    nrf_delay_ms(200);
    
    update_rgb_led();
    pwm_set_indicator_value(0);
    
    system_ticks = 0;
    
    while (true) {
        NRF_LOG_PROCESS();

        if (m_switch_mode_pending) {
            m_switch_mode_pending = false;
            switch_to_next_mode();
        }

        update_mode_indicator();
        handle_value_change();
        
        if (waiting_for_second_click) {
            uint32_t current_time = millis();
            if (current_time - first_click_time >= DOUBLE_CLICK_TIMEOUT_MS) {
                waiting_for_second_click = false;
            }
        }
        
        nrf_delay_ms(1);
        system_ticks++;
    }
}