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
#define LED_RED         LED_2_RED
#define LED_BLUE        LED_2_BLUE  
#define LED_GREEN       LED_2_GREEN
#define BUTTON_PIN      NRF_GPIO_PIN_MAP(1, 6)
#define PWM_TOP_VALUE   1000
#define PWM_STEP        10
#define PWM_UPDATE_INTERVAL_MS  20
#define DOUBLE_CLICK_TIMEOUT_MS  400
#define DEBOUNCE_MS              50

static nrfx_pwm_t m_pwm = NRFX_PWM_INSTANCE(0);

static nrf_pwm_values_individual_t pwm_values;
static nrf_pwm_sequence_t const pwm_seq = {
    .values.p_individual = &pwm_values,
    .length = NRF_PWM_VALUES_LENGTH(pwm_values),
    .repeats = 0,
    .end_delay = 0
};

static volatile bool blinking_active = false;
static volatile uint32_t last_button_time = 0;
static volatile uint32_t first_click_time = 0;
static volatile bool waiting_for_second_click = false;

static int16_t current_duty = 0;
static int8_t duty_direction = 1;
static uint8_t current_led_index = 0;
static const uint8_t led_count = 3;

static volatile uint32_t system_ticks = 0;
static uint32_t last_pwm_update = 0;

/**
 * @brief
 */
static uint32_t millis(void)
{
    return system_ticks;
}

/**
 * @brief 
 */
static uint16_t duty_to_compare(uint16_t duty)
{
    return (uint16_t)(PWM_TOP_VALUE - duty);
}

/**
 * @brief
 */
static void update_pwm_output(void)
{
    pwm_values.channel_0 = PWM_TOP_VALUE;
    pwm_values.channel_1 = PWM_TOP_VALUE;
    pwm_values.channel_2 = PWM_TOP_VALUE;
    
    uint16_t compare_value = duty_to_compare(current_duty);
    
    switch (current_led_index) {
        case 0:
            pwm_values.channel_0 = compare_value;
            break;
        case 1:
            pwm_values.channel_1 = compare_value;
            break;
        case 2:
            pwm_values.channel_2 = compare_value;
            break;
    }
    
    nrfx_pwm_simple_playback(&m_pwm, &pwm_seq, 1, NRFX_PWM_FLAG_LOOP);
}

/**
 * @brief
 */
static void update_duty_cycle(void)
{
    if (!blinking_active) {
        return;
    }
    
    uint32_t current_time = millis();
    if (current_time - last_pwm_update < PWM_UPDATE_INTERVAL_MS) {
        return;
    }
    last_pwm_update = current_time;
    
    current_duty += duty_direction * PWM_STEP;
    
    if (current_duty >= 1000) {
        current_duty = 1000;
        duty_direction = -1;
    } else if (current_duty <= 0) {
        current_duty = 0;
        duty_direction = 1;
        
        current_led_index = (current_led_index + 1) % led_count;
    }
    
    update_pwm_output();
}

/**
 * @brief
 */
static void gpiote_event_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if (pin != BUTTON_PIN || action != NRF_GPIOTE_POLARITY_HITOLO) {
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
            
            blinking_active = !blinking_active;
            
            if (blinking_active) {
                current_led_index = 0;
                current_duty = 0;
                duty_direction = 1;
                last_pwm_update = current_time;
            }
            
            update_pwm_output();
        } else {
            first_click_time = current_time;
            waiting_for_second_click = true;
        }
    } else {
        first_click_time = current_time;
        waiting_for_second_click = true;
    }
}

/**
 * @brief
 */
static void gpiote_init(void)
{
    nrfx_err_t err_code;
    
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

/**
 * @brief
 */
static void pwm_init(void)
{
    nrfx_pwm_config_t pwm_config = NRFX_PWM_DEFAULT_CONFIG;
    pwm_config.output_pins[0] = LED_RED;
    pwm_config.output_pins[1] = LED_BLUE;
    pwm_config.output_pins[2] = LED_GREEN;
    pwm_config.output_pins[3] = NRFX_PWM_PIN_NOT_USED;
    pwm_config.base_clock = NRF_PWM_CLK_1MHz;
    pwm_config.count_mode = NRF_PWM_MODE_UP;
    pwm_config.top_value = PWM_TOP_VALUE;
    pwm_config.load_mode = NRF_PWM_LOAD_INDIVIDUAL;
    pwm_config.step_mode = NRF_PWM_STEP_AUTO;
    
    nrfx_err_t err_code = nrfx_pwm_init(&m_pwm, &pwm_config, NULL);
    if (err_code != NRFX_SUCCESS) {
        return;
    }
    
    pwm_values.channel_0 = PWM_TOP_VALUE;
    pwm_values.channel_1 = PWM_TOP_VALUE;
    pwm_values.channel_2 = PWM_TOP_VALUE;
    pwm_values.channel_3 = 0;
    
    nrfx_pwm_simple_playback(&m_pwm, &pwm_seq, 1, NRFX_PWM_FLAG_LOOP);
}

/**
 * @brief
 */
int main(void)
{
    pwm_init();
    gpiote_init();
    
    pwm_values.channel_0 = 0;
    pwm_values.channel_1 = 0;
    pwm_values.channel_2 = 0;
    nrfx_pwm_simple_playback(&m_pwm, &pwm_seq, 1, NRFX_PWM_FLAG_LOOP);
    nrf_delay_ms(100);
    
    pwm_values.channel_0 = PWM_TOP_VALUE;
    pwm_values.channel_1 = PWM_TOP_VALUE;
    pwm_values.channel_2 = PWM_TOP_VALUE;
    nrfx_pwm_simple_playback(&m_pwm, &pwm_seq, 1, NRFX_PWM_FLAG_LOOP);
    
    system_ticks = 0;
    last_pwm_update = 0;
    
    while (true) {
        update_duty_cycle();
        
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