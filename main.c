#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrfx_pwm.h"
#include "nrfx_gpiote.h"
#include <stdbool.h>
#include <stdint.h>

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

static nrfx_pwm_t m_pwm_rgb = NRFX_PWM_INSTANCE(0);
static nrfx_pwm_t m_pwm_indicator = NRFX_PWM_INSTANCE(1);

static nrf_pwm_values_individual_t pwm_rgb_values;
static nrf_pwm_sequence_t const pwm_rgb_seq = {
    .values.p_individual = &pwm_rgb_values,
    .length = NRF_PWM_VALUES_LENGTH(pwm_rgb_values),
    .repeats = 0,
    .end_delay = 0
};

static nrf_pwm_values_individual_t pwm_indicator_values;
static nrf_pwm_sequence_t const pwm_indicator_seq = {
    .values.p_individual = &pwm_indicator_values,
    .length = NRF_PWM_VALUES_LENGTH(pwm_indicator_values),
    .repeats = 0,
    .end_delay = 0
};

static volatile input_mode_t current_mode = MODE_NO_INPUT;
static hsv_color_t current_hsv;
static volatile uint32_t last_button_time = 0;
static volatile uint32_t first_click_time = 0;
static volatile bool waiting_for_second_click = false;
static volatile uint32_t system_ticks = 0;
static uint32_t last_mode_blink_time = 0;
static uint32_t last_value_change_time = 0;
static bool mode_led_state = false;

static uint32_t millis(void)
{
    return system_ticks;
}

static void hsv_to_rgb_simple(uint16_t h, uint8_t s, uint8_t v, uint16_t *r, uint16_t *g, uint16_t *b)
{
    h = h % 360;
    
    if (s == 0) {
        *r = *g = *b = (v * PWM_TOP_VALUE) / 100;
        return;
    }
    
    uint8_t sector = h / 60;
    uint16_t remainder = (h % 60) * 6;
    
    uint32_t p = ((uint32_t)v * (100 - s)) / 100;
    uint32_t q = ((uint32_t)v * (100 - ((s * remainder) / 360))) / 100;
    uint32_t t = ((uint32_t)v * (100 - ((s * (360 - remainder)) / 360))) / 100;
    
    uint16_t v_pwm = (v * PWM_TOP_VALUE) / 100;
    uint16_t p_pwm = (p * PWM_TOP_VALUE) / 100;
    uint16_t q_pwm = (q * PWM_TOP_VALUE) / 100;
    uint16_t t_pwm = (t * PWM_TOP_VALUE) / 100;
    
    switch(sector) {
        case 0:
            *r = v_pwm; *g = t_pwm; *b = p_pwm;
            break;
        case 1:
            *r = q_pwm; *g = v_pwm; *b = p_pwm;
            break;
        case 2:
            *r = p_pwm; *g = v_pwm; *b = t_pwm;
            break;
        case 3:
            *r = p_pwm; *g = q_pwm; *b = v_pwm;
            break;
        case 4
            *r = t_pwm; *g = p_pwm; *b = v_pwm;
            break;
        default:
            *r = v_pwm; *g = p_pwm; *b = q_pwm;
            break;
    }
}

static void update_rgb_led(void)
{
    uint16_t r, g, b;
    hsv_to_rgb_simple(current_hsv.h, current_hsv.s, current_hsv.v, &r, &g, &b);
    
    pwm_rgb_values.channel_0 = r;
    pwm_rgb_values.channel_1 = b;
    pwm_rgb_values.channel_2 = g;
    pwm_rgb_values.channel_3 = 0;
    
    nrfx_pwm_simple_playback(&m_pwm_rgb, &pwm_rgb_seq, 1, NRFX_PWM_FLAG_LOOP);
}

static void update_mode_indicator(void)
{
    uint32_t current_time = millis();
    uint32_t blink_interval;
    
    switch (current_mode) {
        case MODE_NO_INPUT:
            pwm_indicator_values.channel_0 = 0;
            break;
            
        case MODE_HUE:
            blink_interval = MODE_BLINK_SLOW_MS;
            if (current_time - last_mode_blink_time >= blink_interval) {
                mode_led_state = !mode_led_state;
                pwm_indicator_values.channel_0 = mode_led_state ? PWM_TOP_VALUE : 0;
                last_mode_blink_time = current_time;
            }
            break;
            
        case MODE_SATURATION:
            blink_interval = MODE_BLINK_FAST_MS;
            if (current_time - last_mode_blink_time >= blink_interval) {
                mode_led_state = !mode_led_state;
                pwm_indicator_values.channel_0 = mode_led_state ? PWM_TOP_VALUE : 0;
                last_mode_blink_time = current_time;
            }
            break;
            
        case MODE_BRIGHTNESS:
            pwm_indicator_values.channel_0 = PWM_TOP_VALUE;
            break;
    }
    
    nrfx_pwm_simple_playback(&m_pwm_indicator, &pwm_indicator_seq, 1, NRFX_PWM_FLAG_LOOP);
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
}

static void switch_to_next_mode(void)
{
    current_mode = (input_mode_t)((current_mode + 1) % 4);
    last_mode_blink_time = millis();
    mode_led_state = false;
    update_mode_indicator();
}

static void gpiote_event_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if (pin != BUTTON_PIN) {
        return;
    }
    
    uint32_t current_time = millis();
    
    if (current_time - last_button_time < DEBOUNCE_MS) {
        return;
    }
    last_button_time = current_time;
    
    if (waiting_for_second_click) {
        if (current_time - first_click_time < DOUBLE_CLICK_TIMEOUT_MS) {
            waiting_for_second_click = false;
            switch_to_next_mode();
        } else {
            first_click_time = current_time;
            waiting_for_second_click = true;
        }
    } else {
        first_click_time = current_time;
        waiting_for_second_click = true;
    }
}

static void gpiote_init(void)
{
    nrfx_err_t err_code;
    
    nrf_gpio_cfg_input(BUTTON_PIN, NRF_GPIO_PIN_PULLUP);
    
    err_code = nrfx_gpiote_init();
    if (err_code != NRFX_SUCCESS && err_code != NRFX_ERROR_INVALID_STATE) {
        return;
    }
    
    nrfx_gpiote_in_config_t button_config = NRFX_GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    button_config.pull = NRF_GPIO_PIN_PULLUP;
    
    err_code = nrfx_gpiote_in_init(BUTTON_PIN, &button_config, gpiote_event_handler);
    if (err_code == NRFX_SUCCESS) {
        nrfx_gpiote_in_event_enable(BUTTON_PIN, true);
    }
}

static void pwm_rgb_init(void)
{
    nrfx_pwm_config_t pwm_config = NRFX_PWM_DEFAULT_CONFIG;
    pwm_config.output_pins[0] = LED_2_RED;
    pwm_config.output_pins[1] = LED_2_BLUE;
    pwm_config.output_pins[2] = LED_2_GREEN;
    pwm_config.output_pins[3] = NRFX_PWM_PIN_NOT_USED;
    pwm_config.base_clock = NRF_PWM_CLK_1MHz;
    pwm_config.count_mode = NRF_PWM_MODE_UP;
    pwm_config.top_value = PWM_TOP_VALUE;
    pwm_config.load_mode = NRF_PWM_LOAD_INDIVIDUAL;
    pwm_config.step_mode = NRF_PWM_STEP_AUTO;
    
    nrfx_pwm_init(&m_pwm_rgb, &pwm_config, NULL);
    
    pwm_rgb_values.channel_0 = 0;
    pwm_rgb_values.channel_1 = 0;
    pwm_rgb_values.channel_2 = 0;
    pwm_rgb_values.channel_3 = 0;
    
    nrfx_pwm_simple_playback(&m_pwm_rgb, &pwm_rgb_seq, 1, NRFX_PWM_FLAG_LOOP);
}

static void pwm_indicator_init(void)
{
    nrfx_pwm_config_t pwm_config = NRFX_PWM_DEFAULT_CONFIG;
    pwm_config.output_pins[0] = LED_1_GREEN;
    pwm_config.output_pins[1] = NRFX_PWM_PIN_NOT_USED;
    pwm_config.output_pins[2] = NRFX_PWM_PIN_NOT_USED;
    pwm_config.output_pins[3] = NRFX_PWM_PIN_NOT_USED;
    pwm_config.base_clock = NRF_PWM_CLK_1MHz;
    pwm_config.count_mode = NRF_PWM_MODE_UP;
    pwm_config.top_value = PWM_TOP_VALUE;
    pwm_config.load_mode = NRF_PWM_LOAD_INDIVIDUAL;
    pwm_config.step_mode = NRF_PWM_STEP_AUTO;
    
    nrfx_pwm_init(&m_pwm_indicator, &pwm_config, NULL);
    
    pwm_indicator_values.channel_0 = 0;
    pwm_indicator_values.channel_1 = 0;
    pwm_indicator_values.channel_2 = 0;
    pwm_indicator_values.channel_3 = 0;
    
    nrfx_pwm_simple_playback(&m_pwm_indicator, &pwm_indicator_seq, 1, NRFX_PWM_FLAG_LOOP);
}

int main(void)
{
    current_hsv.h = (DEFAULT_HUE_PERCENT * 360) / 100;
    current_hsv.s = 100;
    current_hsv.v = 100;
    
    pwm_rgb_init();
    pwm_indicator_init();
    gpiote_init();
    
    pwm_rgb_values.channel_0 = PWM_TOP_VALUE;
    pwm_rgb_values.channel_1 = PWM_TOP_VALUE;
    pwm_rgb_values.channel_2 = PWM_TOP_VALUE;
    nrfx_pwm_simple_playback(&m_pwm_rgb, &pwm_rgb_seq, 1, NRFX_PWM_FLAG_LOOP);
    
    pwm_indicator_values.channel_0 = PWM_TOP_VALUE;
    nrfx_pwm_simple_playback(&m_pwm_indicator, &pwm_indicator_seq, 1, NRFX_PWM_FLAG_LOOP);
    
    nrf_delay_ms(200);
    
    pwm_rgb_values.channel_0 = 0;
    pwm_rgb_values.channel_1 = 0;
    pwm_rgb_values.channel_2 = 0;
    nrfx_pwm_simple_playback(&m_pwm_rgb, &pwm_rgb_seq, 1, NRFX_PWM_FLAG_LOOP);
    
    pwm_indicator_values.channel_0 = 0;
    nrfx_pwm_simple_playback(&m_pwm_indicator, &pwm_indicator_seq, 1, NRFX_PWM_FLAG_LOOP);
    
    nrf_delay_ms(200);
    
    update_rgb_led();
    update_mode_indicator();
    
    system_ticks = 0;
    last_mode_blink_time = 0;
    last_value_change_time = 0;
    
    while (true) {
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